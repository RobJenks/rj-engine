#pragma once

// Macro used to define new dynamic terrain types, and register required data at the same time
#define DYNAMIC_TERRAIN_CLASS(ClassName) \
\
class ClassName : public DynamicTerrain \
{ \
public: \
\
	static const char *						DynamicTerrainClassName(void) { return #ClassName; } \
	__forceinline virtual DynamicTerrain *	Clone(void) const { return static_cast<DynamicTerrain*>(new ClassName(*this)); }




// Macro used to define new dynamic terrain types that are an abstract superclass for others, and register required data at the same time
#define DYNAMIC_TERRAIN_ABSTRACT_SUPERCLASS(ClassName) \
\
class ClassName : public DynamicTerrain \
{ \
public: \
\
	static const char *						DynamicTerrainClassName(void) { return #ClassName; } \
	__forceinline virtual DynamicTerrain *	Clone(void) const { return static_cast<DynamicTerrain*>(new ClassName(*this)); }



// Macro used to define new dynamic terrain types that inherit from an abstract superclass, and register required data at the same time
#define DYNAMIC_TERRAIN_DERIVED_CLASS(ClassName, SuperClass) \
\
class ClassName : public SuperClass \
{ \
public: \
	static const char *						DynamicTerrainClassName(void) { return #ClassName; } \
	static ClassName *						Create(const TerrainDefinition *def) { return static_cast<ClassName*>(SuperClass::Create(def, new ClassName())); } \
	__forceinline virtual DynamicTerrain *	Clone(void) const { return static_cast<DynamicTerrain*>(new ClassName(*this)); } \
};


