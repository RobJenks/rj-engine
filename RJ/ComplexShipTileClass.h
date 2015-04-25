#pragma once

#ifndef __ComplexShipTileClassH__
#define __ComplexShipTileClassH__

#include <string>
#include <vector>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Utility.h"
#include "FastMath.h"
#include "ComplexShipInfrastructure.h"
class ComplexShipTile;
class ComplexShipObjectClass;
using namespace std;

class ComplexShipTileClass
{
public:

	// Validates a tile against the set of all criteria, or the set of only hard-stop criteria
	bool													ValidateAllRequirements(ComplexShipTile *tile);
	bool													ValidateHardStopRequirements(ComplexShipTile *tile);

	// Validates a tile against the class definition to see if it complies with each requirement
	bool													ValidateTileSize(ComplexShipTile *tile);

	// Defines a requirement that the tile class has access to a specific class of infrastructure
	struct InfrastructureRequirement
	{
		ComplexShipInfrastructure::InfrastructureClass		Class;

		InfrastructureRequirement(void) { Class = ComplexShipInfrastructure::InfrastructureClass::Unknown; }
		InfrastructureRequirement(ComplexShipInfrastructure::InfrastructureClass cls) { Class = cls; }
	};
	typedef vector<InfrastructureRequirement>				InfrastructureRequirementCollection;

	// Defines a requirement that the tile of this class contain specific object(s)
	struct ObjectRequirement
	{
		ComplexShipObjectClass * 							Class;
		int													Minimum;
		int													Maximum;

		ObjectRequirement(void) { Class = NULL; Minimum = 0; Maximum = 0; }
		ObjectRequirement(ComplexShipObjectClass *obj, int mincount, int maxcount) { Class = obj; Minimum = mincount; Maximum = maxcount; } 
	};
	typedef vector<ObjectRequirement>						ObjectRequirementCollection;

	// Defines a size requirements for the tile class
	struct SizeRequirement
	{
		INTVECTOR3											MinimumSize;
		INTVECTOR3											MaximumSize;
		bool												InterchangeableXY;		// Determines whether e.g. a 3x2x1 tile will satisfy a 2x3x1 requirement

		SizeRequirement(void) { MinimumSize = NULL_INTVECTOR3; MaximumSize = NULL_INTVECTOR3; InterchangeableXY = false; }
	};
	typedef vector<SizeRequirement>							SizeRequirementCollection;
	
	// Basic properties of the class
	CMPINLINE string										GetCode(void)					{ return m_code; }
	CMPINLINE D::TileClass									GetClassType(void)				{ return m_classtype; }
	Result													SetCode(string code);
	CMPINLINE bool											IsPrimaryTile(void)				{ return m_primarytile; }
	CMPINLINE void											SetPrimaryTile(bool primary)	{ m_primarytile = primary; }

	// Collections of requirements associated with this class
	InfrastructureRequirementCollection						InfrastructureRequirements;
	ObjectRequirementCollection								ObjectRequirements;
	SizeRequirement											SizeRequirements;

	// Constructor / destructor
	ComplexShipTileClass(void);
	~ComplexShipTileClass(void);

	
	// Static methods to translate between tile classes and the string representation of those classes
	static string						TranslateTypeToString(D::TileClass type);
	static D::TileClass					TranslateStringToType(string type);

private:

	string													m_code;
	D::TileClass											m_classtype;
	bool													m_primarytile;

};


#endif