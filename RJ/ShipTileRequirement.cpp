#include "ComplexShipTileDefinition.h"
class ComplexShipTileClass; 

#include "ShipTileRequirement.h"

// Main constructor; called with any supplied parameters, and derives any that should be defaulted when not specified
ShipTileRequirement::ShipTileRequirement(ComplexShipTileClass *_Class, ComplexShipTileDefinition *_Def, int _Level, int _Count)
{
	// If we have not specified a tile class or definition then this is not a valid requirement
	if (!_Class && !_Def) { Class = NULL; Definition = NULL; Level = 0; Count = 0; return; }

	// Set properties based on the supplied parameters
	Class = _Class;
	Definition = _Def;
	Level = _Level;
	Count = _Count;

	// If we have a definition specified we should derive the class and tile level from it (regardless of whether 
	// these fields were also specified - we want to keep the requirement consistent within itself)
	if (_Def)
	{
		Class = _Def->GetClassObject();
		Level = _Def->GetTileLevel();
	}
}
