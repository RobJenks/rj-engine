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

// Macro used to define new dynamic terrain types, and register required data at the same time
#define DYNAMIC_TERRAIN_CLASS(ClassName) \
\
class ClassName : public DynamicTerrain \
{ \
public: \
\
	static const char *	DynamicTerrainClassName(void) { return #ClassName; } \
	DYNAMIC_TERRAIN_CLONE_METHOD(ClassName)




// Macro used to define new dynamic terrain types that are an abstract superclass for others, and register required data at the same time
#define DYNAMIC_TERRAIN_ABSTRACT_SUPERCLASS(ClassName) \
\
class ClassName : public DynamicTerrain \
{ \
public: \
\
	static const char *	DynamicTerrainClassName(void) { return #ClassName; } \
	DYNAMIC_TERRAIN_CLONE_METHOD(ClassName)



// Macro used to define new dynamic terrain types that inherit from an abstract superclass, and register required data at the same time
#define DYNAMIC_TERRAIN_DERIVED_CLASS(ClassName, SuperClass) \
\
class ClassName : public SuperClass \
{ \
public: \
	static const char *	DynamicTerrainClassName(void) { return #ClassName; } \
	DYNAMIC_TERRAIN_CLONE_METHOD(ClassName) \
};


