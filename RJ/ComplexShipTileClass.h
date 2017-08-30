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
	typedef std::vector<InfrastructureRequirement>			InfrastructureRequirementCollection;

	// Defines a requirement that the tile of this class contain specific object(s)
	struct ObjectRequirement
	{
		ComplexShipObjectClass * 							Class;
		int													Minimum;
		int													Maximum;

		ObjectRequirement(void) { Class = NULL; Minimum = 0; Maximum = 0; }
		ObjectRequirement(ComplexShipObjectClass *obj, int mincount, int maxcount) { Class = obj; Minimum = mincount; Maximum = maxcount; } 
	};
	typedef std::vector<ObjectRequirement>					ObjectRequirementCollection;

	// Defines a size requirements for the tile class
	struct SizeRequirement
	{
		INTVECTOR3											MinimumSize;
		INTVECTOR3											MaximumSize;
		bool												InterchangeableXY;		// Determines whether e.g. a 3x2x1 tile will satisfy a 2x3x1 requirement

		SizeRequirement(void) { MinimumSize = NULL_INTVECTOR3; MaximumSize = NULL_INTVECTOR3; InterchangeableXY = false; }
	};
	typedef std::vector<SizeRequirement>					SizeRequirementCollection;
	
	// Uniquely-identifying string code for the class
	CMPINLINE std::string									GetCode(void) const					{ return m_code; }
	Result													SetCode(const std::string & code);

	// Friendly string name of the class
	CMPINLINE std::string									GetName(void) const					{ return m_name; }
	CMPINLINE void											SetName(const std::string & name)	{ m_name = name; }

	// Class type of the tile, based upon the string code
	CMPINLINE D::TileClass									GetClassType(void)					{ return m_classtype; }

	// TODO: Is primary tile property required any more?
	CMPINLINE bool											IsPrimaryTile(void)					{ return m_primarytile; }
	CMPINLINE void											SetPrimaryTile(bool primary)		{ m_primarytile = primary; }

	// Collections of requirements associated with this class
	InfrastructureRequirementCollection						InfrastructureRequirements;
	ObjectRequirementCollection								ObjectRequirements;
	SizeRequirement											SizeRequirements;

	// Shutdown method - not required for this class
	CMPINLINE void Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	// Constructor / destructor
	ComplexShipTileClass(void);
	~ComplexShipTileClass(void);

	
	// Static methods to translate between tile classes and the string representation of those classes
	static std::string										TranslateTypeToString(D::TileClass type);
	static D::TileClass										TranslateStringToType(const std::string & type);

private:

	std::string												m_code;
	std::string												m_name;
	D::TileClass											m_classtype;
	bool													m_primarytile;

};


#endif