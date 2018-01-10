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
#include <PlayRho/Collision/Shapes/MultiShapeConf.hpp>
#include <PlayRho/Collision/Shapes/Shape.hpp>
#include <PlayRho/Common/VertexSet.hpp>
#include <array>

using namespace playrho;
using namespace playrho::d2;

TEST(MultiShapeConf, ByteSize)
{
    switch (sizeof(Real))
    {
        case  4:
#if defined(_WIN64)
#if !defined(NDEBUG)
            EXPECT_EQ(sizeof(MultiShapeConf), std::size_t(56));
#else
            EXPECT_EQ(sizeof(MultiShapeConf), std::size_t(40));
#endif
#elif defined(_WIN32)
#if !defined(NDEBUG)
            EXPECT_EQ(sizeof(MultiShapeConf), std::size_t(36));
#else
            EXPECT_EQ(sizeof(MultiShapeConf), std::size_t(28));
#endif
#else
            EXPECT_EQ(sizeof(MultiShapeConf), std::size_t(40));
#endif
            break;
        case  8: EXPECT_EQ(sizeof(MultiShapeConf), std::size_t(56)); break;
        case 16: EXPECT_EQ(sizeof(MultiShapeConf), std::size_t(96)); break;
        default: FAIL(); break;
    }
}

TEST(MultiShapeConf, DefaultConstruction)
{
    const auto foo = MultiShapeConf{};
    const auto defaultMassData = MassData{};
    const auto defaultConf = MultiShapeConf{};
    
    EXPECT_EQ(typeid(foo), typeid(MultiShapeConf));
    EXPECT_EQ(GetChildCount(foo), ChildCounter{0});
    EXPECT_EQ(GetMassData(foo), defaultMassData);
    
    EXPECT_EQ(GetVertexRadius(foo), MultiShapeConf::GetDefaultVertexRadius());
    EXPECT_EQ(GetDensity(foo), defaultConf.density);
    EXPECT_EQ(GetFriction(foo), defaultConf.friction);
    EXPECT_EQ(GetRestitution(foo), defaultConf.restitution);
}

TEST(MultiShapeConf, GetInvalidChildThrows)
{
    const auto foo = MultiShapeConf{};
    
    ASSERT_EQ(GetChildCount(foo), ChildCounter{0});
    EXPECT_THROW(GetChild(foo, 0), InvalidArgument);
    EXPECT_THROW(GetChild(foo, 1), InvalidArgument);
}

TEST(MultiShapeConf, Accept)
{
    auto visited = false;
    auto shapeVisited = false;
    
    const auto foo = MultiShapeConf{};
    ASSERT_FALSE(visited);
    ASSERT_FALSE(shapeVisited);
    Accept(Shape(foo), [&](const std::type_info &ti, const void *) {
        visited = true;
        if (ti == typeid(MultiShapeConf))
        {
            shapeVisited = true;
        }
    });
    EXPECT_TRUE(visited);
    EXPECT_TRUE(shapeVisited);
}
#if 0
TEST(MultiShapeConf, BaseVisitorForDiskShape)
{
    const auto shape = MultiShapeConf{};
    auto visitor = IsVisitedShapeVisitor{};
    ASSERT_FALSE(visitor.IsVisited());
    shape.Accept(visitor);
    EXPECT_TRUE(visitor.IsVisited());
}
#endif
TEST(MultiShapeConf, AddConvexHullWithOnePointSameAsDisk)
{
    const auto defaultMassData = MassData{};
    const auto center = Length2(1_m, -4_m);

    auto pointSet = VertexSet{};
    ASSERT_EQ(pointSet.size(), std::size_t(0));
    pointSet.add(center);
    ASSERT_EQ(pointSet.size(), std::size_t(1));

    auto conf = MultiShapeConf{};
    conf.density = 2.3_kgpm2;
    conf.vertexRadius = 0.7_m;

    auto foo = MultiShapeConf{conf};
    ASSERT_EQ(GetChildCount(foo), ChildCounter{0});
    ASSERT_EQ(GetMassData(foo), defaultMassData);
    ASSERT_EQ(GetVertexRadius(foo), conf.vertexRadius);
    ASSERT_EQ(GetDensity(foo), conf.density);

    conf.AddConvexHull(pointSet);
    foo = MultiShapeConf{conf};
    EXPECT_EQ(GetChildCount(foo), ChildCounter{1});

    const auto child = GetChild(foo, 0);
    EXPECT_EQ(child.GetVertexCount(), VertexCounter(1));
    
    const auto massData = GetMassData(foo);
    EXPECT_NE(massData, defaultMassData);
    EXPECT_EQ(massData.center, center);
    
    const auto diskMassData = playrho::d2::GetMassData(conf.vertexRadius, conf.density, center);
    EXPECT_EQ(massData, diskMassData);
}

TEST(MultiShapeConf, AddConvexHullWithTwoPointsSameAsEdge)
{
    const auto defaultMassData = MassData{};
    const auto p0 = Length2(1_m, -4_m);
    const auto p1 = Length2(1_m, +4_m);
    
    auto pointSet = VertexSet{};
    ASSERT_EQ(pointSet.size(), std::size_t(0));
    pointSet.add(p0);
    pointSet.add(p1);
    ASSERT_EQ(pointSet.size(), std::size_t(2));
    
    auto conf = MultiShapeConf{};
    conf.density = 2.3_kgpm2;
    conf.vertexRadius = 0.7_m;
    
    auto foo = MultiShapeConf{conf};
    ASSERT_EQ(GetChildCount(foo), ChildCounter{0});
    ASSERT_EQ(GetMassData(foo), defaultMassData);
    ASSERT_EQ(GetVertexRadius(foo), conf.vertexRadius);
    ASSERT_EQ(GetDensity(foo), conf.density);
    
    conf.AddConvexHull(pointSet);
    foo = MultiShapeConf{conf};
    EXPECT_EQ(GetChildCount(foo), ChildCounter{1});
    
    const auto child = GetChild(foo, 0);
    EXPECT_EQ(child.GetVertexCount(), VertexCounter(2));
    
    const auto massData = GetMassData(foo);
    EXPECT_NE(massData, defaultMassData);
    EXPECT_EQ(massData.center, (p0 + p1) / Real(2));
    
    const auto edgeMassData = playrho::d2::GetMassData(conf.vertexRadius, conf.density, p0, p1);
    EXPECT_EQ(massData.center, edgeMassData.center);
    /// @note Units of L^-2 M^-1 QP^2.

    EXPECT_NEAR(static_cast<double>(Real{massData.I / (SquareMeter*1_kg/SquareRadian)}),
                static_cast<double>(Real{edgeMassData.I / (SquareMeter*1_kg/SquareRadian)}),
                228.4113/1000000.0);
    EXPECT_EQ(massData.mass, edgeMassData.mass);
}

TEST(MultiShapeConf, AddTwoConvexHullWithOnePoint)
{
    const auto defaultMassData = MassData{};
    const auto p0 = Length2(1_m, -4_m);
    const auto p1 = Length2(1_m, +4_m);

    auto pointSet = VertexSet{};
    ASSERT_EQ(pointSet.size(), std::size_t(0));

    auto conf = MultiShapeConf{};
    conf.density = 2.3_kgpm2;
    conf.vertexRadius = 0.7_m;
    
    auto foo = MultiShapeConf{conf};
    ASSERT_EQ(GetChildCount(foo), ChildCounter{0});
    ASSERT_EQ(GetMassData(foo), defaultMassData);
    ASSERT_EQ(GetVertexRadius(foo), conf.vertexRadius);
    ASSERT_EQ(GetDensity(foo), conf.density);
    
    pointSet.clear();
    ASSERT_EQ(pointSet.size(), std::size_t(0));
    pointSet.add(p0);
    ASSERT_EQ(pointSet.size(), std::size_t(1));

    conf.AddConvexHull(pointSet);
    foo = MultiShapeConf{conf};
    EXPECT_EQ(GetChildCount(foo), ChildCounter{1});

    const auto child0 = GetChild(foo, 0);
    EXPECT_EQ(child0.GetVertexCount(), VertexCounter(1));
    EXPECT_EQ(child0.GetVertex(0), p0);
    
    pointSet.clear();
    ASSERT_EQ(pointSet.size(), std::size_t(0));
    pointSet.add(p1);
    ASSERT_EQ(pointSet.size(), std::size_t(1));
    
    conf.AddConvexHull(pointSet);
    foo = MultiShapeConf{conf};
    EXPECT_EQ(GetChildCount(foo), ChildCounter{2});
    
    const auto child1 = GetChild(foo, 1);
    EXPECT_EQ(child1.GetVertexCount(), VertexCounter(1));
    EXPECT_EQ(child1.GetVertex(0), p1);

    const auto massData = GetMassData(foo);
    EXPECT_NE(massData, defaultMassData);
    EXPECT_EQ(massData.center, (p0 + p1) / Real(2));
    
    const auto massDataP0 = playrho::d2::GetMassData(conf.vertexRadius, conf.density, p0);
    const auto massDataP1 = playrho::d2::GetMassData(conf.vertexRadius, conf.density, p1);
    EXPECT_EQ(massData.mass, Mass{massDataP0.mass} + Mass{massDataP1.mass});
    EXPECT_EQ(massData.I, RotInertia{massDataP0.I} + RotInertia{massDataP1.I});
}

TEST(MultiShapeConf, Equality)
{
    EXPECT_TRUE(MultiShapeConf() == MultiShapeConf());
    
    auto pointSet = VertexSet{};
    pointSet.add(Length2{1_m, 2_m});
    
    EXPECT_FALSE(MultiShapeConf().AddConvexHull(pointSet) == MultiShapeConf());
    EXPECT_TRUE(MultiShapeConf().AddConvexHull(pointSet) == MultiShapeConf().AddConvexHull(pointSet));
    
    EXPECT_FALSE(MultiShapeConf().UseVertexRadius(10_m) == MultiShapeConf());
    EXPECT_TRUE(MultiShapeConf().UseVertexRadius(10_m) == MultiShapeConf().UseVertexRadius(10_m));
    
    EXPECT_FALSE(MultiShapeConf().UseDensity(10_kgpm2) == MultiShapeConf());
    EXPECT_TRUE(MultiShapeConf().UseDensity(10_kgpm2) == MultiShapeConf().UseDensity(10_kgpm2));
    
    EXPECT_FALSE(MultiShapeConf().UseFriction(Real(10)) == MultiShapeConf());
    EXPECT_TRUE(MultiShapeConf().UseFriction(Real(10)) == MultiShapeConf().UseFriction(Real(10)));
    
    EXPECT_FALSE(MultiShapeConf().UseRestitution(Real(10)) == MultiShapeConf());
    EXPECT_TRUE(MultiShapeConf().UseRestitution(Real(10)) == MultiShapeConf().UseRestitution(Real(10)));
}

TEST(MultiShapeConf, Inequality)
{
    EXPECT_FALSE(MultiShapeConf() != MultiShapeConf());
    
    auto pointSet = VertexSet{};
    pointSet.add(Length2{1_m, 2_m});
    
    EXPECT_TRUE(MultiShapeConf().AddConvexHull(pointSet) != MultiShapeConf());
    EXPECT_FALSE(MultiShapeConf().AddConvexHull(pointSet) != MultiShapeConf().AddConvexHull(pointSet));
    
    EXPECT_TRUE(MultiShapeConf().UseVertexRadius(10_m) != MultiShapeConf());
    EXPECT_FALSE(MultiShapeConf().UseVertexRadius(10_m) != MultiShapeConf().UseVertexRadius(10_m));
    
    EXPECT_TRUE(MultiShapeConf().UseDensity(10_kgpm2) != MultiShapeConf());
    EXPECT_FALSE(MultiShapeConf().UseDensity(10_kgpm2) != MultiShapeConf().UseDensity(10_kgpm2));
    
    EXPECT_TRUE(MultiShapeConf().UseFriction(Real(10)) != MultiShapeConf());
    EXPECT_FALSE(MultiShapeConf().UseFriction(Real(10)) != MultiShapeConf().UseFriction(Real(10)));
    
    EXPECT_TRUE(MultiShapeConf().UseRestitution(Real(10)) != MultiShapeConf());
    EXPECT_FALSE(MultiShapeConf().UseRestitution(Real(10)) != MultiShapeConf().UseRestitution(Real(10)));
}
