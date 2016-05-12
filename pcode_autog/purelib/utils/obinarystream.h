#ifndef _OBINARYSTREAM_H_
#define _OBINARYSTREAM_H_
#include <string>
#include <sstream>
#include <vector>
#include "endian_portable.h"
class obinarystream
{
public:
	obinarystream(size_t buffersize = 256);
	obinarystream(const obinarystream& right);
	obinarystream(obinarystream&& right);
	~obinarystream();

	obinarystream& operator=(const obinarystream& right);
	obinarystream& operator=(obinarystream&& right);

	std::vector<char> move();

	template<typename _Nty>
	size_t write_i(const _Nty value);

	void write_v(const std::string&);

	void write_v(const void* v, int vl);

	void write_array(const void* v, int vl);

	/*void write_binarybuf(const obinarystream&);
	obinarystream read_binarybuf(void);*/
	void write_length();

	template<typename _Nty>
	void modify_i(std::streamoff offset, const _Nty value);

public:
	void save(const char* filename);

protected:
	std::vector<char>    buffer_;
};

template <typename _Nty>
inline size_t obinarystream::write_i(const _Nty value)
{
	size_t offset = buffer_.size();
	auto nv = purelib::endian::htonv(value);
	buffer_.insert(buffer_.end(), (const char*)&nv, (const char*)&nv + sizeof(nv));
	return offset;
}

template <typename _Nty>
inline void obinarystream::modify_i(std::streamoff offset, const _Nty value)
{
	auto pvalue = (_Nty*)(buffer_.data() + offset);
	*pvalue = purelib::endian::htonv(value);
}

#endif
