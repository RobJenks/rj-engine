#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <Audio.h>
#include "CompilerSettings.h"

class AudioItem
{

public:

	// Collection type and ID used to uniquely identify audio items
	typedef std::vector<AudioItem>						AudioCollection;
	typedef std::vector<AudioItem>::size_type			AudioID;
	typedef std::unordered_map<std::string, AudioID>	AudioIDMap;

	// Enumeration of possible audio types.  Default = Effect
	enum AudioType { Effect = 0, Music, Voice };

	// Constructor with all mandatory parameters
	AudioItem(AudioID id, const std::string & name, AudioType type, const std::string & filename);

	// Copy construction and assignment is disallowed
	CMPINLINE AudioItem(const AudioItem & other) = delete;
	CMPINLINE AudioItem & operator=(const AudioItem & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than
	// copied by STL containers
	AudioItem(AudioItem && other) noexcept;


	// Return key parameters
	CMPINLINE AudioID									GetID(void) const			{ return m_id; }
	CMPINLINE std::string								GetName(void) const			{ return m_name; }
	CMPINLINE AudioType									GetType(void) const			{ return m_type; }
	CMPINLINE std::string								GetFilename(void) const		{ return m_filename; }


	// Return a reference to the sound effect object
	CMPINLINE DirectX::SoundEffect *					GetEffect(void)				{ return m_effect.get(); }

	// Assign an audio resource to this item
	Result												AssignResource(SoundEffect *resource);

	// Indicates whether the resources for this audio item have already been loaded
	CMPINLINE bool										ResourcesLoaded(void) const { return (m_effect.get() != NULL); }

	// Releases any audio resources currently held by this item
	CMPINLINE  void										ReleaseResources(void)		{ m_effect.release(); }



	// Destructor; no-exception guarantee to ensure these objects are moved rather than
	// copied by STL containers
	~AudioItem(void) noexcept;

	// Translate a sound type to/from its string representation
	static AudioType									TranslateAudioTypeFromString(const std::string & name);
	static std::string									TranslateAudioTypeToString(AudioType type);

private:

	AudioID										m_id;
	std::string									m_name;
	AudioType									m_type;
	std::string									m_filename;
	size_t										m_duration;	// ms, not accounting for any looping

	std::unique_ptr<DirectX::SoundEffect>		m_effect;


};
