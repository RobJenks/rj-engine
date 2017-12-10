#pragma once

#include <string>


class TransformerComponent
{
public:

#	define TRANSFORM_ERROR std::cerr << "Error [" << GetName() << "]: " 

	virtual std::string				GetName(void) const = 0;

private:


};