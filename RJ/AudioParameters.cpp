#include "CoreEngine.h"
#include "AudioManager.h"
#include "Audio.h"
#include "AudioParameters.h"


// Initialise static variables
const AudioParameters AudioParameters::Null(AudioManager::NULL_AUDIO, 0.0f);

// Constructor
AudioParameters::AudioParameters(Audio::AudioID audio_id, float volume) noexcept
{
	SetAudioId(audio_id);
	SetVolume(volume);
}

// Constructor
AudioParameters::AudioParameters(const std::string & audio_name, float volume) noexcept
{
	Audio::AudioID id = Game::Engine->GetAudioManager()->GetAudioID(audio_name);
	SetAudioId(id);
	SetVolume(volume);
}

// Copy constructor
AudioParameters::AudioParameters(const AudioParameters & other) noexcept
	:
	AudioId(other.AudioId), Volume(other.Volume)
{
}

// Set the audio ID for an AudioParameters object, including validation
void AudioParameters::SetAudioId(Audio::AudioID audio_id)
{
	AudioId = (Game::Engine->GetAudioManager()->IsValidID(audio_id) ? audio_id : AudioManager::NULL_AUDIO);
}

// Set the volume for an AudioParameters object, including validation
void AudioParameters::SetVolume(float volume)
{
	volume = clamp(volume, 0.0f, AudioManager::MAXIMUM_VOLUME);
}

