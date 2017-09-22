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


#define HEXICORD_ENABLE_IMPLICIT_JSON_CONVERSIONS(Type) \
    inline void from_json(const nlohmann::json& json, Type& t) { t = Type(json);  } \
    inline void to_json  (nlohmann::json& json, const Type& t) { json = t.dump(); }

#endif // HEXICORD_TYPES_NLOHMANN_HELPER_HPP
