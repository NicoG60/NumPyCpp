#include <numpycpp/np_unicode.h>
#include <ww898/utf_converters.hpp>

using namespace ww898::utf;

namespace np
{

std::string copy(const char* str, std::size_t max)
{
    std::string r;
    r.reserve(max);

    const char* end = str + max;

    while(*str != 0 && str != end)
        r.push_back(*str++);

    return r;
}

std::u32string copy(const char32_t* str, std::size_t max);

std::string u32_to_u8(const std::u32string& u32)
{
    return conv<char>(u32);
}

std::u32string u8_to_u32(const std::string& u8)
{
    return conv<char32_t>(u8);
}

}

