#ifndef NP_SHAPE_T_H
#define NP_SHAPE_T_H

#include <vector>
#include <string>

namespace np
{

/**
 * @brief the shape_t typedef reprensents the shape of the array
 *
 * It's just a vector of dimension sizes as in {d0, d1, d2, ..., dn}
 */
typedef std::vector<std::size_t> shape_t;

std::string shape_to_string(const shape_t& shape);


/**
 * @brief Computes recursivly the index of an element in C ordering with the
 * given a @a shape and coordinates.
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
 * @brief Computes the index of an element in Fortran ordering with the given
 * shape and coordinates
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
