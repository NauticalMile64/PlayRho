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

#include <PlayRho/Collision/Shapes/Shape.hpp>
#include <PlayRho/Collision/Shapes/ShapeDef.hpp>

namespace playrho {

Shape::Shape(const ShapeDef& conf) noexcept:
    m_vertexRadius{conf.vertexRadius},
    m_density{conf.density},
    m_friction{conf.friction},
    m_restitution{conf.restitution}
{
    // Intentionally empty.
}

// Free functions...

bool TestPoint(const Shape& shape, Length2D point) noexcept
{
    const auto childCount = shape.GetChildCount();
    for (auto i = decltype(childCount){0}; i < childCount; ++i)
    {
        if (playrho::TestPoint(shape.GetChild(i), point))
        {
            return true;
        }
    }
    return false;
}

} // namespace playrho
