#pragma once

#include <Audio.h>
#include "CompilerSettings.h"
#include "DX11_Core.h"


class AudioInstance
{
public:

	// Collection type and ID used to uniquely identify audio instances
	typedef std::vector<AudioInstance>					AudioInstanceCollection;
	typedef std::vector<AudioInstance>::size_type		AudioInstanceID;


	// Default constructor
	AudioInstance(std::unique_ptr<SoundEffectInstance> effect_instance);

	// Copy constructor & assignment are disallowed
	CMPINLINE										AudioInstance(const AudioInstance & other) = delete;
	CMPINLINE										AudioInstance & operator=(const AudioInstance & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than
	// copied by STL containers
	AudioInstance(AudioInstance && other) noexcept;

	// Return key parameters
	CMPINLINE unsigned int							GetStartTime(void) const			{ return m_starttime; }
	CMPINLINE unsigned int							GetTerminationTime(void) const		{ return m_terminates; }
	CMPINLINE bool									Is3DAudio(void) const				{ return m_is3d; }

	// Indicates whether this instance is currently active, based on its termination time
	CMPINLINE bool									IsActive(void) const				{ return (m_terminates > Game::ClockMs); }

	// Sets the flag that indicates whether this instance supports 3D audio
	CMPINLINE void									Set3DSupportFlag(bool supports_3d)	{ m_is3d = supports_3d; }

	// Sets the 3D position of a 3D audio instance
	void											SetPosition(const XMFLOAT3 & position);

	// Updates the 3D position of a 3D audio instance.   Should only be called after the 
	// initial call to Set3DPosition
	void											UpdatePosition(const FXMVECTOR position, float time_delta);

	// Assign an audio resource to this instance 
	void											AssignResource(std::unique_ptr<SoundEffectInstance> instance);

	// Set audio format properties for this instance
	void											SetAudioFormat(UINT32 channel_count);

	// Start instance playback
	void											Start(unsigned int duration, bool loop);


	// Stop any audio that is currently playing and release the instance.  Resources are retained
	void											Terminate(void);

	// Destructor; releases any resources currently held by this audio instance.  No-exception guarantee 
	// to ensure these objects are moved rather than copied by STL containers
	~AudioInstance(void) noexcept;

private:

	std::unique_ptr<DirectX::SoundEffectInstance>	m_instance;

	unsigned int									m_starttime;		// ms, time at which this instance started playback
	unsigned int									m_terminates;		// ms, time at which this instance will terminate (if applicable, otherwise UINT_MAX)
	UINT32											m_channel_count;	

	// Applicable to 3D sounds only
	bool											m_is3d;	
	DirectX::AudioEmitter							m_emitter;

};




