#include "np_bytes_utils.h"

#include <cstdint>
#include <cstddef>

namespace np
{


/**
 * @brief swap the bytes of a 16 bits value
 */
std::uint16_t byte_swap16(std::uint16_t value)
{
    return ((value & 0x00ff) << 8u) |
           ((value & 0xff00) >> 8u);
}

/**
 * @brief swap the bytes of a 32 bits value
 */
std::uint32_t byte_swap32(std::uint32_t value)
{
    return ((value & 0x000000ff) << 24u) |
           ((value & 0x0000ff00) << 8u ) |
           ((value & 0x00ff0000) >> 8u ) |
           ((value & 0xff000000) >> 24u);
}

/**
 * @brief swap the bytes of a 64 bits value
 */
std::uint64_t byte_swap64(std::uint64_t value)
{
    return ((value & 0x00000000000000ff) << 56u) |
           ((value & 0x000000000000ff00) << 40u) |
           ((value & 0x0000000000ff0000) << 24u) |
           ((value & 0x00000000ff000000) << 8u ) |
           ((value & 0x000000ff00000000) >> 8u ) |
           ((value & 0x0000ff0000000000) >> 24u) |
           ((value & 0x00ff000000000000) >> 40u) |
           ((value & 0xff00000000000000) >> 56u);
}

/**
 * @brief take a pointer to a value and a size and swap bytes accordingly
 */
void byte_swap(void* value, std::size_t size)
{
    switch(size)
    {
    case 2:
    {
        std::uint16_t* ptr = reinterpret_cast<std::uint16_t*>(value);
        *ptr = byte_swap16(*ptr);
        break;
    }

    case 4:
    {
        std::uint32_t* ptr = reinterpret_cast<std::uint32_t*>(value);
        *ptr = byte_swap32(*ptr);
        break;
    }

    case 8:
    {
        std::uint64_t* ptr = reinterpret_cast<std::uint64_t*>(value);
        *ptr = byte_swap64(*ptr);
        break;
    }

    default:
        break;
    }
}

}
