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

#include "hexicord/types/image.hpp"

#include <boost/asio/io_service.hpp>   // boost::asio::io_service
#include "hexicord/internal/utils.hpp" // Hexicord::Utils::Magic, Hexicord::Utils::base64Encode
#include "hexicord/exceptions.hpp"     // Hexicord::LogicError
#include "hexicord/internal/rest.hpp"  // Hexicord::REST

namespace Hexicord {

Image::Image(const File& file, ImageFormat format)
    : format(format == ImageFormat::Detect ? detectFormat(file) : format)
    , file(file)
{}

std::string Image::toAvatarData() const {
    std::string mimeType;
    if (format == Jpeg) mimeType = "image/jpeg";
    if (format == Png)  mimeType = "image/png";
    if (format == Webp) mimeType = "image/webp";
    if (format == Gif)  mimeType = "image/gif";

    return std::string("data:") + mimeType + ";base64," + Utils::base64Encode(file.bytes);
}

ImageFormat Image::detectFormat(const File& file) {
    if (Utils::Magic::isJfif(file.bytes)) return ImageFormat::Jpeg;
    if (Utils::Magic::isPng(file.bytes))  return ImageFormat::Png;
    if (Utils::Magic::isWebp(file.bytes)) return ImageFormat::Webp;
    if (Utils::Magic::isGif(file.bytes))  return ImageFormat::Gif;

    throw LogicError("Failed to detect image format.", -1);
}

namespace _Detail {
    std::vector<uint8_t> cdnDownload(boost::asio::io_service& ioService, const std::string& path) {
        REST::HTTPSConnection connection(ioService, "cdn.discordapp.com");
        connection.open();

        REST::HTTPRequest request;
        request.method  = "GET";
        request.path    = path;
        request.version = 11;

        REST::HTTPResponse response = connection.request(request);
        if (response.body.empty()) {
            throw LogicError("Response body is empty (are you trying to download non-animated avatar as GIF?)", -1);
        }
        if (response.statusCode != 200) {
            throw LogicError(std::string("HTTP status code: ") + std::to_string(response.statusCode), -1);
        }

        return response.body;
    }
} // namespace _Detail

} // namespace Hexicord
