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

#ifndef HEXICORD_EXCEPTIONS_HPP
#define HEXICORD_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

/**
 * \file exceptions.hpp
 *
 * This file defines set of exceptions thrown by Hexicord.
 */

namespace Hexicord {
    /// Base class for errors that can't be predicted in most cases.
    class RuntimeError : public std::runtime_error {
    public:
        RuntimeError(const std::string& message, int errorCode)
            : std::runtime_error(message), message(message), code(errorCode) {}

        const std::string message;
        const int         code;
    }; 

    /// Base class for errors that can be predicted in most cases.
    class LogicError : public std::logic_error {
    public:
        LogicError(const std::string& message, int errorCode)
            : std::logic_error(message), message(message), code(errorCode) {}

        const std::string message;
        const int         code;
    }; 


    /// The class for errors in REST API.
    /// Some errors have separate classes, which inherit RESTError.
    class RESTError : public LogicError {
    public:
       RESTError(const std::string& message = "Unknown REST API error", int errorCode = -1, int httpCode = -1)
           : LogicError(message, errorCode), httpCode(httpCode) {}

       const int httpCode;
    };

    /// Thrown on REST API ratelimit hit is HEXICORD_RATELIMIT_HIT_AS_ERROR.
    class RatelimitHit : public RESTError {
    public:
        RatelimitHit(const std::string& route) 
            : RESTError(std::string("Ratelimit hit for route ") + route, -1, 429) {}
    };

    /// Thrown when client tries to access non-existent entity (probably invalid snowflake).
    /// See entityType for details.
    class UnknownEntity : public RESTError {
    public:
        enum Entity : int {
            Account      = 1,
            Application  = 2,
            Channel      = 3,
            Guild        = 4,
            Integration  = 5,
            Invite       = 6,
            Member       = 7,
            Message      = 8,
            Overwrite    = 9,
            Provider     = 10,
            Role         = 11,
            Token        = 12,
            User         = 13,
            Emoji        = 14,
        };

        UnknownEntity(const std::string& message, int errorCode)
            : RESTError(message, errorCode, 404), entityType(Entity(errorCode % 10000)) {}

        Entity entityType;
    };

    /// Thrown if client reaches some limitation (except ratelimit, which have separate 
    /// exception), use type to determine what happened.
    /// \sa \ref RatelimitHit
    class LimitReached : public RESTError {
    public:
        enum Type : unsigned {
            Guilds, 
            Friends,
            Pins,
            GuildRoles,
            Reactions
        };

        LimitReached(const std::string& message, int errorCode)
            : RESTError(message, errorCode), type(Type(errorCode % 30000)) {}

        /// Limit of what hit.
        Type type;
    };

    /// Thrown if either pre-request parameter validation fails or server returns
    /// message about invalid parameter.
    /// Errors parameter contained in \ref parameter, error description in \ref description.
    /// \note Name in parameter may or not may be same as exact invalid parameter name.
    class InvalidParameter : public RESTError {
    public:
        InvalidParameter(const std::string& parameter, const std::string& message) 
            : RESTError(std::string("Invalid parameter: ") + parameter + ", " + message, -1, 400) {}

        const std::string parameter;
        const std::string description;
    };
} // namespace Hexicord

#endif // HEXICORD_EXCEPTIONS_HPP
