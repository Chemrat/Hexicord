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

#ifndef HEXICORD_TYPES_FILE_HPP
#define HEXICORD_TYPES_FILE_HPP

#include <cstdint>  // uint8_t
#include <string>   // std::string
#include <vector>   // std::vector
#include <iosfwd>   // std::istream

namespace Hexicord {
    /**
     * Struct for (filename, bytes) pair.
     */
    struct File {
        /// Construct empty file.
        File() = default;

        /**
         * Read file specified by `path` to \ref bytes.
         * Last component of path used as filename.
         */
        File(const std::string& path);

        /**
         * Read stream until EOF to \ref bytes.
         */
        File(const std::string& filename, std::istream&& stream);

        /**
         * Use passed vector as file contents.
         */
        File(const std::string& filename, const std::vector<uint8_t>& bytes);

        /**
         * Helper function, write file to std::ofstream(targetPath).
         */
        void write(const std::string& targetPath) const;

        std::string filename;
        std::vector<uint8_t> bytes;
    };
} // namespace Hexicord

#endif // HEXICORD_TYPES_FILE_HPP
