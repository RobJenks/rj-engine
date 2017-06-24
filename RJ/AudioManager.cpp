#include "ErrorCodes.h"
#include "Logging.h"
#include "AudioManager.h"


// Default constructor
AudioManager::AudioManager(void)
	:
	m_engine(NULL)
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