/*
 * Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
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

#ifndef PLAYRHO_PLAYRHO_HPP
#define PLAYRHO_PLAYRHO_HPP

/**
@mainpage PlayRho API Documentation

@section intro_sec Getting Started

For source code, please see: https://github.com/louis-langholtz/PlayRho .

For issues, visit: https://github.com/louis-langholtz/PlayRho/issues .
*/

// These include files constitute the main PlayRho API

/// @defgroup TestPointGroup Functions Testing For Point Containment

/// @defgroup ExceptionsGroup Library Defined Exceptions
/// @details The PlayRho library defines its own exception classes to recognize errors.
///    These classes are all sub-classed from sub-classes of the C++ Standard Library
///    std::exception class.

#include <PlayRho/Common/Settings.hpp>

#include <PlayRho/Collision/Shapes/DiskShape.hpp>
#include <PlayRho/Collision/Shapes/EdgeShape.hpp>
#include <PlayRho/Collision/Shapes/ChainShape.hpp>
#include <PlayRho/Collision/Shapes/PolygonShape.hpp>
#include <PlayRho/Collision/Shapes/MultiShape.hpp>

#include <PlayRho/Collision/Collision.hpp>
#include <PlayRho/Collision/Manifold.hpp>
#include <PlayRho/Collision/WorldManifold.hpp>
#include <PlayRho/Collision/Distance.hpp>
#include <PlayRho/Collision/DistanceProxy.hpp>

#include <PlayRho/Dynamics/Body.hpp>
#include <PlayRho/Dynamics/BodyDef.hpp>
#include <PlayRho/Dynamics/Fixture.hpp>
#include <PlayRho/Dynamics/WorldCallbacks.hpp>
#include <PlayRho/Dynamics/StepConf.hpp>
#include <PlayRho/Dynamics/World.hpp>

#include <PlayRho/Dynamics/Contacts/Contact.hpp>

#include <PlayRho/Dynamics/Joints/DistanceJoint.hpp>
#include <PlayRho/Dynamics/Joints/FrictionJoint.hpp>
#include <PlayRho/Dynamics/Joints/GearJoint.hpp>
#include <PlayRho/Dynamics/Joints/MotorJoint.hpp>
#include <PlayRho/Dynamics/Joints/MouseJoint.hpp>
#include <PlayRho/Dynamics/Joints/PrismaticJoint.hpp>
#include <PlayRho/Dynamics/Joints/PulleyJoint.hpp>
#include <PlayRho/Dynamics/Joints/RevoluteJoint.hpp>
#include <PlayRho/Dynamics/Joints/RopeJoint.hpp>
#include <PlayRho/Dynamics/Joints/WeldJoint.hpp>
#include <PlayRho/Dynamics/Joints/WheelJoint.hpp>

#endif // PLAYRHO_PLAYRHO_HPP
