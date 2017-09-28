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

#ifndef HEXICORD_REST_HPP
#define HEXICORD_REST_HPP

#include <cstdint>        // uint8_t
#include <string>         // std::string
#include <vector>         // std::vector
#include <unordered_map>  // std::unordered_map
#include <memory>         // std::shared_ptr
namespace boost { namespace asio { class io_service; }}

namespace Hexicord { namespace REST {
    struct HTTPSConnectionInternal;

    namespace _Detail {
        std::string stringToLower(const std::string& input);

        struct CaseInsensibleStringEqual {
            inline bool operator()(const std::string& lhs, const std::string& rhs) const {
                return stringToLower(lhs) == stringToLower(rhs);
            }
        };
    } // namespace _Detail

    /// Hash-map with case-insensible string keys.
    using HeadersMap = std::unordered_map<std::string, std::string, std::hash<std::string>, _Detail::CaseInsensibleStringEqual>;

    struct HTTPResponse {
        unsigned statusCode;

        HeadersMap headers;
        std::vector<uint8_t> body;
    };

    struct HTTPRequest {
        std::string method;
        std::string path;

        unsigned version;
        std::vector<uint8_t> body;
        HeadersMap headers;
    };

    class HTTPSConnection {
    public:
        HTTPSConnection(boost::asio::io_service& ioService, const std::string& serverName);

        void open();
        void close();

        bool isOpen() const;

        HTTPResponse request(const HTTPRequest& request);

        HeadersMap connectionHeaders;
        const std::string serverName;

    private:
        std::shared_ptr<HTTPSConnectionInternal> connection;

        bool alive = false;
    };

    struct MultipartEntity {
        std::string name;
        std::string filename;
        HeadersMap additionalHeaders;

        std::vector<uint8_t> body;
    };

    HTTPRequest buildMultipartRequest(const std::vector<MultipartEntity>& elements);

}} // namespace Hexicord::REST

#endif // HEXICORD_REST_HPP 
