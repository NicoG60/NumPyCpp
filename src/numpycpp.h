/**
Copyright 2018 Nicolas Jarnoux

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE
*/
#ifndef NUMPYCPP_H
#define NUMPYCPP_H

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <typeinfo>
#include <typeindex>

/**
 * NumPyCpp : A simple interface to .npy and .npz file
 */

namespace NumPy {

typedef uint8_t						base_t;
typedef std::vector<size_t>			shape_t;
typedef std::shared_ptr<base_t>		data_t;
typedef std::type_index				descr_t;

/*
The Array class represent one npy file (one n-dimensionnal array)
The array has a fixed size and can't be re-allocated.
*/
class Array
{
	friend class Npz;

protected:
	Array(descr_t d, size_t ds, shape_t s, bool f);
	Array(std::istream& st);

public:
	Array();						//Contruct an empty array
	Array(const std::string& fn);	//Construct & Load an array from a file
	Array(const Array& c);			//Copy an existing array

	Array& operator =(const Array& c); //Copy an existing array

	// Returns a value of type T at index i (all dimensions in line)
	// if sizeo T == 4 bytes, so the value returned will start from 4 * i
	// the 4 bytes after that position are returned as a T
	// T must have the same typeid than the one stored internally
	template<class T>
	T& at(size_t i)
	{
		if(i > _size)
			throw std::out_of_range("Array: index out of range");

		if(_descr != std::type_index(typeid (T)))
			throw  std::bad_cast();

		return reinterpret_cast<T*>(_data.get())[i];
	}

	// Same functionnality but different way.
	// Take an initializer_list {x, y, z, ...} of corrdinate in the n-dimensionnal matrix
	template<class T>
	T& get(std::initializer_list<size_t> l)
	{
		if(l.size() != _shape.size())
			throw  std::out_of_range("Array: get with list of wrong size");

		if(_descr != std::type_index(typeid (T)))
			throw  std::bad_cast();

		size_t s = 0;
		size_t i = 0;
		for(auto it = l.begin(); it != l.end(); it++)
		{
			if(*it >= _shape[i])
				throw std::out_of_range("Array: get index out of rande");

			if(i == 0)
				s += *it;
			else
				s += _shape[i-1]*(*it);

			i++;
		}

		return reinterpret_cast<T*>(_data.get())[i];
	}

	// Make an array of dimension {x, y, z, ...}
	template<class T>
	static Array make(std::initializer_list<size_t> l)
	{
		return Array(typeid (T), sizeof (T), l, false);
	}

	size_t dimensions() const;		//number of dimensions
	size_t size() const;			//number of stored items
	size_t size(size_t d) const;	//number of item in the dimension n
	descr_t descr() const;			//typeid of the stored items
	size_t descr_size() const;		//sizeof the stored items
	shape_t shape() const;			//shape of the array, size of each dimensions

	void load(const std::string& fn);	//load from a file (erase all existing datas int the obejct)
	void save(const std::string& fn);	// save to a file

	// Return a the internal buffer
	template<class T>
	T* data()
	{
		if(_descr != std::type_index(typeid (T)))
			throw  std::bad_cast();

		return reinterpret_cast<T*>(_data.get());
	}

	// Return a pointer to the first element
	template<class T>
	T* begin()
	{
		return data<T>();
	}

	// Return a pointer after the last element
	template<class T>
	T* end()
	{
		return data<T>()  + _size;
	}

	// Return the buffer as a vector
	template<class T>
	std::vector<T> vec()
	{
		return std::vector<T>(begin<T>(), end<T>());
	}

protected:
	void load(std::istream& st);
	char parse_header(std::string h);
	void parse_type(std::string d);
	void parse_shape(std::string s);

	void save(std::ostream& st);

	std::string build_header();
	char get_type();

	data_t	_data;
	shape_t	_shape;
	descr_t	_descr;
	size_t	_descr_size;
	bool	_fortran_order;
	size_t	_size;
};


/*
The Npz class represent one npz file (zip file with some npy file inside)
You can add/remove arrays to/from the existing
you can modify the arrays
*/
class Npz
{
public:
	Npz() {;}					//Contruct empty npz
	Npz(const std::string& fn); //Contruct and Load from a file

	void load(const std::string& fn); //Load from a file (erase all existing data)
	void save(const std::string& fn); //Save to a file

	void add(const std::string& fn, const Array& a); //Add an array
	Array& get(const std::string& fn);				 //Get a reference to the array
	void remove(const std::string& fn);				 //Remove an array

	bool canBeMapped();	//If all arrays have the same shape, return true.

	//Return 2 arrays in a map form, 1 array as key, the other as value
	template<class K, class V>
	std::map<K,V> map(const std::string& k, const std::string& v)
	{
		Array& keys = _arrays[k];
		Array& values = _arrays[v];

		std::map<K,V> r;
		for(size_t i = 0; i < keys.size(); i++)
			r[keys.at<K>(i)] = values.at<V>(i);

		return r;
	}

	size_t size();							//Returns the number of arrays
	bool contains(const std::string& s);	//Returns true if the array exists
	std::vector<std::string> files();		//Return a list a array names

protected:
	std::map<std::string, Array> _arrays;
};

}

#endif // NUMPYCPP_H
