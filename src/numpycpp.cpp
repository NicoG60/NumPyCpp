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

std::uint16_t byte_swap16(std::uint16_t value)
{
    return ((value & 0x00ff) << 8u) |
           ((value & 0xff00) >> 8u);
}

std::uint32_t byte_swap32(std::uint32_t value)
{
    return ((value & 0x000000ff) << 24u) |
           ((value & 0x0000ff00) << 8u ) |
           ((value & 0x00ff0000) >> 8u ) |
           ((value & 0xff000000) >> 24u);
}

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



bool type_t::operator==(const type_t& o) const
{
    return     index      == o.index
            && ptype      == o.ptype
            && size       == o.size
            && offset     == o.offset
            && endianness == o.endianness;
}

bool type_t::operator!=(const type_t& o) const
{
    return !(*this == o);
}

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

void descr_t::swap(descr_t& o)
{
    fields.swap(o.fields);
    std::swap(stride, o.stride);
}

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

bool array::iterator::operator <(const iterator& other) const
{
    if(_array != other._array)
        throw error("comparing iterators from different arrays");

    return _data < other._data;
}

bool array::iterator::operator >(const iterator& other) const
{
    if(_array != other._array)
        throw error("comparing iterators from different arrays");

    return _data > other._data;
}

bool array::iterator::operator <=(const iterator& other) const
{
    if(_array != other._array)
        throw error("comparing iterators from different arrays");

    return _data <= other._data;
}

bool array::iterator::operator >=(const iterator& other) const
{
    if(_array != other._array)
        throw error("comparing iterators from different arrays");

    return _data >= other._data;
}

array::iterator& array::iterator::operator++()
{
    _data += _array->_descr.stride;
    return *this;
}

array::iterator  array::iterator::operator++(int)
{
    iterator it = *this;
    ++*this;
    return it;
}

array::iterator  array::iterator::operator+(difference_type s) const
{
    iterator it = *this;
    it += s;
    return it;
}

array::iterator& array::iterator::operator+=(difference_type s)
{
    _data += s * _array->_descr.stride;
    return *this;
}

array::iterator& array::iterator::operator--()
{
    _data -= _array->_descr.stride;
    return *this;
}

array::iterator  array::iterator::operator--(int)
{
    iterator it = *this;
    --*this;
    return it;
}

array::iterator::difference_type
array::iterator::operator-(const iterator& it) const
{
    if(_array != it._array)
        throw error("comparing iterators from different arrays");

    return (_data - it._data) / _array->_descr.stride;
}

array::iterator array::iterator::operator-(difference_type s) const
{
    iterator it = *this;
    it -= s;
    return it;
}

array::iterator& array::iterator::operator-=(difference_type s)
{
    _data -= s * _array->_descr.stride;
    return *this;
}

array::iterator::pointer   array::iterator::operator->()
{
    return this;
}

array::iterator::reference array::iterator::operator*()
{
    return *this;
}

char* array::iterator::ptr()
{
    return _data;
}

char* array::iterator::ptr(const std::string& field)
{
    auto& f = _array->_descr.fields.at(field);
    return _data + f.offset;
}

const char* array::iterator::ptr() const
{
    return _data;
}

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
    return at(index);
}

const array::iterator array::operator[](std::size_t index) const
{
    return at(index);
}

array::iterator array::at(std::size_t index)
{
    if(index >= size())
        throw error("out of range");

    iterator it;
    it._array = this;
    it._data = _data + index * _descr.stride;

    return it;
}

const array::iterator array::at(std::size_t index) const
{
    if(index >= size())
        throw error("out of range");

    iterator it;
    it._array = const_cast<array*>(this);
    it._data = _data + index * _descr.stride;

    return it;
}

array::iterator array::at(std::vector<std::size_t> indices)
{
    return at(index(indices));
}

const array::iterator array::at(std::vector<std::size_t> indices) const
{
    return at(index(indices));
}

std::size_t array::index(std::vector<std::size_t> indices) const
{
    if(indices.size() < _shape.size())
        throw error("size does not match");

    std::size_t idx = 0;

    if(_fortran_order)
    {
        std::size_t k = 0;

        for(auto& nk : indices)
            idx += index_f_order(k++, nk);
    }
    else
    {
        std::size_t k = 0;

        for(auto& nk : indices)
            idx += index_c_order(k++, nk);
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

}














































//namespace NumPy {

//array::array(descr_t d, size_t ds, shape_t s, bool f) :
//    _shape(s),
//    _descr(d),
//    _descr_size(ds),
//    _fortran_order(f),
//    _size(1)
//{
//    for(size_t _s : _shape)
//        _size *= _s;

//    size_t ts = _size * _descr_size;
//    _data.reset(new base_t[ts], std::default_delete<base_t[]>());
//    memset(_data.get(), 0, ts);
//}

//array::array(std::istream& st) :
//    _descr(typeid (base_t)),
//    _descr_size(sizeof (base_t)),
//    _fortran_order(false),
//    _size(1)
//{
//    load(st);
//}

//array::array() :
//    _descr(typeid (base_t)),
//    _descr_size(sizeof (base_t)),
//    _fortran_order(false),
//    _size(1)
//{

//}

//array::array(const std::string& fn) :
//    _descr(typeid (base_t)),
//    _descr_size(sizeof (base_t)),
//    _fortran_order(false),
//    _size(1)
//{
//    load(fn);
//}

//array::array(const array& c) :
//    _descr(typeid (base_t)),
//    _descr_size(sizeof (base_t)),
//    _fortran_order(false),
//    _size(1)
//{
//    *this = c;
//}

//array& array::operator =(const array& c)
//{
//    _shape = c._shape;
//    _descr = c._descr;
//    _descr_size = c._descr_size;
//    _fortran_order = c._fortran_order;
//    _size = c._size;

//    _data.reset(new base_t[_size * _descr_size], std::default_delete<base_t[]>());
//    memcpy(_data.get(), c._data.get(), _size * _descr_size);

//    return *this;
//}

//size_t array::dimensions() const
//{
//    return _shape.size();
//}

//size_t array::size() const
//{

//    return _size;
//}

//size_t array::size(size_t d) const
//{
//    return _shape[d];
//}

//descr_t array::descr() const
//{
//    return _descr;
//}

//size_t array::descr_size() const
//{
//    return _descr_size;
//}

//shape_t array::shape() const
//{
//    return _shape;
//}

//void array::load(const std::string& fn)
//{
//    std::ifstream ifs(fn, std::ios_base::in | std::ios_base::binary);

//    load(ifs);
//}

//void array::load(std::istream& st)
//{
//    //Check the magic phrase
//    char magic[7];
//    memset(magic, 0, 7);
//    st.read(magic, 6); // read 6 * 1 char

//    if(strcmp(magic, "\x93NUMPY") != 0)
//        throw std::runtime_error("not a numpy file");

//    //Check version
//    uint8_t maj, min;
//    st.read(reinterpret_cast<char*>(&maj), 1); // read 1 * 1 uint8
//    st.read(reinterpret_cast<char*>(&min), 1); // read 1 * 1 uint8

//    //header_len field size
//    size_t hls;

//    if		(maj == 1 && min == 0)	hls = 2; // 2 bytes for v1.0
//    else if	(maj == 2 && min == 0)	hls = 4; // 4 bytes for v2.0
//    else
//        throw std::runtime_error(std::to_string(maj) + "." + std::to_string(min) + ": sorry, I can't read that version...");

//    //read header len, little endian uint16
//    size_t header_len = 0;

//    if(IS_BIG_ENDIAN)
//    {
//        size_t w = 1;
//        for(size_t i = 0; i < hls; i++) //read it one byte by one byte to avoid endianness problem
//        {
//            uint8_t t;
//            st.read(reinterpret_cast<char*>(&t), 1);
//            header_len += t * w;
//            w *= 256;
//        }
//    }
//    else
//        st.read(reinterpret_cast<char*>(&header_len), static_cast<std::streamsize>(hls));

//    //Read the header
//    std::string header(header_len, 0);
//    st.read(&header[0], static_cast<std::streamsize>(header_len));
//    char o = parse_header(header);

//    //Read raw data
//    _data.reset(new base_t[_size * _descr_size], std::default_delete<base_t[]>());
//    st.read(reinterpret_cast<char*>(_data.get()), static_cast<std::streamsize>(_size * _descr_size));

//    //re-order bytes for the system
//    bool swap = false;

//    if(IS_BIG_ENDIAN)
//        swap = o == '<';
//    else
//        swap = o == '>';

//    if(swap)
//    {
//        base_t* p = _data.get();
//        for(size_t i = 0; i < _size; i++)
//        {
//            std::reverse(p, p + _descr_size);
//            p += _descr_size;
//        }
//    }
//}

//char array::parse_header(std::string h)
//{
//    h.erase(std::remove_if(h.begin(), h.end(), isspace), h.end());

//    size_t fop = h.find("'fortran_order':") + 16;
//    _fortran_order = h.substr(fop, 4) == "True";

//    size_t dp = h.find("'descr':") + 8;
//    std::regex descr_check("'[<>|][fiub][1248]'");

//    std::string descr = h.substr(dp, 5);

//    if(!std::regex_match(descr.begin(), descr.end(), descr_check))
//        throw std::runtime_error(descr + ": Sorry, I don't cover that format :(");

//    char e = descr[1];

//    parse_type(descr);

//    size_t sp = h.find("'shape':") + 8;
//    std::regex shape_check("\\((\\d+,?)+\\),?");

//    std::string shape;

//    if(sp > dp && sp > fop)
//    {
//        shape = h.substr(sp);
//        shape.erase(shape.size()-1);
//    }
//    else
//    {
//        size_t min = std::min(dp, fop);
//        size_t max = std::max(dp, fop);

//        if(sp < min)
//            shape = h.substr(sp, min - sp);
//        else
//            shape = h.substr(sp, max - sp);
//    }

//    if(!std::regex_match(shape.begin(), shape.end(), shape_check))
//        throw std::runtime_error("shape tuple not well formed");

//    parse_shape(shape);

//    if(_fortran_order)
//        std::reverse(_shape.begin(), _shape.end());

//    return e;
//}

//void array::parse_type(std::string d)
//{
//    _descr_size = static_cast<size_t>(d[3] - 0x30);

//    char t = d[2];

//    if(t == 'b')
//    {
//        if(d[1] != '|' || _descr_size != 1)
//            throw std::runtime_error(d + ": boolean can't have endianness");
//        _descr = typeid (bool);
//    }
//    else if(t == 'i')
//    {
//             if(_descr_size == sizeof (int8_t))		_descr = typeid (int8_t);
//        else if(_descr_size == sizeof (int16_t))	_descr = typeid (int16_t);
//        else if(_descr_size == sizeof (int32_t))	_descr = typeid (int32_t);
//        else if(_descr_size == sizeof (int64_t))	_descr = typeid (int64_t);
//        else
//            throw std::runtime_error(d + ": unhandle integer type");
//    }
//    else if(t == 'u')
//    {
//             if(_descr_size == sizeof (uint8_t))	_descr = typeid (uint8_t);
//        else if(_descr_size == sizeof (uint16_t))	_descr = typeid (uint16_t);
//        else if(_descr_size == sizeof (uint32_t))	_descr = typeid (uint32_t);
//        else if(_descr_size == sizeof (uint64_t))	_descr = typeid (uint64_t);
//        else
//            throw std::runtime_error(d + ": unhandle unsigned type");
//    }
//    else if(t == 'f')
//    {
//             if(_descr_size == sizeof (float))	_descr = typeid (float);
//        else if(_descr_size == sizeof (double))	_descr = typeid (double);
//        else
//            throw std::runtime_error(d + ": unhandle float type");
//    }
//    else
//        throw std::runtime_error(d + ": wrong type");
//}

//void array::parse_shape(std::string s)
//{
//    std::regex r("\\d+");

//    std::sregex_iterator b = std::sregex_iterator(s.begin(), s.end(), r);
//    std::sregex_iterator e = std::sregex_iterator();

//    _shape.clear();
//    _size = 1;

//    for(auto it = b; it != e; it++)
//    {
//        std::string str = it->str();
//        size_t c = std::stoul(str);
//        _shape.push_back(c);
//        _size *= c;
//    }
//}

//void array::save(const std::string& fn)
//{
//    std::ofstream ofs(fn, std::ios_base::out | std::ios_base::binary);
//    save(ofs);
//}

//void array::save(std::ostream& st)
//{
//    //magic
//    st.write("\x93NUMPY", 6);

//    std::string h = build_header();

//    //Version depends on header size
//    uint8_t v = 0;
//    if(h.size() < 65526)
//        v = 1;
//    else
//        v = 2;

//    st.put(static_cast<char>(v));
//    st.put('\0');

//    size_t m = 64 - ((6+2+2*v+h.size())%64);
//    for(size_t i = 0; i < m; i++)
//        h += ' ';

//    size_t s = h.size();

//    //little endian uint16 / uint32 representing header size;
//    char* c = reinterpret_cast<char*>(&s);
//    size_t ts = sizeof (size_t);

//    if(v == 1)
//    {
//        if(IS_BIG_ENDIAN)
//        {
//            st.write(c+ts-1, 1);
//            st.write(c+ts-2, 1);
//        }
//        else
//            st.write(c, 2);
//    }
//    else
//    {
//        if(IS_BIG_ENDIAN)
//        {
//            st.write(c+ts-1, 1);
//            st.write(c+ts-2, 1);
//            st.write(c+ts-3, 1);
//            st.write(c+ts-4, 1);
//        }
//        else
//            st.write(c, 4);
//    }

//    st.write(h.c_str(), static_cast<std::streamsize>(h.size()));

//    st.write(reinterpret_cast<char*>(_data.get()), static_cast<std::streamsize>(_size * _descr_size));
//}

//std::string array::build_header()
//{
//    std::string h;

//    h += "{'descr': '";

//    if(_descr_size == 1)
//        h += '|';
//    else
//        h += IS_BIG_ENDIAN ? '>' : '<';

//    h += get_type();

//    h += std::to_string(_descr_size);

//    h += "', 'fortran_order': ";

//    if(_fortran_order)
//        h += "True";
//    else
//        h += "False";

//    h += ", 'shape': (";

//    for(size_t s : _shape)
//        h += std::to_string(s) + ", ";

//    h += "), }\n";

//    return h;
//}

//char array::get_type()
//{
//    if(_descr == typeid (bool))
//        return 'b';

//    if(_descr == typeid (int8_t) ||
//        _descr == typeid (int16_t) ||
//        _descr == typeid (int32_t) ||
//        _descr == typeid (int64_t))
//        return 'i';

//    if(_descr == typeid (uint8_t) ||
//        _descr == typeid (uint16_t) ||
//        _descr == typeid (uint32_t) ||
//        _descr == typeid (uint64_t))
//        return 'u';

//    if(_descr == typeid (float) ||
//        _descr == typeid (double))
//        return 'f';

//    throw std::runtime_error("unable to write the type stored in the array");
//}

//Npz::Npz(const std::string& fn)
//{
//    load(fn);
//}

//void Npz::load(const std::string& fn)
//{
//    _arrays.clear();
//    miniz_cpp::zip_file f;

//    f.load(fn);

//    for(auto npy_n : f.namelist())
//    {
//        std::string content = f.read(npy_n);
//        std::istringstream iss (content);
//        npy_n.erase(npy_n.size()-4);
//        try {
//            _arrays[npy_n] = array(iss);
//        } catch(std::runtime_error& e) {
//            throw std::runtime_error(npy_n + " - " + e.what());
//        }
//    }
//}

//void Npz::save(const std::string& fn)
//{
//    miniz_cpp::zip_file f;

//    for(auto e : _arrays)
//    {
//        std::stringstream st;
//        try {
//            e.second.save(st);
//            f.writestr(e.first+".npy", st.str());
//        } catch(std::runtime_error& ex) {
//            throw std::runtime_error(e.first + " - " + ex.what());
//        }
//    }

//    f.save(fn);
//}

//bool Npz::add(const std::string& fn, const array& a, bool override)
//{
//    if(contains(fn))
//    {
//        if(!override)
//            return false;

//        _arrays[fn] = a;
//        return true;
//    }

//    return _arrays.emplace(fn, a).second;
//}

//array& Npz::get(const std::string& fn)
//{
//    return _arrays.at(fn);
//}

//const array& Npz::get(const std::string& fn) const
//{
//    return _arrays.at(fn);
//}

//void Npz::remove(const std::string& fn)
//{
//    _arrays.erase(fn);
//}

//bool Npz::move(const std::string& from, const std::string& to, bool override)
//{
//    if(!contains(from))
//        return false;

//    if(!override && contains(to))
//        return false;

//    add(to, _arrays.at(from));
//    remove(from);

//    return true;
//}

//bool Npz::canBeMapped() const
//{
//    const array& f = _arrays.cbegin()->second;
//    return std::all_of(++_arrays.cbegin(), _arrays.cend(), [&](const std::pair<std::string, array>& e) {
//        return e.second._shape == f._shape;
//    });
//}

//size_t Npz::size() const
//{
//    return _arrays.size();
//}

//bool Npz::contains(const std::string& s) const
//{
//    return _arrays.find(s) != _arrays.end();
//}

//std::vector<std::string> Npz::files() const
//{
//    std::vector<std::string> r;
//    for(auto e : _arrays)
//        r.push_back(e.first);

//    return r;
//}

//}
