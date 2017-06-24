#include "ErrorCodes.h"
#include "Utility.h"
#include "Logging.h"
#include "AudioManager.h"

// Initialise static constant values
const float AudioManager::DEFAULT_VOLUME = 1.0f;
const float AudioManager::DEFAULT_PITCH_SHIFT = 0.0f;
const float AudioManager::DEFAULT_PAN = 0.0f;



// Default constructor
AudioManager::AudioManager(void)
	:
	m_engine(NULL), m_in_error_state(false)
{
}

// Initialise all audio manager resources
Result AudioManager::Initialise(void)
{
	// Audio engine mode
	DirectX::AUDIO_ENGINE_FLAGS engine_flags = DirectX::AudioEngine_Default;
#	ifdef _DEBUG
	Game::Log << LOG_INFO << "Enabling audio debug mode\n";
	engine_flags = engine_flags | DirectX::AudioEngine_Debug;
#	endif

	// Initialise the audio component
	Game::Log << LOG_INFO << "Initialising audio engine (" << engine_flags << ")\n";
	m_engine = new DirectX::AudioEngine(engine_flags);


	effect = new DirectX::SoundEffect(m_engine, L"C:\\Users\\robje\\Documents\\Visual Studio 2013\\Projects\\RJ\\RJ\\Data\\Audio\\test1.wav");
	
		
	// Return success
	return ErrorCodes::NoError;
}

// Per-frame update method
void AudioManager::Update(void)
{
	if (!m_engine->Update())
	{
		// Engine component will return false if no audio processing was performed this frame.  This
		// may be because there is nothing to process, or it may be due to an error (e.g. loss of speaker
		// connectivity).  Test for the latter case here and handle appropriately
		if (m_engine->IsCriticalError())
		{
			// TODO: Handle critical audio engine failures with e.g. reset/re-initialise
			if (!m_in_error_state)
			{
				m_in_error_state = true;
				Game::Log << LOG_ERROR << "Audio engine encountered critical error\n";
				return;
			}
		}
	}
	
	// Audio engine is operating normally; raise a notification if this was not previously the case
	if (m_in_error_state)
	{
		m_in_error_state = false;
		Game::Log << LOG_INFO << "Audio engine recovered from critical error\n";
	}
}

// Play a one-shot sound effect based on the specified audio resource.  Less efficient than playing a
// pre-loaded audio effect, so should be used sparingly
// Volume: default = 1
// PitchShift: In the range [-1 +1], default with no shift = 0
// Pan: In the range [-1 = full left, +1 = full right], default with no panning = 0
void AudioManager::PlayOneShot(const wchar_t *filename, float volume, float pitch_shift, float pan) const
{ 

	effect->Play(volume, pitch_shift, pan);
}

// Play a one-shot sound effect based on the specified audio resource.  Less efficient than playing a
// pre-loaded audio effect, so should be used sparingly
// Volume: default = 1
// PitchShift: In the range [-1 +1], default with no shift = 0
// Pan: In the range [-1 = full left, +1 = full right], default with no panning = 0
void AudioManager::PlayOneShot(const std::string & filename, float volume, float pitch_shift, float pan) const
{
	std::wstring wstr = ConvertStringToWString(filename);
	PlayOneShot(wstr.c_str(), volume, pitch_shift, pan);
}

// Play a one-shot sound effect based on the specified audio resource.  Less efficient than playing a
// pre-loaded audio effect, so should be used sparingly
void AudioManager::PlayOneShot(const wchar_t *filename)
{
	PlayOneShot(filename, DEFAULT_VOLUME, DEFAULT_PITCH_SHIFT, DEFAULT_PAN);
}

// Play a one-shot sound effect based on the specified audio resource.  Less efficient than playing a
// pre-loaded audio effect, so should be used sparingly
void AudioManager::PlayOneShot(const std::string & filename)
{
	std::wstring wstr = ConvertStringToWString(filename);
	PlayOneShot(wstr.c_str(), DEFAULT_VOLUME, DEFAULT_PITCH_SHIFT, DEFAULT_PAN);
}



// Shutdown method to deallocate all audio manager resources
Result AudioManager::Shutdown(void)
{
	if (m_engine)
	{
		SafeDelete(m_engine);
	}

	return ErrorCodes::NoError;
}

// Default destructor
AudioManager::~AudioManager(void)
{

}