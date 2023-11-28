#include <catch2/catch_all.hpp>
#include <numpycpp/numpycpp.h>

TEST_CASE("type_t unit test", "[array]")
{
    SECTION("ctor")
    {
        np::type_t t;

        REQUIRE(t.index() == typeid (void));
        REQUIRE(t.ptype() == '\0');
        REQUIRE(t.size() == 0);
        REQUIRE(t.offset() == 0);
        REQUIRE(t.endianness() == np::NativeEndian);
        REQUIRE(t.suffix().empty());
        REQUIRE_THROWS(t.to_string());
    }

    SECTION("from type")
    {
        auto t = np::type_t::from_type<std::int32_t>(np::BigEndian);

        REQUIRE(t.index()      == typeid (std::int32_t));
        REQUIRE(t.ptype()      == '\0');
        REQUIRE(t.size()       == 4);
        REQUIRE(t.offset()     == 0);
        REQUIRE(t.endianness() == np::BigEndian);
        REQUIRE_NOTHROW(t.to_string());
        REQUIRE(t.to_string() == "'>i4'");
    }

    SECTION("from string")
    {
        auto t = np::type_t::from_string("'<M8[ns]'");

        REQUIRE(t.index()      == typeid (std::int64_t));
        REQUIRE(t.ptype()      == 'M');
        REQUIRE(t.size()       == 8);
        REQUIRE(t.offset()     == 0);
        REQUIRE(t.endianness() == np::LittleEndian);
        REQUIRE(t.suffix()     == "[ns]");

        REQUIRE_NOTHROW(t.to_string());
        REQUIRE(t.to_string() == "'<M8[ns]'");

        t = np::type_t::from_string("\"<u2\"");

        REQUIRE(t.index()      == typeid (std::uint16_t));
        REQUIRE(t.ptype()      == 'u');
        REQUIRE(t.size()       == 2);
        REQUIRE(t.offset()     == 0);
        REQUIRE(t.endianness() == np::LittleEndian);
        REQUIRE(t.suffix().empty());

        REQUIRE(t == np::type_t::from_string("'<u2'"));

        REQUIRE_THROWS(np::type_t::from_string(""));
        REQUIRE_THROWS(np::type_t::from_string("\"\""));
        REQUIRE_THROWS(np::type_t::from_string("''"));
        REQUIRE_THROWS(np::type_t::from_string("test"));
        REQUIRE_THROWS(np::type_t::from_string("'test'"));
        REQUIRE_THROWS(np::type_t::from_string("\"test\""));

        REQUIRE_THROWS(np::type_t::from_string("=O2"));
        REQUIRE_THROWS(np::type_t::from_string("<i16"));
        REQUIRE_THROWS(np::type_t::from_string("<i6"));
        REQUIRE_THROWS(np::type_t::from_string("=u3"));
        REQUIRE_THROWS(np::type_t::from_string("=f2"));
        REQUIRE_THROWS(np::type_t::from_string("<c2"));
    }

    SECTION("Unsupported type")
    {
        auto t = np::type_t::from_type<long double>();
        REQUIRE_THROWS(t.to_string());
    }
}
