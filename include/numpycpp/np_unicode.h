#ifndef NP_UNICODE_H
#define NP_UNICODE_H

#include <string>
#include <string_view>

namespace np
{

template<class Char>
std::basic_string<Char> copy(const Char* str, std::size_t max)
{
    std::basic_string<Char> r;
    r.reserve(max);

    const Char* end = str + max;

    while(*str != 0 && str != end)
        r.push_back(*str++);

    return r;
}

std::string u32_to_u8(const std::u32string &u32);
std::u32string u8_to_u32(const std::string &u8);

}

#endif // NP_UNICODE_H
