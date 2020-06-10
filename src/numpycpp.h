/**
Copyright 2018 Nicolas Jarnoux

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE
*/
#ifndef NUMPYCPP_H
#define NUMPYCPP_H

#include <cstdio>
#include <vector>
#include <string>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <filesystem>

/**
 * NumPyCpp : A simple interface to .npy and .npz file
 */

namespace np {

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

template<class T>
T byte_swap(T value, Endianness from, Endianness to)
{
    if(from != to)
        byte_swap(&value, sizeof (T));

    return value;
}



//==============================================================================

struct type_t
{
    bool operator==(const type_t& o) const;
    bool operator!=(const type_t& o) const;

    std::type_index index      = typeid (void);
    char            ptype      = '\0';
    std::size_t     size       = 0;
    std::size_t     offset     = 0;
    Endianness      endianness = NativeEndian;
    std::string     suffix;

    static type_t from_string(const std::string& str);
    template<class T>
    static type_t from_type()
    {
        type_t r;

        r.index = typeid (T);
        r.size  = sizeof (T);

        return r;
    }

    std::string to_string() const;
};

struct descr_t
{
    std::unordered_map<std::string, type_t> fields;
    std::size_t stride = 0;

    void swap(descr_t& o);

    static descr_t from_string(const std::string& str);
    static std::string extract(std::string::iterator& it, std::string::iterator end, char b, char e);
    static std::pair<std::string, type_t> parse_tuple(const std::string& str);
    std::string to_string() const;
};

typedef std::vector<std::size_t> shape_t;

std::string shape_to_string(const shape_t& shape);



//==============================================================================



class error : public std::exception
{
public:
    error(const std::string& msg) : std::exception(), _msg(msg) {}

    const char * what() const noexcept override { return _msg.c_str(); }

private:
    std::string _msg;
};



//==============================================================================



/*
The array class represent one npy file (one n-dimensionnal array)
The array has a fixed size and can't be re-allocated.
*/
class array
{



//==============================================================================



private:
    class iterator
    {
        friend class array;

    public:
        typedef iterator  value_type;
        typedef value_type&    reference;
        typedef value_type*    pointer;
        typedef std::ptrdiff_t difference_type;

        iterator();
        iterator(const iterator&) = default;
        iterator(iterator&&) = default;

        iterator& operator=(const iterator&) = default;
        iterator& operator=(iterator&&) = default;

        bool operator ==(const iterator& other) const;
        bool operator !=(const iterator& other) const;

        bool operator <(const iterator& other) const;
        bool operator >(const iterator& other) const;
        bool operator <=(const iterator& other) const;
        bool operator >=(const iterator& other) const;

        iterator& operator++();
        iterator  operator++(int);
        iterator  operator+(difference_type s) const;
        iterator& operator+=(difference_type s);

        iterator& operator--();
        iterator  operator--(int);
        difference_type operator-(const iterator& it) const;
        iterator  operator-(difference_type s) const;
        iterator& operator-=(difference_type s);

        pointer   operator->();
        reference operator*();

        char* ptr();
        char* ptr(const std::string& field);

        const char* ptr() const;
        const char* ptr(const std::string& field) const;

        template<class T>
        T& value()
        {
            if(_array->type().index != typeid (T))
                throw error("bad type cast");

            return *reinterpret_cast<T*>(ptr());
        }

        template<class T>
        T& value(const std::string& field)
        {
            if(_array->type(field).index != typeid (T))
                throw error("bad type cast");

            return *reinterpret_cast<T*>(ptr(field));
        }

        template<class T>
        const T& value() const
        {
            return *reinterpret_cast<const T*>(ptr());
        }

        template<class T>
        const T& value(const std::string& field) const
        {
            return *reinterpret_cast<const T*>(ptr(field));
        }

    protected:
        array*  _array;
        char* _data;
    };



//==============================================================================



public:
    array();
    array(const array& c);
    array(array &&m);

protected:
    array(descr_t d, shape_t s, bool f);

public:
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

    template<class T>
    const T* data_as() const
    {
        return reinterpret_cast<const T*>(_data);
    }

    static array load(const std::filesystem::path& file);
    static array load(std::istream& stream);

    template<class T>
    static array make(const shape_t& shape, bool fortran_order = false)
    {
        descr_t d;
        d.fields.emplace("f0", type_t::from_type<T>());
        d.stride = sizeof (T);

        return array(d, shape, fortran_order);
    }

    void save(const std::filesystem::path& file) const;
    void save(std::ostream& stream) const;

    std::string header() const;

    void convert_to(Endianness e = NativeEndian);

//    void transpose_order();

    iterator begin();
    const iterator begin() const;
    const iterator cbegin() const;

    iterator end();
    const iterator end() const;
    const iterator cend() const;

    iterator operator[](std::size_t index);
    const iterator operator[](std::size_t index) const;

    iterator at(std::size_t index);
    const iterator at(std::size_t index) const;

    iterator at(std::vector<std::size_t> indices);
    const iterator at(std::vector<std::size_t> indices) const;

    template<class... Args>
    iterator at(Args... args)
    {
        return at(index(args...));
    }

    template<class... Args>
    const iterator at(Args... args) const
    {
        return at(index(args...));
    }

    std::size_t index(std::vector<std::size_t> indices) const;

    template<class... Args>
    std::size_t index(Args... args) const
    {
        if(_fortran_order)
            return index_f_order(0, args...);
        else
            return index_c_order(0, args...);
    }

private:
    std::size_t data_size() const;

    template<class... Args>
    std::size_t index_c_order(std::size_t k, std::size_t nk, Args... args) const
    {
        return index_c_order(k, nk) + index_c_order(k+1, args...);
    }

    template<>
    std::size_t index_c_order(std::size_t k, std::size_t nk) const
    {
        if(k > _shape.size())
            throw error("size does not match");

        std::size_t l = k+1;
        std::size_t Nl = 1;
        for(; l < _shape.size(); l++)
            Nl *= _shape[l];

        return Nl * nk;
    }

    template<class... Args>
    std::size_t index_f_order(std::size_t k, std::size_t nk, Args... args) const
    {
        return index_f_order(k, nk) + index_f_order(k+1, args...);
    }

    template<>
    std::size_t index_f_order(std::size_t k, std::size_t nk) const
    {
        if(k > _shape.size())
            throw error("size does not match");

        std::size_t Nl = 1;
        for(std::size_t l = 0; l < k; l++)
            Nl *= _shape[l];

        return Nl * nk;
    }

private:
    char*	_data;
    shape_t	_shape;
    descr_t	_descr;
    bool	_fortran_order;
};

typedef std::unordered_map<std::string, array> npz;

npz npz_load(const std::filesystem::path& file);

void npz_save(const npz& arrays, const std::filesystem::path& file);

}

void swap(np::descr_t& a, np::descr_t& b);
void swap(np::array& a, np::array& b);

#endif // NUMPYCPP_H
