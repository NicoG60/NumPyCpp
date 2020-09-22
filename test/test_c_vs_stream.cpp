#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>
#include <numpycpp.h>
#include <fstream>
#include "global.h"

TEST_CASE("Benchmark C vs std::stream", "[array]")
{
    BENCHMARK("load stream")
    {
        std::ifstream stream(NPY_HUGE, std::ios_base::in | std::ios_base::binary);
        return np::array::load(stream);
    };

    BENCHMARK("load c style")
    {
        auto file = std::fopen(NPY_HUGE.c_str(), "rb");
        return np::array::load(file);
    };
}
