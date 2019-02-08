#include <catch2/catch.hpp>
#include "global.h"


TEMPLATE_TEST_CASE("Open npz file", "[npz]",
				   bool,
				   int8_t, uint8_t,
				   int16_t, uint16_t,
				   int32_t, uint32_t,
				   int64_t, uint64_t,
				   float, double)
{
	NumPy::Npz z(NPZ_GOOD);
	for(auto f : z.files())
	{
		NumPy::Array& a = z.get(f);

		REQUIRE(a.dimensions() == 1);
		REQUIRE(a.size() == 5);

		if(std::type_index(typeid (TestType))  == a.descr())
		{
			if(typeid (TestType) == typeid (bool))
			{
				bool b0 = a.at<bool>(0);
				bool b1 = a.at<bool>(1);
				bool b2 = a.at<bool>(2);
				bool b3 = a.at<bool>(3);
				bool b4 = a.at<bool>(4);

				REQUIRE(b0);
				REQUIRE_FALSE(b1);
				REQUIRE(b2);
				REQUIRE(b3);
				REQUIRE_FALSE(b4);
			}
			else
			{
				TestType t0 = a.at<TestType>(0);
				TestType t1 = a.at<TestType>(1);
				TestType t2 = a.at<TestType>(2);
				TestType t3 = a.at<TestType>(3);
				TestType t4 = a.at<TestType>(4);

				REQUIRE(t0 == static_cast<TestType>(-1));
				REQUIRE(t1 == static_cast<TestType>(0));
				REQUIRE(t2 == static_cast<TestType>(1));
				REQUIRE(t3 == static_cast<TestType>(2));
				REQUIRE(t4 == static_cast<TestType>(3));
			}
		}
		else
			REQUIRE_THROWS(a.at<TestType>(0));


		a = NumPy::Array::make<TestType>({3, 3, 3});
	}

	for(auto f : z.files())
	{
		NumPy::Array& a = z.get(f);

		REQUIRE(a.dimensions() == 3);
		REQUIRE(a.size(0) == 3);
		REQUIRE(a.size(1) == 3);
		REQUIRE(a.size(2) == 3);
		REQUIRE(a.size() == 27);

		TestType* d = a.data<TestType>();
		for(size_t i = 0; i < a.size(); i++)
			REQUIRE(d[i] == 0);

		REQUIRE_THROWS(a.get<TestType>({3, 3, 3}));
		REQUIRE_NOTHROW(a.get<TestType>({0, 1, 2}));
	}

	/*z.save(NPZ_GOOD
		   "_new");

	NumPy::Npz b(NPZ_GOOD
				 "_new");*/
}
