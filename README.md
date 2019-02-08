# NumPyCpp
 A simple library to read and save Numpy npy and npz files.

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
NumPy::Array a("./your/file.npy");

// Get a value by its index
float v1 = a.at<float>(0);

// Get a value by its coordinates
float v2 = a.get<float>({1, 2, 3});

// Get all values in a vector
auto vec = a.vec<float>();

//Modify a value
a.at<float>(0) = 123;
```

Create an array
```cpp
// Make an empty 3D int matrix of size 3
NumPy::Array a = NumPy::Array::make<int>({3, 3, 3});

//Modify it
a.get<int>({1, 2, 3}) = 123;

//save it
a.save("./save/file.npy");
```

Npz files
```cpp
NumPy::Npz z;

//load
z.load("./your/file.npz");

//get all arrays names
auto names = z.files();

//Use references to modify the arrays
NumPy::Array& a = z.get(names[0]);

a.at<float>(0) = 1.23;

//Create a new array
NumPy::Array b = NumPy::Array::make<int>({100});

for(size_t i = 0; i < b.size(); i++)
	b.at<int>(i) = i;

// Add the new array to the existing
z.add("100ints", b);

//Save it
z.save("./your/new_file.npz");

```