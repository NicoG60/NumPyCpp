# NumPyCpp
 A simple library to read and save Numpy npy and npz files.

 ![C/C++ CI](https://github.com/NicoG60/NumPyCpp/workflows/C/C++%20CI/badge.svg?branch=master)



## Building

You'll need a `c++17` compatible compiler as it depends on the `std::filesystem`  standard library.

The following commands should build:

- The unit tests
- The shared library
- The static library

```bash
git submodule init && git submodule update
mkdir build && cd build
cmake .. && make && make test
```



You can disabled some of it at configuration time:

```bash
cmake -DENABLE_STATIC=OFF -DENABLE_DYNAMIC=OFF -DENABLE_TESTING=OFF ..
```



## CMake integration

You can easily  integrate it to CMake as a subdirectory.

It creates an `INTERFACE` target you can link against. You dont even have to build the static or shared libraries.

```cmake
add_subdirectory(path/to/NumPyCpp)
target_link_libraries(${YOUR_TARGET} NumPyCpp) # just link the interface and you're done
```



## Usage

Don't forget to

```cpp
#include <numpycpp.h>
```



Read a simple array (Let's say it's an array of floats)

```cpp
np::array a = np::array::load("./your/file.npy");

// ==== Read values

// Get a value by its index
float v1 = a[0].value<float>();

// This is equivalent to
v1 = a.at_index(0).value<float>();

// Get a value by its coordinates
// for n-dimensionnal arrays
float v2 = a.at(1, 2, 3).value<float>();

// Note that although at() and at_index()
// are equivalent for 1-dim arrays,
// at_index() is more efficient.

// ==== Modify a value

a[0].value<float>() = 123.0;
a.at(1, 2, 3).value<float>() = 123.0;

// ==== Iterate
for(auto& it : a)
{
  it.value<float>() += 123.0;
}
```



Read a structured array

```cpp
np::array a = np::array::load("./your/structured.npy");

// ==== Read values

// Get a value by its index
float v1 = a[0].value<float>("my_data");

// This is equivalent to
v1 = a.at_index(0).value<float>("my_data");

// Get a value by its coordinates
// for n-dimensionnal arrays
float v2 = a.at(1, 2, 3).value<float>("my_data");

// Note that although at() and at_index()
// are equivalent for 1-dim arrays,
// at_index() is more efficient.

// ==== Modify a value

a[0].value<float>("my_data") = 123.0;
a.at(1, 2, 3).value<float>("my_data") = 123.0;

// ==== Iterate
for(auto& it : a)
{
  it.value<float>("my_data") += 123.0;
}
```



At the moment you can create simple arrays but not structure one. sorry.

```cpp
// Make an empty 3D int array of size 3
np::array a(np::descr_t::make<int>(), {3, 3, 3});

//Modify it
a.at(0, 1, 2).value<int>() = 123;

//save it
a.save("./save/file.npy");
```



Npz files are bound to `std::unordered_map<std::string, np::array>`

```cpp
auto z = np::npz_load("./your/file.npz");

for(auto& p : z)
{
   std::cout << "array name: " << p.first << std::endl;
   np::array& a = p.second;
   
   //Do stuff with the array
}

// Add a new array
z.emplace("new_name", np::array::make<int>({3, 3, 3}));

np::npz_save(z, "./new/name.npz");
```
