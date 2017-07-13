#pragma once

#ifndef __SpaceProjectileDefinitionH__
#define __SpaceProjectileDefinitionH__

#include <string>
#include "CompilerSettings.h"
#include "AudioParameters.h"
class SpaceProjectile;
class Model;

// Class does not have any special alignment requirements
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
	CMPINLINE const std::string & 		GetCode(void) const							{ return m_code; }
	CMPINLINE void						SetCode(const std::string & code)			{ m_code = code; }

	// Return or set the descriptive string name for this projectile type
	CMPINLINE const std::string & 		GetName(void) const							{ return m_name; }
	CMPINLINE void						SetName(const std::string & name)			{ m_name = name; }

	// Set or return the model for this projectile type
	CMPINLINE Model *					GetModel(void) const						{ return m_model; }
	CMPINLINE void						SetModel(Model *model)						{ m_model = model; }

	// Set or return other key definition fields
	CMPINLINE float						GetMass(void) const							{ return m_mass; }
	CMPINLINE void						SetMass(float m)							{ m_mass = m; }

	// Set or return the projectile type
	CMPINLINE ProjectileType			GetProjectileType(void) const				{ return m_projtype; }
	CMPINLINE void						SetProjectileType(ProjectileType t)			{ m_projtype = t; }

	// Set or return the lifetime-end action for this projectile
	CMPINLINE LifetimeEndAction			GetLifetimeEndAction(void) const			{ return m_lifeendaction; }
	CMPINLINE void						SetLifetimeEndAction(LifetimeEndAction a)	{ m_lifeendaction = a; }

	// Return or set other key object fields
	CMPINLINE float						GetDefaultLifetime(void) const				{ return m_defaultlifetime; }
	CMPINLINE void						SetDefaultLifetime(float L)					{ m_defaultlifetime = L; }

	// Audio item played when this projectile is launched
	CMPINLINE AudioParameters			GetLaunchAudio(void) const					{ return m_launch_audio; }
	CMPINLINE void						SetLaunchAudio(AudioParameters audio)		{ m_launch_audio = audio; }

	// Creates and returns a new projectile based upon this definition
	SpaceProjectile *					CreateProjectile(void) const;

	// Shutdown method - not required for this class
	CMPINLINE void Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	// Static methods to translate a projectile type to/from its string representation
	static ProjectileType				TranslateProjectileTypeFromString(std::string type);
	static std::string					TranslateProjectileTypeToString(ProjectileType type);

	// Static methods to translate a lifetime-end action to/from its string representation
	static LifetimeEndAction			TranslateLifetimeEndActionFromString(std::string action);
	static std::string					TranslateLifetimeEndActionToString(LifetimeEndAction action);

protected:

	std::string									m_code;								// Unique string code for this projectile type
	std::string									m_name;								// Descriptive string name of the projectile type
	Model *										m_model;							// Geometry for this projectile type
	float										m_mass;								// Mass of the projectile

	ProjectileType								m_projtype;							// Type of projectile
	float										m_defaultlifetime;					// The default lifetime (secs) for this projectile type to exist
	LifetimeEndAction							m_lifeendaction;					// The action taken once projectile lifetime is exceeded

	AudioParameters								m_launch_audio;						// Audio item to be played when this projectile is launched

};



#endif