#include <catch2/catch_all.hpp>
#include <numpycpp/numpycpp.h>
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
                REQUIRE(b4);
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

TEST_CASE("Open simple string file")
{
    np::array a = np::array::load(NPY_STR);

    REQUIRE(a.dimensions() == 1);
    REQUIRE(a.size() == 5);

    REQUIRE(a[0].string() == "Element -1");
    REQUIRE(a[1].string() == "Element 0");
    REQUIRE(a[2].string() == "Element 1");
    REQUIRE(a[3].string() == "Element 2");
    REQUIRE(a[4].string() == "Element 3");
}

TEST_CASE("Open huge file", "[npy]")
{
    np::array huge = np::array::load(NPY_HUGE);
    REQUIRE(huge.dimensions() == 1);
    REQUIRE(huge.descr().stride() == 48);
}
