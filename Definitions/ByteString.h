#pragma once

#include <vector>
#include <string>

#ifdef RJ_MODULE_RJ
#	include "../RJ/Logging.h"
#endif


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

	// Constructor with pre-allocated space
	inline ByteString(size_type pre_allocated_size)
		:
		std::vector<char>(pre_allocated_size),
		m_readpoint(0U)
	{
	}

	// Write a new object into the serialisation buffer
	template <typename T>
	void					WriteObject(const T & object);

	// Write a string character-by-character
	inline void				WriteString(std::string data)
	{
		for (auto ch : data) WriteObject<std::string::value_type>(ch);
	}

	// Reset the object deserialisation pointer for a new read from the start of the buffer
	inline void				ResetRead(void) { m_readpoint = 0U; }


	// Read an object from the current read buffer pointer
	template <typename T>
	T						ReadObject(void);
	template <typename T>
	void					ReadObject(T & object);


	// Attempt to read a file identifier from the byte string and validate it against the given required value
	inline bool				ReadAndVerifyIdentifier(const std::string & expected)
	{
		auto length = expected.size();
		for (std::string::size_type i = 0U; i < length; ++i)
		{
			char id = ReadObject<char>();
			if (id != expected[i])
			{
#			ifdef LOGGING_AVAILABLE
#				ifdef _DEBUG
				Game::Log << LOG_ERROR << "File failed \"" << expected << "\" identifier validation; id[" << i << "] == " << id << " which is != " << expected[i] << "\n";
#				else
				Game::Log << LOG_ERROR << "File is not of expected type\n";
#				endif
#			endif

				return false;
			}
		}

		return true;
	}


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
	if ((m_readpoint + n) >= size()) return T();

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





