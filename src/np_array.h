#ifndef NP_ARRAY_H
#define NP_ARRAY_H

#include "np_descr_t.h"
#include "np_shape_t.h"
#include "np_base_iterator.h"

#include <filesystem>

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

    void save(const std::filesystem::path& file) const;
    void save(std::ostream& stream) const;

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



// ====== NPZ ==================================================================



typedef std::unordered_map<std::string, array> npz;

npz npz_load(const std::filesystem::path& file);

void npz_save(const npz& arrays, const std::filesystem::path& file);

}

void swap(np::array& a, np::array& b);

#endif // NP_ARRAY_H
