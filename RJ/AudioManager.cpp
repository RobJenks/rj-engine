#include "ErrorCodes.h"
#include "Utility.h"
#include "Logging.h"
#include "GameDataExtern.h"
#include "AudioManager.h"


// NOTE: If any issues are encountered with LoadWAVAudioFromFileEx(), note that it was modified
// in WAVFileReader.h to directly specify a __cdecl calling convention on the function declaration
// This is best practice, and without it the linker was failing with a missing symbol error due to
// mismatched mangled function names (since the target was __cdecl but project default is __vectorcall)
#include "DirectXTK\Audio\WAVFileReader.h"
#pragma comment(lib, "DirectXTKAudioWin8.lib")


// Initialise static constant values
const AudioItem::AudioID AudioManager::NULL_AUDIO = 0U;
const std::string AudioManager::NULL_AUDIO_NAME = "NULL";
const float AudioManager::DEFAULT_VOLUME = 1.0f;
const float AudioManager::DEFAULT_PITCH_SHIFT = 0.0f;
const float AudioManager::DEFAULT_PAN = 0.0f;


// Default constructor
AudioManager::AudioManager(void)
	:
	m_engine(NULL), m_in_error_state(false), m_audio_count(0U)
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

	// Register an initial null audio item.  This will always be in slot NULL_AUDIO (== 0)
	// and will be returned in response to invalid requests
	RegisterSound(NULL_AUDIO_NAME.c_str(), AudioItem::TranslateAudioTypeToString(AudioItem::AudioType::Effect).c_str(), "", false);
	assert( GetAudioID(NULL_AUDIO_NAME) == NULL_AUDIO );		// Will always be in slot 0
		
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

// Registers a new sound with the audio manager.  Flag 'load_resouce' determines whether 
// the audio resource will be loaded immediately.  If it is not loaded now, LoadResource()
// must be called on the specific audio item before it can be used
Result AudioManager::RegisterSound(const char *name, const char *type, const char *filename, bool load_resource)
{
	if (!name || !type || !filename) return ErrorCodes::CannotRegisterAudioItemWithInvalidDetails;

	// Make sure there are no conflicts
	std::string audio_name = name;
	if (m_sound_name_map.count(audio_name) != 0) return ErrorCodes::CannotRegisterDuplicateAudioItem;

	// Process other parameters
	AudioItem::AudioType audio_type = AudioItem::TranslateAudioTypeFromString(type);

	// Create a new entry in both collections
	AudioItem::AudioID id = m_sounds.size();
	m_sounds.push_back(AudioItem(id, audio_name, audio_type, filename));
	m_sound_name_map[audio_name] = id;

	// Update the audio resource count
	m_audio_count = m_sounds.size();

	// Initialise the resource immediately if requested
	if (load_resource)
	{
		Result load_result = LoadAudioResource(id);
		if (load_result != ErrorCodes::NoError) return load_result;
	}

	// Return success
	return ErrorCodes::NoError;

}

// Load the resource associated with the given audio item
Result AudioManager::LoadAudioResource(AudioItem::AudioID id)
{
	if (!IsValidID(id)) return ErrorCodes::CannotLoadAudioResourceWithInvalidID;
	if (m_sounds[id].ResourcesLoaded()) return ErrorCodes::AudioResourceAlreadyLoaded;

	// Attempt to load this external resource.  Replicate some of the logic 
	// in SoundEffect::SoundEffect(Audioengine*, wchar_t*) so we can avoid the 
	// exceptions and call the overloaded constructor directly with all info
	DirectX::WAVData wavInfo;
	std::unique_ptr<uint8_t[]> wavData; 
	std::string str = BuildStrFilename(D::DATA, m_sounds[id].GetFilename());
	std::wstring wstr = ConvertStringToWString(str);
	
	HRESULT hr = DirectX::LoadWAVAudioFromFileEx(wstr.c_str(), wavData, wavInfo);
	if (FAILED(hr))
	{
		Game::Log << LOG_ERROR << "Failed to load audio resource \"" << m_sounds[id].GetFilename() << "\" (ID: " << id << ")\n";
		return ErrorCodes::FailedToLoadAudioResource;
	}

	// Create the effect based on this data and assign it to the audio item	
	Result assign_result = m_sounds[id].AssignResource(
		new SoundEffect(m_engine, wavData, wavInfo.wfx, wavInfo.startAudio, wavInfo.audioBytes, wavInfo.loopStart, wavInfo.loopLength));
	if (assign_result != ErrorCodes::NoError) return assign_result;


	// Return success
	return ErrorCodes::NoError;
}

// Load all audio resources that have not yet been loaded
Result AudioManager::LoadAllAudioResources(void)
{
	Result overallresult = ErrorCodes::NoError;

	// Process every item, excluding the NULL_AUDIO item at id == 0
	for (AudioItem::AudioID id = 1U; id < m_audio_count; ++id)
	{
		Result result = LoadAudioResource(id);
		if (result != ErrorCodes::NoError) overallresult = result;
	}

	return overallresult;
}

// Load the resource associated with the given audio item
Result AudioManager::LoadAudioResource(const std::string & name)
{
	return LoadAudioResource(GetAudioID(name));
}


// Play a one-shot sound effect based on the specified audio resource
// Volume: default = 1
// PitchShift: In the range [-1 +1], default with no shift = 0
// Pan: In the range [-1 = full left, +1 = full right], default with no panning = 0
void AudioManager::Play(AudioItem::AudioID id, float volume, float pitch_shift, float pan)
{ 
	if (!IsValidID(id)) return;

	SoundEffect *effect = m_sounds[id].GetEffect();
	if (effect) effect->Play(volume, pitch_shift, pan);	
}

// Play a one-shot sound effect based on the specified audio resource
// Volume: default = 1
// PitchShift: In the range [-1 +1], default with no shift = 0
// Pan: In the range [-1 = full left, +1 = full right], default with no panning = 0
void AudioManager::Play(const std::string & name, float volume, float pitch_shift, float pan)
{
	Play(GetAudioID(name), volume, pitch_shift, pan);
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