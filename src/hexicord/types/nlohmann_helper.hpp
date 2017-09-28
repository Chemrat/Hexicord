// Hexicord - Discord API library for C++11 using boost libraries.
// Copyright © 2017 Maks Mazurov (fox.cpp) <foxcpp@yandex.ru>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef HEXICORD_TYPES_NLOHMANN_HELPER_HPP
#define HEXICORD_TYPES_NLOHMANN_HELPER_HPP

#include <type_traits>
#include <vector>
#include <unordered_map>
#include "hexicord/json.hpp"

/**
 * \file types/nlohamnn_helper.hpp
 *
 * Adapter from nlohmann's from_json and to_json to T(json) and T.dump() calls.
 */

namespace Hexicord {
    namespace _Detail {
        template<typename T>
        struct ConvertableFromJson : std::is_constructible<T, const nlohmann::json&> {};

        template<typename T>
        struct ConvertableToJson {
        private:
            template <typename C> static std::true_type  test(decltype(std::declval<const C>().dump()));
            template <typename C> static std::false_type test(...);
        public:
            static constexpr bool value = decltype(test<T>(0))::value;
        };
    } // namespace _Detail

    template<typename T, typename SFINAE = typename std::enable_if<_Detail::ConvertableToJson<T>::value, void>::type >
    void to_json(nlohmann::json& json, const T& t) {
        json = t.dump();
    }

    template<typename T, typename SFINAE = typename std::enable_if<_Detail::ConvertableFromJson<T>::value, void>::type >
    void from_json(const nlohmann::json& json, T& t) {
        t = T(json);
    }

} // namespace Hexicord

#endif // HEXICORD_TYPES_NLOHMANN_HELPER_HPP
