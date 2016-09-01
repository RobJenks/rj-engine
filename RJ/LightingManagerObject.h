#pragma once

#ifndef __LightingManagerH__
#define __LightingManagerH__

#include "GameVarsExtern.h"
#include "Data\\Shaders\\render_constants.h"
#include "Data\\Shaders\\light_definition.h"
class LightSource;

class LightingManagerObject
{
public:

	// Struct wrapping a light source and including additional render-time information
	struct LightSourceEntry
	{
		const LightSource *							Source;				// The light source itself
		float										DistSq;				// Distance sq from the camera viewing position to the light (only calculated if required for prioritisation)

		LightSourceEntry(const LightSource *source) : Source(source)								{ }
		LightSourceEntry(const LightSource *source, float distsq) : Source(source), DistSq(distsq)	{ }
	};

	// Typedefs
	typedef std::vector<LightData>					DirLightData;				// Collection of directional light data
	typedef std::vector<LightSourceEntry>			LightSources;				// Collection of light source entries

	// Limit on the number of lights that can contribute to a scene
	static const unsigned int						DIR_LIGHT_LIMIT = C_DIR_LIGHT_LIMIT;
	static const unsigned int						LIGHT_LIMIT = C_LIGHT_LIMIT;
	static const unsigned int						TOTAL_LIGHT_LIMIT = C_TOTAL_LIGHT_LIMIT;

	// Default constructor
	LightingManagerObject(void);

	// Initialise the lighting manager for a new frame
	void								AnalyseNewFrame(void);

	// Register a new light source for this scene
	bool								RegisterLightSource(const LightSource *light);

	// Clear all registered light sources
	void								ClearAllLightSources(void);

	// Returns the number of directional light sources present
	CMPINLINE DirLightData::size_type	GetDirectionalLightSourceCount(void) const	{ return m_dir_light_count; }

	// Returns a pointer to data on the global directional light source(s)
	CMPINLINE const LightData *			GetDirectionalLightData(void)								{ return &(m_dir_light[0]); }
	CMPINLINE LightData 				GetDirectionalLightDataEntry(DirLightData::size_type index)	{ return m_dir_light[index]; }

	// Return the number of light sources currently registered
	LightSources::size_type				GetLightSourceCount(void) const				{ return m_source_count; }

	// Returns a pointer to the core light data for each source in scope for this frame
	CMPINLINE const LightData *			GetLightData(void) const					{ return &(m_light_data[0]); }

	// Return the light configuration relevant to the specified object
	Game::LIGHT_CONFIG					GetLightingConfigurationForObject(const iObject *object);

	// Returns the currently-active lighting configuration
	Game::LIGHT_CONFIG					GetActiveLightingConfiguration(void) const								{ return m_active_config; }

	// Set the active lighting configuration
	CMPINLINE void						SetActiveLightingConfiguration(Game::LIGHT_CONFIG light_config)			{ m_active_config = light_config; }

	// Set an appropriate lighting configuration for rendering the specified object
	CMPINLINE void						SetActiveLightingConfigurationForObject(const iObject *object)
	{
		if (object) SetActiveLightingConfiguration(GetLightingConfigurationForObject(object));
	}

	// Add, change or remove directional light data from the scene
	bool								AddDirectionalLight(const LightData & data);
	bool								CanAddNewDirectionalLight(void) const;
	void								UpdateDirectionalLight(DirLightData::size_type index, const LightData & data);
	void								RemoveDirectionalLight(DirLightData::size_type index);
	void								ClearDirectionalLightData(void);

	// Returns data for a basic, default unsituated directional light
	void								GetDefaultDirectionalLightData(LightData & outLight);

	// Returns data for a basic, default point light
	void								GetDefaultPointLightData(LightData & outLight);

	// Returns data for a basic, default spot light
	void								GetDefaultSpotLightData(LightData & outLight);

	// Default destructor
	~LightingManagerObject(void);

protected:

	// The collection of light sources that are contributing to the scene this frame
	LightSources											m_sources;

	// The array of light data that will be transferred to shaders at render-time; constructed once per frame during analysis phase
	LightData												m_light_data[LightingManagerObject::LIGHT_LIMIT];

	// Number of light sources currently active
	LightSources::size_type									m_source_count;

	// The lighting manager is responsible for storing the active lighting configuration during rendering
	Game::LIGHT_CONFIG										m_active_config;

	// Lookup array which translates from a light index to its bit value in a light config bitstring
	Game::LIGHT_CONFIG										m_config_lookup[LightingManagerObject::LIGHT_LIMIT];

	// Global directional light(s)
	DirLightData											m_dir_light;
	DirLightData::size_type									m_dir_light_count;

	// Functor for sorting/searching light sources based on priority
	static struct _LightSourceEntryPriorityComparator
	{
		bool operator() (const LightSourceEntry & lhs, const LightSourceEntry & rhs) const;
	};

	// Static instance of the comparator for performing binary searches
	static _LightSourceEntryPriorityComparator				LightSourceEntryPriorityComparator;

};




#endif





