#include "Utility.h"
#include "GameDataExtern.h"
#include "ComplexShipTile.h"
#include "ComplexShipTileClass.h"


ComplexShipTileClass::ComplexShipTileClass(void)
{
	// Initialise with default values
	m_code = "";
	m_classtype = D::TileClass::Unknown;
	m_primarytile = false;
}


// Set the code of this class, and attempt to derive the class type at the same time
Result ComplexShipTileClass::SetCode(string code)
{
	// Set the string code of this class
	m_code = code;

	// Attempt to derive the class type
	m_classtype = TranslateStringToType(code);

	// Return success if we were able to derive a type other than 'unknown'
	if (m_classtype == D::TileClass::Unknown)		return ErrorCodes::ErrorSettingUnknownTileClassType;
	else											return ErrorCodes::NoError;
}

// Validate a tile against all hard-stop class criteria
bool ComplexShipTileClass::ValidateHardStopRequirements(ComplexShipTile *tile)
{
	// Check each hard-stop criteria in turn.  Use the && operator for each condition to only evaluate as far as necessary
	return (
				ValidateTileSize(tile)
				/* && ValidateXYZ(tile) && ... */
	);

}

// Validate a tile against all class criteria
bool ComplexShipTileClass::ValidateAllRequirements(ComplexShipTile *tile)
{
	// Check all hard-stop criteria, plus any non-critical conditions as well
	return (
				ValidateHardStopRequirements(tile)
				/* && ValidateXYZ(tile) && ... */
	);
}


// Validates a tile size against the requirements in this class definition 
bool ComplexShipTileClass::ValidateTileSize(ComplexShipTile *tile)
{
	// Parameter check; make sure this is a valid tile
	if (!tile) return false;
	INTVECTOR3 size = tile->GetElementSize();

	// Test the size requirements.  We also consider the alternative 'pass' where x/y have been defined as interchangable
	if  ( (	(size.x > 0 && size.x >= SizeRequirements.MinimumSize.x) &&								// Minimum x size
			(size.y > 0 && size.y >= SizeRequirements.MinimumSize.y) &&								// Minimum y size
			(size.z > 0 && size.z >= SizeRequirements.MinimumSize.z) &&								// Minimum z size

			(SizeRequirements.MaximumSize.x <= 0 || size.x <= SizeRequirements.MaximumSize.x) &&	// Maximum x size
			(SizeRequirements.MaximumSize.y <= 0 || size.y <= SizeRequirements.MaximumSize.y) &&	// Maximum x size
			(SizeRequirements.MaximumSize.z <= 0 || size.z <= SizeRequirements.MaximumSize.z) 		// Maximum x size
		)

		|| ( SizeRequirements.InterchangeableXY &&													// Alternative acceptable option, if x can subst for y

			(size.y > 0 && size.y >= SizeRequirements.MinimumSize.x) &&								// Minimum x size (against y)
			(size.x > 0 && size.x >= SizeRequirements.MinimumSize.y) &&								// Minimum y size (against x)
			(size.z > 0 && size.z >= SizeRequirements.MinimumSize.z) &&								// Minimum z size

			(SizeRequirements.MaximumSize.x <= 0 || size.y <= SizeRequirements.MaximumSize.x) &&	// Maximum x size (against y)
			(SizeRequirements.MaximumSize.y <= 0 || size.x <= SizeRequirements.MaximumSize.y) &&	// Maximum y size (against x)
			(SizeRequirements.MaximumSize.z <= 0 || size.z <= SizeRequirements.MaximumSize.z) 		// Maximum z size
		)
	)
	return true;	// Criteria met, so return true

	// We did not meet the size criteria above so return false
	return false;
}

ComplexShipTileClass::~ComplexShipTileClass(void)
{
}


// Translates a tile class to its string representation
string ComplexShipTileClass::TranslateTypeToString(D::TileClass type)
{
	switch (type) 
	{
		case D::TileClass::Corridor:			return "corridor";				
		case D::TileClass::Quarters:			return "quarters";
		case D::TileClass::LifeSupport:			return "lifesupport";
		case D::TileClass::PowerGenerator:		return "powergenerator";
		case D::TileClass::EngineRoom:			return "engineroom";

		default:								return "";							
	}
}

// Translates the string representation of a tile class to the class itself
D::TileClass ComplexShipTileClass::TranslateStringToType(string type)
{
	// Comparisons are case-insensitive
	string val = type;
	StrLowerC(val);

	// Compare against each string representation in turn
	// TODO: This and all similar methods should use either hash comparison or lookup tables (for enum>string) in future
	if (val == "corridor")						return D::TileClass::Corridor;
	else if (val == "quarters")					return D::TileClass::Quarters;
	else if (val == "lifesupport")				return D::TileClass::LifeSupport;
	else if (val == "powergenerator")			return D::TileClass::PowerGenerator;
	else if (val == "engineroom")				return D::TileClass::EngineRoom;

	else										return D::TileClass::Unknown;
}







