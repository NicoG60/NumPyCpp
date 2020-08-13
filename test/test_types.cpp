#include <catch2/catch.hpp>
#include <numpycpp.h>
#include "global.h"

TEMPLATE_TEST_CASE("Open simple file", "[npy]",
                   bool,
                   int8_t, uint8_t,
                   int16_t, uint16_t,
                   int32_t, uint32_t,
                   int64_t, uint64_t,
                   float, double)
{
    for(const auto& f : NPY_TYPE_FILES)
    {
        np::array a = np::array::load(f);

        REQUIRE(a.dimensions() == 1);
        REQUIRE(a.size() == 5);

        if(std::type_index(typeid (TestType))  == a.type().index)
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
    }
}

TEST_CASE("Open huge file", "[npy]")
{
    np::array huge = np::array::load(NPY_HUGE);
    REQUIRE(huge.dimensions() == 1);
    REQUIRE(huge.descr().stride == 40);
}
