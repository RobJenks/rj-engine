#pragma once

#ifndef __SpaceProjectileDefinitionH__
#define __SpaceProjectileDefinitionH__

#include <string>
#include "CompilerSettings.h"
class SpaceProjectile;
class Model;

class SpaceProjectileDefinition
{
public:

	// Enumeration of projectile types
	enum ProjectileType { Impactor = 0, Explosive };

	// Enumeration of actions to be taken at the end of projectile lifetime
	enum LifetimeEndAction { Disappear = 0, Detonate };


	// Default constructor
	SpaceProjectileDefinition(void);

	// Return or set the unique string code for this projectile type
	CMPINLINE const std::string & 		GetCode(void) const					{ return m_code; }
	CMPINLINE void						SetCode(const std::string & code)	{ m_code = code; }

	// Set or return the model for this projectile type
	CMPINLINE Model *					GetModel(void) const				{ return m_model; }
	CMPINLINE void						SetModel(Model *model)				{ m_model = model; }

	// Set or return other key definition fields
	CMPINLINE float						GetMass(void) const					{ return m_mass; }
	CMPINLINE void						SetMass(float m)					{ m_mass = m; }

	// Return or set other key object fields
	CMPINLINE float						GetDefaultLifetime(void) const		{ return m_defaultlifetime; }
	CMPINLINE void						SetDefaultLifetime(float L)			{ m_defaultlifetime = L; }

	// Creates and returns a new projectile based upon this definition
	SpaceProjectile *					CreateProjectile(void) const;



protected:

	std::string									m_code;								// Unique string code for this projectile type
	Model *										m_model;							// Geometry for this projectile type
	float										m_mass;								// Mass of the projectile

	ProjectileType								m_projtype;							// Type of projectile
	float										m_defaultlifetime;					// The default lifetime (secs) for this projectile type to existt
	LifetimeEndAction							m_lifeendaction;					// The action taken once projectile lifetime is exceeded

};



#endif