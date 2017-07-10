#pragma once

#include <Audio.h>
#include "ErrorCodes.h"
#include "ObjectReference.h"
#include "Audio.h"
#include "AudioItem.h"
#include "AudioInstanceObjectBinding.h"


class AudioManager
{
public:

	// Key constants
	static const Audio::AudioID						NULL_AUDIO;
	static const std::string						NULL_AUDIO_NAME;
	static const Audio::AudioInstanceIdentifier		NULL_INSTANCE_IDENTIFIER;
	static const Audio::AudioInstanceID				GLOBAL_AUDIO_INSTANCE_LIMIT;			// Max instances across all audio items
	static const Audio::AudioInstanceID				DEFAULT_AUDIO_ITEM_INSTANCE_LIMIT;		// Default max instances per audio item
	static const Audio::AudioInstanceID				HARD_INSTANCE_LIMIT_PER_AUDIO;			// Hard limit for instance count per audio, cannot be overridden
	static const float								MAXIMUM_VOLUME;							// Maximum possible volume for any audio instance
	static const float								DEFAULT_VOLUME;
	static const float								DEFAULT_PITCH_SHIFT;
	static const float								DEFAULT_PAN;
	static const unsigned int						PERIODIC_AUDIO_MANAGER_AUDIO_INTERVAL;	// ms; time between periodic audits

	// Note: inner range should always be some % < 1.0 of max range to avoid in/out of range thrash at boundary
	static const float								SPACE_AUDIO_MAX_RANGE;		 // Max distance for audio, beyond which audio sources will not be audible (space events, when in space)
	static const float								SPACE_AUDIO_INNER_RANGE;	 // Inner range, % of max range, within which audio sources will become audible
	static const float								ENV_AUDIO_MAX_RANGE;		 // Max distance for audio, beyond which audio sources will not be audible (env events, when in env)
	static const float								ENV_AUDIO_INNER_RANGE;		 // Inner range, % of max range, within which audio sources will become audible
	static const float								ENV_SPACE_AUDIO_MAX_RANGE;	 // Max distance for audio, beyond which audio sources will not be audible (space events, when in env)
	static const float								ENV_SPACE_AUDIO_INNER_RANGE; // Inner range, % of max range, within which audio sources will become audible
	static const float								ENV_SPACE_VOLUME_MODIFIER;	 // Modifier to space event volume when in an environment

	static const float								AUDIBLE_DISTANCE_AT_DEFAULT_VOLUME;		// Max audible distance for 3D audio instance at volume = 1.0
	static const float								MAXIMUM_AUDIBLE_DISTANCE;				// Limit on audible distance, regardless of volume

	// Default constructor
	AudioManager(void);

	// Initialise all audio manager resources
	Result								Initialise(void);

	// Per-frame update method
	void								Update(void);

	// Registers a new sound with the audio manager.  Flag 'load_resouce' determines whether 
	// the audio resource will be loaded immediately.  If it is not loaded now, LoadResource()
	// must be called on the specific audio item before it can be used
	Result								RegisterSound(	const char *name, const char *type, const char *filename, 
														bool default_loop_state, float default_volume, bool load_resource);

	// Load the resource associated with the given audio item
	Result								LoadAudioResource(Audio::AudioID id);
	Result								LoadAudioResource(const std::string & name);

	// Load all audio resources that have not yet been loaded
	Result								LoadAllAudioResources(void);

	// Release the specified audio resource.  Audio item is preserved, but resource must be
	// re-loaded before it can be used again
	void								ReleaseAudioResource(Audio::AudioID id);

	// Releases all audio resources.  Audio items are preserved, but resources must be re-loaded 
	// before they can be used
	void								ReleaseAllAudioResources(void);

	// Determines whether the supplied audio ID is valid
	CMPINLINE bool						IsValidID(Audio::AudioID id) const { return (id > 0U && id < m_audio_count); }

	// Returns the ID of the audio item with the given name, or NULL_AUDIO if no such item exists
	CMPINLINE Audio::AudioID		GetAudioID(const std::string & name) const 
	{
		Audio::AudioIDMap::const_iterator it = m_sound_name_map.find(name);
		return (it != m_sound_name_map.end() ? it->second : AudioManager::NULL_AUDIO);
	}

	// Indicates whether the audio engine is currently in a failure state
	CMPINLINE bool						IsInErrorState(void) const			{ return m_in_error_state; }

	// Returns the audio listener centered at the player position and orientation
	CMPINLINE static const DirectX::AudioListener & 
										GetPlayerAudioListener(void)		{ return PLAYER_AUDIO_LISTENER; }

	// Play a one-shot sound effect based on the specified audio resource
	// Volume: default = 1
	// PitchShift: In the range [-1 +1], default with no shift = 0
	// Pan: In the range [-1 = full left, +1 = full right], default with no panning = 0
	void								Play(Audio::AudioID id, float volume, float pitch_shift, float pan); 
	void								Play(const std::string & name, float volume, float pitch_shift, float pan);

	// Play a one-shot sound effect based on the specified audio resource
	CMPINLINE void						Play(Audio::AudioID id) 		{ Play(id, AudioManager::DEFAULT_VOLUME, AudioManager::DEFAULT_PITCH_SHIFT, AudioManager::DEFAULT_PAN); }
	CMPINLINE void						Play(const std::string & name)		{ Play(name, AudioManager::DEFAULT_VOLUME, AudioManager::DEFAULT_PITCH_SHIFT, AudioManager::DEFAULT_PAN); }
	
	// Ensures that a slot is available for a new audio instance.  Will terminate an instance of the current 
	// audio item if necessary to remain under the global audio instance limit
	void								EnsureInstanceIsAvailable(Audio::AudioID id, bool requires_3d_support);

	// Create a new instance of an audio item, if posssible.  Returns non-zero if instantiation fails
	Audio::AudioInstanceIdentifier				CreateInstance(Audio::AudioID id, float base_volume, float volume_modifier);
	CMPINLINE Audio::AudioInstanceIdentifier	CreateInstance(const std::string & name, float base_volume, float volume_modifier) {
		return CreateInstance(GetAudioID(name), base_volume, volume_modifier); 
	}

	// Create a new 3D instance of an audio item, if possible.  Returns non-zero if instantiation fails
	Audio::AudioInstanceIdentifier				Create3DInstance(Audio::AudioID id, const XMFLOAT3 & position, float base_volume, float volume_modifier);
	CMPINLINE Audio::AudioInstanceIdentifier	Create3DInstance(const std::string & name, const XMFLOAT3 & position, float base_volume, float volume_modifier) {
		return Create3DInstance(GetAudioID(name), position, base_volume, volume_modifier); 
	}

	// Return the base type volume modifier for a specific audio item type
	CMPINLINE float										GetBaseTypeVolumeModifier(AudioItem::AudioType type) { return m_type_volume_modifiers[(int)type]; }

	// Set the base type volume modifier for a specific audio item type, and update all affected audio items & instances accordingly
	void												SetBaseTypeVolumeModifier(AudioItem::AudioType type, float volume_modifier);

	// Set the base type volume modifier for all audio item types, and update all audio items & instances accordingly
	void												SetBaseTypeVolumeModifiers(float const(&type_modifiers)[AudioItem::AudioType::_COUNT]);

	// Calculates the final volume level of an instance based on all relevant parameters
	// Final volume = (base_volume_modifier * volume_modifier * base_volume)
	// base_volume_modifier = base modifier based on audio type (effect vs music vs ...)
	// volume_modifier = instance-specific modifier (e.g. for interior vs exterior sounds)
	// base_volume = desired base volume before any modification
	static float										DetermineVolume(float base_volume, float volume_modifier, float base_volume_modifier);

	// Determines the maximum audible distance for a 3D audio instance, based upon its defined volume
	static float										DetermineMaximumAudibleDistance(float instance_volume);

	// Update the player audio listener to the current player position and orientation
	void								UpdatePlayerAudioListener(void);

	// Perform a periodic audit of active instances and update our internal state
	void								PerformPeriodicAudit(void);

	// Iterates through all audio items to get an accurate count of active audio instances.  This is required since
	// during per-frame operation the audio manager will only approximate this count based on incrementing for new 
	// instances created.  It will not search for completed instances per-frame for efficiency
	Audio::AudioInstanceID				DetermineExactAudioInstanceCount(void);

	// Returns the number of active instances being maintained by this audio manager, across all audio items.  This is
	// an upper-bound approximation that is periodically corrected downwards if necessary by the audit process
	CMPINLINE Audio::AudioInstanceID	GetTotalAudioInstanceCount(void) const { return m_instance_count; }

	// Generates a new instance identifier, which is unique across all audio items
	CMPINLINE static Audio::AudioInstanceIdentifier GetNewInstanceIdentifier(void) { return ++AUDIO_INSTANCE_COUNTER; }

	// Pauses global sound playback
	CMPINLINE void						PausePlayback(void)					{ m_engine->Suspend(); }

	// Resumes global sound playback
	CMPINLINE void						ResumePlayback(void)				{ m_engine->Resume(); }

	// Returns a const reference to an item in the audio item collection
	CMPINLINE const AudioItem * const	GetAudioItem(Audio::AudioID id) const { return (IsValidID(id) ? &(m_sounds[id]) : NULL); }

	// Shutdown method to deallocate all audio manager resources
	Result								Shutdown(void);

	// Default destructor
	~AudioManager(void);


private:

	// Core audio engine component
	DirectX::AudioEngine *								m_engine;

	// Collection of all loaded audio entries.  Items are never removed, so collection index is used
	// as an identifier
	Audio::AudioCollection								m_sounds;

	// Map of audio item names to the corresponding audio ID
	Audio::AudioIDMap									m_sound_name_map;

	// Count of total audio resources registered with this manager
	Audio::AudioID										m_audio_count;

	// Count of total audio instances that are currently active.  Updated on a periodic basis by the AudioManager
	Audio::AudioInstanceID								m_instance_count;

	// Flag indicating whether we are currently in an error state
	bool												m_in_error_state;

	// Clock time at which we should perform the next periodic audit
	unsigned int										m_next_audit_time;

	// Clock time at which the last object binding check was performed
	unsigned int										m_object_bindings_last_valid;

	// Single STATIC audio listener centered at the player position & orientation, used for all 3D audio calculations
	static DirectX::AudioListener						PLAYER_AUDIO_LISTENER;

	// Position and orientation of the player listener, stored locally for more efficient per-frame calculations
	// TODO: orientation removed, may not be required locally?
	XMFLOAT3											m_player_listener_position;

	// Base type volume modifiers based on audio item type
	float												m_type_volume_modifiers[AudioItem::AudioType::_COUNT];

	// Generates a new instance identifier, which is unique across all audio items
	static Audio::AudioInstanceIdentifier				AUDIO_INSTANCE_COUNTER;

	// Updates the instance_count total to reflect the creation of a new instance.  This count is however
	// an upper-bound estimate that is refined periodically when the audio manager checks for completed instances
	CMPINLINE void										RecordNewInstanceCreation()			{ ++m_instance_count; }

	// Identify any object-audio bindings that are no longer valid, terminate them and reclaim the instance resources
	// Accepts the time of the relevant audit verification as a parameter.  Any binding which does not have that
	// same clock time as it's "valid_at" parameter will be cleaned up
	void												TerminateExpiredObjectBindings(unsigned int verification_time);
	
	// Identify any new object-audio bindings that need to be made based on proximity to the player
	void												GenerateNewObjectBindings(void);

	// Generate new audio bindings for objects in the current player environment.  Bindings are established for new objects which get 
	// within audio_inner_range of the player
	void												GenerateNewEnvironmentObjectBindings(iSpaceObjectEnvironment *env, iEnvironmentObject *listener, 
																							 float audio_inner_range, float audio_outer_range, float volume_modifier);

	// Generate new audio bindings for objects in the current player environment.  Bindings are established for new objects which get
	// within audio_inner_range of the player
	void												GenerateNewSpaceObjectBindings(iObject *listener, float audio_inner_range, 
																					   float audio_outer_range, float volume_modifier);

	// Update any active object/audio bindings based on the current world state
	void												UpdateObjectAudioBindings(unsigned int verification_time);

	// Store position and orientation data for the player listener
	void												UpdatePlayerListenerPositionData(const FXMVECTOR position, const FXMVECTOR orientation);

	// Recalculates the final volume of all audio items and instances, based on a change to the audio manager volume settings
	void												RecalculateAllAudioItemVolumeSettings(void);

	// Squared versions of object binding audio for runtime efficiency
	static const float									SPACE_AUDIO_MAX_RANGE_SQ;
	static const float									SPACE_AUDIO_INNER_RANGE_SQ;
	static const float									ENV_AUDIO_MAX_RANGE_SQ;
	static const float									ENV_AUDIO_INNER_RANGE_SQ;
	static const float									ENV_SPACE_AUDIO_MAX_RANGE_SQ;
	static const float									ENV_SPACE_AUDIO_INNER_RANGE_SQ;


	// Vector of currently active object-audio bindings
	std::vector<AudioInstanceObjectBinding>	m_object_bindings;

};






