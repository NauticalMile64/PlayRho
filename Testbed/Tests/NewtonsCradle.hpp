/*
 * Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef PLAYRHO_NEWTONS_CRADLE_HPP
#define PLAYRHO_NEWTONS_CRADLE_HPP

#include "../Framework/Test.hpp"

namespace testbed {
    
    /// Newton's Cradle test.
    /// @details
    /// Demonstrates the problems that are endemic to the handling multiple collisions.
    /// @sa http://www.myphysicslab.com/Collision-methods.html
    /// @sa https://en.wikipedia.org/wiki/Newton%27s_cradle
    class NewtonsCradle : public Test
    {
    public:
        static Test::Conf GetTestConf()
        {
            auto conf = Test::Conf{};
            conf.description = "Demonstrates the physics engine's behavior in a simulation of "
                "the classic Newton's cradle. "
                "Drag a circle with mouse, then let go to see how the physics is simulated.";
            return conf;
        }

        const Real scale = Real(1);
        const Length ball_radius = scale * 2_m; // 2
        const Length frame_width_per_arm = ball_radius * Real{2};
        const Length frame_height = scale * 30_m; // 30
        const Length arm_length = scale * 16_m; // 16
        static const auto default_num_arms = 5;

        NewtonsCradle(): Test(GetTestConf())
        {
            for (auto&& body: m_swings)
            {
                body = nullptr;
            }
            CreateCradle();
            
            RegisterForKey(GLFW_KEY_PERIOD, GLFW_PRESS, 0, "Toggle bullet mode", [&](KeyActionMods) {
                ToggleBulletMode();
            });
            RegisterForKey(GLFW_KEY_D, GLFW_PRESS, 0, "Toggle right side wall", [&](KeyActionMods) {
                ToggleRightSideWall();
            });
            RegisterForKey(GLFW_KEY_A, GLFW_PRESS, 0, "Toggle left side wall", [&](KeyActionMods) {
                ToggleLeftSideWall();
            });
            RegisterForKey(GLFW_KEY_1, GLFW_PRESS, 0, "Set to 1 ball.", [&](KeyActionMods) {
                DestroyCradle();
                m_num_arms = 1;
                CreateCradle();
            });
            RegisterForKey(GLFW_KEY_2, GLFW_PRESS, 0, "Set to 2 balls.", [&](KeyActionMods) {
                DestroyCradle();
                m_num_arms = 2;
                CreateCradle();
            });
            RegisterForKey(GLFW_KEY_3, GLFW_PRESS, 0, "Set to 3 balls.", [&](KeyActionMods) {
                DestroyCradle();
                m_num_arms = 3;
                CreateCradle();
            });
            RegisterForKey(GLFW_KEY_4, GLFW_PRESS, 0, "Set to 4 balls.", [&](KeyActionMods) {
                DestroyCradle();
                m_num_arms = 4;
                CreateCradle();
            });
            RegisterForKey(GLFW_KEY_5, GLFW_PRESS, 0, "Set to 5 balls.", [&](KeyActionMods) {
                DestroyCradle();
                m_num_arms = 5;
                CreateCradle();
            });
        }
        
        void CreateCradle()
        {
            if (m_frame)
            {
                return;
            }
            m_frame = [&]() {
                BodyConf bd;
                bd.type = BodyType::Static;
                bd.location = Length2{0, frame_height};
                const auto body = m_world.CreateBody(bd);
                
                const auto frame_width = frame_width_per_arm * static_cast<Real>(m_num_arms);
                const auto shape = PolygonShapeConf{}.SetAsBox(frame_width / 2, frame_width / 24).UseDensity(20_kgpm2);
                body->CreateFixture(Shape(shape));
                return body;
            }();
            
            for (auto i = decltype(m_num_arms){0}; i < m_num_arms; ++i)
            {
                const auto x = (((i + Real(0.5f)) - Real(m_num_arms) / 2) * frame_width_per_arm);
                
                BodyConf bd;
                bd.type = BodyType::Dynamic;
                bd.linearAcceleration = m_gravity;
                bd.bullet = m_bullet_mode;
                bd.location = Length2{x, frame_height - (arm_length / 2)};
                
                m_swings[i] = m_world.CreateBody(bd);
                CreateArm(m_swings[i], arm_length);
                CreateBall(m_swings[i], Length2{0, -arm_length / 2}, ball_radius);
                
                m_world.CreateJoint(RevoluteJointConf(m_frame, m_swings[i], Length2{x, frame_height}));
            }            
        }

        void DestroyCradle()
        {
            if (m_frame)
            {
                m_world.Destroy(m_frame);
                m_frame = nullptr;
            }
            for (auto&& body: m_swings)
            {
                if (body)
                {
                    m_world.Destroy(body);
                    body = nullptr;
                }
            }
            DestroyLeftSideWall();
            DestroyRightSideWall();
        }

        void CreateRightSideWall()
        {
            if (!m_right_side_wall) {
                const auto frame_width = static_cast<Real>(m_num_arms) * frame_width_per_arm;

                BodyConf def;
                def.type = BodyType::Static;
                def.location = Length2{frame_width / 2 + frame_width / 24, frame_height - (arm_length / 2)};
                const auto body = m_world.CreateBody(def);
                
                const auto shape = PolygonShapeConf{}.SetAsBox(frame_width / 24, arm_length / 2 + frame_width / 24).UseDensity(20_kgpm2);
                body->CreateFixture(Shape(shape));
                
                m_right_side_wall = body;
            }
        }
        
        void CreateLeftSideWall()
        {
            if (!m_left_side_wall) {
                const auto frame_width = static_cast<Real>(m_num_arms) * frame_width_per_arm;

                BodyConf def;
                def.type = BodyType::Static;
                def.location = Length2{
                    -(frame_width / Real{2} + frame_width / Real{24}),
                    frame_height - (arm_length / Real{2})
                };
                const auto body = m_world.CreateBody(def);
                
                const auto shape = PolygonShapeConf{}.SetAsBox(frame_width/Real{24}, (arm_length / Real{2} + frame_width / Real{24})).UseDensity(20_kgpm2);
                body->CreateFixture(Shape(shape));
                
                m_left_side_wall = body;
            }
        }

        void DestroyRightSideWall()
        {
            if (m_right_side_wall)
            {
                m_world.Destroy(m_right_side_wall);
                m_right_side_wall = nullptr;
            }
        }

        void DestroyLeftSideWall()
        {
            if (m_left_side_wall)
            {
                m_world.Destroy(m_left_side_wall);
                m_left_side_wall = nullptr;
            }
        }

        Fixture* CreateBall(Body* body, Length2 pos, Length radius)
        {
            auto conf = DiskShapeConf{};
            conf.vertexRadius = radius;
            conf.location = pos;
            conf.density = 20_kgpm2;
            conf.restitution = 1;
            conf.friction = 0;
            return body->CreateFixture(Shape(conf));
        }

        Fixture* CreateArm(Body* body, Length length = 10_m)
        {
            const auto shape = PolygonShapeConf{}.SetAsBox(length / Real{2000}, length / Real{2}).UseDensity(20_kgpm2);
            return body->CreateFixture(Shape(shape));
        }

        void ToggleRightSideWall()
        {
            if (m_right_side_wall)
                DestroyRightSideWall();
            else
                CreateRightSideWall();
        }

        void ToggleLeftSideWall()
        {
            if (m_left_side_wall)
                DestroyLeftSideWall();
            else
                CreateLeftSideWall();
        }

        void ToggleBulletMode()
        {
            m_bullet_mode = !m_bullet_mode;
            for (auto&& body: m_world.GetBodies())
            {
                auto& b = GetRef(body);
                if (b.GetType() == BodyType::Dynamic)
                {
                    b.SetBullet(m_bullet_mode);
                }
            }
        }
        
        void PostStep(const Settings&, Drawer&) override
        {
            std::stringstream stream;
            stream << "Bullet mode currently " << (m_bullet_mode? "on": "off") << ".";
            m_status = stream.str();
        }
    
        int m_num_arms = default_num_arms;
        bool m_bullet_mode = false;
        Body *m_frame = nullptr;
        Body *m_right_side_wall = nullptr;
        Body *m_left_side_wall = nullptr;
        Body *m_swings[5];
    };

} // namespace testbed
        
#endif /* PLAYRHO_NEWTONS_CRADLE_HPP */
