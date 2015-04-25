#pragma once

#ifndef __ShipTileRequirementH__
#define __ShipTileRequirementH__

class ComplexShipTileClass;
class ComplexShipTileDefinition;

class ShipTileRequirement
{
public:

	// Requirement can be for a tile class and/or specific tile definition.  We can also specify how many are required
	ComplexShipTileClass *							Class;			// If not set, will be determined from definition anyway
	ComplexShipTileDefinition *						Definition;		// Can be set to NULL to avoid this being a criteria (then we just look for tiles of the specified class)
	int												Level;			// If not set, will default to zero (allowing any level of tile)
	int												Count;			// Minimum number of this tile class/def required (or 0 will disable this requirement)


	// Default constructor; will create a null requirement if no parameters are provided
	ShipTileRequirement(void)
	{
		Class = NULL;
		Definition = NULL;
		Level = 0;
		Count = 0;
	}

	// Main constructor; called with any supplied parameters, and derives any that should be defaulted when not specified
	ShipTileRequirement(ComplexShipTileClass *_Class, ComplexShipTileDefinition *_Def, int _Level, int _Count);

	// Default destructor
	~ShipTileRequirement(void) { }
};


#endif


