#pragma once

#ifndef __CSLifeSupportTileH__
#define __CSLifeSupportTileH__

#include "CompilerSettings.h"
#include "GameDataExtern.h"
#include "AdjustableParameter.h"
#include "Oxygen.h"
#include "CSLifeSupportTileDefinition.h"
#include "ComplexShipTile.h"
class TiXmlElement;

class CSLifeSupportTile : public ComplexShipTile
{
public:
	// Public inherited virtual method to return the type of ship tile this object represents
	CMPINLINE D::TileClass			GetClass(void) const	{ return D::TileClass::LifeSupport; }

	// Static method to return the class; allows this to be determined based on the class and not an instance
	CMPINLINE static D::TileClass	GetClassStatic(void)	{ return D::TileClass::LifeSupport; }

	// Simulation method for this tile
	void							PerformTileSimulation(unsigned int delta_ms);

	// Key properties maintained by the life support system
	AdjustableParameter<float>			Gravity;
	AdjustableParameter<Oxygen::Type>	OxygenLevel;
	AdjustableParameter<Oxygen::Type>	OxygenRange;

	// Life support system has a general effectivity property dependent on its current state (e.g. damage, maintenance level)
	CMPINLINE float				GetEffectivity(void) const				{ return m_effectivity; }

	// One of the primary tile methods, to return the percentage of total gravity strength generated by this system 
	// at a particular location in the environment
	CMPINLINE float				GetGravityPercentage(const INTVECTOR3 & location) const { return GetGravityPercentage(location.x, location.y, location.z); }
	float						GetGravityPercentage(int x, int y, int z) const;

	// Primary tile method, to return the actual strength of gravity generated at a particular location in the environment
	CMPINLINE float				GetGravityStrength(const INTVECTOR3 & location) const { return GetGravityStrength(location.x, location.y, location.z); }
	CMPINLINE float				GetGravityStrength(int x, int y, int z) const
	{
		return (GetGravityPercentage(x, y, z) * Gravity.Value);
	}

	// Returns the maximum gravity range of this tile
	CMPINLINE int				GetGravityRange(void) const				{ return m_gravityrange; }

	// Updates the maximum effective gravity range of the tile.  Should only be updated by the tile definition, not directly at runtime
	CMPINLINE void				SetGravityRange(int r)					{ m_gravityrange = r; }

	// Return oxygen data
	CMPINLINE Oxygen::Type		GetOxygenOutput(void) const				{ return OxygenLevel.Value; }
	CMPINLINE Oxygen::Type		GetTargetOxygenOutput(void) const		{ return OxygenLevel.Target; }

	// We store a direct reference to the life support definition for more efficient runtime access
	const CSLifeSupportTileDefinition *	GetLifeSupportTileDefinition(void) const								{ return m_lifesupportdef; }
	void								StoreLifeSupportTileDefinition(const CSLifeSupportTileDefinition *def)	{ m_lifesupportdef = def; }

	// Apply the contents of the tile to its parent objects.  Called upon linking, plus on repair of the ship.  Inherited virtual.
	void						ApplyTileSpecific(void);

	// Virtual inherited method to make a copy of this tile and return it
	ComplexShipTile *			Copy(void) const;

	// Constructor/copy constructor/destructor
	CSLifeSupportTile(void);
	CSLifeSupportTile(const CSLifeSupportTile &C);
	~CSLifeSupportTile(void);

	// Virtual & static methods respectively to generate and read XML data representing the tile
	TiXmlElement *				GenerateXML(void);						// Virtual inherited since is called on an instance of a tile

	// Virtual method to read any class-specific data for this tile type
	void						ReadClassSpecificXMLData(TiXmlElement *node);

	// Processes a debug tile command from the console
	void						ProcessDebugTileCommand(GameConsoleCommand & command);

private:

	// Store a direct reference to the life support definition for faster runtime lookups
	const CSLifeSupportTileDefinition *	m_lifesupportdef;

	// Store the maximum effective gravity range of this tile (taken from the tile definition)
	int									m_gravityrange;

	// Tile has an effectivity modifier based upon its current state (e.g. damage, maintenance level) that is multiplied to all output values
	float								m_effectivity;

};


#endif