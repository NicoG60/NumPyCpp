#ifndef NP_SHAPE_T_H
#define NP_SHAPE_T_H

#include <vector>

namespace np
{

/**
 * @brief shape_t reprensents the shape of the array
 */
typedef std::vector<std::size_t> shape_t;

std::string shape_to_string(const shape_t& shape);


/**
 * @brief compute recursivly the index of an element in C ordering given a shape
 * and coordinate of that element in the array
 *
 * @param shape the shape of the array
 * @param k the depth of recursion
 * @param nk the leftest corrdinate
 * @param args the other coordinates
 */
template<class... Args>
std::size_t index_c_order(const shape_t& shape,
                          std::size_t k,
                          std::size_t nk,
                          Args... args)
{
    return index_c_order(shape, k, nk) + index_c_order(shape, k+1, args...);
}

template<>
std::size_t index_c_order(const shape_t& shape, std::size_t k, std::size_t nk);

/**
 * @brief compute the index of an element in Fortran ordering given a shape and
 * coordinate of that element in the array
 *
 * @param shape the shape of the array
 * @param k the depth of recursion
 * @param nk the leftest corrdinate
 * @param args the other coordinates
 */
template<class... Args>
std::size_t index_f_order(const shape_t& shape,
                          std::size_t k,
                          std::size_t nk,
                          Args... args)
{
    return index_f_order(shape, k, nk) + index_f_order(shape, k+1, args...);
}

template<>
std::size_t index_f_order(const shape_t& shape, std::size_t k, std::size_t nk);


}

#endif // NP_SHAPE_T_H
