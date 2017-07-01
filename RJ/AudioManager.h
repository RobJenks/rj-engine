#pragma once

#include <Audio.h>
#include "ErrorCodes.h"
#include "ObjectReference.h"
#include "AudioItem.h"

class AudioManager
{
public:

	// Key constants
	static const AudioItem::AudioID					NULL_AUDIO;
	static const std::string						NULL_AUDIO_NAME;
	static const AudioInstance::AudioInstanceIdentifier NULL_INSTANCE_IDENTIFIER;
	static const AudioInstance::AudioInstanceID		GLOBAL_AUDIO_INSTANCE_LIMIT;			// Max instances across all audio items
	static const AudioInstance::AudioInstanceID		DEFAULT_AUDIO_ITEM_INSTANCE_LIMIT;		// Default max instances per audio item
	static const AudioInstance::AudioInstanceID		HARD_INSTANCE_LIMIT_PER_AUDIO;			// Hard limit for instance count per audio, cannot be overridden
	static const float								DEFAULT_VOLUME;
	static const float								DEFAULT_PITCH_SHIFT;
	static const float								DEFAULT_PAN;
	static const unsigned int						PERIODIC_AUDIO_MANAGER_AUDIO_INTERVAL;	// ms; time between periodic audits

	// Note: inner range should always be some % < 1.0 of max range to avoid in/out of range thrash at boundary
	static const float								SPACE_AUDIO_MAX_RANGE;		 // Max distance for audio, beyond which audio sources will not be audible (space events, when in space)
	static const float								SPACE_AUDIO_INNER_RANGE;	 // Inner range, % of max range, within which audio sources will become audible
	static const float								ENV_AUDIO_MAX_RANGE;		 // Max distance for audio, beyond which audio sources will not be audible (env events, when in env)
	static const float								ENV_AUDIO_INNER_RANGE;		 // Inner range, % of max range, within which audio sources will become audible
	static const float								ENV_SPACE_AUDIO_MAX_RANGE;	 // Max distance for audio, beyond which audio sources will not be audible (space events, when in env)
	static const float								ENV_SPACE_AUDIO_INNER_RANGE; // Inner range, % of max range, within which audio sources will become audible
	static const float								ENV_SPACE_VOLUME_MODIFIER;	 // Modifier to space event volume when in an environment

	// Default constructor
	AudioManager(void);

	// Initialise all audio manager resources
	Result								Initialise(void);

	// Per-frame update method
	void								Update(void);

	// Registers a new sound with the audio manager.  Flag 'load_resouce' determines whether 
	// the audio resource will be loaded immediately.  If it is not loaded now, LoadResource()
	// must be called on the specific audio item before it can be used
	Result								RegisterSound(	const char *name, const char *type, const char *filename, 
														bool default_loop_state, bool load_resource);

	// Load the resource associated with the given audio item
	Result								LoadAudioResource(AudioItem::AudioID id);
	Result								LoadAudioResource(const std::string & name);

	// Load all audio resources that have not yet been loaded
	Result								LoadAllAudioResources(void);

	// Release the specified audio resource.  Audio item is preserved, but resource must be
	// re-loaded before it can be used again
	void								ReleaseAudioResource(AudioItem::AudioID id);

	// Releases all audio resources.  Audio items are preserved, but resources must be re-loaded 
	// before they can be used
	void								ReleaseAllAudioResources(void);

	// Determines whether the supplied audio ID is valid
	CMPINLINE bool						IsValidID(AudioItem::AudioID id) const { return (id > 0U && id < m_audio_count); }

	// Returns the ID of the audio item with the given name, or NULL_AUDIO if no such item exists
	CMPINLINE AudioItem::AudioID		GetAudioID(const std::string & name) const 
	{
		AudioItem::AudioIDMap::const_iterator it = m_sound_name_map.find(name);
		return (it != m_sound_name_map.end() ? it->second : AudioManager::NULL_AUDIO);
	}

	// Indicates whether the audio engine is currently in a failure state
	CMPINLINE bool						IsInErrorState(void) const			{ return m_in_error_state; }

	// Returns the audio listener centered at the player position and orientation
	CMPINLINE static const DirectX::AudioListener & 
										GetPlayerAudioListener(void)		{ return PLAYER_AUDIO_LISTENER; }

	// Play a one-shot sound effect based on the specified audio resource
	// Volume: default = 1
	// PitchShift: In the range [-1 +1], default with no shift = 0
	// Pan: In the range [-1 = full left, +1 = full right], default with no panning = 0
	void								Play(AudioItem::AudioID id, float volume, float pitch_shift, float pan); 
	void								Play(const std::string & name, float volume, float pitch_shift, float pan);

	// Play a one-shot sound effect based on the specified audio resource
	CMPINLINE void						Play(AudioItem::AudioID id) 		{ Play(id, AudioManager::DEFAULT_VOLUME, AudioManager::DEFAULT_PITCH_SHIFT, AudioManager::DEFAULT_PAN); }
	CMPINLINE void						Play(const std::string & name)		{ Play(name, AudioManager::DEFAULT_VOLUME, AudioManager::DEFAULT_PITCH_SHIFT, AudioManager::DEFAULT_PAN); }
	
	// Ensures that a slot is available for a new audio instance.  Will terminate an instance of the current 
	// audio item if necessary to remain under the global audio instance limit
	void								EnsureInstanceIsAvailable(AudioItem::AudioID id, bool requires_3d_support);

	// Create a new instance of an audio item, if posssible.  Returns non-zero if instantiation fails
	AudioInstance::AudioInstanceIdentifier				CreateInstance(AudioItem::AudioID id, float volume_modifier = 1.0f);
	CMPINLINE AudioInstance::AudioInstanceIdentifier	CreateInstance(const std::string & name, float volume_modifier = 1.0f) { 
		return CreateInstance(GetAudioID(name), volume_modifier); 
	}

	// Create a new 3D instance of an audio item, if possible.  Returns non-zero if instantiation fails
	AudioInstance::AudioInstanceIdentifier				Create3DInstance(AudioItem::AudioID id, const XMFLOAT3 & position, float volume_modifier = 1.0f);
	CMPINLINE AudioInstance::AudioInstanceIdentifier	Create3DInstance(const std::string & name, const XMFLOAT3 & position, float volume_modifier = 1.0f) { 
		return Create3DInstance(GetAudioID(name), position, volume_modifier); 
	}

	// Update the player audio listener to the current player position and orientation
	void								UpdatePlayerAudioListener(void);

	// Perform a periodic audit of active instances and update our internal state
	void								PerformPeriodicAudit(void);

	// Iterates through all audio items to get an accurate count of active audio instances.  This is required since
	// during per-frame operation the audio manager will only approximate this count based on incrementing for new 
	// instances created.  It will not search for completed instances per-frame for efficiency
	AudioInstance::AudioInstanceID		DetermineExactAudioInstanceCount(void);

	// Returns the number of active instances being maintained by this audio manager, across all audio items.  This is
	// an upper-bound approximation that is periodically corrected downwards if necessary by the audit process
	CMPINLINE AudioInstance::AudioInstanceID GetTotalAudioInstanceCount(void) const { return m_instance_count; }

	// Generates a new instance identifier, which is unique across all audio items
	CMPINLINE static AudioInstance::AudioInstanceIdentifier GetNewInstanceIdentifier(void) { return ++AUDIO_INSTANCE_COUNTER; }

	// Pauses global sound playback
	CMPINLINE void						PausePlayback(void)					{ m_engine->Suspend(); }

	// Resumes global sound playback
	CMPINLINE void						ResumePlayback(void)				{ m_engine->Resume(); }

	// Shutdown method to deallocate all audio manager resources
	Result								Shutdown(void);

	// Default destructor
	~AudioManager(void);


private:

	// Core audio engine component
	DirectX::AudioEngine *								m_engine;

	// Collection of all loaded audio entries.  Items are never removed, so collection index is used
	// as an identifier
	AudioItem::AudioCollection							m_sounds;

	// Map of audio item names to the corresponding audio ID
	AudioItem::AudioIDMap								m_sound_name_map;

	// Count of total audio resources registered with this manager
	AudioItem::AudioID									m_audio_count;

	// Count of total audio instances that are currently active.  Updated on a periodic basis by the AudioManager
	AudioInstance::AudioInstanceID						m_instance_count;

	// Flag indicating whether we are currently in an error state
	bool												m_in_error_state;

	// Clock time at which we should perform the next periodic audit
	unsigned int										m_next_audit_time;

	// Clock time at which the last object binding check was performed
	unsigned int										m_object_bindings_last_valid;

	// Single STATIC audio listener centered at the player position & orientation, used for all 3D audio calculations
	static DirectX::AudioListener						PLAYER_AUDIO_LISTENER;

	// Position and orientation of the player listener, stored locally for more efficient per-frame calculations
	// TODO: orientation removed, may not be required locally?
	XMFLOAT3											m_player_listener_position;

	// Generates a new instance identifier, which is unique across all audio items
	static AudioInstance::AudioInstanceIdentifier		AUDIO_INSTANCE_COUNTER;

	// Updates the instance_count total to reflect the creation of a new instance.  This count is however
	// an upper-bound estimate that is refined periodically when the audio manager checks for completed instances
	CMPINLINE void										RecordNewInstanceCreation()			{ ++m_instance_count; }

	// Identify any object-audio bindings that are no longer valid, terminate them and reclaim the instance resources
	// Accepts the time of the relevant audit verification as a parameter.  Any binding which does not have that
	// same clock time as it's "valid_at" parameter will be cleaned up
	void												TerminateExpiredObjectBindings(unsigned int verification_time);
	
	// Identify any new object-audio bindings that need to be made based on proximity to the player
	void												GenerateNewObjectBindings(void);

	// Generate new audio bindings for objects in the current player environment.  Bindings are established for new objects which get 
	// within audio_inner_range of the player
	void												GenerateNewEnvironmentObjectBindings(iSpaceObjectEnvironment *env, iEnvironmentObject *listener, 
																							 float audio_inner_range, float audio_outer_range, float volume_modifier);

	// Generate new audio bindings for objects in the current player environment.  Bindings are established for new objects which get
	// within audio_inner_range of the player
	void												GenerateNewSpaceObjectBindings(iObject *listener, float audio_inner_range, 
																					   float audio_outer_range, float volume_modifier);

	// Update any active object/audio bindings based on the current world state
	void												UpdateObjectAudioBindings(unsigned int verification_time);

	// Store position and orientation data for the player listener
	void												UpdatePlayerListenerPositionData(const FXMVECTOR position, const FXMVECTOR orientation);

	// Squared versions of object binding audio for runtime efficiency
	static const float									SPACE_AUDIO_MAX_RANGE_SQ;
	static const float									SPACE_AUDIO_INNER_RANGE_SQ;
	static const float									ENV_AUDIO_MAX_RANGE_SQ;
	static const float									ENV_AUDIO_INNER_RANGE_SQ;
	static const float									ENV_SPACE_AUDIO_MAX_RANGE_SQ;
	static const float									ENV_SPACE_AUDIO_INNER_RANGE_SQ;

	// Struct holding the binding between an object and some audio instance
	class AudioInstanceObjectBinding
	{
	public:

		// Create a new binding
		CMPINLINE AudioInstanceObjectBinding(iObject *object, AudioItem::AudioID audio_item_id,
			AudioInstance::AudioInstanceIdentifier instance_identifier, float max_dist_sq)
			:
			m_object(object), m_item_id(audio_item_id), m_identifier(instance_identifier), 
			m_max_dist_sq(max_dist_sq), m_last_valid(0U)
		{
			m_obj_id = (object ? object->GetID() : 0);
		}
			

		// Disallow any copy construction or assignment
		CMPINLINE AudioInstanceObjectBinding(const AudioInstanceObjectBinding & other) = delete;
		CMPINLINE AudioInstanceObjectBinding & operator=(const AudioInstanceObjectBinding & other) = delete;

		// Move constructor with a noexcept guarantee to ensure it is used by STL containers
		CMPINLINE AudioInstanceObjectBinding(AudioInstanceObjectBinding && other) noexcept
			: m_object(std::move(other.m_object)), m_obj_id(other.m_obj_id), m_item_id(other.m_item_id), 
			m_identifier(other.m_identifier), m_max_dist_sq(other.m_max_dist_sq), m_last_valid(other.m_last_valid) { }

		// Move assignment with a noexcept guarantee to ensure it is used by STL containers
		AudioInstanceObjectBinding & operator=(const AudioInstanceObjectBinding && other) noexcept;


		// Return data from the binding
		CMPINLINE const iObject *							GetObj(void) const { return m_object(); }
		CMPINLINE Game::ID_TYPE								GetObjectID(void) const { return m_obj_id; }
		CMPINLINE AudioItem::AudioID						GetAudioItemID(void) const { return m_item_id; }
		CMPINLINE AudioInstance::AudioInstanceIdentifier	GetInstanceIdentifier(void) const { return m_identifier; }
		CMPINLINE float										GetMaxDistSq(void) const { return m_max_dist_sq; }
		
		// Clock time at which this object binding was last confirmed to be valid
		CMPINLINE unsigned int								GetLastValid(void) const { return m_last_valid; }
		CMPINLINE void										SetLastValid(unsigned int clock_ms) { m_last_valid = clock_ms; }

		// Destructor with a noexcept guarantee to ensure move semantics are used by STL containers
		CMPINLINE ~AudioInstanceObjectBinding(void) noexcept { }

		// Comparator for efficient sorting and searching of objects
		static const struct _ComparatorLessThan
		{
			CMPINLINE bool operator() (const AudioInstanceObjectBinding & left, const AudioInstanceObjectBinding & right) { return (left.GetObjectID() < right.GetObjectID()); }
			CMPINLINE bool operator() (const AudioInstanceObjectBinding & left, Game::ID_TYPE right) { return (left.GetObjectID() < right); }
			CMPINLINE bool operator() (Game::ID_TYPE left, const AudioInstanceObjectBinding & right) { return (left < right.GetObjectID()); }
		} ComparatorLessThan;

	private:

		ObjectReference<iObject>							m_object;
		Game::ID_TYPE										m_obj_id;
		AudioItem::AudioID									m_item_id;
		AudioInstance::AudioInstanceIdentifier				m_identifier;

		// Distance beyond which the audio instance will be terminated
		float												m_max_dist_sq;	
		
		// Clock time at which this binding was last confirmed to be valid
		unsigned int										m_last_valid;
	};


	// Vector of currently active object-audio bindings
	std::vector<AudioInstanceObjectBinding>					m_object_bindings;

};






