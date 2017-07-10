#include "ErrorCodes.h"
#include "Utility.h"
#include "FastMath.h"
#include "Logging.h"
#include "GameDataExtern.h"
#include "FastMath.h"
#include "ObjectSearch.h"
#include "Player.h"
#include "iSpaceObjectEnvironment.h"
#include "AudioManager.h"


// NOTE: If any issues are encountered with LoadWAVAudioFromFileEx(), note that it was modified
// in WAVFileReader.h to directly specify a __cdecl calling convention on the function declaration
// This is best practice, and without it the linker was failing with a missing symbol error due to
// mismatched mangled function names (since the target was __cdecl but project default is __vectorcall)
#include "DirectXTK\Audio\WAVFileReader.h"
#pragma comment(lib, "DirectXTKAudioWin8.lib")

// Initialise static constant values
DirectX::AudioListener AudioManager::PLAYER_AUDIO_LISTENER;
const Audio::AudioID AudioManager::NULL_AUDIO = 0U;
const std::string AudioManager::NULL_AUDIO_NAME = "NULL";
const Audio::AudioInstanceIdentifier AudioManager::NULL_INSTANCE_IDENTIFIER = 0U;
const Audio::AudioInstanceID AudioManager::GLOBAL_AUDIO_INSTANCE_LIMIT = 400U;
const Audio::AudioInstanceID AudioManager::DEFAULT_AUDIO_ITEM_INSTANCE_LIMIT = 20U;
const Audio::AudioInstanceID AudioManager::HARD_INSTANCE_LIMIT_PER_AUDIO = 50U;
const float	AudioManager::MAXIMUM_VOLUME = 50.0f;
const float AudioManager::DEFAULT_VOLUME = 1.0f;
const float AudioManager::DEFAULT_PITCH_SHIFT = 0.0f;
const float AudioManager::DEFAULT_PAN = 0.0f;
const unsigned int AudioManager::PERIODIC_AUDIO_MANAGER_AUDIO_INTERVAL = 100U;

// Note: inner range should always be some % < 1.0 of max range to avoid in/out of range thrash at boundary
const float AudioManager::SPACE_AUDIO_MAX_RANGE = 20000.0f;											// Space events, when in space
const float AudioManager::SPACE_AUDIO_INNER_RANGE = AudioManager::SPACE_AUDIO_MAX_RANGE * 0.95f;
const float AudioManager::ENV_AUDIO_MAX_RANGE = 200.0f;												// Env events, when in env		 
const float	AudioManager::ENV_AUDIO_INNER_RANGE = AudioManager::ENV_AUDIO_MAX_RANGE * 0.9f;
const float	AudioManager::ENV_SPACE_AUDIO_MAX_RANGE = 10000.0f;										// Space events, when in env	
const float	AudioManager::ENV_SPACE_AUDIO_INNER_RANGE = AudioManager::ENV_SPACE_AUDIO_MAX_RANGE * 0.95f;
const float	AudioManager::ENV_SPACE_VOLUME_MODIFIER = 0.25f; // Modifier to space event volume when player is in an environment	

															   // Squared versions of key object binding constants for runtime efficiency
const float AudioManager::SPACE_AUDIO_MAX_RANGE_SQ = AudioManager::SPACE_AUDIO_MAX_RANGE * AudioManager::SPACE_AUDIO_MAX_RANGE;
const float AudioManager::SPACE_AUDIO_INNER_RANGE_SQ = AudioManager::SPACE_AUDIO_INNER_RANGE * AudioManager::SPACE_AUDIO_INNER_RANGE;
const float	AudioManager::ENV_AUDIO_MAX_RANGE_SQ = AudioManager::ENV_AUDIO_MAX_RANGE * AudioManager::ENV_AUDIO_MAX_RANGE;
const float	AudioManager::ENV_AUDIO_INNER_RANGE_SQ = AudioManager::ENV_AUDIO_INNER_RANGE * AudioManager::ENV_AUDIO_INNER_RANGE;
const float	AudioManager::ENV_SPACE_AUDIO_MAX_RANGE_SQ = AudioManager::ENV_SPACE_AUDIO_MAX_RANGE * AudioManager::ENV_SPACE_AUDIO_MAX_RANGE;
const float	AudioManager::ENV_SPACE_AUDIO_INNER_RANGE_SQ = AudioManager::ENV_SPACE_AUDIO_INNER_RANGE * AudioManager::ENV_SPACE_AUDIO_INNER_RANGE;

const float AudioManager::AUDIBLE_DISTANCE_AT_DEFAULT_VOLUME = 400.0f;		// Max audible distance for 3D audio instance at volume = 1.0
const float AudioManager::MAXIMUM_AUDIBLE_DISTANCE = 20000.0f;				// Limit on audible distance, regardless of volume.  Is currently
																			// identical to (distance at default volume * max distance), which is 
																			// neat but not necessary in future

// Initialise the counter used to assign new audio instance identifiers
Audio::AudioInstanceIdentifier AudioManager::AUDIO_INSTANCE_COUNTER = 0U;

// Comparator for efficient sorting and searching of objects
const struct AudioInstanceObjectBinding::_ComparatorLessThan AudioInstanceObjectBinding::ComparatorLessThan;


// Default constructor
AudioManager::AudioManager(void)
	:
	m_engine(NULL), m_in_error_state(false), m_audio_count(0U), m_instance_count(0U), 
	m_player_listener_position(NULL_FLOAT3), 
	m_next_audit_time(0U), m_object_bindings_last_valid(0U)
{
	// Default volume modifiers for each audio type
	for (int i = 0; i < (int)AudioItem::AudioType::_COUNT; ++i)
		SetBaseTypeVolumeModifier((AudioItem::AudioType)i, 1.0f);

	// Initialise the static player audio listener to a default state
	PLAYER_AUDIO_LISTENER.SetPosition(NULL_FLOAT3);
	PLAYER_AUDIO_LISTENER.SetOrientation(FORWARD_VECTOR_F, UP_VECTOR_F);
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
	RegisterSound(AudioManager::NULL_AUDIO_NAME.c_str(), AudioItem::TranslateAudioTypeToString(AudioItem::AudioType::Effect).c_str(), "", false, 1.0f, false);
	assert( GetAudioID(AudioManager::NULL_AUDIO_NAME) == AudioManager::NULL_AUDIO );		// Will always be in slot 0
		
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

	// Update the player audio listener to the current player position and orientation
	UpdatePlayerAudioListener();

	// Update any active object/audio bindings based on the current world state
	UpdateObjectAudioBindings(Game::ClockMs);

	// Perform a periodic audit of active instances and update our internal state
	if (Game::ClockMs >= m_next_audit_time)
	{
		PerformPeriodicAudit();
		m_next_audit_time = (Game::ClockMs + AudioManager::PERIODIC_AUDIO_MANAGER_AUDIO_INTERVAL);
	}
}

// Registers a new sound with the audio manager.  Flag 'load_resouce' determines whether 
// the audio resource will be loaded immediately.  If it is not loaded now, LoadResource()
// must be called on the specific audio item before it can be used
Result AudioManager::RegisterSound(const char *name, const char *type, const char *filename, bool default_loop_state, float default_volume, bool load_resource)
{
	if (!name || !type || !filename) return ErrorCodes::CannotRegisterAudioItemWithInvalidDetails;

	// Make sure there are no conflicts
	std::string audio_name = name;
	if (m_sound_name_map.count(audio_name) != 0) return ErrorCodes::CannotRegisterDuplicateAudioItem;

	// Process other parameters
	AudioItem::AudioType audio_type = AudioItem::TranslateAudioTypeFromString(type);

	// Create a new entry in both collections
	Audio::AudioID id = m_sounds.size();
	m_sounds.push_back(AudioItem(id, audio_name, audio_type, filename, default_loop_state, default_volume));
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
Result AudioManager::LoadAudioResource(Audio::AudioID id)
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
	for (Audio::AudioID id = 1U; id < m_audio_count; ++id)
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

// Set the base type volume modifier for a specific audio item type, and update all affected audio items & instances accordingly
void AudioManager::SetBaseTypeVolumeModifier(AudioItem::AudioType type, float volume_modifier)
{
	m_type_volume_modifiers[(int)type] = clamp(volume_modifier, 0.0f, 1.0f);
	RecalculateAllAudioItemVolumeSettings();
}

// Set the base type volume modifier for all audio item types, and update all audio items & instances accordingly
void AudioManager::SetBaseTypeVolumeModifiers(float const (&type_modifiers)[AudioItem::AudioType::_COUNT])
{
	for (int i = 0; i < AudioItem::AudioType::_COUNT; ++i)
	{
		m_type_volume_modifiers[i] = type_modifiers[i];
	}

	RecalculateAllAudioItemVolumeSettings();
}

// Recalculates the final volume of all audio items and instances, based on a change to the audio manager volume settings
void AudioManager::RecalculateAllAudioItemVolumeSettings(void)
{
	// Process every audio item in turn
	Audio::AudioCollection::iterator it_end = m_sounds.end();
	for (Audio::AudioCollection::iterator it = m_sounds.begin(); it != it_end; ++it)
	{
		float modifier = GetBaseTypeVolumeModifier((*it).GetType());
		(*it).SetBaseTypeVolumeModifier(modifier);
	}
	 
}


// Play a one-shot sound effect based on the specified audio resource
// Volume: default = 1
// PitchShift: In the range [-1 +1], default with no shift = 0
// Pan: In the range [-1 = full left, +1 = full right], default with no panning = 0
void AudioManager::Play(Audio::AudioID id, float volume, float pitch_shift, float pan)
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

// Ensures that a slot is available for a new audio instance.  Will terminate an instance of the current 
// audio item if necessary to remain under the global audio instance limit
void AudioManager::EnsureInstanceIsAvailable(Audio::AudioID id, bool requires_3d_support)
{
	// We don't need to take any action as long as we are below the global audio instance limit
	if (m_instance_count <= AudioManager::GLOBAL_AUDIO_INSTANCE_LIMIT) return;

	// Otherwise we need to make an existing slot available within this item
	if (IsValidID(id))
	{
		// Disallow new allocations before requesting an instance slot.  This will make sure at least one
		// instance is available, by terminating an existing instance if necessary, however no new 
		// allocations will be performed
		m_sounds[id].DisallowNewInstanceSlots();
		m_sounds[id].MakeInstanceAvailable(requires_3d_support);
		m_sounds[id].AllowNewInstanceSlots();
	}
}

// Create a new instance of an audio item, if posssible.  Returns NULL_INSTANCE_IDENTIFIER if instantiation
// fails for any reason, otherwise returns the identifier of the newly-created instance
Audio::AudioInstanceIdentifier AudioManager::CreateInstance(Audio::AudioID id, float base_volume, float volume_modifier)
{
	if (!IsValidID(id)) return NULL_INSTANCE_IDENTIFIER;
	
	EnsureInstanceIsAvailable(id, false);
	RecordNewInstanceCreation();
	return m_sounds[id].CreateInstance(base_volume, volume_modifier);
}

// Create a new 3D instance of an audio item, if possible.  Returns NULL_INSTANCE_IDENTIFIER if instantiation
// fails for any reason, otherwise returns the identifier of the newly-created instance
Audio::AudioInstanceIdentifier AudioManager::Create3DInstance(Audio::AudioID id, const XMFLOAT3 & position, float base_volume, float volume_modifier)
{
	if (!IsValidID(id)) return NULL_INSTANCE_IDENTIFIER;
	
	EnsureInstanceIsAvailable(id, true);
	RecordNewInstanceCreation(); 
	return m_sounds[id].Create3DInstance(position, base_volume, volume_modifier);
}

// Determines the maximum audible distance for a 3D audio instance, based on all relevant // Calculates the final volume level of an instance based on all relevant parameters
// Final volume = (base_volume_modifier * volume_modifier * base_volume)
// base_volume_modifier = base modifier based on audio type (effect vs music vs ...)
// volume_modifier = instance-specific modifier (e.g. for interior vs exterior sounds)
// base_volume = desired base volume before any modification
float AudioManager::DetermineVolume(float base_volume, float volume_modifier, float base_volume_modifier)
{
	float volume = (base_volume * volume_modifier * base_volume_modifier);
	return clamp(volume, 0.0f, AudioManager::MAXIMUM_VOLUME);
}

// Determines the maximum audible distance for a 3D audio instance, based upon its defined volume
float AudioManager::DetermineMaximumAudibleDistance(float instance_volume)
{
	float distance = (instance_volume * AudioManager::AUDIBLE_DISTANCE_AT_DEFAULT_VOLUME);
	return clamp(distance, 0.0f, AudioManager::MAXIMUM_AUDIBLE_DISTANCE);
}

// Update the player audio listener to the current player position and orientation
void AudioManager::UpdatePlayerAudioListener(void)
{
	// TODO: We currently use SetX() rather than Update().  Consider switching in future, 
	//   i.e. if (currentpos-oldpos < SOME_THRESHOLD) Update() else SetX()
	// If this gives us any benefit, for example Doppler calculation based on PLAYER movement
	Player *player = Game::CurrentPlayer;
	if (player)
	{
		// TODO: accounts for local environment pos?
		UpdatePlayerListenerPositionData(player->GetPosition(), player->GetOrientation());
	}
	else
	{
		UpdatePlayerListenerPositionData(NULL_VECTOR, NULL_VECTOR);
	}
}

// Store position and orientation data for the player listener
void AudioManager::UpdatePlayerListenerPositionData(const FXMVECTOR position, const FXMVECTOR orientation)
{
	// Update the listener object
	// TODO: Switch to using Update() if it enables e.g. velocity calculations and doppler shift
	AudioManager::PLAYER_AUDIO_LISTENER.SetPosition(position);
	AudioManager::PLAYER_AUDIO_LISTENER.SetOrientationFromQuaternion(orientation);

	// Also store data locally for more efficient per-frame calculations
	XMStoreFloat3(&m_player_listener_position, position);
}

// Perform a periodic audit of active instances and update our internal state
void AudioManager::PerformPeriodicAudit(void)
{
	// Iterate through all audio items to get an accurate count of active audio instances.  This is required since
	// during per-frame operation the audio manager will only approximate this count based on incrementing for new 
	// instances created.  It will not search for completed instances per-frame for efficiency
	m_instance_count = DetermineExactAudioInstanceCount();

	// Identify any object-audio bindings that are no longer valid, terminate them and reclaim the instance resources
	// NOTE: This step should always be performed before GenerateNewObjectBindings(), since it means we can then
	// guarantee that all remaining bindings will have object != NULL
	// NOTE: It should also be performed first so that newly-created items aren't immediately removed by this method
	TerminateExpiredObjectBindings(m_object_bindings_last_valid);

	// Identify any new object-audio bindings that need to be made based on proximity to the player
	GenerateNewObjectBindings();
}


// Iterates through all audio items to get an accurate count of active audio instances.  This is required since
// during per-frame operation the audio manager will only approximate this count based on incrementing for new 
// instances created.  It will not search for completed instances per-frame for efficiency
Audio::AudioInstanceID AudioManager::DetermineExactAudioInstanceCount(void)
{
	Audio::AudioInstanceID count = 0U;

	Audio::AudioCollection::const_iterator it_end = m_sounds.end();
	for (Audio::AudioCollection::const_iterator it = m_sounds.begin(); it != it_end; ++it)
	{
		count += (*it).GetActiveInstanceCount();
	}

	return count;
}

// Identify any object-audio bindings that are no longer valid, terminate them and reclaim the instance resources
// Accepts the time of the relevant audit verification as a parameter.  Any binding which does not have that
// same clock time as it's "valid_at" parameter will be cleaned up
void AudioManager::TerminateExpiredObjectBindings(unsigned int verification_time)
{
	// Partition the instance collection based on either (a) object is null, or (b) object 
	// was not valid during the last verification
	std::vector<AudioInstanceObjectBinding>::const_iterator it_start = std::partition(m_object_bindings.begin(), m_object_bindings.end(), 
		[verification_time](const AudioInstanceObjectBinding & binding) {
		return (binding.GetObj() != NULL && binding.GetLastValid() == verification_time);
	});
	
	// Terminate any instances that are no longer required and then remove the bindings
	std::vector<AudioInstanceObjectBinding>::const_iterator it_end = m_object_bindings.end();
	if (it_start != it_end)
	{
		for (std::vector<AudioInstanceObjectBinding>::const_iterator it = it_start; it != it_end; ++it)
		{
			if (!m_sounds[(*it).GetAudioItemID()].TerminateInstanceByIdentifier((*it).GetInstanceIdentifier()))
			{
				Game::Log << LOG_ERROR << "Failed to terminate object-audio binding instance for ID " << (*it).GetInstanceIdentifier() << "; instance not found\n";
			}
		}

		m_object_bindings.erase(it_start, it_end);
	}
}

// Identify any new object-audio bindings that need to be made based on proximity to the player
void AudioManager::GenerateNewObjectBindings(void)
{
	// TODO: Should use a different focal point (rather than player->{actor, ship}) if the player is not 
	// controlling either of those at the current time
	iSpaceObjectEnvironment *env = Game::CurrentPlayer->GetParentEnvironment();
	if (env && Game::CurrentPlayer->GetState() == Player::StateType::OnFoot) 
	{
		GenerateNewEnvironmentObjectBindings(env, (iEnvironmentObject*)Game::CurrentPlayer->GetActor(), 
			AudioManager::ENV_AUDIO_INNER_RANGE, AudioManager::ENV_AUDIO_MAX_RANGE, 1.0f);
		GenerateNewSpaceObjectBindings(Game::CurrentPlayer->GetPlayerShip(), AudioManager::ENV_SPACE_AUDIO_INNER_RANGE, 
			AudioManager::ENV_SPACE_AUDIO_MAX_RANGE, AudioManager::ENV_SPACE_VOLUME_MODIFIER);
	}
	else
	{
		GenerateNewSpaceObjectBindings(Game::CurrentPlayer->GetPlayerShip(), AudioManager::ENV_SPACE_AUDIO_INNER_RANGE, 
			AudioManager::ENV_SPACE_AUDIO_MAX_RANGE, 1.0f);
	}
}

// Generate new audio bindings for objects in the current player environment.  Bindings are established for new objects which get 
// within audio_inner_range of the player
void AudioManager::GenerateNewEnvironmentObjectBindings(iSpaceObjectEnvironment *env, iEnvironmentObject *listener, float audio_inner_range, 
														float audio_outer_range, float volume_modifier)
{
	XMVECTOR listener_pos = listener->GetPosition();

	// Get all objects within range.  We do not support audio sources from terrain objects
	std::vector<iEnvironmentObject*> objects;
	env->GetAllObjectsWithinDistance(listener, audio_inner_range, &objects, NULL);

	// Check all objects to see if they already have a binding
	std::vector<iEnvironmentObject*>::const_iterator it_end = objects.end();
	for (std::vector<iEnvironmentObject*>::const_iterator it = objects.begin(); it != it_end; ++it)
	{
		// Ignore any objects without ambient audio (which will be most of them)
		Audio::AudioID audio_id = (*it)->GetAmbientAudio();
		if (audio_id == AudioManager::NULL_AUDIO) continue;
		float audio_base_volume = (*it)->GetAmbientAudioVolume();

		// Test whether this particular audio instance would be audible at its current distance
		float distsq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract((*it)->GetPosition(), listener_pos)));
		float audible_dist_sq = AudioManager::DetermineMaximumAudibleDistance(					// Get max audible distance
			AudioManager::DetermineVolume(audio_base_volume, volume_modifier,					// Based on volume & volume modifier
				GetBaseTypeVolumeModifier(GetAudioItem(audio_id)->GetType()))					// and base volume modifier for audio type
		);
		audible_dist_sq *= audible_dist_sq;
		if (distsq > audible_dist_sq) continue;

		// Binary search for the object binding
		std::vector<AudioInstanceObjectBinding>::const_iterator entry =
			std::lower_bound(m_object_bindings.begin(), m_object_bindings.end(), (*it)->GetID(), AudioInstanceObjectBinding::ComparatorLessThan);
	
		// Test whether a new binding is required.  Need to test for == end() first in case this element is going at the end
		if (entry == m_object_bindings.end() || (*entry).GetObjectID() != (*it)->GetID())
		{
			// We need to create a binding.  The first object >= us is also != us, so therefore we are not in the vector
			Audio::AudioInstanceIdentifier identifier = Create3DInstance(audio_id, (*it)->GetPositionF(), audio_base_volume, volume_modifier);
			m_object_bindings.insert(entry, AudioInstanceObjectBinding((*it), audio_id, identifier, audible_dist_sq));
		}
	}
}

// Generate new audio bindings for objects in the current player environment.  Bindings are established for new objects which get
// within audio_inner_range of the player
void AudioManager::GenerateNewSpaceObjectBindings(iObject *listener, float audio_inner_range, float audio_outer_range, float volume_modifier)
{
	XMVECTOR listener_pos = listener->GetPosition();

	// Get all objects within range
	std::vector<iObject*> objects;
	Game::ObjectSearch<iObject>::GetAllObjectsWithinDistance(listener, audio_inner_range, objects, Game::ObjectSearchOptions::NoSearchOptions);

	// Check all objects to see if they already have a binding
	std::vector<iObject*>::const_iterator it_end = objects.end();
	for (std::vector<iObject*>::const_iterator it = objects.begin(); it != it_end; ++it)
	{
		// Ignore any objects without ambient audio (which will be most of them)
		Audio::AudioID audio_id = (*it)->GetAmbientAudio();
		if (audio_id == AudioManager::NULL_AUDIO) continue;
		float audio_base_volume = (*it)->GetAmbientAudioVolume();

		// Test whether this particular audio instance would be audible at its current distance
		float distsq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract((*it)->GetPosition(), listener_pos)));
		float audible_dist_sq = AudioManager::DetermineMaximumAudibleDistance(					// Get max audible distance
			AudioManager::DetermineVolume(audio_base_volume, volume_modifier,					// Based on volume & volume modifier
				GetBaseTypeVolumeModifier(GetAudioItem(audio_id)->GetType()))					// and base volume modifier for audio type
		);
		audible_dist_sq *= audible_dist_sq;
		if (distsq > audible_dist_sq) continue;

		// Binary search for the object binding
		std::vector<AudioInstanceObjectBinding>::const_iterator entry =
			std::lower_bound(m_object_bindings.begin(), m_object_bindings.end(), (*it)->GetID(), AudioInstanceObjectBinding::ComparatorLessThan);

		// Test whether a new binding is required.  Need to test for == end() first in case this element is going at the end
		if (entry == m_object_bindings.end() || (*entry).GetObjectID() != (*it)->GetID())
		{
			// We need to create a binding.  The first object >= us is also != us, so therefore we are not in the vector
			Audio::AudioInstanceIdentifier identifier = Create3DInstance(audio_id, (*it)->GetPositionF(), audio_base_volume, volume_modifier);
			m_object_bindings.insert(entry, AudioInstanceObjectBinding((*it), audio_id, identifier, audible_dist_sq));
		}
	}
}

// Update any active object/audio bindings based on the current world state
void AudioManager::UpdateObjectAudioBindings(unsigned int verification_time)
{
	// Record the point at which this check was performed, for future comparison against instances
	m_object_bindings_last_valid = verification_time;

	// Check each binding in turn
	std::vector<AudioInstanceObjectBinding>::iterator it_end = m_object_bindings.end();
	for (std::vector<AudioInstanceObjectBinding>::iterator it = m_object_bindings.begin(); it != it_end; ++it)
	{
		// We only want to continue simulation object bindings that are still within range
		const iObject *object = (*it).GetObj();
		float distsq = Float3DistanceSq(m_player_listener_position, object->GetPositionF());
		if (distsq <= (*it).GetMaxDistSq())
		{
			// Find and update the audio instance with new data
			AudioInstance *instance = m_sounds[(*it).GetAudioItemID()].GetInstanceByIdentifier((*it).GetInstanceIdentifier());
			if (instance)
			{
				// Update the instance
				instance->UpdatePosition(object->GetPosition(), Game::TimeFactor);	// TODO: using delta=TimeFactor here requires that the update will run per-frame

				// Mark the binding as valid during this frame
				(*it).SetLastValid(verification_time);
			}

			// Nothing else to do; if the instance fails any checks above we will not set its 'last valid' 
			// value, and it will therefore be cleaned up in the next audio manager audit
		}
	}
}

// Release the specified audio resource.  Audio item is preserved, but resource must be
// re-loaded before it can be used again
void AudioManager::ReleaseAudioResource(Audio::AudioID id)
{
	if (!IsValidID(id)) return;
	m_sounds[id].ReleaseResources();
}

// Releases all audio resources.  Audio items are preserved, but resources must be re-loaded 
// before they can be used
void AudioManager::ReleaseAllAudioResources(void)
{
	// Process every item, excluding the NULL_AUDIO item at id == 0
	for (Audio::AudioID id = 1U; id < m_audio_count; ++id)
	{
		ReleaseAudioResource(id);
	}
}


// Shutdown method to deallocate all audio manager resources
Result AudioManager::Shutdown(void)
{
	// Deallocate all audio resource data
	ReleaseAllAudioResources();

	// Release the engine itself
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