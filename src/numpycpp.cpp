#include "numpycpp.h"
#include <zip_file.hpp>

#include <algorithm>
#include <numeric>
#include <complex>
#include <regex>
#include <fstream>
namespace fs = std::filesystem;

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



//==============================================================================



/**
 * @brief returns true if the 2 types are equals
 */
bool type_t::operator==(const type_t& o) const
{
    return     index      == o.index
            && ptype      == o.ptype
            && size       == o.size
            && offset     == o.offset
            && endianness == o.endianness;
}

/**
 * @brief same as !(*this == o)
 */
bool type_t::operator!=(const type_t& o) const
{
    return !(*this == o);
}

/**
 * @brief parse a type string (like '<i4') and return a type_t representation
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

    r.suffix = m.str(1);

    switch(t[0])
    {
    case '|':
    case '=':
        r. endianness = NativeEndian;
        break;

    case '<':
        r.endianness = LittleEndian;
        break;

    case '>':
        r.endianness = BigEndian;
        break;

    default:
        throw error("unknown type " + t);
    }

    r.ptype = t[1];
    bool size_check = false;
    char type_size = r.ptype;

    switch(r.ptype)
    {
    case 'b':
        r.index = typeid (bool);
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

    r.size = std::stoul(t.substr(2, 1));

    if(size_check)
    {
        switch (type_size)
        {
        case 'i':
            switch(r.size)
            {
            case 1:
                r.index = typeid (std::int8_t);
                break;

            case 2:
                r.index = typeid (std::int16_t);
                break;

            case 4:
                r.index = typeid (std::int32_t);
                break;

            case 8:
                r.index = typeid (std::int64_t);
                break;

            default:
                throw error("unsupported type " + t);
            }
            break;

        case 'u':
            switch(r.size)
            {
            case 1:
                r.index = typeid (std::uint8_t);
                break;

            case 2:
                r.index = typeid (std::uint16_t);
                break;

            case 4:
                r.index = typeid (std::uint32_t);
                break;

            case 8:
                r.index = typeid (std::uint64_t);
                break;

            default:
                throw error("unsupported type " + t);
            }
            break;

        case 'f':
            switch(r.size)
            {
            case 4:
                r.index = typeid (float);
                break;

            case 8:
                r.index = typeid (double);
                break;

            default:
                throw error("unsupported type " + t);
            }
            break;

        case 'c':
            switch(r.size)
            {
            case 4:
                r.index = typeid (std::complex<float>);
                break;

            case 8:
                r.index = typeid (std::complex<double>);
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
 * @brief transfor the type_t in a string for saving files
 */
std::string type_t::to_string() const
{
    std::string r = "'";

    if(size == 1)
        r += "|";
    else if(endianness == LittleEndian)
        r += "<";
    else
        r += ">";

    if(ptype != '\0')
        r += ptype;
    else if(index == typeid (bool))
        r += "b";
    else if(index == typeid (std::int8_t)  ||
            index == typeid (std::int16_t) ||
            index == typeid (std::int32_t) ||
            index == typeid (std::int64_t))
        r += "i";
    else if(index == typeid (std::uint8_t)  ||
            index == typeid (std::uint16_t) ||
            index == typeid (std::uint32_t) ||
            index == typeid (std::uint64_t))
        r += "u";
    else if(index == typeid (float) ||
            index == typeid (double))
        r += "f";
    else
        throw error("unknown type " + std::string(index.name()));

    r += std::to_string(size) + suffix;

    r += "'";

    return r;
}

/**
 * @brief swap the content of 2 descriptors
 */
void descr_t::swap(descr_t& o)
{
    fields.swap(o.fields);
    std::swap(stride, o.stride);
}

/**
 * @brief parse a descriptor string as given by the numpy headers
 */
descr_t descr_t::from_string(const std::string& str)
{
    descr_t r;

    if((str.front() == '\'' && str.back() == '\'') || (str.front() == '"' && str.back() == '"'))
    {
        type_t t = type_t::from_string(str);
        r.fields.emplace("f0", t);
        r.stride = t.size;
    }
    else if(str.front() == '(' && str.back() == ')')
    {
        auto p = parse_tuple(str);

        if(p.first.empty())
            p.first = "f0";

        r.fields.emplace(p);
        r.stride = p.second.size;
    }
    else if(str.front() == '[' && str.back() == ']')
    {
        std::string t = str.substr(1, str.size()-2);

        std::string tmp;
        int f = 0;

        auto b = t.begin();
        auto e = t.end();

        for(auto it = b; it != e;)
        {
            auto& c = *it;

            if(c == '(')
            {
                tmp = extract(it, e, '(', ')');

                auto p = parse_tuple(tmp);

                if(p.first.empty())
                    p.first = "f" + std::to_string(f);

                p.second.offset = r.stride;
                r.stride += p.second.size;
                r.fields.emplace(p);

                ++f;
            }
            else if(c == '\'')
            {
                tmp = extract(it, e, '\'', '\'');

                std::string name = "f" + std::to_string(f);

                type_t t = type_t::from_string(tmp);

                t.offset = r.stride;
                r.stride += t.size;
                r.fields.emplace(name, t);

                ++f;
            }
            else if(c != ',' && c != ' ')
                throw error("can't parse dtype " + str);
            else
                ++it;
        }
    }
    else
        throw error("can't parse dtype " + str);

    return r;
}

/**
 * @brief Extract a string in between 2 characters (like [ ] or ( ) or ' '
 * @param it an iterator pointing to the openning character
 * @param end the end of the string
 * @param b the openning character
 * @param e the closing character
 * @return the extracted string including the opening/closing characters
 */
std::string descr_t::extract(std::string::iterator& it, std::string::iterator end, char b, char e)
{
    if(*it != b)
        throw error("bad extract call");

    std::string::iterator start = it;

    ++it;

    int d = 0;

    while((*it != e || d != 0) && it != end)
    {
        if(*it == b)
            ++d;
        else if(*it == e)
            --d;

        ++it;
    }

    ++it;

    return std::string(start, it);
}

/**
 * @brief parse a simplu tuple ('name', 'type')
 *
 * any other tuple will raise an error
 */
std::pair<std::string, type_t> descr_t::parse_tuple(const std::string& str)
{
    if(str.front() != '(' || str.back() != ')')
        throw error("wrong tuple");

    std::string_view view(str);
    view.remove_prefix(1);
    view.remove_suffix(1);

    std::size_t idx = view.find(',');

    std::string_view name_view = view.substr(0, idx);

    if((name_view.front() == '\'' && name_view.back() == '\'') || (name_view.front() == '"' && name_view.back() == '"'))
    {
        name_view.remove_prefix(1);
        name_view.remove_suffix(1);
    }

    idx++;

    while(view[idx] == ' ')
        idx++;

    std::size_t idx2 = view.find(',', idx);

    if(idx2 != std::string::npos)
    {
        if(idx2 != view.size()-1)
            throw error("does not handle sub arrays or fixed length strings");
    }
    else
        idx2 = view.size();

    std::string_view type_view = view.substr(idx, idx2-idx);

    type_t type = type_t::from_string({type_view.begin(), type_view.end()});

    return {{name_view.begin(), name_view.end()}, type};
}

/**
 * @brief return s tring representation of the descriptor
 */

// BUG: fileds ordering is wrong!!!
std::string descr_t::to_string() const
{
    if(fields.empty())
        return std::string();

    std::regex unnamed_test("f\\d+");

    if(fields.size() == 1)
    {
        auto& p = *fields.begin();
        if(std::regex_match(p.first, unnamed_test))
            return fields.begin()->second.to_string();
        else
            return "(" + p.first + "," + p.second.to_string() + ")";
    }


    std::string r = "[";

    for(auto& f : fields)
    {
        if(std::regex_match(f.first, unnamed_test))
            return "(''," + f.second.to_string() + "),";
        else
            return "(" + f.first + "," + f.second.to_string() + "),";
    }

    r += "]";

    return r;
}

/**
 * @brief return a string representation of the shape (as a tuple)
 */
std::string shape_to_string(const shape_t& shape)
{
    std::string r = "(";

    for(auto& s : shape)
        r += std::to_string(s) + ",";

    r += ")";

    return r;
}



//==============================================================================



array::iterator::iterator() :
    _array(nullptr),
    _data(nullptr)
{

}

bool array::iterator::operator ==(const iterator& other) const
{
    return _array == other._array && _data == other._data;
}

bool array::iterator::operator !=(const iterator& other) const
{
    return !(*this == other);
}

/**
 * @brief return true if this iterator is before other. throw an error if the
 * two iterators does not belong to the same array
 */
bool array::iterator::operator <(const iterator& other) const
{
    if(_array != other._array)
        throw error("comparing iterators from different arrays");

    return _data < other._data;
}

/**
 * @brief return true if this iterator is after other. throw an error if the
 * two iterators does not belong to the same array
 */
bool array::iterator::operator >(const iterator& other) const
{
    if(_array != other._array)
        throw error("comparing iterators from different arrays");

    return _data > other._data;
}

/**
 * @brief return true if this iterator is before or eq other. throw an error if
 * the two iterators does not belong to the same array
 */
bool array::iterator::operator <=(const iterator& other) const
{
    if(_array != other._array)
        throw error("comparing iterators from different arrays");

    return _data <= other._data;
}

/**
 * @brief return true if this iterator is after or eq other. throw an error if
 * the two iterators does not belong to the same array
 */
bool array::iterator::operator >=(const iterator& other) const
{
    if(_array != other._array)
        throw error("comparing iterators from different arrays");

    return _data >= other._data;
}

/**
 * @brief moves the iterator forward by 1 item
 */
array::iterator& array::iterator::operator++()
{
    _data += _array->_descr.stride;
    return *this;
}

/**
 * @brief moves the iterator forward by 1 item
 */
array::iterator  array::iterator::operator++(int)
{
    iterator it = *this;
    ++*this;
    return it;
}

/**
 * @brief moves the iterator forward by @a s item(s)
 */
array::iterator  array::iterator::operator+(difference_type s) const
{
    iterator it = *this;
    it += s;
    return it;
}

/**
 * @brief moves the iterator forward by @a s item(s)
 */
array::iterator& array::iterator::operator+=(difference_type s)
{
    _data += s * _array->_descr.stride;
    return *this;
}

/**
 * @brief moves the iterator backward by 1 item
 */
array::iterator& array::iterator::operator--()
{
    _data -= _array->_descr.stride;
    return *this;
}

/**
 * @brief moves the iterator backward by 1 item
 */
array::iterator  array::iterator::operator--(int)
{
    iterator it = *this;
    --*this;
    return it;
}

/**
 * @brief return the distance between 2 iterators. throws an error if not
 * belonging to the same array
 */
array::iterator::difference_type
array::iterator::operator-(const iterator& it) const
{
    if(_array != it._array)
        throw error("comparing iterators from different arrays");

    return (_data - it._data) / _array->_descr.stride;
}

/**
 * @brief moves the iterator backward by @a s item(s)
 */
array::iterator array::iterator::operator-(difference_type s) const
{
    iterator it = *this;
    it -= s;
    return it;
}

/**
 * @brief moves the iterator backward by @a s item(s)
 */
array::iterator& array::iterator::operator-=(difference_type s)
{
    _data -= s * _array->_descr.stride;
    return *this;
}

/**
 * @brief providded for api compiance. returns itself
 */
array::iterator::pointer   array::iterator::operator->()
{
    return this;
}

/**
 * @brief providded for api compiance. returns itself
 */
array::iterator::reference array::iterator::operator*()
{
    return *this;
}

/**
 * @brief return a pointer to the item
 */
char* array::iterator::ptr()
{
    return _data;
}

/**
 * @brief return a pointer to @a field in the item
 */
char* array::iterator::ptr(const std::string& field)
{
    auto& f = _array->_descr.fields.at(field);
    return _data + f.offset;
}

/**
 * @brief return a const pointer to the item
 */
const char* array::iterator::ptr() const
{
    return _data;
}

/**
 * @brief return a const pointer to @a field in the item
 */
const char* array::iterator::ptr(const std::string& field) const
{
    auto& f = _array->_descr.fields.at(field);
    return _data + f.offset;
}



//==============================================================================



array::array() :
    _data(nullptr),
    _fortran_order(false)
{}

array::array(const array& c) :
    array()
{
    *this = c;
}

array::array(array&& m) :
    array()
{
    swap(m);
}

array::array(descr_t d, shape_t s, bool f) :
    _data(nullptr),
    _shape(s),
    _descr(d),
    _fortran_order(f)
{
    std::size_t size = data_size();
    if(size > 0)
    {
        _data = new char[size];
        std::memset(_data, 0, size);
    }
}

array::~array()
{
    if(_data)
        delete[] _data;
}

array& array::operator =(const array& c)
{
    if(_data)
        delete[] _data;

    _shape         = c._shape;
    _descr         = c._descr;
    _fortran_order = c._fortran_order;

    std::size_t size = data_size();
    if(size > 0)
    {
        _data = new char[size];
        std::memcpy(_data, c._data, size);
    }
    else
        _data = nullptr;

    return *this;
}

array& array::operator =(array&& m)
{
    swap(m);
    return *this;
}

bool array::empty() const
{
    return _data == nullptr || data_size() == 0;
}

void array::swap(array& o)
{
    _shape.swap(o._shape);
    _descr.swap(o._descr);
    std::swap(_fortran_order, o._fortran_order);
    std::swap(_data, o._data);
}

std::size_t array::size() const
{
    if(_shape.empty())
        return 0;

    if(_descr.stride == 0)
        return 0;

    std::size_t s = 1;

    for(auto& d : _shape)
        s *= d;

    return s;
}

std::size_t array::size(std::size_t d) const
{
    if(d >= _shape.size())
        throw error("out of range");

    return _shape[d];
}

std::size_t array::dimensions() const
{
    return _shape.size();
}

const shape_t& array::shape() const
{
    return _shape;
}

const descr_t& array::descr() const
{
    return _descr;
}

const type_t& array::type() const
{
    if(_descr.fields.empty())
        throw error("fields does not exists");

    return _descr.fields.begin()->second;
}

const type_t& array::type(const std::string& field) const
{
    return _descr.fields.at(field);
}

bool array::fortran_order() const
{
    return _fortran_order;
}

const char* array::data() const
{
    return _data;
}

array array::load(const fs::path& file)
{
    std::ifstream st(file, std::ios_base::in | std::ios_base::binary);

    if(!st || !st.is_open())
        throw error("unable to open file " + file.string());

    return load(st);
}

array array::load(std::istream& stream)
{
    //Check the magic phrase
    char magic[7];
    std::memset(magic, 0, 7);
    stream.read(magic, 6);

    if(std::strcmp(magic, "\x93NUMPY") != 0)
        throw error("not a numpy file");

    //Check version
    std::uint8_t maj, min;
    stream.read(reinterpret_cast<char*>(&maj), 1);
    stream.read(reinterpret_cast<char*>(&min), 1);

    //read header len, little endian
    std::size_t header_len = 0;

    if(maj == 1 && min == 0) // 2 bytes for v1.0
    {
        std::uint16_t tmp;
        stream.read(reinterpret_cast<char*>(&tmp), 2);
        header_len = byte_swap(tmp, LittleEndian, NativeEndian);
    }
    else if(maj == 2 && min == 0) // 4 bytes for v2.0
    {
        stream.read(reinterpret_cast<char*>(&header_len), 4);
        header_len = byte_swap(header_len, LittleEndian, NativeEndian);
    }
    else
        throw error(std::to_string(maj)+"."+std::to_string(min) +
                                 ": sorry, I can't read that version...");

    //Read the header
    std::string header(header_len, 0);
    stream.read(&header[0], header_len);

    bool fortran_order = false;
    shape_t shape;
    descr_t descr;

    // Parse the header
    try
    {
        std::regex dict ("'([a-zA-Z0-9_-]+)':\\s*('[|=<>][a-zA-Z]\\d(\\[[a_zA-Z]+\\])?'|\\[.*\\]|True|False|\\(.*\\))");
        std::unordered_map<std::string, std::string> header_dict;

        auto dict_begin = std::sregex_iterator(header.begin(), header.end(), dict);
        auto dict_end = std::sregex_iterator();

        for(auto it = dict_begin; it != dict_end; ++it)
            header_dict.emplace(it->str(1), it->str(2));

        fortran_order = header_dict.at("fortran_order") == "True";

        std::string shape_str = header_dict.at("shape");
        std::regex shape_value("\\d+");
        auto sbegin = std::sregex_iterator(shape_str.begin(), shape_str.end(), shape_value);
        auto send   = std::sregex_iterator();

        for(auto it = sbegin; it != send; ++it)
        {
            std::string str = it->str();
            std::size_t c = std::stoul(str);
            shape.push_back(c);
        }

        descr = descr_t::from_string(header_dict.at("descr"));
    }
    catch(std::exception& e)
    {
        throw error("unable to parse numpy file header");
    }

    array a(descr, shape, fortran_order);

    stream.read(a._data, a.data_size());

    return a;
}

void array::save(const fs::path& file) const
{
    std::ofstream st(file, std::ios::binary);

    if(!st || !st.is_open())
        throw error("unable to open file");

    return save(st);
}

void array::save(std::ostream& stream) const
{
    const char* magic = "\x93NUMPY\x2\x0";
    std::string header_str = header();
    std::uint32_t len = header_str.length();
    len = byte_swap(len, NativeEndian, LittleEndian);

    stream.write(magic, 8);
    stream.write(reinterpret_cast<char*>(&len), 4);
    stream.write(&header_str[0], len);
    stream.write(_data, data_size());
}

std::string array::header() const
{
    return "{"
           "'descr': " + _descr.to_string() + ", "
           "'fortran_order': " + std::string(_fortran_order ? "True" : "False") + ", "
           "'shape': " + shape_to_string(_shape) +
           "}";
}

void array::convert_to(Endianness e)
{
    for(auto it : *this)
    {
        for(auto& field : _descr.fields)
        {
            auto& type = field.second;

            if(type.endianness != e)
            {
                byte_swap(it.ptr(field.first), field.second.size);
                type.endianness = e;
            }
        }
    }
}

//void array::transpose_order()
//{
//    if(dimensions() <= 1)
//        return;

//    array tmp(_descr, _shape, !_fortran_order);

//    std::size_t s = _descr.stride;

//    std::vector<std::size_t> indices(dimensions(), 0);
//    std::size_t idx = index(indices);
//    std::size_t oidx = tmp.index(indices);

//    do
//    {
//        std::memcpy(at(idx).ptr(), tmp.at(oidx).ptr(), s);

//        for(std::size_t d = 0; d < dimensions(); d++)
//        {
//            indices[d]++;

//            if(indices[d] >= _shape[d])
//                indices[d] = 0;
//            else
//                break;
//        }

//        idx = index(indices);
//        oidx = tmp.index(indices);
//    } while(indices != std::vector<std::size_t>{0, 0, 0});

//    swap(tmp);
//}

array::iterator array::begin()
{
    iterator it;
    it._array = this;
    it._data = _data;

    return it;
}

const array::iterator array::begin() const
{
    return cbegin();
}

const array::iterator array::cbegin() const
{
    iterator it;
    it._array = const_cast<array*>(this);
    it._data = _data;

    return it;
}

array::iterator array::end()
{
    iterator it;
    it._array = this;
    it._data = _data + data_size();

    return it;
}

const array::iterator array::end() const
{
    return cend();
}

const array::iterator array::cend() const
{
    iterator it;
    it._array = const_cast<array*>(this);
    it._data = _data + data_size();

    return it;
}

array::iterator array::operator[](std::size_t index)
{
    return at_index(index);
}

const array::iterator array::operator[](std::size_t index) const
{
    return at_index(index);
}

array::iterator array::at_index(std::size_t index)
{
    if(index >= size())
        throw error("out of range");

    iterator it;
    it._array = this;
    it._data = _data + index * _descr.stride;

    return it;
}

const array::iterator array::at_index(std::size_t index) const
{
    if(index >= size())
        throw error("out of range");

    iterator it;
    it._array = const_cast<array*>(this);
    it._data = _data + index * _descr.stride;

    return it;
}

//array::iterator array::at_index(std::vector<std::size_t> indices)
//{
//    return at(index(indices));
//}

//const array::iterator array::at_index(std::vector<std::size_t> indices) const
//{
//    return at(index(indices));
//}

std::size_t array::index(std::vector<std::size_t> indices) const
{
    if(indices.size() < _shape.size())
        throw error("size does not match");

    std::size_t idx = 0;

    if(_fortran_order)
    {
        std::size_t k = 0;

        for(auto& nk : indices)
            idx += details::index_f_order(_shape, k++, nk);
    }
    else
    {
        std::size_t k = 0;

        for(auto& nk : indices)
            idx += details::index_c_order(_shape, k++, nk);
    }

    return idx;
}

std::size_t array::data_size() const
{
    return size() * _descr.stride;
}

npz npz_load(const fs::path& file)
{
    std::unordered_map<std::string, array> arrays;

    miniz_cpp::zip_file f;

    f.load(file);

    for(auto npy_n : f.namelist())
    {
        std::string content = f.read(npy_n);
        std::istringstream iss (content);
        npy_n.erase(npy_n.size()-4);
        arrays.emplace(npy_n, array::load(iss));
    }

    return arrays;
}

void npz_save(const npz &arrays, const fs::path& file)
{
    miniz_cpp::zip_file f;

    for(auto& e : arrays)
    {
        std::stringstream st;
        try
        {
            e.second.save(st);
            f.writestr(e.first+".npy", st.str());
        }
        catch(std::exception& ex)
        {
            throw error(e.first + " - " + ex.what());
        }
    }

    f.save(file);
}

namespace details
{

template<>
std::size_t index_c_order(const shape_t& shape, std::size_t k, std::size_t nk)
{
    if(k > shape.size())
        throw error("size does not match");

    std::size_t l = k+1;
    std::size_t Nl = 1;
    for(; l < shape.size(); l++)
        Nl *= shape[l];

    return Nl * nk;
}

template<>
std::size_t index_f_order(const shape_t& shape, std::size_t k, std::size_t nk)
{
    if(k > shape.size())
        throw error("size does not match");

    std::size_t Nl = 1;
    for(std::size_t l = 0; l < k; l++)
        Nl *= shape[l];

    return Nl * nk;
}

}

}
