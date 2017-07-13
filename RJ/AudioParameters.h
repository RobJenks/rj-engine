#pragma once

#include "Audio.h"


// Structure holding reference to an audio item to be played, including any information required to instantiate it
struct AudioParameters
{
public:
	Audio::AudioID									AudioId;
	float											Volume;

	// Constructors
	AudioParameters(void) noexcept : AudioId(0U), Volume(0.0f) { }
	AudioParameters(Audio::AudioID audio_id, float volume) noexcept;
	AudioParameters(const std::string & audio_name, float volume) noexcept;
	AudioParameters(const AudioParameters & other) noexcept;

	// Set parameters, including validation
	void											SetAudioId(Audio::AudioID audio_id);
	void											SetVolume(float volume);

	// Null audio 
	static const AudioParameters					Null;
};


