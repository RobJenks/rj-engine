#pragma once

#ifndef __ComplexShipInfrastructureH__
#define __ComplexShipInfrastructureH__

#include <string>
using namespace std;

class ComplexShipInfrastructure
{
public:
	enum InfrastructureClass
	{
		Unknown = 0,
		Power,
		Coolant,
		Ammo
	};

	ComplexShipInfrastructure(void);
	~ComplexShipInfrastructure(void);

	// Static methods to translate between infrastructure classes and the string representation of those classes
	static string						TranslateTypeToString(InfrastructureClass type);
	static InfrastructureClass			TranslateStringToType(string type);
};



#endif