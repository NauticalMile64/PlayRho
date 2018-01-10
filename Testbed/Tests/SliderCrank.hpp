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

#ifndef PLAYRHO_SLIDER_CRANK_HPP
#define PLAYRHO_SLIDER_CRANK_HPP

#include "../Framework/Test.hpp"

namespace testbed {

// A motor driven slider crank with joint friction.
class SliderCrank : public Test
{
public:
    SliderCrank()
    {
        const auto ground = m_world.CreateBody();
        ground->CreateFixture(Shape{EdgeShapeConf{Vec2(-40.0f, 0.0f) * 1_m, Vec2(40.0f, 0.0f) * 1_m}});

        {
            auto prevBody = ground;

            // Define crank.
            {
                BodyConf bd;
                bd.type = BodyType::Dynamic;
                bd.location = Vec2(0.0f, 7.0f) * 1_m;
                const auto body = m_world.CreateBody(bd);
                body->CreateFixture(PolygonShapeConf{}.UseDensity(2_kgpm2).SetAsBox(0.5_m, 2_m));

                RevoluteJointConf rjd{prevBody, body, Vec2(0.0f, 5.0f) * 1_m};
                rjd.motorSpeed = Pi * 1_rad / 1_s;
                rjd.maxMotorTorque = 10000_Nm;
                rjd.enableMotor = true;
                m_joint1 = (RevoluteJoint*)m_world.CreateJoint(rjd);

                prevBody = body;
            }

            // Define follower.
            {
                BodyConf bd;
                bd.type = BodyType::Dynamic;
                bd.location = Vec2(0.0f, 13.0f) * 1_m;
                const auto body = m_world.CreateBody(bd);
                body->CreateFixture(PolygonShapeConf{}.UseDensity(2_kgpm2).SetAsBox(0.5_m, 4_m));

                RevoluteJointConf rjd{prevBody, body, Vec2(0.0f, 9.0f) * 1_m};
                rjd.enableMotor = false;
                m_world.CreateJoint(rjd);

                prevBody = body;
            }

            // Define piston
            {
                BodyConf bd;
                bd.type = BodyType::Dynamic;
                bd.fixedRotation = true;
                bd.location = Vec2(0.0f, 17.0f) * 1_m;
                const auto body = m_world.CreateBody(bd);
                body->CreateFixture(PolygonShapeConf{}.UseDensity(2_kgpm2).SetAsBox(1.5_m, 1.5_m));
                m_world.CreateJoint(RevoluteJointConf{prevBody, body, Vec2(0.0f, 17.0f) * 1_m});

                PrismaticJointConf pjd(ground, body, Vec2(0.0f, 17.0f) * 1_m, UnitVec::GetTop());
                pjd.maxMotorForce = 1000_N;
                pjd.enableMotor = true;
                m_joint2 = static_cast<PrismaticJoint*>(m_world.CreateJoint(pjd));
            }

            // Create a payload
            {
                BodyConf bd;
                bd.type = BodyType::Dynamic;
                bd.location = Vec2(0.0f, 23.0f) * 1_m;
                m_world.CreateBody(bd)->CreateFixture(PolygonShapeConf{}.UseDensity(2_kgpm2).SetAsBox(1.5_m, 1.5_m));
            }
        }
        SetAccelerations(m_world, m_gravity);
        RegisterForKey(GLFW_KEY_F, GLFW_PRESS, 0, "toggle friction", [&](KeyActionMods) {
            m_joint2->EnableMotor(!m_joint2->IsMotorEnabled());
            m_joint2->GetBodyB()->SetAwake();
        });
        RegisterForKey(GLFW_KEY_M, GLFW_PRESS, 0, "toggle motor", [&](KeyActionMods) {
            m_joint1->EnableMotor(!m_joint1->IsMotorEnabled());
            m_joint1->GetBodyB()->SetAwake();
        });
    }

    void PostStep(const Settings& settings, Drawer&) override
    {
        const auto torque = GetMotorTorque(*m_joint1, 1_Hz / settings.dt);
        std::stringstream stream;
        stream << "Motor Torque = ";
        stream << static_cast<double>(Real{torque / 1_Nm});
        stream << " Nm.";
        m_status = stream.str();
    }

    RevoluteJoint* m_joint1;
    PrismaticJoint* m_joint2;
};

} // namespace testbed

#endif
