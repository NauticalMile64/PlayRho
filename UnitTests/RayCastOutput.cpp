/*
 * Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/Box2D
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

#include <Box2D/Collision/RayCastOutput.hpp>
#include <Box2D/Collision/RayCastInput.hpp>
#include <Box2D/Collision/AABB.hpp>
#include <Box2D/Collision/DistanceProxy.hpp>

#include <type_traits>

using namespace box2d;

TEST(RayCastOutput, ByteSize)
{
    switch (sizeof(RealNum))
    {
        case  4: EXPECT_EQ(sizeof(RayCastOutput), std::size_t(16)); break;
        case  8: EXPECT_EQ(sizeof(RayCastOutput), std::size_t(32)); break;
        case 16: EXPECT_EQ(sizeof(RayCastOutput), std::size_t(32)); break;
        default: FAIL(); break;
    }
}

TEST(RayCastOutput, Traits)
{
    EXPECT_TRUE(std::is_default_constructible<RayCastOutput>::value);
    EXPECT_TRUE(std::is_nothrow_default_constructible<RayCastOutput>::value);
    EXPECT_FALSE(std::is_trivially_default_constructible<RayCastOutput>::value);
    
    EXPECT_TRUE(std::is_constructible<RayCastOutput>::value);
    EXPECT_TRUE(std::is_nothrow_constructible<RayCastOutput>::value);
    EXPECT_FALSE(std::is_trivially_constructible<RayCastOutput>::value);
    
    EXPECT_TRUE(std::is_copy_constructible<RayCastOutput>::value);
    EXPECT_TRUE(std::is_nothrow_copy_constructible<RayCastOutput>::value);
    EXPECT_TRUE(std::is_trivially_copy_constructible<RayCastOutput>::value);
    
    EXPECT_TRUE(std::is_copy_assignable<RayCastOutput>::value);
    EXPECT_TRUE(std::is_nothrow_copy_assignable<RayCastOutput>::value);
    EXPECT_TRUE(std::is_trivially_copy_assignable<RayCastOutput>::value);
    
    EXPECT_TRUE(std::is_destructible<RayCastOutput>::value);
    EXPECT_TRUE(std::is_nothrow_destructible<RayCastOutput>::value);
    EXPECT_TRUE(std::is_trivially_destructible<RayCastOutput>::value);
}

TEST(RayCastOutput, DefaultConstruction)
{
    RayCastOutput foo{};
    EXPECT_FALSE(foo.hit);
    EXPECT_FALSE(IsValid(foo.normal));
    EXPECT_FALSE(IsValid(foo.fraction));
}

TEST(RayCastOutput, InitConstruction)
{
    const auto normal = UnitVec2::GetLeft();
    const auto fraction = RealNum(0.8f);
    RayCastOutput foo{normal, fraction};
    EXPECT_TRUE(foo.hit);
    EXPECT_EQ(foo.normal, normal);
    EXPECT_EQ(foo.fraction, fraction);
}

TEST(RayCastOutput, RayCastFreeFunctionHits)
{
    const auto radius = RealNum(0.1) * Meter;
    const auto location = Vec2(5, 2) * Meter;
    const auto p1 = Vec2(10, 2) * Meter;
    const auto p2 = Vec2(0, 2) * Meter;
    const auto maxFraction = RealNum(1);
    auto input = RayCastInput{p1, p2, maxFraction};
    const auto output = RayCast(radius, location, input);
    EXPECT_TRUE(output.hit);
    EXPECT_EQ(output.normal, UnitVec2::GetRight());
    EXPECT_NEAR(static_cast<double>(output.fraction), 0.49, 0.01);
}

TEST(RayCastOutput, RayCastLocationFreeFunctionMisses)
{
    {
        const auto radius = RealNum(0.1) * Meter;
        const auto location = Vec2(15, 2) * Meter;
        const auto p1 = Vec2(10, 2) * Meter;
        const auto p2 = Vec2(0, 2) * Meter;
        const auto maxFraction = RealNum(1);
        auto input = RayCastInput{p1, p2, maxFraction};
        const auto output = RayCast(radius, location, input);
        EXPECT_FALSE(output.hit);
        EXPECT_FALSE(IsValid(output.normal));
        EXPECT_FALSE(IsValid(output.fraction));
    }
    {
        const auto radius = RealNum(0.1) * Meter;
        const auto location = Vec2(10, 3) * Meter;
        const auto p1 = Vec2(0, 2) * Meter;
        const auto p2 = Vec2(10, 2) * Meter;
        const auto maxFraction = RealNum(1);
        auto input = RayCastInput{p1, p2, maxFraction};
        const auto output = RayCast(radius, location, input);
        EXPECT_FALSE(output.hit);
        EXPECT_FALSE(IsValid(output.normal));
        EXPECT_FALSE(IsValid(output.fraction));
    }
}

TEST(RayCastOutput, RayCastAabbFreeFunction)
{
    AABB aabb;
    const auto p1 = Vec2(10, 2) * Meter;
    const auto p2 = Vec2(0, 2) * Meter;
    const auto maxFraction = RealNum(1);
    RayCastInput input{p1, p2, maxFraction};
    const auto output = RayCast(aabb, input);
    EXPECT_FALSE(output.hit);
    EXPECT_FALSE(IsValid(output.normal));
    EXPECT_FALSE(IsValid(output.fraction));
}

TEST(RayCastOutput, RayCastDistanceProxyFF)
{
    const auto pos1 = Vec2{3, 1} * Meter;
    const auto pos2 = Vec2{3, 3} * Meter;
    const auto pos3 = Vec2{1, 3} * Meter;
    const auto pos4 = Vec2{1, 1} * Meter;
    const Length2D squareVerts[] = {pos1, pos2, pos3, pos4};
    const auto n1 = GetUnitVector(GetFwdPerpendicular(pos2 - pos1));
    const auto n2 = GetUnitVector(GetFwdPerpendicular(pos3 - pos2));
    const auto n3 = GetUnitVector(GetFwdPerpendicular(pos4 - pos3));
    const auto n4 = GetUnitVector(GetFwdPerpendicular(pos1 - pos4));
    const UnitVec2 squareNormals[] = {n1, n2, n3, n4};
    const auto radius = RealNum(0.5) * Meter;
    DistanceProxy dp{radius, 4, squareVerts, squareNormals};

    const auto p1 = Vec2(0, 2) * Meter;
    const auto p2 = Vec2(10, 2) * Meter;
    const auto maxFraction = RealNum(1);
    auto input = RayCastInput{p1, p2, maxFraction};
    {
        const auto output = RayCast(dp, input, Transform_identity);
        EXPECT_TRUE(output.hit);
        EXPECT_EQ(output.normal, UnitVec2::GetLeft());
        EXPECT_NEAR(static_cast<double>(output.fraction), 0.05, 0.001);
    }
    
    const auto p0 = Vec2_zero * Meter;
    input = RayCastInput{p0, p1, maxFraction};
    {
        const auto output = RayCast(dp, input, Transform_identity);
        EXPECT_FALSE(output.hit);
        EXPECT_FALSE(IsValid(output.normal));
        EXPECT_FALSE(IsValid(output.fraction));
    }
}
