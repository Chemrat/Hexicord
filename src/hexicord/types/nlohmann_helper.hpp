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

    template<typename T, typename SFINAE = std::enable_if<_Detail::ConvertableToJson<T>::value, void> >
    void to_json(nlohmann::json& json, const T& t) {
        json = t.dump();
    }

    template<typename T, typename SFINAE = std::enable_if<_Detail::ConvertableFromJson<T>::value, void> >
    void from_json(const nlohmann::json& json, T& t) {
        t = T(json);
    }

/*
 * If we don't overload it for STL used containers, it [compiler] will attempt
 * to pick overload above and fail with hard error (Is I did something wrong????).
 */
#ifndef HEXICORD_JSON_HELPER_NO_STL_OVERLOADS
    template<typename ValueT>
    void to_json(nlohmann::json& json, const std::unordered_map<std::string, ValueT>& map) {
        json = nlohmann::json::object_t::value_type();
        for (const std::pair<std::string, ValueT>& pair : map) {
            json[pair.first] = pair.second;
        }
    }

    template<typename ValueT>
    void from_json(const nlohmann::json& json, std::unordered_map<std::string, ValueT>& map) {
        assert(json.is_object());
        for (auto it = json.begin(); it != json.end(); ++it) {
            map.insert({ it.key(), it.value().get<ValueT>() });
        }
    }

    template<typename ValueT>
    void to_json(nlohmann::json& json, const std::vector<ValueT>& vector) {
        json = nlohmann::json::array_t::value_type();
        for (const ValueT& value : vector) {
            json.push_back(value);
        }
    }

    template<typename ValueT>
    void from_json(const nlohmann::json& json, std::vector<ValueT>& vector) {
        assert(json.is_array());
        for (const auto& subjson : json) {
            vector.push_back(subjson.get<ValueT>());
        }
    }
#endif // HEXICORD_JSON_HELPER_NO_STL_OVERLOADS

} // namespace Hexicord

#endif // HEXICORD_TYPES_NLOHMANN_HELPER_HPP
