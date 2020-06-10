#include <catch2/catch.hpp>
#include "global.h"

TEST_CASE("array unit test", "[array]")
{
    SECTION("constructor")
    {
        np::array a;

        REQUIRE(a.empty());
        REQUIRE(a.dimensions() == 0);
        REQUIRE(a.size() == 0);

        np::array b(np::array::make<int>({3, 3, 3}));

        REQUIRE(!b.empty());
        REQUIRE(b.dimensions() == 3);
        REQUIRE(b.size() == 27);

        np::array c(b);

        REQUIRE(!b.empty());
        REQUIRE(b.dimensions() == 3);
        REQUIRE(b.size() == 27);

        REQUIRE(!c.empty());
        REQUIRE(c.dimensions() == 3);
        REQUIRE(c.size() == 27);
    }

    SECTION("assignment")
    {
        np::array a;

        REQUIRE(a.empty());
        REQUIRE(a.dimensions() == 0);
        REQUIRE(a.size() == 0);

        np::array b = np::array::make<int>({3, 3, 3});

        REQUIRE(!b.empty());
        REQUIRE(b.dimensions() == 3);
        REQUIRE(b.size() == 27);

        np::array c = b;

        REQUIRE(!b.empty());
        REQUIRE(b.dimensions() == 3);
        REQUIRE(b.size() == 27);

        REQUIRE(!c.empty());
        REQUIRE(c.dimensions() == 3);
        REQUIRE(c.size() == 27);
    }

    SECTION("swap")
    {
        np::array a;

        REQUIRE(a.empty());
        REQUIRE(a.dimensions() == 0);
        REQUIRE(a.size() == 0);

        np::array b = np::array::make<int>({3, 3, 3});

        REQUIRE(!b.empty());
        REQUIRE(b.dimensions() == 3);
        REQUIRE(b.size() == 27);

        a.swap(b);

        REQUIRE(!a.empty());
        REQUIRE(a.dimensions() == 3);
        REQUIRE(a.size() == 27);

        REQUIRE(b.empty());
        REQUIRE(b.dimensions() == 0);
        REQUIRE(b.size() == 0);
    }

    SECTION("accesser")
    {
        np::array a;

        REQUIRE(a.size() == 0);
        REQUIRE(a.dimensions() == 0);
        REQUIRE_THROWS(a.size(0));
        REQUIRE(a.shape().empty());
        REQUIRE(a.descr().stride == 0);
        REQUIRE(a.descr().fields.empty());
        REQUIRE_THROWS(a.type());
        REQUIRE_THROWS(a.type("test"));
        REQUIRE(a.fortran_order() == false);
        REQUIRE(a.data() == nullptr);

        a = np::array::make<double>({3, 4, 5});

        REQUIRE(a.size() == 60);
        REQUIRE(a.dimensions() == 3);
        REQUIRE_NOTHROW(a.size(0));
        REQUIRE(a.size(0) == 3);
        REQUIRE(a.size(1) == 4);
        REQUIRE(a.size(2) == 5);
        REQUIRE(a.shape().size() == 3);
        REQUIRE(a.descr().stride == 8);
        REQUIRE(a.descr().fields.size() == 1);
        REQUIRE_NOTHROW(a.type());
        REQUIRE_THROWS(a.type("test"));
        REQUIRE_NOTHROW(a.type("f0"));
        REQUIRE(a.type() == a.type("f0"));
        REQUIRE(a.type().index == typeid (double));
        REQUIRE(a.type().ptype == '\0');
        REQUIRE(a.type().size == sizeof (double));
        REQUIRE(a.type().offset == 0);
        REQUIRE(a.type().endianness == np::NativeEndian);
        REQUIRE(a.fortran_order() == false);
        REQUIRE(a.data() != nullptr);
        REQUIRE(a.data_as<double>() != nullptr);
    }

    SECTION("type_t")
    {
        np::type_t t;

        REQUIRE(t.index      == typeid (void));
        REQUIRE(t.ptype      == '\0');
        REQUIRE(t.size       == 0);
        REQUIRE(t.offset     == 0);
        REQUIRE(t.endianness == np::NativeEndian);
        REQUIRE(t.suffix.empty());
        REQUIRE_THROWS(t.to_string());

        t.index = typeid (std::int32_t);
        t.size  = sizeof (std::int32_t);
        t.endianness = np::BigEndian;

        REQUIRE_NOTHROW(t.to_string());
        REQUIRE(t.to_string() == "'>i4'");

        t = np::type_t::from_string("'<M8[ns]'");

        REQUIRE(t.index      == typeid (std::int64_t));
        REQUIRE(t.ptype      == 'M');
        REQUIRE(t.size       == 8);
        REQUIRE(t.offset     == 0);
        REQUIRE(t.endianness == np::NativeEndian);
        REQUIRE(t.suffix     == "[ns]");

        REQUIRE_NOTHROW(t.to_string());
        REQUIRE(t.to_string() == "'<M8[ns]'");

        t = np::type_t::from_string("\"<u2\"");

        REQUIRE(t.index      == typeid (std::uint16_t));
        REQUIRE(t.ptype      == 'u');
        REQUIRE(t.size       == 2);
        REQUIRE(t.offset     == 0);
        REQUIRE(t.endianness == np::NativeEndian);
        REQUIRE(t.suffix.empty());

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

        t = np::type_t::from_type<long double>();
        REQUIRE_THROWS(t.to_string());
    }

    SECTION("descr_t")
    {
        np::descr_t d;

        REQUIRE(d.fields.empty());
        REQUIRE(d.stride == 0);

        d = np::descr_t::from_string("'>i4'");

        REQUIRE(d.fields.size() == 1);
        REQUIRE(d.fields.begin()->first == "f0");
        REQUIRE(d.fields.begin()->second == np::type_t::from_string("'>i4'"));
        REQUIRE(d.stride == 4);

        d = np::descr_t::from_string("('name', '>i4',)");

        REQUIRE(d.fields.size() == 1);
        REQUIRE(d.fields.begin()->first == "name");
        REQUIRE(d.fields.begin()->second == np::type_t::from_string("'>i4'"));
        REQUIRE(d.stride == 4);

        REQUIRE_THROWS(np::descr_t::from_string("('name', '>i4', (2,3,))"));

        d = np::descr_t::from_string("[('index', '<i8'), ('timestamp', '<M8[ns]'), ('swh', '<f4'), ('mwd', '<f4'), ('mwp', '<f4'), ('dwi', '<f4'), ('wind', '<f4'), ('pp1d', '<f4')]");

        REQUIRE(d.fields.size() == 8);
        REQUIRE(d.stride == 40);
        REQUIRE(d.fields["index"].offset     == 0);
        REQUIRE(d.fields["timestamp"].offset == 8);
        REQUIRE(d.fields["swh"].offset       == 16);
        REQUIRE(d.fields["mwd"].offset       == 20);
        REQUIRE(d.fields["mwp"].offset       == 24);
        REQUIRE(d.fields["dwi"].offset       == 28);
        REQUIRE(d.fields["wind"].offset      == 32);
        REQUIRE(d.fields["pp1d"].offset      == 36);
    }

    SECTION("iterators + conversion")
    {
        np::array a = np::array::make<int>({3, 3, 3});
        np::array c = np::array::make<int>({3, 3, 3}, true);

        auto it = a.begin();
        auto end = a.end();

        auto cit = c.begin();
        auto cend = c.end();

        REQUIRE(it != end);
        REQUIRE(it < end);
        REQUIRE((end - it) == a.size());

        int idx = 0;

        while(it != end && cit != cend)
        {
            it.value<int>() = idx;
            cit.value<int>() = idx;
            idx++;
            ++it;
            ++cit;
        }

        np::array b = a;

        b.convert_to(np::OpositeEndian);
        b.convert_to(np::NativeEndian);

        it = a.begin();
        end = a.end();

        auto bit = b.begin();
        auto bend = b.end();

        REQUIRE(it != bit);
        REQUIRE_THROWS(it < bit);

        while(it != end && bit != bend)
        {
            REQUIRE(it.value<int>() == bit.value<int>());
            ++it;
            ++bit;
        }

        int test = 0;

        for(std::size_t z = 0; z < 3; z++)
        {
            for(std::size_t y = 0; y < 3; y++)
            {
                for(std::size_t x = 0; x < 3; x++)
                {
                    REQUIRE(c.at(x, y, z).value<int>() == test);
                    test++;
                }
            }
        }

        test = 0;

        for(std::size_t x = 0; x < 3; x++)
        {
            for(std::size_t y = 0; y < 3; y++)
            {
                for(std::size_t z = 0; z < 3; z++)
                {
                    REQUIRE(a.at(x, y, z).value<int>() == test);
                    test++;
                }
            }
        }
    }
}