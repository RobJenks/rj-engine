#pragma once

#ifndef __ComplexShipObjectClassH__
#define __ComplexShipObjectClassH__

#include <string>

class ComplexShipObjectClass
{
public:
	ComplexShipObjectClass(void);
	~ComplexShipObjectClass(void);

	std::string 									GetCode(void) const							{ return m_code; }
	void											SetCode(const std::string & code)			{ m_code = code; }



protected:

	std::string										m_code;

};



#endif