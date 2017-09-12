/*
 * Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
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

#ifndef PLAYRHO_ORBITER_HPP
#define PLAYRHO_ORBITER_HPP

#include "../Framework/Test.hpp"

namespace playrho {
    
    class Orbiter: public Test
    {
    public:
        
        Orbiter()
        {
            m_world->SetGravity(Vec2{0, 0} * MeterPerSquareSecond);

            BodyDef bd;
            const auto radius = Real{12.0f};

            bd.type = BodyType::Static;
            bd.position = m_center;
            const auto ctrBody = m_world->CreateBody(bd);
            const auto ctrShape = std::make_shared<DiskShape>();
            ctrShape->SetRadius(Real{3} * Meter);
            ctrBody->CreateFixture(ctrShape);

            bd.type = BodyType::Dynamic;
            bd.position = Length2D{GetX(m_center), GetY(m_center) + radius * Meter};
            m_orbiter = m_world->CreateBody(bd);
            const auto ballShape = std::make_shared<DiskShape>();
            ballShape->SetRadius(Real{0.5f} * Meter);
            ballShape->SetDensity(Real(1) * KilogramPerSquareMeter);
            m_orbiter->CreateFixture(ballShape);
            
            const auto velocity = Velocity{
                Vec2{Pi * radius / Real{2}, 0} * MeterPerSecond,
                Real{360.0f} * Degree / Second
            };
            m_orbiter->SetVelocity(velocity);
            
            auto conf = ChainShape::Conf{};
            conf.vertices = GetCircleVertices(Real(20.0f) * Meter, 180);
            conf.UseVertexRadius(Real(0.1) * Meter);
            conf.UseDensity(Real(1) * KilogramPerSquareMeter);
            const auto outerCicle = std::make_shared<ChainShape>(conf);

            bd.type = BodyType::Dynamic;
            bd.position = m_center;
            bd.bullet = true;
            const auto dysonSphere = m_world->CreateBody(bd);
            dysonSphere->CreateFixture(outerCicle);
        }
        
        void PreStep(const Settings&, Drawer&) override
        {
            const auto force = GetCentripetalForce(*m_orbiter, m_center);
            const auto linAccel = force * m_orbiter->GetInvMass();
            const auto angAccel = Real{0.0f} * RadianPerSquareSecond;
            m_orbiter->SetAcceleration(linAccel, angAccel);
        }
        
    private:
        Body* m_orbiter = nullptr;
        Length2D const m_center = Vec2{0, 20} * Meter;

    };
    
}

#endif /* PLAYRHO_ORBITER_HPP */
