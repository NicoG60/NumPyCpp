#include <catch2/catch.hpp>
#include <numpycpp.h>
#include "global.h"

TEMPLATE_TEST_CASE("Open npz file", "[npz]",
                   bool,
                   int8_t, uint8_t,
                   int16_t, uint16_t,
                   int32_t, uint32_t,
                   int64_t, uint64_t,
                   float, double)
{
    np::npz z = np::npz_load(NPZ_TYPES);

    for(auto& p : z)
    {
        np::array& a = p.second;

        REQUIRE(a.dimensions() == 1);
        REQUIRE(a.size() == 5);

        if(std::type_index(typeid (TestType))  == a.type().index())
        {
            if(typeid (TestType) == typeid (bool))
            {
                bool b0 = a[0].value<bool>();
                bool b1 = a[1].value<bool>();
                bool b2 = a[2].value<bool>();
                bool b3 = a[3].value<bool>();
                bool b4 = a[4].value<bool>();

                REQUIRE(b0);
                REQUIRE_FALSE(b1);
                REQUIRE(b2);
                REQUIRE(b3);
                REQUIRE_FALSE(b4);
            }
            else
            {
                TestType t0 = a[0].value<TestType>();
                TestType t1 = a[1].value<TestType>();
                TestType t2 = a[2].value<TestType>();
                TestType t3 = a[3].value<TestType>();
                TestType t4 = a[4].value<TestType>();

                REQUIRE(t0 == static_cast<TestType>(-1));
                REQUIRE(t1 == static_cast<TestType>(0));
                REQUIRE(t2 == static_cast<TestType>(1));
                REQUIRE(t3 == static_cast<TestType>(2));
                REQUIRE(t4 == static_cast<TestType>(3));
            }
        }
        else
            REQUIRE_THROWS(a[0].value<TestType>());


        a = np::array(np::descr_t::make<TestType>(), {3, 3, 3});

        REQUIRE(a.dimensions() == 3);
        REQUIRE(a.size(0) == 3);
        REQUIRE(a.size(1) == 3);
        REQUIRE(a.size(2) == 3);
        REQUIRE(a.size() == 27);

        const TestType* d = a.data_as<TestType>();
        for(size_t i = 0; i < a.size(); i++)
            REQUIRE(d[i] == 0);

        REQUIRE_THROWS(a.at(3, 3, 3));
        REQUIRE_NOTHROW(a.at(0, 1, 2));
    }
}
