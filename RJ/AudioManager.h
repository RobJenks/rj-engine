#pragma once

#include <Audio.h>
#include "ErrorCodes.h"
#include "AudioItem.h"

class AudioManager
{
public:

	// Static constants
	static const AudioItem::AudioID					NULL_AUDIO;
	static const std::string						NULL_AUDIO_NAME;
	static const AudioInstance::AudioInstanceID		GLOBAL_AUDIO_INSTANCE_LIMIT;			// Max instances across all audio items
	static const AudioInstance::AudioInstanceID		DEFAULT_AUDIO_ITEM_INSTANCE_LIMIT;		// Default max instances per audio item
	static const AudioInstance::AudioInstanceID		HARD_INSTANCE_LIMIT_PER_AUDIO;			// Hard limit for instance count per audio, cannot be overridden
	static const float								DEFAULT_VOLUME;
	static const float								DEFAULT_PITCH_SHIFT;
	static const float								DEFAULT_PAN;

	// Default constructor
	AudioManager(void);

	// Initialise all audio manager resources
	Result								Initialise(void);

	// Per-frame update method
	void								Update(void);

	// Registers a new sound with the audio manager.  Flag 'load_resouce' determines whether 
	// the audio resource will be loaded immediately.  If it is not loaded now, LoadResource()
	// must be called on the specific audio item before it can be used
	Result								RegisterSound(const char *name, const char *type, const char *filename, bool load_resource);

	// Load the resource associated with the given audio item
	Result								LoadAudioResource(AudioItem::AudioID id);
	Result								LoadAudioResource(const std::string & name);

	// Load all audio resources that have not yet been loaded
	Result								LoadAllAudioResources(void);

	// Release the specified audio resource.  Audio item is preserved, but resource must be
	// re-loaded before it can be used again
	void								ReleaseAudioResource(AudioItem::AudioID id);

	// Releases all audio resources.  Audio items are preserved, but resources must be re-loaded 
	// before they can be used
	void								ReleaseAllAudioResources(void);

	// Determines whether the supplied audio ID is valid
	CMPINLINE bool						IsValidID(AudioItem::AudioID id) const { return (id > 0U && id < m_audio_count); }

	// Returns the ID of the audio item with the given name, or NULL_AUDIO if no such item exists
	CMPINLINE AudioItem::AudioID		GetAudioID(const std::string & name) const 
	{
		AudioItem::AudioIDMap::const_iterator it = m_sound_name_map.find(name);
		return (it != m_sound_name_map.end() ? it->second : NULL_AUDIO);
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
	void								Play(AudioItem::AudioID id, float volume, float pitch_shift, float pan); 
	void								Play(const std::string & name, float volume, float pitch_shift, float pan);

	// Play a one-shot sound effect based on the specified audio resource
	CMPINLINE void						Play(AudioItem::AudioID id) 		{ Play(id, DEFAULT_VOLUME, DEFAULT_PITCH_SHIFT, DEFAULT_PAN); }
	CMPINLINE void						Play(const std::string & name)		{ Play(name, DEFAULT_VOLUME, DEFAULT_PITCH_SHIFT, DEFAULT_PAN); }
	
	// Ensures that a slot is available for a new audio instance.  Will terminate an instance of the current 
	// audio item if necessary to remain under the global audio instance limit
	void								EnsureInstanceIsAvailable(AudioItem::AudioID id, bool requires_3d_support);

	// Create a new instance of an audio item, if posssible.  Returns non-zero if instantiation fails
	Result								CreateInstance(AudioItem::AudioID id);
	Result								CreateInstance(const std::string & name) { return CreateInstance(GetAudioID(name)); }

	// Create a new 3D instance of an audio item, if possible.  Returns non-zero if instantiation fails
	Result								Create3DInstance(AudioItem::AudioID id, const XMFLOAT3 & position);
	Result								Create3DInstance(const std::string & name, const XMFLOAT3 & position) { return Create3DInstance(GetAudioID(name), position); }


	// Update the player audio listener to the current player position and orientation
	void								UpdatePlayerAudioListener(void);

	// Pauses global sound playback
	CMPINLINE void						PausePlayback(void)					{ m_engine->Suspend(); }

	// Resumes global sound playback
	CMPINLINE void						ResumePlayback(void)				{ m_engine->Resume(); }

	// Shutdown method to deallocate all audio manager resources
	Result								Shutdown(void);

	// Default destructor
	~AudioManager(void);


private:

	// Core audio engine component
	DirectX::AudioEngine *								m_engine;

	// Collection of all loaded audio entries.  Items are never removed, so collection index is used
	// as an identifier
	AudioItem::AudioCollection							m_sounds;

	// Map of audio item names to the corresponding audio ID
	AudioItem::AudioIDMap								m_sound_name_map;

	// Count of total audio resources registered with this manager
	AudioItem::AudioID									m_audio_count;

	// Count of total audio instances that are currently active.  Updated on a periodic basis by the AudioManager
	AudioItem::AudioID									m_instance_count;

	// Flag indicating whether we are currently in an error state
	bool												m_in_error_state;

	// Single STATIC audio listener centered at the player position & orientation, used for all 3D audio calculations
	static DirectX::AudioListener						PLAYER_AUDIO_LISTENER;

	// Updates the instance_count total to reflect the creation of a new instance.  This count is however
	// an upper-bound estimate that is refined periodically when the audio manager checks for completed instances
	CMPINLINE void										RecordNewInstanceCreation()			{ ++m_instance_count; }

};






