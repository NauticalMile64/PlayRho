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

#ifndef PLAYRHO_VELOCITY_HPP
#define PLAYRHO_VELOCITY_HPP

#include <PlayRho/Common/Settings.hpp>
#include <PlayRho/Common/Vector2D.hpp>

namespace playrho
{
    /// @brief Velocity related data structure.
    /// @note This data structure is 12-bytes (with 4-byte Real on at least one 64-bit platform).
    struct Velocity
    {
        LinearVelocity2D linear; ///< Linear velocity.
        AngularVelocity angular; ///< Angular velocity.
    };
    
    /// @brief Determines if the given value is valid.
    template <>
    constexpr inline bool IsValid(const Velocity& value) noexcept
    {
        return IsValid(value.linear) && IsValid(value.angular);
    }
    
    /// @brief Equality operator.
    constexpr inline bool operator==(const Velocity& lhs, const Velocity& rhs)
    {
        return (lhs.linear == rhs.linear) && (lhs.angular == rhs.angular);
    }
    
    /// @brief Inequality operator.
    constexpr inline bool operator!=(const Velocity& lhs, const Velocity& rhs)
    {
        return (lhs.linear != rhs.linear) || (lhs.angular != rhs.angular);
    }
    
    /// @brief Multiplication assignment operator.
    constexpr inline Velocity& operator*= (Velocity& lhs, const Real rhs)
    {
        lhs.linear *= rhs;
        lhs.angular *= rhs;
        return lhs;
    }
    
    /// @brief Division assignment operator.
    constexpr inline Velocity& operator/= (Velocity& lhs, const Real rhs)
    {
        lhs.linear /= rhs;
        lhs.angular /= rhs;
        return lhs;
    }
    
    /// @brief Addition assignment operator.
    constexpr inline Velocity& operator+= (Velocity& lhs, const Velocity& rhs)
    {
        lhs.linear += rhs.linear;
        lhs.angular += rhs.angular;
        return lhs;
    }
    
    /// @brief Addition operator.
    constexpr inline Velocity operator+ (const Velocity& lhs, const Velocity& rhs)
    {
        return Velocity{lhs.linear + rhs.linear, lhs.angular + rhs.angular};
    }
    
    /// @brief Subtraction assignment operator.
    constexpr inline Velocity& operator-= (Velocity& lhs, const Velocity& rhs)
    {
        lhs.linear -= rhs.linear;
        lhs.angular -= rhs.angular;
        return lhs;
    }
    
    /// @brief Subtraction operator.
    constexpr inline Velocity operator- (const Velocity& lhs, const Velocity& rhs)
    {
        return Velocity{lhs.linear - rhs.linear, lhs.angular - rhs.angular};
    }
    
    /// @brief Negation operator.
    constexpr inline Velocity operator- (const Velocity& value)
    {
        return Velocity{-value.linear, -value.angular};
    }
    
    /// @brief Positive operator.
    constexpr inline Velocity operator+ (const Velocity& value)
    {
        return value;
    }
    
    /// @brief Multiplication operator.
    constexpr inline Velocity operator* (const Velocity& lhs, const Real rhs)
    {
        return Velocity{lhs.linear * rhs, lhs.angular * rhs};
    }
    
    /// @brief Multiplication operator.
    constexpr inline Velocity operator* (const Real lhs, const Velocity& rhs)
    {
        return Velocity{rhs.linear * lhs, rhs.angular * lhs};
    }
    
    /// @brief Division operator.
    constexpr inline Velocity operator/ (const Velocity& lhs, const Real rhs)
    {
        /*
         * While it can be argued that division operations shouldn't be supported due to
         * hardware support for division typically being significantly slower than hardware
         * support for multiplication, it can also be argued that it shouldn't be the
         * software developer's role to attempt to optimize what the compiler should be
         * much better at knowing how to optimize. So here the code chooses the latter
         * strategy which allows the intention to be clearer, and just passes the division
         * on down to the Vec2 and Angle types (rather than manually rewriting the divisions
         * as multiplications).
         */
        return Velocity{lhs.linear / rhs, lhs.angular / rhs};
    }
    
}

#endif /* PLAYRHO_VELOCITY_HPP */
