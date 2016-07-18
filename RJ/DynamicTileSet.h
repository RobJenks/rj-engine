#pragma once

#ifndef __DynamicTileSetH__
#define __DynamicTileSetH__

#include <string>
#include <vector>
#include "CompilerSettings.h"
#include "TileConnections.h"
class ComplexShipTile;
class ComplexShipTileDefinition;


class DynamicTileSet
{
public:

	// Structure holding details of one possible tile selection
	struct DynamicTileRequirements
	{
		const ComplexShipTileDefinition *		TileDefinition;			// The tile definition being considered
		Rotation90Degree						Rotation;				// The orientation of the tile in this scenario

		TileConnections							Connections;			// The required set of connections for this tile to be selected

		// Constructors
		DynamicTileRequirements(void) : TileDefinition(NULL), Rotation(Rotation90Degree::Rotate0) { }
	};

	// Structure holding the result returned by a DTS when asked to select a tile
	struct DynamicTileSetResult
	{
		const ComplexShipTileDefinition *		TileDefinition;
		Rotation90Degree						Rotation;

		// Constructors
		DynamicTileSetResult(void) : TileDefinition(NULL), Rotation(Rotation90Degree::Rotate0) { }
		DynamicTileSetResult(const ComplexShipTileDefinition *definition, Rotation90Degree rotation)
			: TileDefinition(definition), Rotation(rotation) { }
	};

	// Default constructor
	DynamicTileSet(void);

	// Add a new option to this tile set
	void										AddEntry(const DynamicTileRequirements & option);

	// Set the default entry for this tile set
	void										SetDefault(const ComplexShipTileDefinition * default_option);

	// Remove an option from this tile set
	void										RemoveEntry(const ComplexShipTileDefinition * option);

	// Clear all entries from this tile set
	void										Clear(void);

	// Validate whether the specified tile still meets its selection criteria.  If not, selects another tile
	// definition from the set which does meet the criteria (or default, if none).  Returns a pointer to the 
	// correct tile definition for this scenario
	DynamicTileSetResult						GetMostAppropriateTileDefinition(ComplexShipTile *tile) const;

	// Set or retrieve the unique string code for this tile set
	CMPINLINE std::string						GetCode(void) const						{ return m_code; }
	CMPINLINE void								SetCode(const std::string & code)		{ m_code = code; }

	// Shutdown method - not required for this class
	CMPINLINE void								Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	// Default destructor
	~DynamicTileSet(void);


protected:

	// Unique code for this dynamic tile set
	std::string									m_code;

	// The set of possile tiles that can be selected
	std::vector<DynamicTileRequirements>		m_options;

	// The default option that will be selected if no criteria are met
	const ComplexShipTileDefinition *			m_default_option;

	// Tests whether the specified option is valid for the tile in question
	bool										TileMeetsCriteria(ComplexShipTile *tile, const DynamicTileRequirements & criteria) const;

};



#endif





