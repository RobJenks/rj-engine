#include "FastMath.h"
#include "AudioManager.h"

#include "AudioInstance.h"


// Default constructor
AudioInstance::AudioInstance(std::unique_ptr<SoundEffectInstance> effect_instance)
	:
	m_identifier(0U), m_starttime(0U), m_terminates(0U), m_is3d(false), 
	m_channel_count(1U), 
	m_base_volume_modifier(AudioManager::DEFAULT_VOLUME), m_volume_modifier(AudioManager::DEFAULT_VOLUME), m_base_volume(AudioManager::DEFAULT_VOLUME), 
	m_instance(std::move(effect_instance))
{
}

// Move constructor
AudioInstance::AudioInstance(AudioInstance && other) noexcept
	:
	m_instance(std::move(other.m_instance)), 
	m_starttime(other.m_starttime), m_terminates(other.m_terminates), m_is3d(other.m_is3d), 
	m_identifier(0U), m_channel_count(other.m_channel_count), 
	m_base_volume_modifier(other.m_base_volume_modifier), m_volume_modifier(other.m_volume_modifier), m_base_volume(other.m_base_volume), 
	m_emitter(std::move(other.m_emitter))
{
}


// Assign an audio resource to this instance 
void AudioInstance::AssignResource(std::unique_ptr<SoundEffectInstance> instance)
{
	// If we already have a resource assigned, make sure it has been terminated before replacing it
	Terminate();

	// Assign the new resources, or if instance == NULL this will simply remove the current resource
	m_instance = std::move(instance);
}

// Start instance playback
void AudioInstance::Start(unsigned int duration, bool loop)
{
	// Begin playback, if we have a valid resource
	if (m_instance.get())
	{
		m_instance.get()->Stop();
		UpdateVolume();
		m_instance.get()->Play(loop);
	}

	// Set the start and termination timers accordingly (if looping, we have no specific end time)
	m_starttime = Game::ClockMs;
	m_terminates = (loop ? (0U - 1) : (m_starttime + duration));
}

// Sets the 3D position of a 3D audio instance
void AudioInstance::SetPosition(const XMFLOAT3 & position)
{
	if (m_instance.get())
	{
		// Set position and a dummmy orientation.  We (currently) use only omnidirectional audio 
		// soures, so orientation of the emitter is not important.  Use XMFLOAT versions of pos
		// and orient since emitter library will otherwise have to XMStore from vector versions
		m_emitter.SetPosition(position);
		m_emitter.SetOrientation(FORWARD_VECTOR_F, UP_VECTOR_F);
		m_instance.get()->Apply3D(AudioManager::GetPlayerAudioListener(), m_emitter);
	}
}

// Updates the 3D position of a 3D audio instance.   Should only be called after the 
// initial call to Set3DPosition
void AudioInstance::UpdatePosition(const FXMVECTOR position, float time_delta)
{
	if (m_instance.get())
	{
		m_emitter.Update(position, UP_VECTOR, time_delta);
		m_instance.get()->Apply3D(AudioManager::GetPlayerAudioListener(), m_emitter);
	}
}

// Sets the base volume modifier for this instance.  Restricted to the range [0.0 1.0]
void AudioInstance::SetBaseVolumeModifier(float base_volume_modifier)
{
	m_base_volume_modifier = clamp(base_volume_modifier, 0.0f, 1.0f);
	UpdateVolume();
}

// Sets the volume modifier for this instance.  Restricted to the range [0.0 1.0]
void AudioInstance::SetVolumeModifier(float volume_modifier) 
{ 
	m_volume_modifier = clamp(volume_modifier, 0.0f, 1.0f);
	UpdateVolume();
}

// Sets the desired base volume for this audio instance
void AudioInstance::SetVolume(float volume) 
{ 
	m_base_volume = max(0.0f, volume); 
	UpdateVolume();
}

// Sets all volume-related parameters for this instance
void AudioInstance::SetVolumeParameters(float base_volume, float volume_modifier, float base_volume_modifier)
{
	m_base_volume = base_volume;
	m_volume_modifier = volume_modifier;
	m_base_volume_modifier = base_volume_modifier;
	UpdateVolume();
}

// Update the instance volume.  Accepts a base volume modifier (based on audio item type) as input and
// calculates as final volume = (base_modifier * volume_modifier * base_volume)
void AudioInstance::UpdateVolume(void)
{
	if (m_instance.get())
	{
		// m_base_volume_modifier = base modifier based on audio type (effect vs music vs ...)
		// m_volume_modifier = instance-specific modifier (e.g. for interior vs exterior sounds)
		// m_base_volume = desired base volume before any modification
		float volume = (m_base_volume_modifier * m_volume_modifier * m_base_volume);
		m_instance.get()->SetVolume(max(0.0f, volume));
	}
}


// Assigns a new unique identifier to this instance, which is unique across all audio items
void AudioInstance::AssignNewIdentifier(void) 
{ 
	m_identifier = AudioManager::GetNewInstanceIdentifier(); 
}

// Set audio format properties for this instance
void AudioInstance::SetAudioFormat(UINT32 channel_count)
{
	// Store required data
	m_channel_count = channel_count;

	// Apply any updates based on this data
	m_emitter.ChannelCount = m_channel_count;
}


// Stop any audio that is currently playing and release the instance.  Resources are retained
void AudioInstance::Terminate(void)
{
	// Stop any playback immediately
	if (m_instance.get()) m_instance.get()->Stop(true);
	
	// Mark this instance as available for cleanup immediately
	m_terminates = 0U;
}

// Destructor; releases any resources currently held by this audio instance
AudioInstance::~AudioInstance(void) noexcept
{
	// Stop any audio that is currently playing, so that the instance can be deallocated safely
	Terminate();

	// Resources will be deallocated automatically by unique_ptr<instance> 
}




