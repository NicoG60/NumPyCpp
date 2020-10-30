#ifndef NP_ARRAY_H
#define NP_ARRAY_H

#include "np_descr_t.h"
#include "np_shape_t.h"
#include "np_base_iterator.h"

#include <filesystem>
#include <fstream>
#include <regex>

namespace np
{

/**
 * @brief The array class represents the actual numpy ndarray
 */
class array
{

public:
    typedef base_iterator<array, false> iterator;
    typedef base_iterator<array, true>  const_iterator;

public:
    array() = default;
    array(const array& c);
    array(array&& m);
    array(descr_t d, shape_t s, bool f = false);

    ~array();

    array& operator =(const array& c);
    array& operator =(array&& m);

    bool empty() const;

    void swap(array& o);

    std::size_t size() const;
    std::size_t size(std::size_t d) const;
    std::size_t dimensions() const;
    const shape_t& shape() const;
    const descr_t& descr() const;
    const type_t& type() const;
    const type_t& type(const std::string& field) const;
    bool fortran_order() const;

    const char* data() const;

    /**
     * @brief Returns a pointer to the raw data blob reintepreted as @a T
     *
     * This is useful if you know it is a linear array of a single type and want
     * to access the data at low level.
     */
    template<class T>
    const T* data_as() const
    {
        return reinterpret_cast<const T*>(_data);
    }

    static array load(const std::filesystem::path& file);
    static array load(std::istream& stream);
    static array load(std::FILE* file);

    template<class IOHelper, class Handle>
    static array load(Handle& h)
    {
        IOHelper io(h);

        //Check the magic phrase
        char magic[7];
        magic[6] = '\0';
        io.read(magic, 6);

        if(std::strcmp(magic, "\x93NUMPY") != 0)
            throw error("not a numpy file");

        //Check version
        std::uint8_t maj, min;
        io.read(&maj, 1);
        io.read(&min, 1);

        //read header len, little endian
        std::size_t header_len = 0;

        if(maj == 1 && min == 0) // 2 bytes for v1.0
        {
            std::uint16_t tmp;
            io.read(&tmp, 2);
            header_len = byte_swap(tmp, LittleEndian, NativeEndian);
        }
        else if((maj == 2 || maj == 3) && min == 0) // 4 bytes for v2.0 and v3.0
        {
            io.read(&header_len, 4);
            header_len = byte_swap(header_len, LittleEndian, NativeEndian);
        }
        else
            throw error(std::to_string(maj)+"."+std::to_string(min) +
                                     ": sorry, I can't read that version...");

        //Read the header
        std::string header(header_len, 0);
        io.read(header.data(), header_len);

        bool fortran_order = false;
        shape_t shape;
        descr_t descr;

        // Parse the header
        try
        {
            std::regex dict ("'([a-zA-Z0-9_-]+)':\\s*('[|=<>][a-zA-Z]\\d+(\\[[a_zA-Z]+\\])?'|\\[.*\\]|True|False|\\(.*\\))");
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

        std::size_t available = io.available();
        std::size_t expected = a.data_size();

        if(available != expected)
            throw error("error while reading file. "
                        "only " + std::to_string(available) + "bytes available "
                        "where " + std::to_string(expected) + "bytes were expected");

        io.read(a._data, expected);

        return a;
    }

    void save(const std::filesystem::path& file) const;
    void save(std::ostream& stream) const;
    void save(FILE* file) const;

    template<class IOHelper, class Handle>
    void save(Handle& h) const
    {
        IOHelper io(h);

        const char* magic = "\x93NUMPY\x2\x0";
        std::string header_str = header();

        while((8 + header_str.length()) % 64 != 0)
            header_str.push_back(' ');

        std::size_t len = header_str.length();
        len = byte_swap(len, NativeEndian, LittleEndian);

        io.write(magic, 8);
        io.write(&len, 4);
        io.write(header_str.data(), len);
        io.write(_data, data_size());
    }

    std::string header() const;

    void convert_to(Endianness e = NativeEndian);

//    void transpose_order();

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    iterator operator[](std::size_t index);
    const_iterator operator[](std::size_t index) const;

    iterator at_index(std::size_t index);
    const_iterator at_index(std::size_t index) const;

    iterator at(std::vector<std::size_t> indices);
    const_iterator at(std::vector<std::size_t> indices) const;

    /**
     * @brief Returns an iterator pointing at the coordinates @a args
     *
     * i.e `auto it = array.at(1, 2, 3);`
     */
    template<class... Args,
             std::enable_if_t<std::is_integral<Args>::value>...>
    iterator at(Args... args)
    {
        return at_index(index(args...));
    }

    /**
     * @brief Returns a constant iterator pointing at the coordinates @a args
     *
     * i.e `auto it = array.at(1, 2, 3);`
     */
    template<class... Args,
             std::enable_if_t<std::is_integral<Args>::value>...>
    const_iterator at(Args... args) const
    {
        return at_index(index(args...));
    }

    std::size_t index(std::vector<std::size_t> indices) const;

    /**
     * @brief returns the index corresponding to the coordinates @a args
     */
    template<class... Args>
    std::size_t index(Args... args) const
    {
        if(_fortran_order)
            return index_f_order(_shape, 0, args...);
        else
            return index_c_order(_shape, 0, args...);
    }

    std::size_t data_size() const;

private:
    char*	_data = nullptr;
    shape_t	_shape;
    descr_t	_descr;
    bool	_fortran_order = false;
};



// ====== Utilities ============================================================



class stream_reader
{
public:
    stream_reader(std::istream& stream);

    void read(void* ptr, std::size_t size);
    std::size_t available();

private:
    std::istream& stream;
};

class stream_writer
{
public:
    stream_writer(std::ostream& stream);

    void write(const void* ptr, std::size_t size);

private:
    std::ostream& stream;
};

class file_io
{
public:
    file_io(std::FILE* file);

    void read(void* ptr, std::size_t size);
    void write(const void* ptr, std::size_t size);
    std::size_t available();

private:
    std::FILE* file;
};

template<class CB>
class finally
{
public:
    finally(CB cb) : cb(cb) {}
    ~finally() { cb(); }

private:
    CB cb;
};

// ====== NPZ ==================================================================



typedef std::unordered_map<std::string, array> npz;

npz npz_load(const std::filesystem::path& file);

void npz_save(const npz& arrays, const std::filesystem::path& file);

}

void swap(np::array& a, np::array& b);

#endif // NP_ARRAY_H
