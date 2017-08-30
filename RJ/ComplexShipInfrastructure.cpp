#include <string>
#include "Utility.h"

#include "ComplexShipInfrastructure.h"

ComplexShipInfrastructure::ComplexShipInfrastructure(void)
{
}

ComplexShipInfrastructure::~ComplexShipInfrastructure(void)
{
}


// Static method to translate from infrastructure class to the string representation of that classes
std::string ComplexShipInfrastructure::TranslateTypeToString(ComplexShipInfrastructure::InfrastructureClass type)
{
	switch (type)
	{
		case ComplexShipInfrastructure::InfrastructureClass::Power:			
			return "power";		break;
		case ComplexShipInfrastructure::InfrastructureClass::Ammo:
			return "ammo";		break;
		case ComplexShipInfrastructure::InfrastructureClass::Coolant:
			return "coolant";	break;

		default:
			return "";			break;

	}
}

// Static method to translate from the string representation of an infrastructure class to the class itself
ComplexShipInfrastructure::InfrastructureClass ComplexShipInfrastructure::TranslateStringToType(std::string type)
{
	// Convert to lower case for comparison
	std::string val = type;
	StrLowerC(type);

	// Test against each infrastructure type
	if (val == "power")
		return ComplexShipInfrastructure::InfrastructureClass::Power;
	else if (val == "ammo")
		return ComplexShipInfrastructure::InfrastructureClass::Ammo;
	else if (val == "coolant")
		return ComplexShipInfrastructure::InfrastructureClass::Coolant;

	else
		return ComplexShipInfrastructure::InfrastructureClass::Unknown;
}