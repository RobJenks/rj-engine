#include <string>
#include "HashFunctions.h"
#include "ErrorCodes.h"
#include "AudioItem.h"

// Constructor with all mandatory parameters
AudioItem::AudioItem(AudioItem::AudioID id, const std::string & name, AudioType type, const std::string & filename)
	:
	m_id(id), m_name(name), m_type(type), m_filename(filename), m_duration(0U)
{
}

// Move constructor; no-exception guarantee to ensure these objects are moved rather than
// copied by STL containers
AudioItem::AudioItem(AudioItem && other) noexcept
	:
	m_id(std::move(other.m_id)), m_name(std::move(other.m_name)), m_type(std::move(other.m_type)), 
	m_filename(std::move(other.m_filename)), m_effect(std::move(other.m_effect))
{
}


// Assign an audio resource to this item
Result AudioItem::AssignResource(SoundEffect *resource)
{
	// A null value can be assigned if we want to release the audio resource
	if (!resource)
	{
		m_effect.release();
		m_duration = 0U;

		return ErrorCodes::NoError;
	}
	
	// We now own this new resource; any existing resource will be deallocated
	m_effect.reset(resource);
	m_duration = resource->GetSampleDurationMS();

	return ErrorCodes::NoError;
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





