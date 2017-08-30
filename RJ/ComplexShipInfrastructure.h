#pragma once

#ifndef __ComplexShipInfrastructureH__
#define __ComplexShipInfrastructureH__

#include <string>

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
	static std::string					TranslateTypeToString(InfrastructureClass type);
	static InfrastructureClass			TranslateStringToType(std::string type);
};



#endif