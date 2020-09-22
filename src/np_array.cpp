#include "np_array.h"
#include "np_error.h"

#include <fstream>
#include <regex>

namespace fs = std::filesystem;

#include <zip_file.hpp>

namespace np
{

/**
 * @brief Copy ctor
 * @param c another array.
 */
array::array(const array& c) :
    array()
{
    *this = c;
}

/**
 * @brief Move ctor
 * @param m another array.
 */
array::array(array&& m) :
    array()
{
    swap(m);
}

/**
 * @brief Constructs an array with the given property.
 *
 * @param d The structure descriptor.
 * @param s The dimension shape.
 * @param f Wether the array is in Fortran order.
 */
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

/**
 * @brief dtor
 */
array::~array()
{
    if(_data)
        delete[] _data;
}

/**
 * @brief Copy assignment.
 */
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

/**
 * @brief Move assignment.
 */
array& array::operator =(array&& m)
{
    swap(m);
    return *this;
}

/**
 * @brief Returns wether the array is empty.
 */
bool array::empty() const
{
    return _data == nullptr || data_size() == 0;
}

/**
 * @brief Swap the content of 2 arrays.
 * @param o another array.
 */
void array::swap(array& o)
{
    _shape.swap(o._shape);
    _descr.swap(o._descr);
    std::swap(_fortran_order, o._fortran_order);
    std::swap(_data, o._data);
}

/**
 * @brief Returns the number of elements in the array.
 *
 * This is equivalent to `dim_0 * dim_1 * ... * dim_n`.
 */
std::size_t array::size() const
{
    if(_shape.empty())
        return 0;

    if(_descr._stride == 0)
        return 0;

    std::size_t s = 1;

    for(auto& d : _shape)
        s *= d;

    return s;
}

/**
 * @brief Returns the size of the given dimension @a d.
 * @throw an np::error if out of range.
 */
std::size_t array::size(std::size_t d) const
{
    if(d >= _shape.size())
        throw error("out of range");

    return _shape[d];
}

/**
 * @brief Returns the number of dimensions.
 */
std::size_t array::dimensions() const
{
    return _shape.size();
}

/**
 * @brief Returns the shape description.
 *
 * This is just a vector containing the sizes of all the dimensions i.e :
 * `{d0, d1, ..., dn}`
 */
const shape_t& array::shape() const
{
    return _shape;
}

/**
 * @brief Returns the structure descriptor.
 * @see np::descr_t
 */
const descr_t& array::descr() const
{
    return _descr;
}

/**
 * @brief Returns the type of the first columns of the structure.
 *
 * This is especially useful for unstructured array.
 *
 * @throw a np::error if the descriptor is empty.
 */
const type_t& array::type() const
{
    if(_descr._fields.empty())
        throw error("fields does not exists");

    return _descr.begin()->second;
}

/**
 * @brief Returns the type of the given @a field.
 * @throw a np::error if the field name does not exists.
 */
const type_t& array::type(const std::string& field) const
{
    return _descr[field];
}

/**
 * @brief Wether the array is in fortran order.
 */
bool array::fortran_order() const
{
    return _fortran_order;
}

/**
 * @brief The raw pointer to the data.
 *
 * Note that the value returned by size() is not the size of the raw data.
 * It is just the number of element in the array. use data_size() for that
 * purpose.
 */
const char* array::data() const
{
    return _data;
}

/**
 * @brief Loads the given npy @a file.
 * @throw a np::error on failure.
 */
array array::load(const fs::path& file)
{
    std::ifstream st(file, std::ios_base::in | std::ios_base::binary);

    if(!st || !st.is_open())
        throw error("unable to open file " + file.string());

    return load(st);
}

/**
 * @brief Load the numpy data from the given @a strem
 * @throw a np::error on failure.
 */
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

/**
 * @brief loads numpy data from the given @a file
 */
array array::load(std::FILE* file)
{
    //Check the magic phrase
    char magic[7];
    std::memset(magic, 0, 7);
    std::fread(magic, 1, 6, file);

    if(std::strcmp(magic, "\x93NUMPY") != 0)
        throw error("not a numpy file");

    //Check version
    std::uint8_t maj, min;
    std::fread(&maj, 1, 1, file);
    std::fread(&min, 1, 1, file);

    //read header len, little endian
    std::size_t header_len = 0;

    if(maj == 1 && min == 0) // 2 bytes for v1.0
    {
        std::uint16_t tmp;
        std::fread(&tmp, 2, 1, file);
        header_len = byte_swap(tmp, LittleEndian, NativeEndian);
    }
    else if(maj == 2 && min == 0) // 4 bytes for v2.0
    {
        std::fread(&header_len, 4, 1, file);
        header_len = byte_swap(header_len, LittleEndian, NativeEndian);
    }
    else
        throw error(std::to_string(maj)+"."+std::to_string(min) +
                                 ": sorry, I can't read that version...");

    //Read the header
    std::string header(header_len, 0);
    std::fread(header.data(), 1, header_len, file);

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

    std::fread(a._data, 1, a.data_size(), file);

    return a;
}

/**
 * @brief Saves the current array into @a file.
 * @throw a np::error on failure.
 */
void array::save(const fs::path& file) const
{
    std::ofstream st(file, std::ios::binary);

    if(!st || !st.is_open())
        throw error("unable to open file");

    return save(st);
}

/**
 * @brief Saves the current array into the given @a stream.
 * @throw a np::error on failure
 */
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

/**
 * @brief Returns the string header of the current array.
 */
std::string array::header() const
{
    return "{"
           "'descr': " + _descr.to_string() + ", "
           "'fortran_order': " + std::string(_fortran_order ? "True" : "False") + ", "
           "'shape': " + shape_to_string(_shape) +
           "}";
}

/**
 * @brief Swap the bytes of the array in respect of @a e.
 */
void array::convert_to(Endianness e)
{
    for(auto it : *this)
    {
        for(auto& field : _descr._fields)
        {
            auto& type = field.second;

            if(type._endianness != e)
            {
                byte_swap(it.ptr(field.first), type._size);
                type._endianness = e;
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

/**
 * @brief Returns an iterator pointing to the first element.
 */
array::iterator array::begin()
{
    iterator it;
    it._array = this;
    it._data = _data;

    return it;
}

/**
 * @brief Returns a constant iterator pointing to the first element.
 */
array::const_iterator array::begin() const
{
    return cbegin();
}

/**
 * @brief Returns a constant iterator pointing to the first element.
 */
array::const_iterator array::cbegin() const
{
    iterator it;
    it._array = const_cast<array*>(this);
    it._data = _data;

    return it;
}

/**
 * @brief Returns an iterator pointing past the last element.
 */
array::iterator array::end()
{
    iterator it;
    it._array = this;
    it._data = _data + data_size();

    return it;
}

/**
 * @brief Returns a constant iterator pointing past the last element.
 */
array::const_iterator array::end() const
{
    return cend();
}

/**
 * @brief Returns a constant iterator pointing past the last element.
 */
array::const_iterator array::cend() const
{
    const_iterator it;
    it._array = const_cast<array*>(this);
    it._data = _data + data_size();

    return it;
}

/**
 * @brief Returns an iterator pointing at the element at @a index
 */
array::iterator array::operator[](std::size_t index)
{
    return at_index(index);
}

/**
 * @brief Returns a constant iterator pointing at the element at @a index
 */
array::const_iterator array::operator[](std::size_t index) const
{
    return at_index(index);
}

/**
 * @brief Returns an iterator pointing at the element at @a index
 */
array::iterator array::at_index(std::size_t index)
{
    if(index >= size())
        throw error("out of range");

    iterator it;
    it._array = this;
    it._data = _data + index * _descr.stride();

    return it;
}

/**
 * @brief Returns a constant iterator pointing at the element at @a index
 */
array::const_iterator array::at_index(std::size_t index) const
{
    if(index >= size())
        throw error("out of range");

    const_iterator it;
    it._array = const_cast<array*>(this);
    it._data = _data + index * _descr.stride();

    return it;
}

/**
 * @brief Returns an iterator pointing to the coordinates @a indices
 */
array::iterator array::at(std::vector<std::size_t> indices)
{
    return at(index(indices));
}

/**
 * @brief Returns a constant iterator pointing to the coordinates @a indices
 */
array::const_iterator array::at(std::vector<std::size_t> indices) const
{
    return at(index(indices));
}

/**
 * @brief Returns the index of the element at coordinates @a indices
 */
std::size_t array::index(std::vector<std::size_t> indices) const
{
    if(indices.size() < _shape.size())
        throw error("size does not match");

    std::size_t idx = 0;

    if(_fortran_order)
    {
        std::size_t k = 0;

        for(auto& nk : indices)
            idx += index_f_order(_shape, k++, nk);
    }
    else
    {
        std::size_t k = 0;

        for(auto& nk : indices)
            idx += index_c_order(_shape, k++, nk);
    }

    return idx;
}

/**
 * @brief Returns the size of the underlying data blob.
 */
std::size_t array::data_size() const
{
    return size() * _descr.stride();
}



// ====== NPZ ==================================================================



/**
 * @brief Loads the give npz @a file
 */
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

/**
 * @brief Saves the given set of @a arrays into a npz @a file
 */
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
