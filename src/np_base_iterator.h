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

    /**
     * @brief Copy ctor
     */
    template<bool other_is_const>
    base_iterator(const base_iterator<Array, other_is_const>& copy) :
        _array(copy._array),
        _data(copy._data)
    {}

    /**
     * @brief Move ctor
     */
    template<bool other_is_const>
    base_iterator(base_iterator<Array, other_is_const>&& move) :
        _array(move._array),
        _data(move._data)
    {}

    /**
     * @brief copy assignment
     */
    template<bool other_is_const>
    base_iterator& operator=(const base_iterator<Array, other_is_const>& copy)
    {
        _array = copy._array;
        _data = copy._data;
    }

    /**
     * @brief move assignment
     */
    template<bool other_is_const>
    base_iterator& operator=(base_iterator<Array, other_is_const>&& move)
    {
        _array = move._array;
        _data = move._data;
    }

    /**
     * @brief checks iterator validity
     */
    explicit operator bool() const
    {
        return _array;
    }

    /**
     * @brief checks iterator invalidity
     */
    bool operator!() const
    {
        return !_array;
    }

    /**
     * @brief Wether the other is pointing at the same element as this.
     * @throw a np::error if the iterator comes from another array.
     */
    template<bool other_is_const>
    bool operator ==(const base_iterator<Array, other_is_const>& other) const
    {
        return _array == other._array && _data == other._data;
    }

    /**
     * @brief Wether the other is pointing at a different element as this.
     * @throw a np::error if the iterator comes from another array.
     */
    template<bool other_is_const>
    bool operator !=(const base_iterator<Array, other_is_const>& other) const
    {
        return !(*this == other);
    }

    /**
     * @brief Wether the other is pointing an element after this
     * @throw a np::error if the iterator comes from another array.
     */
    template<bool other_is_const>
    bool operator <(const base_iterator<Array, other_is_const>& other) const
    {
        if(_array != other._array)
        throw error("comparing iterators from different arrays");

        return _data < other._data;
    }

    /**
     * @brief Wether the other is pointing an element before this
     * @throw a np::error if the iterator comes from another array.
     */
    template<bool other_is_const>
    bool operator >(const base_iterator<Array, other_is_const>& other) const
    {
        return other < *this;
    }

    /**
     * @brief Wether the other is pointing an element before or equal to this
     * @throw a np::error if the iterator comes from another array.
     */
    template<bool other_is_const>
    bool operator <=(const base_iterator<Array, other_is_const>& other) const
    {
        return !(*this < other);
    }

    /**
     * @brief Wether the other is pointing an element after or equal to this
     * @throw a np::error if the iterator comes from another array.
     */
    template<bool other_is_const>
    bool operator >=(const base_iterator<Array, other_is_const>& other) const
    {
        return !(*this > other);
    }

    /**
     * @brief pre-increment. Make this move 1 element ahead.
     */
    base_iterator& operator++()
    {
        _data += _array->descr().stride();
        return *this;
    }

    /**
     * @brief post increment. Make this move 1 element ahead
     */
    base_iterator operator++(int)
    {
        base_iterator<Array, is_const> it = *this;
        ++*this;
        return it;
    }

    /**
     * @brief Return an iterator pointing @a s element ahead
     */
    base_iterator operator+(difference_type s) const
    {
        base_iterator<Array, is_const> it = *this;
        it += s;
        return it;
    }

    /**
     * @brief Move this @a s element ahead
     */
    base_iterator& operator+=(difference_type s)
    {
        _data += s * _array->descr().stride();
        return *this;
    }

    /**
     * @brief pre-decrement. Move this 1 element backward
     */
    base_iterator& operator--()
    {
        _data -= _array->descr().stride();
        return *this;
    }

    /**
     * @brief post-decrement. Move this 1 element backward
     */
    base_iterator operator--(int)
    {
        base_iterator<Array, is_const> it = *this;
        --*this;
        return it;
    }

    /**
     * @brief Returns the number of element between this and @a it
     */
    template<bool other_is_const>
    difference_type operator-(const base_iterator<Array, other_is_const>& it) const
    {
        if(_array != it._array)
            throw error("comparing iterators from different arrays");

        return (_data - it._data) / _array->descr().stride();
    }

    /**
     * @brief Returns an iterator @a s element before this
     */
    base_iterator operator-(difference_type s) const
    {
        base_iterator<Array, is_const> it = *this;
        it -= s;
        return it;
    }

    /**
     * @brief Move this @a s element backward
     */
    base_iterator& operator-=(difference_type s)
    {
        _data -= s * _array->descr().stride();
        return *this;
    }

    /**
     * @brief Return this... yeah I know
     */
    pointer operator->()
    {
        return this;
    }

    /**
     * @brief Return *this
     */
    reference operator*()
    {
        return *this;
    }

    /**
     * @brief Returns a raw pointer of the element pointed
     */
    template<bool is_not_const = !is_const, typename std::enable_if_t<is_not_const, int> = 0>
    char* ptr()
    {
        return _data;
    }

    /**
     * @brief Return a raw pointer to the @a field of the current element
     */
    template<bool is_not_const = !is_const, typename std::enable_if_t<is_not_const, int> = 0>
    char* ptr(const std::string& field)
    {
        auto& f = _array->descr()[field];
        return _data + f.offset();
    }

    /**
     * @brief Returns a raw constant pointer of the element pointed
     */
    const char* ptr() const
    {
        return _data;
    }

    /**
     * @brief Return a raw constant pointer to the @a field of the current element
     */
    const char* ptr(const std::string& field) const
    {
        auto& f = _array->descr()[field];
        return _data + f.offset();
    }

    /**
     * @brief returns a reference to the current element cast as T
     */
    template<class T, bool is_not_const = !is_const, typename std::enable_if_t<is_not_const, int> = 0>
    T& value()
    {
        if(_array->type().index() != typeid (T))
            throw error("bad type cast");

        return *reinterpret_cast<T*>(ptr());
    }

    /**
     * @brief returns a reference to the @a field of the current element cast as T
     */
    template<class T, bool is_not_const = !is_const, typename std::enable_if_t<is_not_const, int> = 0>
    T& value(const std::string& field)
    {
        if(_array->type(field).index() != typeid (T))
            throw error("bad type cast");

        return *reinterpret_cast<T*>(ptr(field));
    }

    /**
     * @brief returns a constant reference to the current element cast as T
     */
    template<class T>
    const T& value() const
    {
        return *reinterpret_cast<const T*>(ptr());
    }

    /**
     * @brief returns a constant reference to the @a field of the current
     * element cast as T
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
