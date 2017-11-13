#include "DynamicTerrainClass.h"

#include "DataObjectRelay.h"
#include "DataObjectDebugLogger.h"
#include "DataObjectRegister.h"
#include "DataObjectOutput.h"
#include "DataObjectSwitch.h"
#include "DataObjectEngineThrustController.h"

// Macro used to register dynamic terrain classes with this factory class
#define REGISTER_DYNAMIC_TERRAIN_CLASS(DTClass) \
\
if(strcmp(class_name, DTClass::DynamicTerrainClassName()) == 0) return (new DTClass())



// Returns a new instance of the specified dynamic terrain class, or null if the 
// given class name is invalid
DynamicTerrain * DynamicTerrainClass::Create(const char *class_name)
{
	if (class_name == NULL) return NULL;

	// Basic circuitry
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRelay);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectDebugLogger);

	// Data register types
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister1);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister2);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister3);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister4);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister5);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister6);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister7);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister8);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister9);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectRegister10);

	// Data output types
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput1);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput2);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput3);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput4);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput5);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput6);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput7);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput8);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput9);
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectOutput10);

	// Switch components
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectSwitch);

	// Engine-related components
	REGISTER_DYNAMIC_TERRAIN_CLASS(DataObjectEngineThrustController);


	// Class could not be found
	return NULL;
}







