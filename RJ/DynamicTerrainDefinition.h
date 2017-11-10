#pragma once

#include <string>
#include "CompilerSettings.h"
class DynamicTerrain;

class DynamicTerrainDefinition
{
public:

	// Default constructor
	DynamicTerrainDefinition(void);

	// Unique string code of the dynamic terrain definition
	CMPINLINE std::string					GetCode(void) const { return m_code; }
	CMPINLINE void							SetCode(const std::string & code) { m_code = code; }

	// Create a new dynamic terrain object based upon this definition
	DynamicTerrain *						Create(void) const;

	// Prototype terrain object used for instantiation
	CMPINLINE const DynamicTerrain *		GetPrototype(void) const { return m_prototype; }
	CMPINLINE void							SetPrototype(DynamicTerrain *prototype) { m_prototype = prototype; }


	// Explicit shutdown method is not required for definition objects; all deallocation is performed in the destructor
	CMPINLINE void							Shutdown(void) { }

	// Default destructor
	~DynamicTerrainDefinition(void);

protected:

	// Unique string code of the dynamic terrain definition
	std::string								m_code;

	// Prototype terrain object used for instantiation
	DynamicTerrain *						m_prototype;

};