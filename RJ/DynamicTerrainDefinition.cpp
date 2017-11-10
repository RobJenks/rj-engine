#include "DynamicTerrain.h"
#include "DynamicTerrainDefinition.h"

// Default constructor
DynamicTerrainDefinition::DynamicTerrainDefinition(void)
	:
	m_code(NullString), m_prototype(NULL)
{
}

// Create a new dynamic terrain object based upon this definition
DynamicTerrain * DynamicTerrainDefinition::Create(void) const
{
	// We must have a valid prototype
	if (m_prototype == NULL) return NULL;

	// Create a new instance based upon our prototype
	DynamicTerrain *instance = m_prototype->Clone();

	// Apply other post-instantiation changes as required

	// Return the new instance
	return instance;
}

// Default destructor
DynamicTerrainDefinition::~DynamicTerrainDefinition(void)
{
	// The parent object (generally an environment) is always in charge of deallocating the static/dynamic terrain objects 
	// which it owns.  In this case however we have a standalone terrain object being used as a prototype which belongs
	// to us, and not any definition.  We therefore deallocate the prototype terrain object when this definition is deallocated
	if (m_prototype)
	{
		SafeDelete(m_prototype);
	}
}



