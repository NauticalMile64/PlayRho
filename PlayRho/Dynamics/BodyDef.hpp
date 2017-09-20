/*
 * Original work Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
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

#ifndef PLAYRHO_DYNAMICS_BODYDEF_HPP
#define PLAYRHO_DYNAMICS_BODYDEF_HPP

/// @file
/// Declarations of the BodyDef struct and free functions associated with it.

#include <PlayRho/Common/Settings.hpp>
#include <PlayRho/Common/BoundedValue.hpp>
#include <PlayRho/Common/Math.hpp>
#include <PlayRho/Dynamics/BodyType.hpp>

namespace playrho {

    class Body;

    /// @brief Body Definition.
    ///
    /// @details A body definition holds all the data needed to construct a rigid body.
    ///   You can safely re-use body definitions.
    ///
    /// @note This is a value class meant for passing in to the World::CreateBody method.
    ///
    /// @sa World.
    ///
    struct BodyDef
    {
        // Builder-styled methods...

        /// @brief Use the given type.
        constexpr BodyDef& UseType(BodyType t) noexcept;

        /// @brief Use the given location.
        constexpr BodyDef& UseLocation(Length2D l) noexcept;
        
        /// @brief Use the given angle.
        constexpr BodyDef& UseAngle(Angle a) noexcept;
        
        /// @brief Use the given linear velocity.
        constexpr BodyDef& UseLinearVelocity(LinearVelocity2D v) noexcept;
        
        /// @brief Use the given angular velocity.
        constexpr BodyDef& UseAngularVelocity(AngularVelocity v) noexcept;
        
        /// @brief Use the given linear acceleration.
        constexpr BodyDef& UseLinearAcceleration(LinearAcceleration2D v) noexcept;
        
        /// @brief Use the given angular acceleration.
        constexpr BodyDef& UseAngularAcceleration(AngularAcceleration v) noexcept;
        
        /// @brief Use the given linear damping.
        constexpr BodyDef& UseLinearDamping(NonNegative<Frequency> v) noexcept;
        
        /// @brief Use the given angular damping.
        constexpr BodyDef& UseAngularDamping(NonNegative<Frequency> v) noexcept;
        
        /// @brief Use the given under active time.
        constexpr BodyDef& UseUnderActiveTime(Time v) noexcept;
        
        /// @brief Use the given allow sleep value.
        constexpr BodyDef& UseAllowSleep(bool value) noexcept;
        
        /// @brief Use the given awake value.
        constexpr BodyDef& UseAwake(bool value) noexcept;
        
        /// @brief Use the given fixed rotation state.
        constexpr BodyDef& UseFixedRotation(bool value) noexcept;
        
        /// @brief Use the given bullet state.
        constexpr BodyDef& UseBullet(bool value) noexcept;
        
        /// @brief Use the given enabled state.
        constexpr BodyDef& UseEnabled(bool value) noexcept;
        
        /// @brief Use the given user data.
        constexpr BodyDef& UseUserData(void* value) noexcept;
        
        // Public member variables...
        
        /// @brief Type of the body: static, kinematic, or dynamic.
        /// @note If a dynamic body would have zero mass, the mass is set to one.
        BodyType type = BodyType::Static;
        
        /// The world position of the body. Avoid creating bodies at the origin
        /// since this can lead to many overlapping shapes.
        Length2D position = Length2D{};
        
        /// The world angle of the body.
        Angle angle = Angle{0};
        
        /// The linear velocity of the body's origin in world co-ordinates (in m/s).
        LinearVelocity2D linearVelocity = LinearVelocity2D{};
        
        /// The angular velocity of the body.
        AngularVelocity angularVelocity = AngularVelocity{0};
        
        /// Initial linear acceleration of the body.
        /// @note Usually this should be 0.
        LinearAcceleration2D linearAcceleration = LinearAcceleration2D{};
        
        /// Initial angular acceleration of the body.
        /// @note Usually this should be 0.
        AngularAcceleration angularAcceleration = AngularAcceleration{0};
        
        /// Linear damping is use to reduce the linear velocity. The damping parameter
        /// can be larger than 1 but the damping effect becomes sensitive to the
        /// time step when the damping parameter is large.
        NonNegative<Frequency> linearDamping = NonNegative<Frequency>{0};
        
        /// Angular damping is use to reduce the angular velocity. The damping parameter
        /// can be larger than 1 but the damping effect becomes sensitive to the
        /// time step when the damping parameter is large.
        NonNegative<Frequency> angularDamping = NonNegative<Frequency>{0};
        
        /// Under-active time.
        /// @details Set this to the value retrieved from Body::GetUnderActiveTime() or leave it as 0.
        Time underActiveTime = Second * Real{0};
        
        /// Set this flag to false if this body should never fall asleep. Note that
        /// this increases CPU usage.
        bool allowSleep = true;
        
        /// Is this body initially awake or sleeping?
        bool awake = true;
        
        /// Should this body be prevented from rotating? Useful for characters.
        bool fixedRotation = false;
        
        /// Is this a fast moving body that should be prevented from tunneling through
        /// other moving bodies? Note that all bodies are prevented from tunneling through
        /// kinematic and static bodies. This setting is only considered on dynamic bodies.
        /// @note Use this flag sparingly since it increases processing time.
        bool bullet = false;
        
        /// Does this body start out enabled?
        bool enabled = true;
        
        /// Use this to store application specific body data.
        void* userData = nullptr;
    };
    
    constexpr inline BodyDef& BodyDef::UseType(BodyType t) noexcept
    {
        type = t;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseLocation(Length2D l) noexcept
    {
        position = l;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseAngle(Angle a) noexcept
    {
        angle = a;
        return *this;
    }
    
    constexpr BodyDef& BodyDef::UseLinearVelocity(LinearVelocity2D v) noexcept
    {
        linearVelocity = v;
        return *this;
    }
    
    constexpr BodyDef& BodyDef::UseLinearAcceleration(LinearAcceleration2D v) noexcept
    {
        linearAcceleration = v;
        return *this;
    }
    
    constexpr BodyDef& BodyDef::UseAngularVelocity(AngularVelocity v) noexcept
    {
        angularVelocity = v;
        return *this;
    }
    
    constexpr BodyDef& BodyDef::UseAngularAcceleration(AngularAcceleration v) noexcept
    {
        angularAcceleration = v;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseLinearDamping(NonNegative<Frequency> v) noexcept
    {
        linearDamping = v;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseAngularDamping(NonNegative<Frequency> v) noexcept
    {
        angularDamping = v;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseUnderActiveTime(Time v) noexcept
    {
        underActiveTime = v;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseAllowSleep(bool value) noexcept
    {
        allowSleep = value;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseAwake(bool value) noexcept
    {
        awake = value;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseFixedRotation(bool value) noexcept
    {
        fixedRotation = value;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseBullet(bool value) noexcept
    {
        bullet = value;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseEnabled(bool value) noexcept
    {
        enabled = value;
        return *this;
    }
    
    constexpr inline BodyDef& BodyDef::UseUserData(void* value) noexcept
    {
        userData = value;
        return *this;
    }
    
    /// @brief Gets the default body definition.
    /// @relatedalso BodyDef
    constexpr BodyDef GetDefaultBodyDef() noexcept
    {
        return BodyDef{};
    }

    /// @brief Gets the body definition for the given body.
    /// @param body Body to get the BodyDef for.
    /// @relatedalso Body
    BodyDef GetBodyDef(const Body& body) noexcept;

} // namespace playrho

#endif // PLAYRHO_DYNAMICS_BODYDEF_HPP
