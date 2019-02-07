#ifndef NUMPYCPP_H
#define NUMPYCPP_H

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <typeinfo>
#include <typeindex>

namespace NumPy {

typedef uint8_t						base_t;
typedef std::vector<size_t>			shape_t;
typedef std::shared_ptr<base_t>		data_t;
typedef std::type_index				descr_t;

extern bool is_big_endian();

class Array
{
	friend class Npz;

protected:
	Array(descr_t d, size_t ds, shape_t s, bool f);
	Array(std::istream& st);

public:
	Array();
	Array(const std::string& fn);
	Array(const Array& c);

	Array& operator =(const Array& c);

	template<class T>
	T& at(size_t i)
	{
		if(i > _size)
			throw std::out_of_range("Numpy Array out of range []");

		if(_descr != std::type_index(typeid (T)))
			throw  std::bad_cast();

		return reinterpret_cast<T*>(_data.get())[i];
	}

	template<class T>
	T& get(std::initializer_list<size_t> l)
	{
		if(l.size() != _shape.size())
			throw  std::out_of_range("Numpy Array out of rande {}");

		if(_descr != std::type_index(typeid (T)))
			throw  std::bad_cast();

		size_t s = 0;
		size_t i = 0;
		for(auto it = l.begin(); it != l.end(); it++)
		{
			if(*it >= _shape[i])
				throw std::out_of_range("umpy Array out of rande {}");

			if(i == 0)
				s += *it;
			else
				s += _shape[i-1]*(*it);

			i++;
		}

		return reinterpret_cast<T*>(_data.get())[i];
	}

	template<class T>
	static Array make(std::initializer_list<size_t> l)
	{
		return Array(typeid (T), sizeof (T), l, false);
	}

	size_t dimensions() const;
	size_t size() const;
	size_t size(size_t d) const;
	descr_t descr() const;
	size_t descr_size() const;
	shape_t shape() const;

	void save(const std::string& fn);

	template<class T>
	T* data()
	{
		if(_descr != std::type_index(typeid (T)))
			throw  std::bad_cast();

		return reinterpret_cast<T*>(_data.get());
	}

	template<class T>
	T* begin()
	{
		return data<T>();
	}

	template<class T>
	T* end()
	{
		return data<T>()  + _size;
	}

	template<class T>
	std::vector<T> vec()
	{
		return std::vector<T>(begin<T>(), end<T>());
	}

protected:
	void parse(const std::string& fn);
	void parse(std::istream& st);
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

class Npz
{
public:
	Npz() {;}
	Npz(const std::string& fn);

	void load(const std::string& fn);
	//void save(const std::string& fn);

	void add(const std::string& fn, const Array& a);
	Array& get(const std::string& fn);

	bool canBeMapped();

	template<class K, class V>
	std::map<K,V> map(const std::string& k, const std::string& v)
	{
		Array& keys = _arrays[k];
		Array& values = _arrays[v];

		std::map<K,V> r;
		for(size_t i = 0; i < keys.size(); i++)
			r[keys.get<K>(i)] = values.get<V>(i);

		return r;
	}

	size_t size();
	bool contains(const std::string& s);
	std::vector<std::string> files();

protected:
	std::map<std::string, Array> _arrays;
};

}

#endif // NUMPYCPP_H
