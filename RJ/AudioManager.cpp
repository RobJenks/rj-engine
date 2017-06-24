#include "ErrorCodes.h"
#include "Logging.h"
#include "AudioManager.h"


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