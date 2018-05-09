#pragma once

#include <iostream>
#include <assimp\LogStream.hpp>


class AssimpLogStream : public Assimp::LogStream
{
public:

	inline AssimpLogStream(void) { }
	inline ~AssimpLogStream(void) { }

	// Stream method to retrieve internal Assimp logging data
	inline void write(const char *message)
	{
		std::cout << "Transform [Assimp]: " << message << "\n";
	}

};