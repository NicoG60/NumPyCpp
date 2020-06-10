#include <catch2/catch.hpp>
#include "global.h"


TEST_CASE("Open wrong type file", "[npy][npz][failure]")
{
    try {
        np::array a = np::array::load(NPY_F16);
        FAIL("should not open this one.");
    } catch (...) {
        INFO("nice");
    }

    try {
        np::npz npz = np::npz_load(NPZ);
        FAIL("should not open this one.");
    } catch (...) {
        INFO("nice");
    }
}
