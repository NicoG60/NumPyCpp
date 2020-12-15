#include "np_array.h"
#include "np_error.h"

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
    auto f = std::fopen(file.string().c_str(), "rb");

    if(!f)
        throw error("unable to open file");

    finally cleanup([f](){ std::fclose(f); });

    return load(f);
}

/**
 * @brief Load the numpy data from the given @a strem
 * @throw a np::error on failure.
 */
array array::load(std::istream& stream)
{
    return load<stream_reader>(stream);
}

/**
 * @brief loads numpy data from the given @a file
 */
array array::load(std::FILE* file)
{
    return load<file_io>(file);
}

/**
 * @brief Saves the current array into @a file.
 * @throw a np::error on failure.
 */
void array::save(const fs::path& file) const
{
    auto f = std::fopen(file.string().c_str(), "wb");

    if(!f)
        throw error("unable to open file");

    finally cleanup([f](){ std::fclose(f); });

    save(f);
}

/**
 * @brief Saves the current array into the given @a stream.
 * @throw a np::error on failure
 */
void array::save(std::ostream& stream) const
{
    save<stream_writer>(stream);
}

/**
 * @brief Saves the current array into the given @a stream.
 * @throw a np::error on failure
 */
void array::save(FILE* file) const
{
    save<file_io>(file);
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



// ====== Utilities ============================================================



stream_reader::stream_reader(std::istream& stream) :
    stream(stream)
{
    if(!stream)
        throw error("input stream in fail state");
}

void stream_reader::read(void *ptr, std::size_t size)
{
    stream.read(reinterpret_cast<char*>(ptr), size);

    if(size != stream.gcount())
        throw error("error while reading input stream: "
                    "only " + std::to_string(stream.gcount()) + " byte(s) read "
                    "where " + std::to_string(size) + " byte(s) were expected");

    if(!stream)
        throw error("error while reading input stream");
}

std::size_t stream_reader::available()
{
    auto data_start = stream.tellg();
    stream.seekg(0, std::ios_base::end);
    auto data_end = stream.tellg();
    stream.seekg(data_start, std::ios_base::beg);

    return data_end-data_start;
}

stream_writer::stream_writer(std::ostream& stream) :
    stream(stream)
{
    if(!stream)
        throw error("output stream in fail state");
}

void stream_writer::write(const void* ptr, std::size_t size)
{
    stream.write(reinterpret_cast<const char*>(ptr), size);

    if(!stream)
        throw error("error while writing output stream");
}

file_io::file_io(std::FILE* file) :
    file(file)
{
    if(!file)
        throw error("file not openend");
}

void file_io::read(void* ptr, std::size_t size)
{
    auto count = std::fread(ptr, 1, size, file);

    if(std::feof(file))
        throw error("end of file");
    else if(std::ferror(file))
        throw error(std::strerror(errno));
    else if(count != size)
        throw error("error while reading file: "
                    "only " + std::to_string(count) + "byte(s) read "
                    "where " + std::to_string(size) + "byte(s) were expected");
}

void file_io::write(const void* ptr, std::size_t size)
{
    auto count = std::fwrite(ptr, 1, size, file);

    if(std::feof(file))
        throw error("end of file");
    else if(std::ferror(file))
        throw error(std::strerror(errno));
    else if(count != size)
        throw error("error while writing file: "
                    "only " + std::to_string(count) + "byte(s) written "
                    "where " + std::to_string(size) + "byte(s) were expected");
}

std::size_t file_io::available()
{
    auto data_start = std::ftell(file);
    std::fseek(file, 0, SEEK_END);
    auto data_end = std::ftell(file);
    std::fseek(file, data_start, SEEK_SET);

    return data_end-data_start;
}




// ====== NPZ ==================================================================



/**
 * @brief Loads the give npz @a file
 */
npz npz_load(const fs::path& file)
{
    std::unordered_map<std::string, array> arrays;

    miniz_cpp::zip_file f;

    f.load(file.string());

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

    f.save(file.string());
}

}
