#include <string>
#include "HashFunctions.h"
#include "ErrorCodes.h"
#include "FastMath.h"
#include "Logging.h"
#include "AudioManager.h"
#include "AudioItem.h"


// Constructor with all mandatory parameters
AudioItem::AudioItem(AudioItem::AudioID id, const std::string & name, AudioType type, const std::string & filename, bool default_loop_state)
	:
	m_id(id), m_name(name), m_type(type), m_filename(filename), m_duration(0U), m_channel_count(1U), 
	m_default_loop(default_loop_state), m_instance_limit(AudioManager::DEFAULT_AUDIO_ITEM_INSTANCE_LIMIT), 
	m_allow_new_instance_slots(true)
{
}

// Move constructor; no-exception guarantee to ensure these objects are moved rather than
// copied by STL containers
AudioItem::AudioItem(AudioItem && other) noexcept
	:
	m_id(other.m_id), m_name(std::move(other.m_name)), m_type(other.m_type), 
	m_filename(std::move(other.m_filename)), m_duration(other.m_duration), m_default_loop(other.m_default_loop), 
	m_channel_count(other.m_channel_count), m_instance_limit(other.m_instance_limit), 
	m_allow_new_instance_slots(other.m_allow_new_instance_slots), 
	m_effect(std::move(other.m_effect)), 
	m_instances(std::move(other.m_instances))
{
}


// Assign an audio resource to this item
Result AudioItem::AssignResource(SoundEffect *resource)
{
	// Audio resource can only be assigned once; it can never be reassigned
	if (m_effect.get() != NULL) return ErrorCodes::CannotReassignExistingAudioItemResource;

	// A null value can be assigned if we want to release the audio resource
	if (!resource)
	{
		m_effect.release();
		m_duration = 0U;

		return ErrorCodes::NoError;
	}
	
	// We now own this new resource; any existing resource will be deallocated
	m_effect.reset(resource);

	// Extract and store information from the audio resource
	m_duration = (unsigned int)resource->GetSampleDurationMS();
	m_channel_count = (UINT32)resource->GetFormat()->nChannels;

	return ErrorCodes::NoError;
}



// Create a new instance of this audio item, if posssible.  Returns identifier for the new
// instance, or NULL_INSTANCE if one could not be created
AudioInstance::AudioInstanceIdentifier AudioItem::CreateInstance(bool loop, float volume_modifier)
{
	// Find an available instance slot.  This should ALWAYS return a valid slot
	AudioInstance::AudioInstanceID id = GetAvailableInstanceSlot(false);
	assert( IsValidInstanceID(id) );

	// Assign a new identifier to this instance, which is unique across all audio items
	m_instances[id].AssignNewIdentifier();

	// Populate the instance with required details and start playback
	m_instances[id].SetVolumeModifier(volume_modifier);
	m_instances[id].Start(m_duration, loop);

	// Return success
	return m_instances[id].GetIdentifier();
}

// Create a new 3D instance of this audio item, if possible.  Returns identifier for the new
// instance, or NULL_INSTANCE if one could not be created
AudioInstance::AudioInstanceIdentifier AudioItem::Create3DInstance(bool loop, const XMFLOAT3 & position, float volume_modifier)
{
	// Find an available instance slot.  This should ALWAYS return a valid slot
	AudioInstance::AudioInstanceID id = GetAvailableInstanceSlot(true);
	assert( IsValidInstanceID(id) );

	// Assign a new identifier to this instance, which is unique across all audio items
	m_instances[id].AssignNewIdentifier();

	// Populate the instance with required details and start playback
	m_instances[id].SetPosition(position);
	m_instances[id].SetVolumeModifier(volume_modifier);
	m_instances[id].Start(m_duration, loop);

	// Return success
	return m_instances[id].GetIdentifier();
}

// Returns a pointer to a specific instance, or NULL if none exists with the given ID
// TOOD: REMOVE THIS once we are no longer using it for debug purposes; should not expose individual instances via ID
AudioInstance * AudioItem::GetInstance(AudioInstance::AudioInstanceID id)
{
	return (IsValidInstanceID(id) ? &(m_instances[id]) : NULL);
}


// Identifies an instance slot for this audio item.  Will ALWAYS return a slot ID, even if it requires
// terminating an existing instance.  In this case, the oldest instance will be selected for termination
// Flag indicates whether the slot must support 3D audio; may result in a new resource instance being
// created if no suitable slots are available
AudioInstance::AudioInstanceID AudioItem::GetAvailableInstanceSlot(bool requires_3d_support)
{
	AudioInstance::AudioInstanceID oldest_instance = 0U;	// To be safe; will always replace a valid instance
	AudioInstance::AudioInstanceID oldest_starttime = (0U - 1);
	AudioInstance::AudioInstanceID inactive_but_incompatible_properties = (0U - 1);

	AudioInstance::AudioInstanceID count = m_instances.size();
	for (AudioInstance::AudioInstanceID i = 0; i < count; ++i)
	{
		// If this slot is not active we may be able to use it
		if (!m_instances[i].IsActive())
		{
			if (m_instances[i].Is3DAudio() == requires_3d_support)
			{
				AUDIO_ITEM_DEBUG_LOG(concat("AudioItem ")(m_id)(" [\"")(m_name)("\"]: Using inactive slot ")(i)("\n").str().c_str());
				return i;
			}
			else inactive_but_incompatible_properties = i;
		}

		// If this is the new oldest instance, record it for later
		if (m_instances[i].GetStartTime() < oldest_starttime)
		{
			oldest_instance = i;
			oldest_starttime = m_instances[i].GetStartTime();
		}
	}

	// If we got here, there are no existing slots that are suitable and inactive.  See if we have scope to 
	// extend the instance collection (and make sure this is not currently disallowed)
	if (count < m_instance_limit && m_allow_new_instance_slots)
	{
		AudioInstance::AudioInstanceID new_id = AllocateNewInstanceSlot(requires_3d_support);

		AUDIO_ITEM_DEBUG_LOG(concat("AudioItem ")(m_id)(" [\"")(m_name)("\"]: No existing inactive slots; extending to new slot ")(new_id)("\n").str().c_str());
		return new_id;
	}

	// If we found a slot which was inactive, but had incompatible properties, replace the resource instance
	// now and use this slot
	if (IsValidInstanceID(inactive_but_incompatible_properties))
	{
		m_instances[inactive_but_incompatible_properties].Set3DSupportFlag(requires_3d_support);
		m_instances[inactive_but_incompatible_properties].AssignResource(
			std::move(CreateNewEffectInstance(requires_3d_support)));

		AUDIO_ITEM_DEBUG_LOG(concat("AudioItem ")(m_id)(" [\"")(m_name)("\"]: Using existing slot ")(inactive_but_incompatible_properties)(", but had to override properties to [3d=")(requires_3d_support)("]\n").str().c_str());
		return inactive_but_incompatible_properties;
	}

	// There are no inactive slots at all, and we cannot extend the instance collection.  We need to replace
	// the oldest instance.  Update it to make properties compatible first if required.  Note this is 
	// guaranteed to always make a slot available, so we can always return a slot here as a last resort
	AUDIO_ITEM_DEBUG_LOG(concat("AudioItem ")(m_id)(" [\"")(m_name)("\"]: No other option; had to terminate and reuse active slot ")(oldest_instance)("\n").str().c_str());
	if (m_instances[oldest_instance].Is3DAudio() != requires_3d_support)
	{
		AUDIO_ITEM_DEBUG_LOG(concat("AudioItem ")(m_id)(" [\"")(m_name)("\"]: Resource also had to be overridden with properties [3d=")(requires_3d_support)("]\n").str().c_str());
		m_instances[oldest_instance].Set3DSupportFlag(requires_3d_support);
		m_instances[oldest_instance].AssignResource(
			std::move(CreateNewEffectInstance(requires_3d_support)));
	}
	return oldest_instance;
}

// Ensures that at least one instance slot is available within this item, by terminating existing audio
// instances if necessary
void AudioItem::MakeInstanceAvailable(bool requires_3d_support)
{
	// Simply call the internal method to retrieve the next available instance slot, since it will
	// make space available if necessary to ensure a slot is available
	GetAvailableInstanceSlot(requires_3d_support);
}

// Allocates a new slot in the instance vector.  Returns the instance ID of the newly-allocated item
AudioInstance::AudioInstanceID AudioItem::AllocateNewInstanceSlot(bool requires_3d_support)
{
	m_instances.push_back(AudioInstance(std::move(CreateNewEffectInstance(requires_3d_support))));

	// Assign any additional required data to the instance
	AudioInstance::AudioInstanceID new_id = (m_instances.size() - 1U);
	m_instances[new_id].Set3DSupportFlag(requires_3d_support);
	m_instances[new_id].SetAudioFormat(m_channel_count);

	return new_id;
}

// Generates a new effect instance from the base audio resource
std::unique_ptr<SoundEffectInstance> AudioItem::CreateNewEffectInstance(bool include_3d_support)
{
	if (!m_effect.get()) return NULL;

	return m_effect.get()->CreateInstance(include_3d_support ?
		(DirectX::SOUND_EFFECT_INSTANCE_FLAGS::SoundEffectInstance_Use3D) :
		(DirectX::SOUND_EFFECT_INSTANCE_FLAGS::SoundEffectInstance_Default)
	);
}

// Returns the number of instances that are currently active, i.e. which have not yet reached their 
// termination time, or those which are looping indefinitely
AudioInstance::AudioInstanceID AudioItem::GetActiveInstanceCount(void) const
{
	AudioInstance::AudioInstanceID count = 0U;

	AudioInstance::AudioInstanceCollection::const_iterator it_end = m_instances.end();
	for (AudioInstance::AudioInstanceCollection::const_iterator it = m_instances.begin(); it != it_end; ++it)
	{
		if ((*it).IsActive()) ++count;
	}

	return count;
}

// Terminate an instance based on its instance identifier.  Returns a flag indicating whether the instance was found & removed
bool AudioItem::TerminateInstanceByIdentifier(AudioInstance::AudioInstanceIdentifier identifier)
{
	AudioInstance::AudioInstanceCollection::iterator it = std::find_if(m_instances.begin(), m_instances.end(),
		[identifier](const AudioInstance & instance) { return (instance.GetIdentifier() == identifier); });

	if (it != m_instances.end())
	{
		(*it).Terminate();
		return true;
	}

	return false;
}

// Internal method; determines whether the given instance ID is valid for this audio item
bool AudioItem::IsValidInstanceID(AudioInstance::AudioInstanceID id) const
{
	return (id >= 0 && id < m_instances.size());
}

// Overrides the maximum instance count for this specific audio item.  Will be constrained to the 
// range [1 AudioManager::HARD_INSTANCE_LIMIT_PER_AUDIO].  It MUST be possible for audio items to 
// support >= 1 instance otherwise the new slot identification process will throw an exception
void AudioItem::OverrideInstanceLimit(AudioInstance::AudioInstanceID instance_limit)
{
	if (instance_limit < 1) 
		Game::Log << LOG_WARN << "Prevented invalid audio instance limit override of " << instance_limit << "; minimum is 1\n";
	if (instance_limit > AudioManager::HARD_INSTANCE_LIMIT_PER_AUDIO)
		Game::Log << LOG_WARN << "Prevented invalid audio instance limit override of " << instance_limit << "; maximum is " << AudioManager::HARD_INSTANCE_LIMIT_PER_AUDIO << "\n";

	m_instance_limit = clamp(instance_limit, 1, AudioManager::HARD_INSTANCE_LIMIT_PER_AUDIO);
}


// Destructor; no-exception guarantee to ensure these objects are moved rather than
// copied by STL containers
AudioItem::~AudioItem(void) noexcept
{
	// unique_ptr will handle deallocation of the sound effect if required
}

// Translate a sound type from its string representation
AudioItem::AudioType AudioItem::TranslateAudioTypeFromString(const std::string & name)
{
	HashVal hash = HashString(name);
	
	// Return the corresponding audio type
	if (hash == HashedStrings::H_Music)				return AudioType::Music;
	else if (hash == HashedStrings::H_Voice)		return AudioType::Voice;

	// Default
	else											return AudioType::Effect;
}

// Translate a sound type to its string representation
std::string AudioItem::TranslateAudioTypeToString(AudioItem::AudioType type)
{
	switch (type)
	{
		case AudioType::Music:						return HashedStrings::H_Effect.Text;
		case AudioType::Voice:						return HashedStrings::H_Voice.Text;
		default:									return HashedStrings::H_Effect.Text;
	}
}





