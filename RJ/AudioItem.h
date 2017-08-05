#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <Audio.h>
#include "CompilerSettings.h"
#include "AudioInstance.h"
#include "Audio.h"

class AudioItem
{

public:

	// Debug logging flag that can be set if required
	// #define ENABLE_AUDIO_ITEM_DEBUG_LOGGING

	// Enumeration of possible audio types.  Default = Effect
	enum AudioType { Effect = 0, Music, Voice, _COUNT };

	// Constructor with all mandatory parameters
	AudioItem(Audio::AudioID id, const std::string & name, AudioType type, const std::string & filename, bool default_loop_state, float default_volume);

	// Copy construction and assignment is disallowed
	CMPINLINE AudioItem(const AudioItem & other) = delete;
	CMPINLINE AudioItem & operator=(const AudioItem & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than
	// copied by STL containers
	AudioItem(AudioItem && other) noexcept;


	// Return key parameters
	CMPINLINE Audio::AudioID							GetID(void) const							{ return m_id; }
	CMPINLINE std::string								GetName(void) const							{ return m_name; }
	CMPINLINE AudioType									GetType(void) const							{ return m_type; }
	CMPINLINE std::string								GetFilename(void) const						{ return m_filename; }
	CMPINLINE unsigned int								GetDuration(void) const						{ return m_duration; }
	CMPINLINE Audio::AudioInstanceID					GetInstanceLimit(void) const				{ return m_instance_limit; }

	// Return a reference to the sound effect object
	CMPINLINE DirectX::SoundEffect *					GetEffect(void)								{ return m_effect.get(); }

	// Default base volume for this audio item
	CMPINLINE float										GetDefaultBaseVolume(void) const			{ return m_default_base_volume; }

	// Base volume modifier for the item based on its audio item type.  Restricted  to the range [0.0 1.0].  Automatically
	// recalculates volume of all active instances when changed
	CMPINLINE float										GetBaseTypeVolumeModifier(void) const		{ return m_type_volume_modifier; }
	void												SetBaseTypeVolumeModifier(float modifier);

	// Assign an audio resource to this item
	Result												AssignResource(SoundEffect *resource);

	// Indicates whether the resources for this audio item have already been loaded
	CMPINLINE bool										ResourcesLoaded(void) const { return (m_effect.get() != NULL); }

	// Releases any audio resources currently held by this item
	CMPINLINE  void										ReleaseResources(void)		{ m_effect.release(); }


	// Create a new instance of this audio item, if posssible.  Returns identifier for the new
	// instance, or NULL_INSTANCE if one could not be created
	Audio::AudioInstanceIdentifier						CreateInstance(bool loop, float base_volume, float volume_modifier);
	CMPINLINE Audio::AudioInstanceIdentifier			CreateInstance(float base_volume, float volume_modifier) {
		return CreateInstance(m_default_loop, base_volume, volume_modifier);
	}
	CMPINLINE Audio::AudioInstanceIdentifier			CreateInstance(float volume_modifier) {
		return CreateInstance(m_default_loop, m_default_base_volume, volume_modifier);
	}

	// Create a new 3D instance of this audio item, if possible.  Returns identifier for the new
	// instance, or NULL_INSTANCE if one could not be created
	Audio::AudioInstanceIdentifier						Create3DInstance(bool loop, const XMFLOAT3 & position, float base_volume, float volume_modifier);
	CMPINLINE Audio::AudioInstanceIdentifier			Create3DInstance(const XMFLOAT3 & position, float base_volume, float volume_modifier) {
		return Create3DInstance(m_default_loop, position, base_volume, volume_modifier); 
	}
	CMPINLINE Audio::AudioInstanceIdentifier			Create3DInstance(const XMFLOAT3 & position, float volume_modifier) {
		return Create3DInstance(m_default_loop, position, m_default_base_volume, volume_modifier);
	}

	// Returns a pointer to a specific instance, or NULL if none exists with the given ID // TODO: REMOVE
	AudioInstance *										GetInstance(Audio::AudioInstanceID id);

	// Returns a pointer to a specific instance with the given identifier, or NULL if none exists
	AudioInstance *										GetInstanceByIdentifier(Audio::AudioInstanceIdentifier identifier);


	// Specify whether this item is allowed to extend its instance collection when no suitable inactive slots are available
	CMPINLINE void										AllowNewInstanceSlots(void) { m_allow_new_instance_slots = true; }
	CMPINLINE void										DisallowNewInstanceSlots(void) { m_allow_new_instance_slots = false; }

	// Default loop state of all instances created from this audio item
	CMPINLINE bool										IsLoopingByDefault(void) const { return m_default_loop; }
	CMPINLINE void										SetDefaultLoopState(bool loop) { m_default_loop = loop; }

	// Terminate an instance based on its instance identifier.  Returns a flag indicating whether the instance was found & removed
	bool												TerminateInstanceByIdentifier(Audio::AudioInstanceIdentifier identifier);

	// Returns the number of instances that are currently active, i.e. which have not yet reached their 
	// termination time, or those which are looping indefinitely
	Audio::AudioInstanceID								GetActiveInstanceCount(void) const;

	// Ensures that at least one instance slot is available within this item, by terminating existing audio
	// instances if necessary
	void												MakeInstanceAvailable(bool requires_3d_support);

	// Allocates a new slot in the instance vector
	Audio::AudioInstanceID								AllocateNewInstanceSlot(bool requires_3d_support);

	// Overrides the maximum instance count for this specific audio item.  Will be constrained to the 
	// range [1 AudioManager::HARD_INSTANCE_LIMIT_PER_AUDIO].  It MUST be possible for audio items to 
	// support >= 1 instance otherwise the new slot identification process will throw an exception
	void												OverrideInstanceLimit(Audio::AudioInstanceID instance_limit);

	// Destructor; no-exception guarantee to ensure these objects are moved rather than
	// copied by STL containers
	~AudioItem(void) noexcept;

	// Translate a sound type to/from its string representation
	static AudioType									TranslateAudioTypeFromString(const std::string & name);
	static std::string									TranslateAudioTypeToString(AudioType type);

private:

	Audio::AudioID										m_id;
	std::string											m_name;
	AudioType											m_type;
	std::string											m_filename;
	unsigned int										m_duration;		// ms, not accounting for any looping
	bool												m_default_loop;	// Indicates whether the sound loops by default

	// Definition of the audio resource
	std::unique_ptr<DirectX::SoundEffect>				m_effect;

	// Base volume modifier for the item based on its audio item type
	float												m_type_volume_modifier;

	// Default base volume for all instances of this audio item
	float												m_default_base_volume;

	// Audio format properties for the audio resource
	UINT32												m_channel_count;

	// List of instances for this audio resource
	Audio::AudioInstanceCollection						m_instances;

	// Maximum allowed instances of this audio item.  Defaults to AudioManager::DEFAULT_AUDIO_ITEM_INSTANCE_LIMIT
	// unless a value is manually-specified
	Audio::AudioInstanceID								m_instance_limit;

	// Indicates whether this item is currently allowed to extend the instance collection, if no suitable
	// inactive slot can be found.  This extension still observes the instance limit.  The purpose of this
	// flag is to allow the audio manager to request a new slot without any new slot allocation, in the 
	// situation where we have exceeded the global audio instance limit (but this item is still within 
	// its individual limit)
	bool												m_allow_new_instance_slots;

	// Internal method; determines whether the given instance ID is valid for this audio item
	bool												IsValidInstanceID(Audio::AudioInstanceID id) const;

	// Identifies an instance slot for this audio item.  Will ALWAYS return a slot ID, even if it requires
	// terminating an existing instance.  In this case, the oldest instance will be selected for termination
	// Flag indicates whether the slot must support 3D audio; may result in a new resource instance being
	// created if no suitable slots are available
	Audio::AudioInstanceID								GetAvailableInstanceSlot(bool requires_3d_support);

	// Generates a new effect instance from the base audio resource
	std::unique_ptr<SoundEffectInstance>				CreateNewEffectInstance(bool include_3d_support);

	// Generate macros based on debug logging flag
#	if defined(_DEBUG) && defined(ENABLE_AUDIO_ITEM_DEBUG_LOGGING)
#		define AUDIO_ITEM_DEBUG_LOG(str) Game::Log << LOG_DEBUG << str << "\n";
#	else
#		define AUDIO_ITEM_DEBUG_LOG(str) 
#	endif
};
