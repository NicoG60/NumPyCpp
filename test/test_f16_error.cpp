#include <catch2/catch.hpp>
#include <numpycpp/numpycpp.h>
#include "global.h"


TEST_CASE("Open wrong type file", "[npy][npz][failure]")
{
    try {
        np::array a = np::array::load(NPY_F16);
        FAIL("should not open this float16 one.");
    } catch (...) {
        INFO("nice");
    }

    try {
        np::npz npz = np::npz_load(NPZ_F16);
        FAIL("should not open this npz one.");
    } catch (...) {
        INFO("nice");
    }
}
