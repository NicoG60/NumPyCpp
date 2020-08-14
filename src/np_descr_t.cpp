#include "np_descr_t.h"
#include "np_error.h"

#include <regex>
#include <string>
#include <string_view>

namespace np
{
/**
 * @brief Swaps the content of 2 descriptors
 */
void descr_t::swap(descr_t& o)
{
    _fields.swap(o._fields);
    _lookup.swap(o._lookup);
    std::swap(_stride, o._stride);
}

/**
 * @brief Parses the descriptor string @a str as given by the numpy file header.
 * @return A descr_t representation of @a str.
 */
descr_t descr_t::from_string(const std::string& str)
{
    descr_t r;

    if((str.front() == '\'' && str.back() == '\'') || (str.front() == '"' && str.back() == '"'))
    {
        type_t t = type_t::from_string(str);
        r._fields.emplace_back("f0", t);
        r._lookup.emplace("f0", 0);
        r._stride = t._size;
    }
    else if(str.front() == '(' && str.back() == ')')
    {
        auto p = parse_tuple(str);

        if(p.first.empty())
            p.first = "f0";

        r._stride = p.second._size;
        r._lookup.emplace(p.first, 0);
        r._fields.emplace_back(std::move(p));
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

                p.second._offset = r._stride;
                r._stride += p.second._size;
                r._lookup.emplace(p.first, f);
                r._fields.emplace_back(std::move(p));

                ++f;
            }
            else if(c == '\'')
            {
                tmp = extract(it, e, '\'', '\'');

                std::string name = "f" + std::to_string(f);

                type_t t = type_t::from_string(tmp);

                t._offset = r._stride;
                r._stride += t._size;
                r._lookup.emplace(name, f);
                r._fields.emplace_back(name, t);

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
 * @brief Returns a string representation of the descriptor to put in the numpy
 * header.
 */
std::string descr_t::to_string() const
{
    if(_fields.empty())
        return std::string();

    std::regex unnamed_test("f\\d+");

    if(_fields.size() == 1)
    {
        auto& p = *_fields.begin();
        if(std::regex_match(p.first, unnamed_test))
            return _fields.begin()->second.to_string();
        else
            return "('" + p.first + "'," + p.second.to_string() + ")";
    }


    std::string r = "[";

    for(auto& f : _fields)
    {
        if(std::regex_match(f.first, unnamed_test))
            return "(''," + f.second.to_string() + "),";
        else
            return "('" + f.first + "'," + f.second.to_string() + "),";
    }

    r += "]";

    return r;
}

/**
 * @brief Returns an iterator to the first field described so you can iterate
 * through the fields
 */
descr_t::fields_t::const_iterator descr_t::begin() const
{
    return _fields.begin();
}

/**
 * @brief Returns an iterator to the first field described so you can iterate
 * through the fields
 */
descr_t::fields_t::const_iterator descr_t::cbegin() const
{
    return _fields.cbegin();
}

/**
 * @brief Returns the past the end iterator
 */
descr_t::fields_t::const_iterator descr_t::end() const
{
    return _fields.end();
}

/**
 * @brief Returns the past the end iterator
 */
descr_t::fields_t::const_iterator descr_t::cend() const
{
    return _fields.cend();
}

/**
 * @brief Returns the type description pointed by @a field.
 */
const type_t& descr_t::operator[](const std::string& field) const
{
    auto it = _lookup.find(field);
    if(it == _lookup.end())
        throw error("Fields doesn't exists");

    return _fields[it->second].second;
}

/**
 * @brief Returns the type description pointed by @a index
 */
const field_t& descr_t::operator[](std::size_t index) const
{
    if(index >= _fields.size())
        throw error("out of range");

    return _fields[index];
}

/**
 * @brief Returns wether the given @a field exists.
 */
bool descr_t::constains(const std::string& field) const
{
    return _lookup.find(field) != _lookup.end();
}

/**
 * @brief Returns wether the current descriptor is empty
 */
bool descr_t::empty() const
{
    return _fields.empty();
}

/**
 * @brief Returns wether the current descriptor size
 */
std::size_t descr_t::size() const
{
    return _fields.size();
}

/**
 * @brief Extracts a string in between 2 characters (like [ ] or ( ) or ' '
 * @param it an iterator pointing to the openning character
 * @param end the end of the string
 * @param b the openning character
 * @param e the closing character
 * @return the extracted string including the opening/closing characters
 */
std::string descr_t::extract(std::string::iterator& it,
                             const std::string::iterator& end,
                             char b,
                             char e)
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
 * @brief Parses a simple tuple ('name', 'type')
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

namespace details
{
template<>
void make_descr_internal(descr_t* r, const field_t& field)
{
    r->push_back(field.second, field.first);
}
}

}

void swap(np::descr_t& a, np::descr_t& b)
{
    a.swap(b);
}
