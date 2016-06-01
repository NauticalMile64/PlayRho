/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
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

#ifndef BOX2D_SETTINGS_H
#define BOX2D_SETTINGS_H

#include <cstddef>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <limits>
#include <cstdint>

#define BOX2D_NOT_USED(x) ((void)(x))

/// Current version.
#define BOX2D_MAJOR_VERSION 3
#define BOX2D_MINOR_VERSION 0
#define BOX2D_REVISION 0

namespace box2d
{
using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using float32 = float;
using float64 = double;

/// Box2D floating point type.
/// This should be float, double, or long double.
using float_t = float;

using child_count_t = unsigned; // relating to "children" of Shape
using size_t = std::size_t;
using island_count_t = size_t; // relating to items in a Island

constexpr auto MaxFloat = std::numeric_limits<float_t>::max(); // FLT_MAX
constexpr auto Epsilon = std::numeric_limits<float_t>::epsilon(); // FLT_EPSILON;
constexpr auto Pi = float_t(M_PI); // 3.14159265359

/// @file
/// Global tuning constants based on meters-kilograms-seconds (MKS) units.
///

// Collision

/// Maximum manifold points.
/// This is the number of contact points between two convex shapes.
/// Do not change this value.
/// @note For memory efficiency, uses the smallest integral type that can hold the value. 
constexpr auto MaxManifoldPoints = uint8{2};

/// Maximum number of vertices on a convex polygon.
/// You cannot increase this too much because BlockAllocator has a maximum object size.
/// @note For memory efficiency, uses the smallest integral type that can hold the value. 
constexpr auto MaxPolygonVertices = uint8{16}; // 8

/// This is used to fatten AABBs in the dynamic tree. This is used to predict
/// the future position based on the current displacement.
/// This is a dimensionless multiplier.
constexpr auto AabbMultiplier = float_t{2};

/// Length used as a collision and constraint tolerance.
/// Usually chosen to be numerically significant, but visually insignificant.
/// Lower or raise to decrease or increase respectively the minimum of space
/// between bodies at rest.
/// @note Smaller values increases the time it takes for bodies to come to rest.
constexpr auto LinearSlop = float_t{1} / float_t{10000}; // aka 0.0001, originally 0.005

/// Fattens AABBs in the dynamic tree. This allows proxies
/// to move by a small amount without triggering a tree adjustment.
/// This is in meters.
constexpr auto AabbExtension = LinearSlop * float_t{20}; // aka 0.002, originally 0.1

/// A small angle used as a collision and constraint tolerance. Usually it is
/// chosen to be numerically significant, but visually insignificant.
constexpr auto AngularSlop = Pi * float_t(2) / float_t(180);

/// The radius of the polygon/edge shape skin. This should not be modified. Making
/// this smaller means polygons will have an insufficient buffer for continuous collision.
/// Making it larger may create artifacts for vertex collision.
constexpr auto PolygonRadius = LinearSlop * float_t(2);

/// Maximum number of sub-steps per contact in continuous physics simulation.
constexpr auto MaxSubSteps = unsigned{10}; // originally 8, often hit but no apparent help against tunneling

/// Maximum number of sub-step position iterations.
constexpr auto MaxSubStepPositionIterations = unsigned{20};

/// Maximum time of impact iterations.
constexpr auto MaxTOIIterations = unsigned{20};

/// Maximum time of impact root iterator count.
constexpr auto MaxTOIRootIterCount = unsigned{50};

// Dynamics

/// Maximum number of contacts to be handled to solve a TOI impact.
constexpr auto MaxTOIContacts = unsigned{32};

/// A velocity threshold for elastic collisions. Any collision with a relative linear
/// velocity below this threshold will be treated as inelastic.
constexpr auto VelocityThreshold = float_t(0.8); // float_t(1);

/// Maximum linear position correction used when solving constraints.
/// This helps to prevent overshoot.
constexpr auto MaxLinearCorrection = LinearSlop * float_t(40); // aka 0.002, originally 0.2

/// Maximum angular position correction used when solving constraints.
/// This helps to prevent overshoot.
constexpr auto MaxAngularCorrection = Pi * float_t(8) / float_t(180);

/// Maximum linear velocity of a body.
/// This limit is very large and is used to prevent numerical problems.
/// You shouldn't need to adjust this.
constexpr auto MaxTranslation = float_t{4}; // originally 2

/// Maximum angular velocity of a body.
/// This limit is very large and is used to prevent numerical problems.
/// You shouldn't need to adjust this.
constexpr auto MaxRotation = Pi / float_t(2);

/// This scale factor controls how fast overlap is resolved. Ideally this would be 1 so
/// that overlap is removed in one time step. However using values close to 1 often lead
/// to overshoot.
constexpr auto Baumgarte = float_t{2} / float_t{10}; // aka 0.2.

/// Time of impact Baumgarte factor.
/// @sa Baumgarte.
constexpr auto ToiBaumgarte = float_t{75} / float_t{100}; // aka .75


// Sleep

/// The time that a body must be still before it will go to sleep.
constexpr auto TimeToSleep = float_t(0.5);

/// A body cannot sleep if its linear velocity is above this tolerance.
constexpr auto LinearSleepTolerance = float_t(0.01);

/// A body cannot sleep if its angular velocity is above this tolerance.
constexpr auto AngularSleepTolerance = Pi * float_t(2) / float_t(180);

// Memory Allocation

/// Implement this function to use your own memory allocator.
void* alloc(size_t size);

/// Implement this function to use your own memory allocator.
void* realloc(void* ptr, size_t new_size);

/// If you implement alloc, you should also implement this function.
void free(void* mem);

/// Logging function.
void log(const char* string, ...);

/// Version numbering scheme.
/// See http://en.wikipedia.org/wiki/Software_versioning
struct Version
{
	int32 major;		///< significant changes
	int32 minor;		///< incremental changes
	int32 revision;		///< bug fixes
};

constexpr auto BuiltVersion = Version{BOX2D_MAJOR_VERSION, BOX2D_MINOR_VERSION, BOX2D_REVISION};
}

#endif