#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>
#include <numpycpp/numpycpp.h>
#include <fstream>
#include "global.h"

TEST_CASE("Benchmark C vs std::stream", "[array]")
{
    BENCHMARK("load c style")
    {
        auto file = std::fopen(NPY_HUGE.string().c_str(), "rb");
        auto a = np::array::load(file);
        std::fclose(file);
        a.convert_to();
        return a;
    };

    BENCHMARK("load stream")
    {
        std::ifstream stream(NPY_HUGE, std::ios_base::in | std::ios_base::binary);
        auto a = np::array::load(stream);
        a.convert_to();
        return a;
    };
}
