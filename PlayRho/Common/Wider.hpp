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

#ifndef PLAYRHO_WIDER_HPP
#define PLAYRHO_WIDER_HPP

#include <cstdint>
#include <type_traits>

// clang-format off

namespace playrho
{

/// @brief Wider data type obtainer.
///
/// @details Widens a data type to the data type that's twice its original size.
///
template <typename T> struct Wider {};

/// @brief Specialization of the Wider trait for signed 8-bit integers.
template <> struct Wider<std::int8_t> {
    using type = std::int16_t; ///< Wider type.
};

/// @brief Specialization of the Wider trait for unsigned 8-bit integers.
template <> struct Wider<std::uint8_t> {
    using type = std::uint16_t; ///< Wider type.
};

/// @brief Specialization of the Wider trait for signed 16-bit integers.
template <> struct Wider<std::int16_t> {
    using type = std::int32_t; ///< Wider type.
};

/// @brief Specialization of the Wider trait for unsigned 16-bit integers.
template <> struct Wider<std::uint16_t> {
    using type = std::uint32_t; ///< Wider type.
};

/// @brief Specialization of the Wider trait for signed 32-bit integers.
template <> struct Wider<std::int32_t> {
    using type = std::int64_t; ///< Wider type.
};

/// @brief Specialization of the Wider trait for unsigned 32-bit integers.
template <> struct Wider<std::uint32_t> {
    using type = std::uint64_t; ///< Wider type.
};

/// @brief Specialization of the Wider trait for float.
template <> struct Wider<float> {
    using type = double; ///< Wider type.
};

/// @brief Specialization of the Wider trait for double.
template <> struct Wider<double> {
    using type = long double; ///< Wider type.
};

#ifndef _WIN32
// Note: __int128_t not defined for Windows!
    
/// @brief Specialization of the Wider trait for signed 64-bit integers.
template <> struct Wider<std::int64_t> {
    using type = __int128_t; ///< Wider type.
};

/// @brief Specialization of the Wider trait for unsigned 64-bit integers.
template <> struct Wider<std::uint64_t> {
    using type = __uint128_t; ///< Wider type.
};

#endif

} // namespace playrho

namespace std {

#ifndef _WIN32
// This might already be defined by the standard library header, but
// define it here explicitly in case it's not.

/// @brief Make unsigned specialization for the __int128_t type.
template <> struct make_unsigned<__int128_t> {
    typedef __uint128_t type; ///< Wider type.
};
#endif

}

// clang-format on

#endif /* PLAYRHO_WIDER_HPP */
