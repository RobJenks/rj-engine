#pragma once

#ifndef __StaticTerrainDefinitionH__
#define __StaticTerrainDefinitionH__

#include "CompilerSettings.h"
class Model;

// This class has no special alignment requirements
class StaticTerrainDefinition
{

public:

	// Default construtor; initialises all fields to default values
	StaticTerrainDefinition(void) : 
		m_model(NULL), m_defaultextent(NULL_FLOAT3), m_destructible(false), m_maxhealth(1.0f), 
		m_mass(1.0f), m_hardness(1.0f)
	{
	}

	// Unique string code identifying this type of terrain
	CMPINLINE std::string					GetCode(void) const					{ return m_code; }
	CMPINLINE void							SetCode(const std::string & code)	{ m_code = code; }

	// Returns or sets a reference to the renderable model associated with this terrain
	CMPINLINE Model *						GetModel(void) const				{ return m_model; }
	CMPINLINE void							SetModel(Model *m)					{ m_model = m; }

	// Returns or sets the default extent for a terrain object of this type
	CMPINLINE XMFLOAT3						GetDefaultExtent(void) const		{ return m_defaultextent; }
	CMPINLINE void							SetDefaultExtent(const XMFLOAT3 & e){ m_defaultextent = e; }

	// Returns data on whether/how the terrain is destructible
	CMPINLINE bool							IsDestructible(void) const			{ return m_destructible; }
	CMPINLINE void							SetDestructible(bool b)				{ m_destructible = b; }
	CMPINLINE float							GetMaxHealth(void) const			{ return m_maxhealth; }
	CMPINLINE void							SetMaxHealth(float h)				{ m_maxhealth = h; }

	// Data on other attributes of the terrain object
	CMPINLINE float							GetMass(void) const					{ return m_mass; }
	CMPINLINE void							SetMass(float m)					{ m_mass = max(m, Game::C_EPSILON); }
	CMPINLINE float							GetHardness(void) const				{ return m_hardness; }
	CMPINLINE void							SetHardness(float h)				{ m_hardness = max(h, Game::C_EPSILON); }

	// Shutdown method - not required for this class
	CMPINLINE void Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	// Default destructor
	~StaticTerrainDefinition(void) { }

protected:

	std::string								m_code;							// Unique string code for this terrain class
	Model *									m_model;						// Pointer to the model associated with this terrain
	XMFLOAT3								m_defaultextent;				// Default extent for this terrain type; can be overridden by each instance

	bool									m_destructible;					// Determines whether the object can be damaged
	float									m_maxhealth;					// Maximum health value, for destructible objects	

	float									m_mass;							// Mass of the entire terrain object
	float									m_hardness;						// Approximation to object density, used primarily in collision calculations

};


#endif