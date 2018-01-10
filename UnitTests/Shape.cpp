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

#include "gtest/gtest.h"
#include <PlayRho/Collision/Shapes/Shape.hpp>
#include <PlayRho/Collision/Shapes/EdgeShapeConf.hpp>
#include <PlayRho/Collision/Shapes/DiskShapeConf.hpp>
#include <PlayRho/Collision/Shapes/PolygonShapeConf.hpp>
#include <PlayRho/Collision/Distance.hpp>
#include <PlayRho/Collision/Manifold.hpp>
#include <chrono>

using namespace playrho;
using namespace playrho::d2;

TEST(Shape, ByteSize)
{
    switch (sizeof(Real))
    {
        case  4:
#if defined(_WIN32) && !defined(_WIN64)
            EXPECT_EQ(sizeof(Shape), std::size_t(8));
#else
            EXPECT_EQ(sizeof(Shape), std::size_t(16));
#endif
            break;
        case  8: EXPECT_EQ(sizeof(Shape), std::size_t(16)); break;
        case 16: EXPECT_EQ(sizeof(Shape), std::size_t(16)); break;
        default: FAIL(); break;
    }
}

TEST(Shape, TestOverlapSlowerThanCollideShapesForCircles)
{
    const auto shape = DiskShapeConf{2_m};
    const auto xfm = Transformation{Length2{}, UnitVec::GetRight()};
    const auto child = GetChild(shape, 0);

    const auto maxloops = 1000000u;

    std::chrono::duration<double> elapsed_test_overlap;
    std::chrono::duration<double> elapsed_collide_shapes;

    for (auto attempt = 0u; attempt < 2u; ++attempt)
    {
        {
            auto count = 0u;
            const auto start = std::chrono::high_resolution_clock::now();
            for (auto i = decltype(maxloops){0}; i < maxloops; ++i)
            {
                if (TestOverlap(child, xfm, child, xfm) >= Area{0})
                {
                    ++count;
                }
            }
            const auto end = std::chrono::high_resolution_clock::now();
            elapsed_test_overlap = end - start;
            ASSERT_EQ(count, maxloops);
        }
        {
            auto count = 0u;
            const auto start = std::chrono::high_resolution_clock::now();
            for (auto i = decltype(maxloops){0}; i < maxloops; ++i)
            {
                const auto manifold = CollideShapes(child, xfm, child, xfm);
                if (manifold.GetPointCount() > 0)
                {
                    ++count;
                }
            }
            const auto end = std::chrono::high_resolution_clock::now();
            elapsed_collide_shapes = end - start;
            ASSERT_EQ(count, maxloops);
        }
        
        EXPECT_GT(elapsed_test_overlap.count(), elapsed_collide_shapes.count());
    }
}

TEST(Shape, TestOverlapFasterThanCollideShapesForPolygons)
{
    const auto shape = PolygonShapeConf{2_m, 2_m};
    const auto xfm = Transformation{Length2{}, UnitVec::GetRight()};
    const auto child = GetChild(shape, 0);

    const auto maxloops = 1000000u;
    
    std::chrono::duration<double> elapsed_test_overlap;
    std::chrono::duration<double> elapsed_collide_shapes;
    
    for (auto attempt = 0u; attempt < 2u; ++attempt)
    {
        {
            auto count = 0u;
            const auto start = std::chrono::high_resolution_clock::now();
            for (auto i = decltype(maxloops){0}; i < maxloops; ++i)
            {
                if (TestOverlap(child, xfm, child, xfm) >= Area{0})
                {
                    ++count;
                }
            }
            const auto end = std::chrono::high_resolution_clock::now();
            elapsed_test_overlap = end - start;
            ASSERT_EQ(count, maxloops);
        }
        {
            auto count = 0u;
            const auto start = std::chrono::high_resolution_clock::now();
            for (auto i = decltype(maxloops){0}; i < maxloops; ++i)
            {
                const auto manifold = CollideShapes(child, xfm, child, xfm);
                if (manifold.GetPointCount() > 0)
                {
                    ++count;
                }
            }
            const auto end = std::chrono::high_resolution_clock::now();
            elapsed_collide_shapes = end - start;
            ASSERT_EQ(count, maxloops);
        }
        
        EXPECT_LT(elapsed_test_overlap.count(), elapsed_collide_shapes.count());
    }
}

TEST(Shape, Equality)
{
    EXPECT_TRUE(Shape(EdgeShapeConf()) == Shape(EdgeShapeConf()));

    const auto shapeA = Shape(DiskShapeConf{}.UseRadius(100_m));
    const auto shapeB = Shape(DiskShapeConf{}.UseRadius(100_m));
    EXPECT_TRUE(shapeA == shapeB);
    
    EXPECT_FALSE(Shape(DiskShapeConf()) == Shape(EdgeShapeConf()));
}

TEST(Shape, Inequality)
{
    EXPECT_FALSE(Shape(EdgeShapeConf()) != Shape(EdgeShapeConf()));
    
    const auto shapeA = Shape(DiskShapeConf{}.UseRadius(100_m));
    const auto shapeB = Shape(DiskShapeConf{}.UseRadius(100_m));
    EXPECT_FALSE(shapeA != shapeB);

    EXPECT_TRUE(Shape(DiskShapeConf()) != Shape(EdgeShapeConf()));
}
