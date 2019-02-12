#include "numpycpp.h"
#include <regex>
#include <algorithm>
#include <zip_file.hpp>
#include <sstream>

#define IS_BIG_ENDIAN (*reinterpret_cast<const uint16_t *>("\0\xff") < 0x100)

namespace NumPy {

Array::Array(descr_t d, size_t ds, shape_t s, bool f) :
	_shape(s),
	_descr(d),
	_descr_size(ds),
	_fortran_order(f),
	_size(1)
{
	for(size_t _s : _shape)
		_size *= _s;

	size_t ts = _size * _descr_size;
	_data.reset(new base_t[ts], std::default_delete<base_t[]>());
	memset(_data.get(), 0, ts);
}

Array::Array(std::istream& st) :
	_descr(typeid (base_t)),
	_descr_size(sizeof (base_t)),
	_fortran_order(false),
	_size(1)
{
	load(st);
}

Array::Array() :
	_descr(typeid (base_t)),
	_descr_size(sizeof (base_t)),
	_fortran_order(false),
	_size(1)
{

}

Array::Array(const std::string& fn) :
	_descr(typeid (base_t)),
	_descr_size(sizeof (base_t)),
	_fortran_order(false),
	_size(1)
{
	load(fn);
}

Array::Array(const Array& c) :
	_descr(typeid (base_t)),
	_descr_size(sizeof (base_t)),
	_fortran_order(false),
	_size(1)
{
	*this = c;
}

Array& Array::operator =(const Array& c)
{
	_shape = c._shape;
	_descr = c._descr;
	_descr_size = c._descr_size;
	_fortran_order = c._fortran_order;
	_size = c._size;

	_data.reset(new base_t[_size * _descr_size], std::default_delete<base_t[]>());
	memcpy(_data.get(), c._data.get(), _size * _descr_size);

	return *this;
}

size_t Array::dimensions() const
{
	return _shape.size();
}

size_t Array::size() const
{

	return _size;
}

size_t Array::size(size_t d) const
{
	return _shape[d];
}

descr_t Array::descr() const
{
	return _descr;
}

size_t Array::descr_size() const
{
	return _descr_size;
}

shape_t Array::shape() const
{
	return _shape;
}

void Array::load(const std::string& fn)
{
	std::ifstream ifs(fn, std::ios_base::in | std::ios_base::binary);

	load(ifs);
}

void Array::load(std::istream& st)
{
	//Check the magic phrase
	char magic[7];
	memset(magic, 0, 7);
	st.read(magic, 6); // read 6 * 1 char

	if(strcmp(magic, "\x93NUMPY") != 0)
		throw std::runtime_error("not a numpy file");

	//Check version
	uint8_t maj, min;
	st.read(reinterpret_cast<char*>(&maj), 1); // read 1 * 1 uint8
	st.read(reinterpret_cast<char*>(&min), 1); // read 1 * 1 uint8

	//header_len field size
	size_t hls;

	if		(maj == 1 && min == 0)	hls = 2; // 2 bytes for v1.0
	else if	(maj == 2 && min == 0)	hls = 4; // 4 bytes for v2.0
	else
		throw std::runtime_error(std::to_string(maj) + "." + std::to_string(min) + ": sorry, I can't read that version...");

	//read header len, little endian uint16
	size_t header_len = 0;

	if(IS_BIG_ENDIAN)
	{
		size_t w = 1;
		for(size_t i = 0; i < hls; i++) //read it one byte by one byte to avoid endianness problem
		{
			uint8_t t;
			st.read(reinterpret_cast<char*>(&t), 1);
			header_len += t * w;
			w *= 256;
		}
	}
	else
		st.read(reinterpret_cast<char*>(&header_len), static_cast<std::streamsize>(hls));

	//Read the header
	std::string header(header_len, 0);
	st.read(&header[0], static_cast<std::streamsize>(header_len));
	char o = parse_header(header);

	//Read raw data
	_data.reset(new base_t[_size * _descr_size], std::default_delete<base_t[]>());
	st.read(reinterpret_cast<char*>(_data.get()), static_cast<std::streamsize>(_size * _descr_size));

	//re-order bytes for the system
	bool swap = false;

	if(IS_BIG_ENDIAN)
		swap = o == '<';
	else
		swap = o == '>';

	if(swap)
	{
		base_t* p = _data.get();
		for(size_t i = 0; i < _size; i++)
		{
			std::reverse(p, p + _descr_size);
			p += _descr_size;
		}
	}
}

char Array::parse_header(std::string h)
{
	h.erase(std::remove_if(h.begin(), h.end(), isspace), h.end());

	size_t fop = h.find("'fortran_order':") + 16;
	_fortran_order = h.substr(fop, 4) == "True";

	size_t dp = h.find("'descr':") + 8;
	std::regex descr_check("'[<>|][fiub][1248]'");

	std::string descr = h.substr(dp, 5);

	if(!std::regex_match(descr.begin(), descr.end(), descr_check))
		throw std::runtime_error(descr + ": Sorry, I don't cover that format :(");

	char e = descr[1];

	parse_type(descr);

	size_t sp = h.find("'shape':") + 8;
	std::regex shape_check("\\((\\d+,?)+\\),?");

	std::string shape;

	if(sp > dp && sp > fop)
	{
		shape = h.substr(sp);
		shape.erase(shape.size()-1);
	}
	else
	{
		size_t min = std::min(dp, fop);
		size_t max = std::max(dp, fop);

		if(sp < min)
			shape = h.substr(sp, min - sp);
		else
			shape = h.substr(sp, max - sp);
	}

	if(!std::regex_match(shape.begin(), shape.end(), shape_check))
		throw std::runtime_error("shape tuple not well formed");

	parse_shape(shape);

	if(_fortran_order)
		std::reverse(_shape.begin(), _shape.end());

	return e;
}

void Array::parse_type(std::string d)
{
	_descr_size = static_cast<size_t>(d[3] - 0x30);

	char t = d[2];

	if(t == 'b')
	{
		if(d[1] != '|' || _descr_size != 1)
			throw std::runtime_error(d + ": boolean can't have endianness");
		_descr = typeid (bool);
	}
	else if(t == 'i')
	{
			 if(_descr_size == sizeof (int8_t))		_descr = typeid (int8_t);
		else if(_descr_size == sizeof (int16_t))	_descr = typeid (int16_t);
		else if(_descr_size == sizeof (int32_t))	_descr = typeid (int32_t);
		else if(_descr_size == sizeof (int64_t))	_descr = typeid (int64_t);
		else
			throw std::runtime_error(d + ": unhandle integer type");
	}
	else if(t == 'u')
	{
			 if(_descr_size == sizeof (uint8_t))	_descr = typeid (uint8_t);
		else if(_descr_size == sizeof (uint16_t))	_descr = typeid (uint16_t);
		else if(_descr_size == sizeof (uint32_t))	_descr = typeid (uint32_t);
		else if(_descr_size == sizeof (uint64_t))	_descr = typeid (uint64_t);
		else
			throw std::runtime_error(d + ": unhandle unsigned type");
	}
	else if(t == 'f')
	{
			 if(_descr_size == sizeof (float))	_descr = typeid (float);
		else if(_descr_size == sizeof (double))	_descr = typeid (double);
		else
			throw std::runtime_error(d + ": unhandle float type");
	}
	else
		throw std::runtime_error(d + ": wrong type");
}

void Array::parse_shape(std::string s)
{
	std::regex r("\\d+");

	std::sregex_iterator b = std::sregex_iterator(s.begin(), s.end(), r);
	std::sregex_iterator e = std::sregex_iterator();

	_shape.clear();
	_size = 1;

	for(auto it = b; it != e; it++)
	{
		std::string str = it->str();
		size_t c = std::stoul(str);
		_shape.push_back(c);
		_size *= c;
	}
}

void Array::save(const std::string& fn)
{
	std::ofstream ofs(fn, std::ios_base::out | std::ios_base::binary);
	save(ofs);
}

void Array::save(std::ostream& st)
{
	//magic
	st.write("\x93NUMPY", 6);

	std::string h = build_header();

	//Version depends on header size
	uint8_t v = 0;
	if(h.size() < 65526)
		v = 1;
	else
		v = 2;

	st.put(static_cast<char>(v));
	st.put('\0');

	size_t m = 64 - ((6+2+2*v+h.size())%64);
	for(size_t i = 0; i < m; i++)
		h += ' ';

	size_t s = h.size();

	//little endian uint16 / uint32 representing header size;
	char* c = reinterpret_cast<char*>(&s);
	size_t ts = sizeof (size_t);

	if(v == 1)
	{
		if(IS_BIG_ENDIAN)
		{
			st.write(c+ts-1, 1);
			st.write(c+ts-2, 1);
		}
		else
			st.write(c, 2);
	}
	else
	{
		if(IS_BIG_ENDIAN)
		{
			st.write(c+ts-1, 1);
			st.write(c+ts-2, 1);
			st.write(c+ts-3, 1);
			st.write(c+ts-4, 1);
		}
		else
			st.write(c, 4);
	}

	st.write(h.c_str(), static_cast<std::streamsize>(h.size()));

	st.write(reinterpret_cast<char*>(_data.get()), static_cast<std::streamsize>(_size * _descr_size));
}

std::string Array::build_header()
{
	std::string h;

	h += "{'descr': '";

	if(_descr_size == 1)
		h += '|';
	else
		h += IS_BIG_ENDIAN ? '>' : '<';

	h += get_type();

	h += std::to_string(_descr_size);

	h += "', 'fortran_order': ";

	if(_fortran_order)
		h += "True";
	else
		h += "False";

	h += ", 'shape': (";

	for(size_t s : _shape)
		h += std::to_string(s) + ", ";

	h += "), }\n";

	return h;
}

char Array::get_type()
{
	if(_descr == typeid (bool))
		return 'b';

	if(_descr == typeid (int8_t) ||
		_descr == typeid (int16_t) ||
		_descr == typeid (int32_t) ||
		_descr == typeid (int64_t))
		return 'i';

	if(_descr == typeid (uint8_t) ||
		_descr == typeid (uint16_t) ||
		_descr == typeid (uint32_t) ||
		_descr == typeid (uint64_t))
		return 'u';

	if(_descr == typeid (float) ||
		_descr == typeid (double))
		return 'f';

	throw std::runtime_error("unable to write the type stored in the array");
}

Npz::Npz(const std::string& fn)
{
	load(fn);
}

void Npz::load(const std::string& fn)
{
	miniz_cpp::zip_file f;

	f.load(fn);

	for(auto npy_n : f.namelist())
	{
		std::string content = f.read(npy_n);
		std::istringstream iss (content);
		npy_n.erase(npy_n.size()-4);
		try {
			_arrays[npy_n] = Array(iss);
		} catch(std::runtime_error& e) {
			throw std::runtime_error(npy_n + " - " + e.what());
		}
	}
}

void Npz::save(const std::string& fn)
{
	miniz_cpp::zip_file f;

	for(auto e : _arrays)
	{
		std::stringstream st;
		try {
			e.second.save(st);
			f.writestr(e.first+".npy", st.str());
		} catch(std::runtime_error& ex) {
			throw std::runtime_error(e.first + " - " + ex.what());
		}
	}

	f.save(fn);
}

void Npz::add(const std::string& fn, const Array& a)
{
	_arrays[fn] = a;
}

Array& Npz::get(const std::string& fn)
{
	if(!contains(fn))
		throw std::out_of_range(fn + ": array name not found");

	return _arrays[fn];
}

void Npz::remove(const std::string& fn)
{
	_arrays.erase(fn);
}

bool Npz::canBeMapped()
{
	Array& f = _arrays.begin()->second;
	return std::all_of(++_arrays.begin(), _arrays.end(), [&](const std::pair<std::string, Array>& e) {
		return e.second._shape == f._shape;
	});
}

size_t Npz::size()
{
	return _arrays.size();
}

bool Npz::contains(const std::string& s)
{
	return _arrays.find(s) != _arrays.end();
}

std::vector<std::string> Npz::files()
{
	std::vector<std::string> r;
	for(auto e : _arrays)
		r.push_back(e.first);

	return r;
}

}
