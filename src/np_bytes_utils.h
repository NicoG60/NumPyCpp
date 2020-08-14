#ifndef NP_BYTES_UTILS_H
#define NP_BYTES_UTILS_H

#include <cstdint>
#include <cstddef>

namespace np
{

/**
 * @brief The Endianness enum defines some endianness constants
 */
enum Endianness
{
    BigEndian,
    LittleEndian,
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || __WIN32
    NativeEndian = LittleEndian,
    OpositeEndian = BigEndian
#else
    NativeEndian = BigEndian,
    OpositeEndian = LittleEndian
#endif
};

std::uint16_t byte_swap16(std::uint16_t value);
std::uint32_t byte_swap32(std::uint32_t value);
std::uint64_t byte_swap64(std::uint64_t value);

void byte_swap(void* value, std::size_t size);

/**
 * @brief swap the bytes of any value T
 */
template<class T>
T byte_swap(T value, Endianness from, Endianness to)
{
    if(from != to)
        byte_swap(&value, sizeof (T));

    return value;
}

}

#endif // NP_BYTES_UTILS_H
