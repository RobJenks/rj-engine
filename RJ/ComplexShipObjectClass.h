#pragma once

#ifndef __ComplexShipObjectClassH__
#define __ComplexShipObjectClassH__

#include <string>
#include "CompilerSettings.h"

class ComplexShipObjectClass
{
public:
	ComplexShipObjectClass(void);
	~ComplexShipObjectClass(void);

	std::string 									GetCode(void) const							{ return m_code; }
	void											SetCode(const std::string & code)			{ m_code = code; }

	// Shutdown method - not required for this class
	CMPINLINE void Shutdown(void) { throw "Shutdown method not implemented for this class"; }


protected:

	std::string										m_code;

};



#endif