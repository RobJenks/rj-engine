#pragma once

#include "Terrain.h"
#include "DataEnabledObject.h"
#include "UsableObject.h"
#include "DynamicTerrainClassData.h"
class TerrainDefinition;


class DynamicTerrain : public Terrain, public DataEnabledObject, public UsableObject
{
public:

	// Static method to instantiate a new dynamic terrain object based upon its string code
	static DynamicTerrain *					Create(const std::string & code);

	// String code of the dynamic terrain prototype that this instance is based upon
	CMPINLINE std::string					GetCode(void) const { return m_dynamic_terrain_code; }
	CMPINLINE void							SetCode(const std::string & code) { m_dynamic_terrain_code = code; }

	// Event raised when an entity tries to interact with this object
	virtual bool							OnUsed(iObject *user);

	// Set a property of this dynamic terrain object; should be overridden by any subclasses that wish to use them
	CMPINLINE virtual void					SetProperty(const std::string & key, const std::string & value) { };

	// Pure virtual clone method for this object type; automatically implemented for subclasses
	// by the DYNAMIC_TERRAIN_CLASS macro
	virtual DynamicTerrain *				Clone(void) const = 0;


protected:

	// String code of the dynamic terrain prototype that this instance is based upon
	std::string								m_dynamic_terrain_code;

	// Default constructor
	DynamicTerrain(void);


};
