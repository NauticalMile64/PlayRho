/*
 * Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/PlayRho
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

#ifndef PLAYRHO_FILTER_HPP
#define PLAYRHO_FILTER_HPP

/// @file
/// Declarations of the Filter struct and any free functions associated with it.

#include <cstdint>

namespace playrho {
    
    /// @brief A holder for contact filtering data.
    struct Filter
    {
        /// @brief Bits type definition.
        ///
        using bits_type = std::uint16_t;

        /// @brief Index type definition.
        ///
        using index_type = std::int16_t;
        
        /// @brief The collision category bits.
        ///
        /// @note Normally you would just set one bit.
        ///
        bits_type categoryBits = 0x0001;
        
        /// @brief The collision mask bits.
        ///
        /// @details This states the categories that this shape would accept for collision.
        ///
        bits_type maskBits = 0xFFFF;
        
        /// @brief Group index.
        ///
        /// @details Collision groups allow a certain group of objects to never collide
        ///   (negative) or always collide (positive). Zero means no collision group.
        ///    Non-zero group filtering always wins against the mask bits.
        ///
        index_type groupIndex = 0;
    };
    
    /// @brief Determines whether collision processing should be performed.
    inline bool ShouldCollide(const Filter filterA, const Filter filterB) noexcept
    {
        if ((filterA.groupIndex == filterB.groupIndex) && (filterA.groupIndex != 0))
        {
            return filterA.groupIndex > 0;
        }
        return ((filterA.maskBits & filterB.categoryBits) != 0) &&
               ((filterB.maskBits & filterA.categoryBits) != 0);
    }
}

#endif /* PLAYRHO_FILTER_HPP */
