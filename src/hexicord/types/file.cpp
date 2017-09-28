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

#include "hexicord/types/file.hpp"

#include <iterator>                    // std::istreambuf_iterator
#include <algorithm>                   // std::copy
#include <fstream>                     // std::ifstream
#include "hexicord/internal/utils.hpp" // Utils::split

#if _WIN32
    #define PATH_DELIMITER '\\'
#elif !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
    #define PATH_DELIMITER '/'
#endif

namespace Hexicord {

File::File(const std::string& path)
    : filename(Utils::split(path, PATH_DELIMITER).back())
    , bytes(std::istreambuf_iterator<char>(std::ifstream(path, std::ios_base::binary).rdbuf()),
            std::istreambuf_iterator<char>()) {}

File::File(const std::string& filename, std::istream&& stream)
    : filename(filename)
    , bytes(std::istreambuf_iterator<char>(stream.rdbuf()),
            std::istreambuf_iterator<char>()) {}

File::File(const std::string& filename, const std::vector<uint8_t>& bytes)
    : filename(filename)
    , bytes(bytes) {}

void File::write(const std::string& targetPath) const {
    std::ofstream output(targetPath, std::ios_base::binary | std::ios_base::trunc);
    std::copy(bytes.begin(), bytes.end(), std::ostreambuf_iterator<char>(output.rdbuf()));
}

} // namespace Hexicord
