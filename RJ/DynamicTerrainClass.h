#pragma once

#include <string>
class DynamicTerrain;
class TerrainDefinition;


// Static class allowing instantiation of new dynamic terrain classes based upon their string class name
class DynamicTerrainClass
{
public:

	// Returns a new instance of the specified dynamic terrain class, or null if the 
	// given class name is invalid
	static DynamicTerrain *Create(const char *class_name, const TerrainDefinition *terrain_definition_override);

};
