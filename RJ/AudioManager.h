#pragma once

#include <Audio.h>
#include "ErrorCodes.h"
#include "AudioItem.h"

class AudioManager
{
public:

	// Static constants
	static const AudioItem::AudioID		NULL_AUDIO;
	static const std::string			NULL_AUDIO_NAME;
	static const float					DEFAULT_VOLUME;
	static const float					DEFAULT_PITCH_SHIFT;
	static const float					DEFAULT_PAN;

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

	// Play a one-shot sound effect based on the specified audio resource
	// Volume: default = 1
	// PitchShift: In the range [-1 +1], default with no shift = 0
	// Pan: In the range [-1 = full left, +1 = full right], default with no panning = 0
	void								Play(AudioItem::AudioID id, float volume, float pitch_shift, float pan); 
	void								Play(const std::string & name, float volume, float pitch_shift, float pan);

	// Play a one-shot sound effect based on the specified audio resource
	CMPINLINE void						Play(AudioItem::AudioID id) 		{ Play(id, DEFAULT_VOLUME, DEFAULT_PITCH_SHIFT, DEFAULT_PAN); }
	CMPINLINE void						Play(const std::string & name)		{ Play(name, DEFAULT_VOLUME, DEFAULT_PITCH_SHIFT, DEFAULT_PAN); }
	




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

	// Flag indicating whether we are currently in an error state
	bool												m_in_error_state;

};






