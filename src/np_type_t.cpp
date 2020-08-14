#include "np_type_t.h"
#include "np_error.h"

#include <regex>
#include <complex>

namespace np
{

/**
 * @brief Returns wether the 2 types are equals
 */
bool type_t::operator==(const type_t& o) const
{
    return     _index      == o._index
            && _ptype      == o._ptype
            && _size       == o._size
            && _offset     == o._offset
            && _endianness == o._endianness;
}

/**
 * @brief |ame as !(*this == o)
 */
bool type_t::operator!=(const type_t& o) const
{
    return !(*this == o);
}

/**
 * @brief Parse a type string (like '<i4') and return a type_t representation
 */
type_t type_t::from_string(const std::string& str)
{
    if(str.empty())
        throw error("empty type");

    type_t r;

    std::string t = str;

    if((t.front() == '\'' && t.back() == '\'') || (t.front() == '"' && t.back() == '"'))
    {
        t.erase(0, 1);
        t.erase(t.size()-1, 1);
    }

    std::regex check("^[<>|=][a-zA-Z]\\d(\\[[a-zA-Z]+\\])?$");
    std::smatch m;

    if(!std::regex_match(t, m, check))
        throw error("unsupported type " + t);

    r._suffix = m.str(1);

    switch(t[0])
    {
    case '|':
    case '=':
        r._endianness = NativeEndian;
        break;

    case '<':
        r._endianness = LittleEndian;
        break;

    case '>':
        r._endianness = BigEndian;
        break;

    default:
        throw error("unknown type " + t);
    }

    r._ptype = t[1];
    bool size_check = false;
    char type_size = r._ptype;

    switch(r._ptype)
    {
    case 'b':
        r._index = typeid (bool);
        break;

    case 'm':
    case 'M':
        type_size = 'i';

    case 'i':
    case 'u':
    case 'f':
    case 'c':
        size_check = true;
        break;

    case 'O':
    case 'S':
    case 'U':
    case 'V':
        throw error("unsupported type " + t);
        break;
    }

    r._size = std::stoul(t.substr(2, 1));

    if(size_check)
    {
        switch (type_size)
        {
        case 'i':
            switch(r._size)
            {
            case 1:
                r._index = typeid (std::int8_t);
                break;

            case 2:
                r._index = typeid (std::int16_t);
                break;

            case 4:
                r._index = typeid (std::int32_t);
                break;

            case 8:
                r._index = typeid (std::int64_t);
                break;

            default:
                throw error("unsupported type " + t);
            }
            break;

        case 'u':
            switch(r._size)
            {
            case 1:
                r._index = typeid (std::uint8_t);
                break;

            case 2:
                r._index = typeid (std::uint16_t);
                break;

            case 4:
                r._index = typeid (std::uint32_t);
                break;

            case 8:
                r._index = typeid (std::uint64_t);
                break;

            default:
                throw error("unsupported type " + t);
            }
            break;

        case 'f':
            switch(r._size)
            {
            case 4:
                r._index = typeid (float);
                break;

            case 8:
                r._index = typeid (double);
                break;

            default:
                throw error("unsupported type " + t);
            }
            break;

        case 'c':
            switch(r._size)
            {
            case 4:
                r._index = typeid (std::complex<float>);
                break;

            case 8:
                r._index = typeid (std::complex<double>);
                break;

            default:
                throw error("unsupported type " + t);
            }
            break;

        default:
            throw error("unsupported type " + t);
        }
    }

    return r;
}

/**
 * @brief Returns a string representation of the current type
 */
std::string type_t::to_string() const
{
    std::string r = "'";

    if(_size == 1)
        r += "|";
    else if(_endianness == LittleEndian)
        r += "<";
    else
        r += ">";

    if(_ptype != '\0')
        r += _ptype;
    else if(_index == typeid (bool))
        r += "b";
    else if(_index == typeid (std::int8_t)  ||
            _index == typeid (std::int16_t) ||
            _index == typeid (std::int32_t) ||
            _index == typeid (std::int64_t))
        r += "i";
    else if(_index == typeid (std::uint8_t)  ||
            _index == typeid (std::uint16_t) ||
            _index == typeid (std::uint32_t) ||
            _index == typeid (std::uint64_t))
        r += "u";
    else if(_index == typeid (float) ||
            _index == typeid (double))
        r += "f";
    else
        throw error("unknown type " + std::string(_index.name()));

    r += std::to_string(_size) + _suffix;

    r += "'";

    return r;
}

}
