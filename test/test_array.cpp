#include <catch2/catch.hpp>
#include <numpycpp.h>

TEST_CASE("array unit test", "[array]")
{
    SECTION("constructor")
    {
        np::array a;

        REQUIRE(a.empty());
        REQUIRE(a.dimensions() == 0);
        REQUIRE(a.size() == 0);

        np::array b(np::descr_t::make<int>(), {3, 3, 3}, false);

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

        np::array b = np::array(np::descr_t::make<int>(), {3, 3, 3});

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

        np::array b = np::array(np::descr_t::make<int>(), {3, 3, 3});

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
        REQUIRE(a.descr().stride() == 0);
        REQUIRE(a.descr().empty());
        REQUIRE_THROWS(a.type());
        REQUIRE_THROWS(a.type("test"));
        REQUIRE(a.fortran_order() == false);
        REQUIRE(a.data() == nullptr);

        a = np::array(np::descr_t::make<double>(), {3, 4, 5});

        REQUIRE(a.size() == 60);
        REQUIRE(a.dimensions() == 3);
        REQUIRE_NOTHROW(a.size(0));
        REQUIRE(a.size(0) == 3);
        REQUIRE(a.size(1) == 4);
        REQUIRE(a.size(2) == 5);
        REQUIRE(a.shape().size() == 3);
        REQUIRE(a.descr().stride() == 8);
        REQUIRE(a.descr().size() == 1);
        REQUIRE_NOTHROW(a.type());
        REQUIRE_THROWS(a.type("test"));
        REQUIRE_NOTHROW(a.type("f0"));
        REQUIRE(a.type() == a.type("f0"));
        REQUIRE(a.type().index() == typeid (double));
        REQUIRE(a.type().ptype() == '\0');
        REQUIRE(a.type().size() == sizeof (double));
        REQUIRE(a.type().offset() == 0);
        REQUIRE(a.type().endianness() == np::NativeEndian);
        REQUIRE(a.fortran_order() == false);
        REQUIRE(a.data() != nullptr);
        REQUIRE(a.data_as<double>() != nullptr);
    }

    SECTION("iterators + conversion")
    {
        np::array a = np::array(np::descr_t::make<int>(), {3, 3, 3});
        np::array c = np::array(np::descr_t::make<int>(), {3, 3, 3}, true);

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

    SECTION("Data access")
    {
        np::array a = np::array(np::descr_t::make<int>(), {3, 3, 3});

        for(int i = 0; i < 27; i++)
            a.at_index(i).value<int>() = i;

        for(int i = 0; i < 27; i++)
            REQUIRE(a[i].value<int>() == i);

        int i = 0;
        for(auto& v : a)
            REQUIRE(v.value<int>() == i++);

        i = 0;

        for(std::size_t x = 0; x < 3; x++)
        {
            for(std::size_t y = 0; y < 3; y++)
            {
                for(std::size_t z = 0; z < 3; z++)
                {
                    REQUIRE(a.at(x, y, z).value<int>() == i++);
                }
            }
        }
    }

    SECTION("Write array")
    {
        // Following the same kinda process as in gen_test_files.py
        std::size_t years = 20;
        std::size_t in_hours = years*365*24;
        std::size_t nb_points = in_hours / 3;

        auto d = np::descr_t::make(
                    np::field_t::make<std::int64_t>("timestamp"),
                    np::field_t::make<double>      ("wave_h"),
                    np::field_t::make<double>      ("wave_p"),
                    np::field_t::make<double>      ("wave_dir"),
                    np::field_t::make<double>      ("wind_sp"),
                    np::field_t::make<double>      ("wind_dir")
                    );

        np::array a(d, {nb_points});

        for(std::size_t i = 0; i < nb_points; i++)
        {
            auto it = a[i];
            it.value<std::int64_t>("timestamp") = i * 3 * 3600;
            it.value<double>("wave_h")          = std::rand();
            it.value<double>("wave_p")          = std::rand();
            it.value<double>("wave_dir")        = std::rand();
            it.value<double>("wind_sp")         = std::rand();
            it.value<double>("wind_dir")        = std::rand();
        }

        auto tmp = std::filesystem::temp_directory_path();
        auto dst = tmp / "test_save.npy";

        a.save(dst);

#ifdef USE_PYTHON3
        std::string cmd = "python3 open_file_test.py " + dst.string();
#else
        std::string cmd = "pipenv run python open_file_test.py " + dst.string();
#endif
        INFO(cmd)
        int c = std::system(cmd.c_str());

        REQUIRE(c == 0);
    }
}
