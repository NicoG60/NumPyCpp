#include <catch2/catch.hpp>
#include <numpycpp/numpycpp.h>

TEST_CASE("descr_t unit test", "[array]")
{
    SECTION("ctor")
    {
        np::descr_t d;

        REQUIRE(d.empty());
        REQUIRE(d.stride() == 0);
        REQUIRE(d.to_string().empty());
    }

    SECTION("from string")
    {
        auto d = np::descr_t::from_string("'>i4'");

        REQUIRE(d.size() == 1);
        REQUIRE(d.begin()->first == "f0");
        REQUIRE(d.begin()->second == np::type_t::from_string(">i4"));
        REQUIRE(d.stride() == 4);

        REQUIRE(d.to_string() == "'>i4'");

        d = np::descr_t::from_string("('name', '>i4',)");

        REQUIRE(d.size() == 1);
        REQUIRE(d.begin()->first == "name");
        REQUIRE(d.begin()->second == np::type_t::from_string("'>i4'"));
        REQUIRE(d.stride() == 4);

        REQUIRE(d.to_string() == "('name','>i4')");

        REQUIRE_THROWS(np::descr_t::from_string("('name', '>i4', (2,3,))"));

        d = np::descr_t::from_string("[('index', '<i8'), ('timestamp', '<M8[ns]'), ('swh', '<f4'), ('mwd', '<f4'), ('mwp', '<f4'), ('dwi', '<f4'), ('wind', '<f4'), ('pp1d', '<f4')]");

        REQUIRE(d.size() == 8);
        REQUIRE(d.stride() == 40);
        REQUIRE(d["index"].offset()     == 0);
        REQUIRE(d["timestamp"].offset() == 8);
        REQUIRE(d["swh"].offset()       == 16);
        REQUIRE(d["mwd"].offset()       == 20);
        REQUIRE(d["mwp"].offset()       == 24);
        REQUIRE(d["dwi"].offset()       == 28);
        REQUIRE(d["wind"].offset()      == 32);
        REQUIRE(d["pp1d"].offset()      == 36);

        d.push_back<double>("TEST");

        REQUIRE(d.size() == 9);
        REQUIRE(d.stride() == 48);
        REQUIRE(d["TEST"].offset() == 40);

        REQUIRE(d.to_string() == "[('index','<i8'),('timestamp','<M8[ns]'),('swh','<f4'),('mwd','<f4'),('mwp','<f4'),('dwi','<f4'),('wind','<f4'),('pp1d','<f4'),('TEST','<f8'),]");
    }

    SECTION("from type")
    {
        auto d = np::descr_t::make(
                    np::field_t::make<std::int64_t>("index"),
                    np::field_t("timestamp", np::type_t::from_string("<M8[ns]")),
                    np::field_t::make<float>       ("swh"),
                    np::field_t::make<float>       ("mwd"),
                    np::field_t::make<float>       ("mwp"),
                    np::field_t::make<float>       ("dwi"),
                    np::field_t::make<float>       ("wind"),
                    np::field_t::make<float>       ("pp1d")
                    );

        REQUIRE(d.size() == 8);
        REQUIRE(d.stride() == 40);
        REQUIRE(d["index"].offset()     == 0);
        REQUIRE(d["timestamp"].offset() == 8);
        REQUIRE(d["swh"].offset()       == 16);
        REQUIRE(d["mwd"].offset()       == 20);
        REQUIRE(d["mwp"].offset()       == 24);
        REQUIRE(d["dwi"].offset()       == 28);
        REQUIRE(d["wind"].offset()      == 32);
        REQUIRE(d["pp1d"].offset()      == 36);

        REQUIRE(d.to_string() == "[('index','<i8'),('timestamp','<M8[ns]'),('swh','<f4'),('mwd','<f4'),('mwp','<f4'),('dwi','<f4'),('wind','<f4'),('pp1d','<f4'),]");
    }
}
