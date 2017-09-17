/*
* Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
* Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef PLAYRHO_POLY_SHAPES_HPP
#define PLAYRHO_POLY_SHAPES_HPP

#include "../Framework/Test.hpp"
#include <vector>
#include <cstring>

/// This tests stacking. It also shows how to use World::Query
/// and TestOverlap.

namespace playrho {

/// This callback is called by World::QueryAABB. We find all the fixtures
/// that overlap an AABB. Of those, we use TestOverlap to determine which fixtures
/// overlap a circle. Up to 4 overlapped fixtures will be highlighted with a yellow border.
class ShapeDrawer: public IsVisitedShapeVisitor
{
public:

    void Visit(const DiskShape& shape) override
    {
        const auto center = Transform(shape.GetLocation(), m_xf);
        const auto radius = shape.GetRadius();
        g_debugDraw->DrawCircle(center, radius, m_color);
    }

    void Visit(const PolygonShape& shape) override
    {
        const auto vertexCount = shape.GetVertexCount();
        auto vertices = std::vector<Length2D>(vertexCount);
        for (auto i = decltype(vertexCount){0}; i < vertexCount; ++i)
        {
            vertices[i] = Transform(shape.GetVertex(i), m_xf);
        }
        g_debugDraw->DrawPolygon(&vertices[0], vertexCount, m_color);
    }
    
    Color m_color = Color(0.95f, 0.95f, 0.6f);
    Transformation m_xf;
    Drawer* g_debugDraw;
};

class PolyShapes : public Test
{
public:

    enum
    {
        e_maxBodies = 256
    };

    PolyShapes()
    {
        m_circle->SetDensity(Real{1} * KilogramPerSquareMeter);
        m_circle->SetFriction(Real(0.3f));

        // Ground body
        {
            const auto ground = m_world->CreateBody();
            ground->CreateFixture(std::make_shared<EdgeShape>(Vec2(-40.0f, 0.0f) * Meter, Vec2(40.0f, 0.0f) * Meter));
        }

        for (auto&& p: m_polygons)
        {
            p = std::make_shared<PolygonShape>();
            p->SetDensity(Real{1} * KilogramPerSquareMeter);
            p->SetFriction(Real(0.3f));
        }

        m_polygons[0]->Set({Vec2(-0.5f, 0.0f) * Meter, Vec2(0.5f, 0.0f) * Meter, Vec2(0.0f, 1.5f) * Meter});
        m_polygons[1]->Set({Vec2(-0.1f, 0.0f) * Meter, Vec2(0.1f, 0.0f) * Meter, Vec2(0.0f, 1.5f) * Meter});

        {
            const auto w = Real(1);
            const auto b = w / (2.0f + Sqrt(2.0f));
            const auto s = Sqrt(2.0f) * b;

            m_polygons[2]->Set({
                Vec2(0.5f * s, 0.0f) * Meter,
                Vec2(0.5f * w, b) * Meter,
                Vec2(0.5f * w, b + s) * Meter,
                Vec2(0.5f * s, w) * Meter,
                Vec2(-0.5f * s, w) * Meter,
                Vec2(-0.5f * w, b + s) * Meter,
                Vec2(-0.5f * w, b) * Meter,
                Vec2(-0.5f * s, 0.0f) * Meter
            });
        }

        {
            m_polygons[3]->SetAsBox(Real{0.5f} * Meter, Real{0.5f} * Meter);
        }

        m_bodyIndex = 0;
        std::memset(m_bodies, 0, sizeof(m_bodies));
    }

    void Create(int index)
    {
        if (m_bodies[m_bodyIndex])
        {
            m_world->Destroy(m_bodies[m_bodyIndex]);
            m_bodies[m_bodyIndex] = nullptr;
        }

        BodyDef bd;
        bd.type = BodyType::Dynamic;

        const auto x = RandomFloat(-2.0f, 2.0f);
        bd.position = Vec2(x, 10.0f) * Meter;
        bd.angle = Radian * RandomFloat(-Pi, Pi);

        if (index == 4)
        {
            bd.angularDamping = Real(0.02f) * Hertz;
        }

        m_bodies[m_bodyIndex] = m_world->CreateBody(bd);

        if (index < 4)
        {
            m_bodies[m_bodyIndex]->CreateFixture(m_polygons[index]);
        }
        else
        {
            m_bodies[m_bodyIndex]->CreateFixture(m_circle);
        }

        m_bodyIndex = GetModuloNext(m_bodyIndex, static_cast<decltype(m_bodyIndex)>(e_maxBodies));
    }

    void Destroy()
    {
        for (auto i = 0; i < e_maxBodies; ++i)
        {
            if (m_bodies[i])
            {
                m_world->Destroy(m_bodies[i]);
                m_bodies[i] = nullptr;
                return;
            }
        }
    }

    void KeyboardDown(Key key) override
    {
        switch (key)
        {
        case Key_1:
        case Key_2:
        case Key_3:
        case Key_4:
        case Key_5:
            Create(key - Key_1);
            break;

        case Key_A:
            for (auto i = 0; i < e_maxBodies; i += 2)
            {
                if (m_bodies[i])
                {
                    const auto enabled = m_bodies[i]->IsEnabled();
                    m_bodies[i]->SetEnabled(!enabled);
                }
            }
            break;

        case Key_D:
            Destroy();
            break;
                
        default:
            break;
        }
    }

    void PostStep(const Settings&, Drawer& drawer) override
    {
        auto circleConf = DiskShape::Conf{};
        circleConf.location = Vec2(0.0f, 1.1f) * Meter;
        circleConf.vertexRadius = Real(2) * Meter;
        const auto circle = DiskShape{circleConf};

        const auto transform = Transform_identity;

        ShapeDrawer shapeDrawer;
        shapeDrawer.g_debugDraw = &drawer;

        constexpr int e_maxCount = 4;
        int count = 0;
        const auto aabb = ComputeAABB(circle, transform);
        m_world->QueryAABB(aabb, [&](Fixture* f, const ChildCounter) {
            if (count < e_maxCount)
            {
                const auto xfm = GetTransformation(*f);
                const auto shape = f->GetShape();
                const auto shapeChild = shape->GetChild(0);
                const auto circleChild = circle.GetChild(0);
                const auto overlap = TestOverlap(shapeChild, xfm, circleChild, transform);
                if (overlap >= Area{0})
                {
                    shapeDrawer.m_xf = xfm;
                    shape->Accept(shapeDrawer);
                    ++count;
                }
                return true;
            }
            return false;
        });

        const auto color = Color(0.4f, 0.7f, 0.8f);
        drawer.DrawCircle(circle.GetLocation(), circle.GetRadius(), color);

        drawer.DrawString(5, m_textLine, "Press 1-5 to drop stuff");
        m_textLine += DRAW_STRING_NEW_LINE;
        drawer.DrawString(5, m_textLine, "Press 'a' to (de)activate some bodies");
        m_textLine += DRAW_STRING_NEW_LINE;
        drawer.DrawString(5, m_textLine, "Press 'd' to destroy a body");
        m_textLine += DRAW_STRING_NEW_LINE;
    }

    int m_bodyIndex;
    Body* m_bodies[e_maxBodies];
    std::shared_ptr<PolygonShape> m_polygons[4];
    std::shared_ptr<DiskShape> m_circle = std::make_shared<DiskShape>(Real{0.5f} * Meter);
};

} // namespace playrho

#endif
