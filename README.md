# NumPyCpp
 A simple library to read and save Numpy npy and npz files.
 
 ![C/C++ CI](https://github.com/NicoG60/NumPyCpp/workflows/C/C++%20CI/badge.svg?branch=master)

## Building
```console
git submodule init && git submodule update
mkdir build && cd build
cmake .. && make && make test
make install
```

## Usage

Read a simple array
```cpp
np::array a("./your/file.npy");

// Get a value by its index
float v1 = a.at(0).value<float>();

// Get a value by its coordinates
float v2 = a.at(1, 2, 3).value<float>();

//Modify a value
a.at(0).value<float>() = 123;
```

Read a structured array
```cpp
np::array a("./your/structured.npy");

// Get a value by its index
float v1 = a.at(0).value<float>("my_field_name");

// Get a value by its coordinates
float v2 = a.at(1, 2, 3).value<float>("my_field_name");

//Modify a value
a.at(0).value<float>("my_field_name") = 123;
```

Create an array
```cpp
// Make an empty 3D int array of size 3
np::array a = np::array::make<int>({3, 3, 3});

//Modify it
a.at(1, 2, 3).value<int>() = 123;

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
