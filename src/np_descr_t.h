#ifndef NP_DESCR_T_H
#define NP_DESCR_T_H

#include <unordered_map>
#include <vector>
#include <utility>
#include <string>

#include "np_type_t.h"

namespace np
{

/**
 * @brief The field_t class represents a field in the structure descriptor.
 *
 * Under the hood it's just a std::pair with a static function to create it
 * simply from a c++ type
 */
class field_t : public std::pair<std::string, type_t>
{
public:
    field_t() = default;

    field_t(const field_t& c) = default;
    field_t(field_t&& m) = default;

    template<class N, class T>
    field_t(const std::pair<N, T>& c) :
        std::pair<std::string, type_t>(c) {}

    template<class N, class T>
    field_t(std::pair<N, T>&& m) :
        std::pair<std::string, type_t>(m) {}

    template<class N, class T>
    field_t(const N& n, const T& t) :
        std::pair<std::string, type_t>(n, t) {}

    template<class N, class T>
    field_t(N&& n, T&& t) :
        std::pair<std::string, type_t>(n, t) {}

    field_t& operator=(const field_t& c) = default;
    field_t& operator=(field_t&& m) = default;

    template<class N, class T>
    field_t& operator=(const std::pair<N, T>& c)
    {
        std::pair<std::string, type_t>::operator=(c);
    }

    template<class N, class T>
    field_t& operator=(std::pair<N, T>&& m)
    {
        std::pair<std::string, type_t>::operator=(m);
    }

    template<class T>
    static field_t make(const std::string& n = {})
    {
        return {n, type_t::from_type<T>()};
    }
};



// =============================================================================

// Some private declaration

class descr_t;

namespace details
{
template<class... Fields>
void make_descr_internal(descr_t* r, const field_t& field, Fields... other);

template<>
void make_descr_internal(descr_t* r, const field_t& field);

}



// =============================================================================



/**
 * @brief The descr_t class represent the structure descriptor with all the
 * named fields in the array and the global stride of 1 element.
 */
class descr_t
{
    friend class array;

public:
    typedef std::vector<field_t>                         fields_t;
    typedef std::unordered_map<std::string, std::size_t> lookup_t;

public:
    descr_t() = default;
    descr_t(const descr_t& copy) = default;
    descr_t(descr_t&& move) = default;

    descr_t& operator=(const descr_t& copy) = default;
    descr_t& operator=(descr_t&& move) = default;

    void swap(descr_t& o);

    /**
     * @brief Appends a new field to the current descriptor.
     *
     * This is useful to create new arrays.
     */
    template<class T>
    void push_back(std::string name = {})
    {
        push_back(type_t::from_type<T>(), name);
    }

    /**
     * @brief Appends a new field to the current descriptor.
     *
     * This is useful to create new arrays.
     */
    void push_back(type_t t, std::string name = {})
    {
        if(name.empty())
            name = "f" + std::to_string(_fields.size());

        t._offset = _stride;
        _stride += t._size;
        _lookup.emplace(name, _fields.size());
        _fields.emplace_back(name, t);
    }

    /**
     * @brief Makes a new descriptor from a type T with an optionnal name @a n
     */
    template<class T>
    static descr_t make(const std::string& n = {})
    {
        return make(field_t::make<T>(n));
    }

    /**
     * @brief Makes a new descriptor from the given fields.
     *
     * In example:
     * ```
     * np::descr_t d = np::descr_t::make(
     *     np::field_t::make<int>("my_first_field"),
     *     np::field_t::make<double>("my_second_field")
     * );
     * ```
     */
    template<class... Fields>
    static descr_t make(const field_t& field, Fields... other)
    {
        descr_t r;
        np::details::make_descr_internal(&r, field, other...);
        return r;
    }

    static descr_t from_string(const std::string& str);
    std::string to_string() const;

    inline std::size_t stride() const { return _stride; }

    fields_t::const_iterator begin() const;
    fields_t::const_iterator cbegin() const;

    fields_t::const_iterator end() const;
    fields_t::const_iterator cend() const;

    const type_t& operator[](const std::string& field) const;
    const field_t& operator[](std::size_t index) const;
    bool constains(const std::string& field) const;

    bool empty() const;
    std::size_t size() const;

private:


    static std::string extract(std::string::iterator& it,
                               const std::string::iterator& end,
                               char b,
                               char e);

    static std::pair<std::string, type_t> parse_tuple(const std::string& str);

private:
    fields_t    _fields;
    lookup_t    _lookup;
    std::size_t _stride = 0;
};

namespace details
{
template<class... Fields>
void make_descr_internal(descr_t* r, const field_t& field, Fields... other)
{
    r->push_back(field.second, field.first);
    make_descr_internal(r, other...);
}

}

}

void swap(np::descr_t& a, np::descr_t& b);

#endif // NP_DESCR_T_H
