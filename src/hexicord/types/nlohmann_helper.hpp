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
