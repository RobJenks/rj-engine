#pragma once

// Clone method for dynamic terrain subclasses
#define DYNAMIC_TERRAIN_CLONE_METHOD(ClassName) \
	__forceinline virtual DynamicTerrain * Clone(void) const \
	{ \
		ClassName *instance = new ClassName(*this); \
		instance->InitialiseDynamicTerrainBase(); \
		instance->InitialiseDynamicTerrain(); \
		return static_cast<DynamicTerrain*>(instance); \
	}

// Macro used to build a dynamic terrain class header, based on the supplied parameters
#define _DYNAMIC_TERRAIN_CLASS_HEADER(ClassName, SuperClass) \
\
class ClassName : public SuperClass \
{ \
public: \
\
	static const char *	DynamicTerrainClassName(void) { return #ClassName; } \
	DYNAMIC_TERRAIN_CLONE_METHOD(ClassName)




// Macro used to define new dynamic terrain types, and register required data at the same time
#define DYNAMIC_TERRAIN_CLASS(ClassName) _DYNAMIC_TERRAIN_CLASS_HEADER(ClassName, DynamicTerrain)


// Macro used to define new dynamic terrain types that inherit from an abstract superclass, and register required data at the same time
#define DYNAMIC_TERRAIN_DERIVED_CLASS(ClassName, SuperClass) _DYNAMIC_TERRAIN_CLASS_HEADER(ClassName, SuperClass)

// Macro used to define new dynamic terrain types that inherit from an abstract superclass, and register required data at the same time.  
// Generates a complete implementation rather than just the class header
#define DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(ClassName, SuperClass) \
	_DYNAMIC_TERRAIN_CLASS_HEADER(ClassName, SuperClass) \
	}


