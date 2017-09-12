/*
* Original work Copyright (c) 2011 Erin Catto http://www.box2d.org
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

#ifndef PLAYRHO_CONVEX_HULL_HPP
#define  PLAYRHO_CONVEX_HULL_HPP

#include "../Framework/Test.hpp"
#include <vector>

namespace playrho {

class ConvexHull : public Test
{
public:
    enum: std::size_t
    {
        e_count = 16
    };

    ConvexHull()
    {
        Generate();
        m_auto = false;
    }

    void Generate()
    {
        const auto lowerBound = Vec2(-8.0f, -8.0f);
        const auto upperBound = Vec2(8.0f, 8.0f);

        m_points.clear();
        for (auto i = std::size_t{0}; i < e_count; ++i)
        {
            const auto x = 10.0f * RandomFloat();
            const auto y = 10.0f * RandomFloat();

            // Clamp onto a square to help create collinearities.
            // This will stress the convex hull algorithm.
            const auto v = Vec2{
                Clamp(x, GetX(lowerBound), GetX(upperBound)),
                Clamp(y, GetY(lowerBound), GetY(upperBound))
            } * Meter;
            m_points.emplace_back(v);
        }
    }

    void KeyboardDown(Key key) override
    {
        switch (key)
        {
        case Key_A:
            m_auto = !m_auto;
            break;

        case Key_G:
            Generate();
            break;

        default:
            break;
        }
    }

    void PostStep(const Settings&, Drawer& drawer) override
    {
        const auto conf = PolygonShape::Conf{};
        const auto shape = PolygonShape{Span<const Length2D>{&m_points[0], m_points.size()}, conf};

        drawer.DrawString(5, m_textLine, "Press g to generate a new random convex hull");
        m_textLine += DRAW_STRING_NEW_LINE;

        drawer.DrawPolygon(shape.GetVertices().begin(), shape.GetVertexCount(), Color(0.9f, 0.9f, 0.9f));

        for (auto i = std::size_t{0}; i < m_points.size(); ++i)
        {
            drawer.DrawPoint(m_points[i], 3.0f, Color(0.3f, 0.9f, 0.3f));
            drawer.DrawString(m_points[i] + Vec2(0.05f, 0.05f) * Meter, "%d", i);
        }

        if (!Validate(shape))
        {
            drawer.DrawString(5, m_textLine, "Note: Invalid convex hull");
            m_textLine += DRAW_STRING_NEW_LINE;
        }

        if (m_auto)
        {
            Generate();
        }
    }

    std::vector<Length2D> m_points{e_count};
    bool m_auto;
};

} // namespace playrho

#endif
