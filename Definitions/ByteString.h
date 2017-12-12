#pragma once

#include <vector>

class ByteString : public std::vector<char>
{
public:

	// Default constructor
	inline ByteString(void) 
		: 
		std::vector<char>(), 
		m_readpoint(0U)
	{
	}

	// Write a new object into the serialisation buffer
	template <typename T>
	void					WriteObject(const T & object);


	// Reset the object deserialisation pointer for a new read from the start of the buffer
	inline void				ResetRead(void) { m_readpoint = 0U; }


	// Read an object from the current read buffer pointer
	template <typename T>
	T						ReadObject(void);
	template <typename T>
	void					ReadObject(T & object);




private:

	size_t					m_readpoint;

};


template <typename T>
void ByteString::WriteObject(const T & object)
{
	constexpr size_t n = sizeof(T);
	reserve(capacity() + n);

	const value_type *buff = (const value_type*)&object;
	for (size_t i = 0U; i < n; ++i)
	{
		push_back(buff[i]);
	}
}

template <typename T>
T ByteString::ReadObject(void)
{
	constexpr size_t n = sizeof(T);

	T obj;
	memcpy((void*)&obj, (const void*)&(data()[m_readpoint]), n);
	m_readpoint += n;

	return obj;
}

template <typename T>
void ByteString::ReadObject(T & object)
{
	object = ReadObject<T>();
}


