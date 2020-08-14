#ifndef NP_TYPE_T_H
#define NP_TYPE_T_H

#include <typeindex>
#include <string>

#include "np_bytes_utils.h"

namespace np
{

/**
 * @brief The type_t class represents an abstract type in the array.
 *
 * it contains all information needed to navigate in the array.
 */
class type_t
{
    friend class array;
    friend class descr_t;

public:
    type_t() = default;
    type_t(const type_t& copy) = default;
    type_t(type_t&& move) = default;

    type_t& operator=(const type_t& copy) = default;
    type_t& operator=(type_t&& move) = default;

    bool operator==(const type_t& o) const;
    bool operator!=(const type_t& o) const;

    template<class T>
    bool is(bool check_endianness = false)
    {
        return _index == typeid (T)
                && _size == sizeof (T)
                && (!check_endianness || _endianness == NativeEndian);
    }

    static type_t from_string(const std::string& str);

    template<class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    static type_t from_type(Endianness e = NativeEndian)
    {
        type_t r;

        r._index      = typeid (T);
        r._size       = sizeof (T);
        r._endianness = e;

        return r;
    }

    std::string to_string() const;

    inline std::type_index index()      const { return _index;      }
    inline char            ptype()      const { return _ptype;      }
    inline std::size_t     size()       const { return _size;       }
    inline std::size_t     offset()     const { return _offset;     }
    inline Endianness      endianness() const { return _endianness; }
    inline std::string     suffix()     const { return _suffix;     }

private:
    std::type_index _index      = typeid (void);
    char            _ptype      = '\0';
    std::size_t     _size       = 0;
    std::size_t     _offset     = 0;
    Endianness      _endianness = NativeEndian;
    std::string     _suffix;
};

}


#endif // NP_TYPE_T_H
