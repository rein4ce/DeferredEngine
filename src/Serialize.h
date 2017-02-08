#pragma once
#include <fstream>

//////////////////////////////////////////////////////////////////////////
// Serialization
//////////////////////////////////////////////////////////////////////////
class CSerialize
{
public:
	template <typename T>
	static inline void Write(ofstream &fs, T *data);

	template <typename T>
	static inline void Read(ifstream &fs, T *data);
};

template <typename T>
inline void CSerialize::Write(std::ofstream &fs, T *data)
{
	fs.write((char*)data, sizeof(T));
}

template <>
inline void CSerialize::Write(std::ofstream &fs, string *data)
{
	uint16 len = data->size();
	fs.write((char*)&len, sizeof(len));
	fs.write(data->c_str(), data->size()+1);
}

template <typename T>
inline void CSerialize::Read(std::ifstream &fs, T *data)
{
	fs.read((char*)data, sizeof(T));
}

template <>
inline void CSerialize::Read(std::ifstream &fs, string *data)
{
	uint16 len = 0;
	fs.read((char*)&len, sizeof(len));
	char *str = new char[len+1];	
	fs.read((char*)str, len+1);
	data->assign(str); 
	delete [] str;
}