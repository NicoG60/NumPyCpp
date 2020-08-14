#ifndef NP_BASE_ITERATOR_H
#define NP_BASE_ITERATOR_H

#include "np_error.h"

#include <type_traits>
#include <string>

namespace np
{

/**
 * @brief The iterator class is used to navigate the array
 */
template<class Array, bool is_const>
class base_iterator
{
    friend class array;
    friend class base_iterator<Array, !is_const>;

public:
    typedef base_iterator<Array, is_const> value_type;
    typedef value_type&                    reference;
    typedef value_type*                    pointer;
    typedef const value_type&              const_reference;
    typedef const value_type*              const_pointer;
    typedef std::ptrdiff_t                 difference_type;

    base_iterator() = default;

    template<bool other_is_const>
    base_iterator(const base_iterator<Array, other_is_const>& copy) :
        _array(copy._array),
        _data(copy._data)
    {}

    template<bool other_is_const>
    base_iterator(base_iterator<Array, other_is_const>&& move) :
        _array(move._array),
        _data(move._data)
    {}

    template<bool other_is_const>
    base_iterator& operator=(const base_iterator<Array, other_is_const>& copy)
    {
        _array = copy._array;
        _data = copy._data;
    }

    template<bool other_is_const>
    base_iterator& operator=(base_iterator<Array, other_is_const>&& move)
    {
        _array = move._array;
        _data = move._data;
    }

    template<bool other_is_const>
    bool operator ==(const base_iterator<Array, other_is_const>& other) const
    {
        return _array == other._array && _data == other._data;
    }

    template<bool other_is_const>
    bool operator !=(const base_iterator<Array, other_is_const>& other) const
    {
        return !(*this == other);
    }

    template<bool other_is_const>
    bool operator <(const base_iterator<Array, other_is_const>& other) const
    {
        if(_array != other._array)
        throw error("comparing iterators from different arrays");

        return _data < other._data;
    }

    template<bool other_is_const>
    bool operator >(const base_iterator<Array, other_is_const>& other) const
    {
        return other < *this;
    }

    template<bool other_is_const>
    bool operator <=(const base_iterator<Array, other_is_const>& other) const
    {
        return !(*this < other);
    }

    template<bool other_is_const>
    bool operator >=(const base_iterator<Array, other_is_const>& other) const
    {
        return !(*this > other);
    }

    base_iterator& operator++()
    {
        _data += _array->descr().stride();
        return *this;
    }

    base_iterator operator++(int)
    {
        base_iterator<Array, is_const> it = *this;
        ++*this;
        return it;
    }

    base_iterator operator+(difference_type s) const
    {
        base_iterator<Array, is_const> it = *this;
        it += s;
        return it;
    }

    base_iterator& operator+=(difference_type s)
    {
        _data += s * _array->descr().stride();
        return *this;
    }

    base_iterator& operator--()
    {
        _data -= _array->descr().stride();
        return *this;
    }

    base_iterator operator--(int)
    {
        base_iterator<Array, is_const> it = *this;
        --*this;
        return it;
    }

    template<bool other_is_const>
    difference_type operator-(const base_iterator<Array, other_is_const>& it) const
    {
        if(_array != it._array)
            throw error("comparing iterators from different arrays");

        return (_data - it._data) / _array->descr().stride();
    }

    base_iterator operator-(difference_type s) const
    {
        base_iterator<Array, is_const> it = *this;
        it -= s;
        return it;
    }

    base_iterator& operator-=(difference_type s)
    {
        _data -= s * _array->descr().stride();
        return *this;
    }

    pointer operator->()
    {
        return this;
    }

    reference operator*()
    {
        return *this;
    }

    template<bool is_not_const = !is_const, typename std::enable_if_t<is_not_const, int> = 0>
    char* ptr()
    {
        return _data;
    }

    template<bool is_not_const = !is_const, typename std::enable_if_t<is_not_const, int> = 0>
    char* ptr(const std::string& field)
    {
        auto& f = _array->descr()[field];
        return _data + f.offset();
    }

    const char* ptr() const
    {
        return _data;
    }

    const char* ptr(const std::string& field) const
    {
        auto& f = _array->descr()[field];
        return _data + f.offset();
    }

    /**
     * @brief returns a ref to the item cast as T
     */
    template<class T, bool is_not_const = !is_const, typename std::enable_if_t<is_not_const, int> = 0>
    T& value()
    {
        if(_array->type().index() != typeid (T))
            throw error("bad type cast");

        return *reinterpret_cast<T*>(ptr());
    }

    /**
     * @brief returns a ref to @a field in the item cast as T
     */
    template<class T, bool is_not_const = !is_const, typename std::enable_if_t<is_not_const, int> = 0>
    T& value(const std::string& field)
    {
        if(_array->type(field).index() != typeid (T))
            throw error("bad type cast");

        return *reinterpret_cast<T*>(ptr(field));
    }

    /**
     * @brief returns a const ref to the item cast as T
     */
    template<class T>
    const T& value() const
    {
        return *reinterpret_cast<const T*>(ptr());
    }

    /**
     * @brief returns a const ref to @a field in the item cast as T
     */
    template<class T>
    const T& value(const std::string& field) const
    {
        return *reinterpret_cast<const T*>(ptr(field));
    }

protected:
    Array*  _array = nullptr;
    char*   _data  = nullptr;
};

}

#endif // NP_BASE_ITERATOR_H
