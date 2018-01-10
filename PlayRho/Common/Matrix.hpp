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

#ifndef PLAYRHO_COMMON_MATRIX_HPP
#define PLAYRHO_COMMON_MATRIX_HPP

#include <PlayRho/Common/Vector.hpp>
#include <PlayRho/Common/Vector2.hpp>
#include <PlayRho/Common/Templates.hpp>
#include <PlayRho/Common/Real.hpp>
#include <PlayRho/Common/Units.hpp>

namespace playrho {
    
    /// @brief Generic N by M matrix.
    template <typename T, std::size_t N, std::size_t M>
    using Matrix = Vector<Vector<T, M>, N>;
    
    /// @brief 2 by 2 matrix.
    template <typename T>
    using Matrix22 = Matrix<T, 2, 2>;
    
    /// @brief 3 by 3 matrix.
    template <typename T>
    using Matrix33 = Matrix<T, 3, 3>;
    
    /// @brief 2 by 2 matrix of Real elements.
    using Mat22 = Matrix22<Real>;
    
    /// @brief 2 by 2 matrix of Mass elements.
    using Mass22 = Matrix22<Mass>;

    /// @brief 2 by 2 matrix of <code>InvMass</code> elements.
    using InvMass22 = Matrix22<InvMass>;
    
    /// @brief 3 by 3 matrix of Real elements.
    using Mat33 = Matrix33<Real>;
    
    /// @brief Determines if the given value is valid.
    template <>
    PLAYRHO_CONSTEXPR inline bool IsValid(const Mat22& value) noexcept
    {
        return IsValid(Get<0>(value)) && IsValid(Get<1>(value));
    }
    
    /// @brief Gets an invalid value for a <code>Mat22</code>.
    template <>
    PLAYRHO_CONSTEXPR inline Mat22 GetInvalid() noexcept
    {
        return Mat22{GetInvalid<Vec2>(), GetInvalid<Vec2>()};
    }

} // namespace playrho

#endif // PLAYRHO_COMMON_MATRIX_HPP
