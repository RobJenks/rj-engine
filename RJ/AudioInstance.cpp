#include "FastMath.h"

#include "AudioInstance.h"


// Default constructor
AudioInstance::AudioInstance(std::unique_ptr<SoundEffectInstance> effect_instance)
	:
	m_starttime(0U), m_terminates(0U), m_is3d(false), m_position(NULL_FLOAT4), 
	m_instance(std::move(effect_instance))
{
}

// Move constructor
AudioInstance::AudioInstance(AudioInstance && other) noexcept
	:
	m_instance(std::move(other.m_instance)), 
	m_starttime(other.m_starttime), m_terminates(other.m_terminates), m_is3d(other.m_is3d), 
	m_position(std::move(other.m_position))
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
void AudioInstance::Start(unsigned int duration)
{
	// Begin playback, if we have a valid resource
	if (m_instance.get())
	{
		m_instance.get()->Stop();
		m_instance.get()->Play();
	}

	// Set the start and termination timers accordingly
	m_starttime = Game::ClockMs;
	m_terminates = (m_starttime + duration);
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




