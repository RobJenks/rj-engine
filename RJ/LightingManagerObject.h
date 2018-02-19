#pragma once

#ifndef __LightingManagerH__
#define __LightingManagerH__

#include "GameVarsExtern.h"
#include "Light.h"
#include "LightSource.h"
#include "iAcceptsConsoleCommands.h"
#include "StructuredBufferDX11.h"


class LightingManagerObject : public iAcceptsConsoleCommands
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
	typedef std::vector<LightSourceEntry>			LightSources;				// Collection of light source entries

	// Limit on the number of lights that can contribute to a scene.  Should be far less of a 
	// constraint under deferred rendering system
	static const unsigned int						LIGHT_LIMIT = 512U;

	// Default constructor
	LightingManagerObject(void);

	// Initialise the lighting manager for a new frame
	void								AnalyseNewFrame(void);

	// Register a new light source for this scene
	bool								RegisterLightSource(const LightSource *light);

	// Clear all registered light sources
	void								ClearAllLightSources(void);

	// Returns a reference to the collection of currently active light sources
	CMPINLINE const LightSources &		GetCurrentLightSourceData(void) const		{ return m_sources; }

	// Return the number of light sources currently registered
	LightSources::size_type				GetLightSourceCount(void) const				{ return m_source_count; }

	// Returns a pointer to the core light data for each source in scope for this frame
	CMPINLINE const LightData *			GetLightData(void) const					{ return &(m_light_data[0]); }

	// Returns a pointer to the structured buffer holding all light data for this frame
	CMPINLINE StructuredBufferDX11 *	GetLightDataBuffer(void)					{ return m_sb_lights; }

	// Return the light configuration relevant to the specified object
	//Game::LIGHT_CONFIG					GetLightingConfigurationForObject(const iObject *object);

	// Returns the currently-active lighting configuration
	/*Game::LIGHT_CONFIG					GetActiveLightingConfiguration(void) const								{ return m_active_config; }

	// Set the active lighting configuration
	CMPINLINE void						SetActiveLightingConfiguration(Game::LIGHT_CONFIG light_config)			{ m_active_config = light_config; }

	// Set an appropriate lighting configuration for rendering the specified object
	CMPINLINE void						SetActiveLightingConfigurationForObject(const iObject *object)
	{
		if (object) SetActiveLightingConfiguration(GetLightingConfigurationForObject(object));
	}*/

	// Called at the end of a frame to perform any final lighting-related activities
	void								EndFrame(void);

	// Returns data for a basic, default unsituated directional light
	void								GetDefaultDirectionalLightData(LightData & outLight);
	LightData							GetDefaultDirectionalLightData(void);

	// Returns data for a basic, default point light
	void								GetDefaultPointLightData(LightData & outLight);
	LightData							GetDefaultPointLightData(void);

	// Returns data for a basic, default spot light
	void								GetDefaultSpotLightData(LightData & outLight);
	LightData							GetDefaultSpotLightData(void);

	// Indicates whether the lighting state is overridden for this frame
	CMPINLINE bool						LightingIsOverridden(void) const					{ return m_lighting_is_overridden; }

	// Add a light to the lighting override for this frame
	CMPINLINE void						AddOverrideLight(LightSource *source)				
	{ 
		m_override_lights.push_back((iObject*)source); 
		m_lighting_is_overridden = true;
	}

	// Add a set of lights to the lighting override for this frme
	CMPINLINE void						AddOverrideLights(const std::vector<iObject*> & lights)
	{
		m_override_lights.insert(m_override_lights.end(), lights.begin(), lights.end());
		m_lighting_is_overridden = true;
	}

	// Clear the lighting override 
	CMPINLINE void						DisableLightingOverride(void)						
	{ 
		m_override_lights.clear(); 
		m_lighting_is_overridden = false;
	}

	// Apply a standard lighting override this frame
	void								ApplyStandardCameraFacingLightOverride(void);

	// Generates a debug output string for the current lighting state
	std::string							DebugOutputLightingState(void) const;

	// Virtual inherited method to accept a command from the console
	bool								ProcessConsoleCommand(GameConsoleCommand & command);

	// Default destructor
	~LightingManagerObject(void);

protected:

	// The collection of light sources that are contributing to the scene this frame
	LightSources											m_sources;

	// The array of light data that will be transferred to shaders at render-time; constructed once per frame during analysis phase
	LightData												m_light_data[LightingManagerObject::LIGHT_LIMIT];

	// Number of light sources currently active
	LightSources::size_type									m_source_count;

	// Flag indicating whether the light source collection is currently sorted; reset each frame and only
	// relevant when we exceed the maximum number of allowed light sources.  In that case, the source
	// vector is sorted and items are added in priority order
	bool													m_source_vector_is_sorted;

	// Structured buffer that is populated with all frame lighting data and compiled during frame analysis
	StructuredBufferDX11 *									m_sb_lights;

	// The lighting manager is responsible for storing the active lighting configuration during rendering
	/*Game::LIGHT_CONFIG										m_active_config;

	// Lookup array which translates from a light index to its bit value in a light config bitstring
	Game::LIGHT_CONFIG										m_config_lookup[LightingManagerObject::LIGHT_LIMIT];*/

	// Lighting override data
	bool													m_lighting_is_overridden;
	std::vector<iObject*>									m_override_lights;

	// Standard pre-configured set of override lights.  Directional lighting only which primarily shines out 
	// of the camera, but also includes a more limited amount of ambient/surrounding light from other angles
	std::vector<iObject*>									m_std_override_cameralights;

	// Initialise the pre-configured lighting setups that can be used as a standard override
	void													InitialiseStandardLightingOverrides(void);

	// Orients a lighting set with the camera
	void													OrientLightingOverrideSetWithCamera(std::vector<iObject*> & lighting_override);

	// Functor for sorting/searching light sources based on priority
	struct _LightSourceEntryPriorityComparator
	{
		bool operator() (const LightSourceEntry & lhs, const LightSourceEntry & rhs) const;
	};

	// Static instance of the comparator for performing binary searches
	static _LightSourceEntryPriorityComparator				LightSourceEntryPriorityComparator;

	// Temporary vector used to hold light source objects while the frame is being analysed
	std::vector<iObject*>									_m_frame_light_sources;
	
	// Unary predicate for locating specfic light objects
	class LightOfSpecificType
	{
	public:
		LightOfSpecificType(LightType _type) : type(_type){ }
		bool operator()(iObject* obj) const 
		{ 
			return (obj->GetObjectType() == iObject::ObjectType::LightSourceObject &&		// Object must be a LightSource...
				((LightType)((LightSource*)obj)->GetLight().Data.Type) == type);			// ...and have specific light type
		}
	protected:
		LightType type;
	};

	// Unary predicate for excluding specific light objects
	class LightNotOfSpecificType 
	{
	public:
		LightNotOfSpecificType(LightType _type) : type(_type){ }
		bool operator()(iObject* obj) const
		{
			return (obj->GetObjectType() == iObject::ObjectType::LightSourceObject &&		// Object must be a LightSource...
				((LightType)((LightSource*)obj)->GetLight().Data.Type) != type);			// ...and NOT be the specified light type
		}
	protected:
		LightType type;
	};
};


#endif





