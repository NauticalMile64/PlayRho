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

#include "gtest/gtest.h"
#include <PlayRho/Dynamics/Contacts/Contact.hpp>
#include <PlayRho/Dynamics/Fixture.hpp>
#include <PlayRho/Dynamics/Body.hpp>
#include <PlayRho/Dynamics/BodyDef.hpp>
#include <PlayRho/Dynamics/FixtureDef.hpp>
#include <PlayRho/Collision/Shapes/DiskShape.hpp>

using namespace playrho;

TEST(Contact, ByteSize)
{
    switch (sizeof(Real))
    {
        case  4: EXPECT_EQ(sizeof(Contact), std::size_t(104)); break;
        case  8: EXPECT_EQ(sizeof(Contact), std::size_t(184)); break;
        case 16: EXPECT_EQ(sizeof(Contact), std::size_t(352)); break;
        default: FAIL(); break;
    }
}

TEST(Contact, IsNotDefaultConstructible)
{
    EXPECT_FALSE(std::is_default_constructible<Contact>::value);
}

TEST(Contact, IsCopyConstructible)
{
    EXPECT_TRUE(std::is_copy_constructible<Contact>::value);
}

TEST(Contact, IsNotCopyAssignable)
{
    EXPECT_FALSE(std::is_copy_assignable<Contact>::value);    
}

TEST(Contact, SetAwake)
{
    const auto shape = std::make_shared<DiskShape>();
    auto bA = Body{nullptr, BodyDef{}.UseType(BodyType::Dynamic)};
    auto bB = Body{nullptr, BodyDef{}.UseType(BodyType::Dynamic)};
    auto fA = Fixture{&bA, FixtureDef{}, shape};
    auto fB = Fixture{&bB, FixtureDef{}, shape};
    const auto c = Contact{&fA, 0, &fB, 0};
    
    bA.UnsetAwake();
    ASSERT_FALSE(bA.IsAwake());

    bB.UnsetAwake();
    ASSERT_FALSE(bB.IsAwake());
    
    SetAwake(c);

    EXPECT_TRUE(bA.IsAwake());
    EXPECT_TRUE(bB.IsAwake());
}

TEST(Contact, ResetFriction)
{
    const auto shape = std::make_shared<DiskShape>();
    auto bA = Body{nullptr, BodyDef{}.UseType(BodyType::Dynamic)};
    auto bB = Body{nullptr, BodyDef{}.UseType(BodyType::Dynamic)};
    auto fA = Fixture{&bA, FixtureDef{}, shape};
    auto fB = Fixture{&bB, FixtureDef{}, shape};
    auto c = Contact{&fA, 0, &fB, 0};
    
    ASSERT_GT(shape->GetFriction(), Real(0));
    ASSERT_EQ(c.GetFriction(), shape->GetFriction());
    c.SetFriction(shape->GetFriction() * Real(2));
    ASSERT_NE(c.GetFriction(), shape->GetFriction());
    ResetFriction(c);
    EXPECT_EQ(c.GetFriction(), shape->GetFriction());
}

TEST(Contact, ResetRestitution)
{
    const auto shape = std::make_shared<DiskShape>();
    auto bA = Body{nullptr, BodyDef{}.UseType(BodyType::Dynamic)};
    auto bB = Body{nullptr, BodyDef{}.UseType(BodyType::Dynamic)};
    auto fA = Fixture{&bA, FixtureDef{}, shape};
    auto fB = Fixture{&bB, FixtureDef{}, shape};
    auto c = Contact{&fA, 0, &fB, 0};
    
    ASSERT_EQ(shape->GetRestitution(), Real(0));
    ASSERT_EQ(c.GetRestitution(), shape->GetRestitution());
    c.SetRestitution(Real(2));
    ASSERT_NE(c.GetRestitution(), shape->GetRestitution());
    ResetRestitution(c);
    EXPECT_EQ(c.GetRestitution(), shape->GetRestitution());
}
