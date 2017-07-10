#pragma once

#include <Audio.h>
#include "CompilerSettings.h"
#include "DX11_Core.h"
#include "GameVarsExtern.h"
#include "Audio.h"

class AudioInstance
{
public:

	// Collection type and ID used to uniquely identify audio instances
	// NOTE: AudioInstanceID is NOT a persistent identifier, and may change between frames 
	// based on actions by the AudioManager
	typedef std::vector<AudioInstance>					AudioInstanceCollection;
	typedef std::vector<AudioInstance>::size_type		AudioInstanceID;

	// Type used to uniquely number instances as they are generated.  Unique across all audio items
	typedef size_t										AudioInstanceIdentifier;

	// Default constructor
	AudioInstance(std::unique_ptr<SoundEffectInstance> effect_instance);

	// Copy constructor & assignment are disallowed
	CMPINLINE										AudioInstance(const AudioInstance & other) = delete;
	CMPINLINE										AudioInstance & operator=(const AudioInstance & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than
	// copied by STL containers
	AudioInstance(AudioInstance && other) noexcept;

	// Return key parameters
	CMPINLINE AudioInstanceIdentifier				GetIdentifier(void) const			{ return m_identifier; }
	CMPINLINE unsigned int							GetStartTime(void) const			{ return m_starttime; }
	CMPINLINE unsigned int							GetTerminationTime(void) const		{ return m_terminates; }
	CMPINLINE bool									Is3DAudio(void) const				{ return m_is3d; }
	CMPINLINE float									GetVolumeModifier(void) const		{ return m_volume_modifier; }

	// Indicates whether this instance is currently active, based on its termination time
	CMPINLINE bool									IsActive(void) const				{ return (m_terminates > Game::ClockMs); }

	// Sets the flag that indicates whether this instance supports 3D audio
	CMPINLINE void									Set3DSupportFlag(bool supports_3d)	{ m_is3d = supports_3d; }

	// Returns the current position of the 3D audio instance
	CMPINLINE XMFLOAT3								GetPosition(void) const				{ return static_cast<XMFLOAT3>(m_emitter.Position); }

	// Sets the 3D position of a 3D audio instance
	void											SetPosition(const XMFLOAT3 & position);

	// Sets the base volume modifier for this instance.  Restricted to the range [0.0 1.0]
	void											SetBaseVolumeModifier(float base_volume_modifier);

	// Sets the volume modifier for this instance.  Restricted to the range [0.0 1.0]
	void											SetVolumeModifier(float volume_modifier);

	// Sets the desired volume for this audio instance
	void											SetVolume(float volume);

	// Sets all volume-related parameters for this instance
	void											SetVolumeParameters(float base_volume, float volume_modifier = 1.0f, float base_volume_modifier = 1.0f);

	// Updates the 3D position of a 3D audio instance.   Should only be called after the 
	// initial call to Set3DPosition
	void											UpdatePosition(const FXMVECTOR position, float time_delta);

	// Returns the effective maximum distance (SQ), beyond which this instance is considered
	// inaudible and will be terminated
	CMPINLINE float									GetMaximumAudibleDistanceSq(void) const { return m_max_audible_distance_sq; }

	// Assigns a new unique identifier to this instance, which is unique across all audio items
	void											AssignNewIdentifier(void);

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

	AudioInstanceIdentifier							m_identifier;		// Unique across all audio items

	std::unique_ptr<DirectX::SoundEffectInstance>	m_instance;

	unsigned int									m_starttime;		// ms, time at which this instance started playback
	unsigned int									m_terminates;		// ms, time at which this instance will terminate (if applicable, otherwise UINT_MAX)
	UINT32											m_channel_count;	

	// Final instance volume = (base volume * volume modifier)
	float											m_base_volume;			// Target volume for the instance
	float											m_volume_modifier;		// Modifier to instance volume (e.g. interior/exterior) applied per-instance
	float											m_base_volume_modifier;	// Base modifier, based on audio item type (interior vs exterior vs ...)

	// Effective maximum distance (SQ), beyond which this instance should be considered
	// inaudible and will be terminated
	float											m_max_audible_distance_sq;

	// Applicable to 3D sounds only
	bool											m_is3d;	
	DirectX::AudioEmitter							m_emitter;

	// Recalculate audio instance parameters based on a change to the instance data
	void											RecalculateAudioParameters(void);

};




