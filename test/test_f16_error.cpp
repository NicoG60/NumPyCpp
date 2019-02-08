#include <catch2/catch.hpp>
#include "global.h"


TEST_CASE("Open wrong type file", "[npy][npz][failure]")
{
	try {
		NumPy::Array a(NPY_F16);
		FAIL("should not open this one.");
	} catch (...) {
		INFO("nice");
	}

	try {
		NumPy::Npz a(NPZ);
		FAIL("should not open this one.");
	} catch (...) {
		INFO("nice");
	}
}
