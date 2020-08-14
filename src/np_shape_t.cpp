#include "np_shape_t.h"
#include "np_error.h"

namespace np
{

/**
 * @brief Returns a string representation of the shape (as a tuple)
 */
std::string shape_to_string(const shape_t& shape)
{
    std::string r = "(";

    for(auto& s : shape)
        r += std::to_string(s) + ",";

    r += ")";

    return r;
}

/**
 *  @brief Computes recursivly the index of an element in C ordering with the
 * given a @a shape and coordinates.
 *
 * @param shape the shape of the array
 * @param k the depth of recursion
 * @param nk the leftest corrdinate
 */
template<>
std::size_t index_c_order(const shape_t& shape, std::size_t k, std::size_t nk)
{
    if(k > shape.size())
        throw error("size does not match");

    std::size_t l = k+1;
    std::size_t Nl = 1;
    for(; l < shape.size(); l++)
        Nl *= shape[l];

    return Nl * nk;
}

/**
 * @brief Computes the index of an element in Fortran ordering with the given
 * shape and coordinates
 *
 * @param shape the shape of the array
 * @param k the depth of recursion
 * @param nk the leftest corrdinate
 */
template<>
std::size_t index_f_order(const shape_t& shape, std::size_t k, std::size_t nk)
{
    if(k > shape.size())
        throw error("size does not match");

    std::size_t Nl = 1;
    for(std::size_t l = 0; l < k; l++)
        Nl *= shape[l];

    return Nl * nk;
}

}
