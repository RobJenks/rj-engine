#pragma once

#include "CompilerSettings.h"
#include "GameVarsExtern.h"
#include "ObjectReference.h"
#include "Audio.h"
class iObject;



// Struct holding the binding between an object and some audio instance
class AudioInstanceObjectBinding
{
public:

	// Create a new binding
	CMPINLINE AudioInstanceObjectBinding(iObject *object, Audio::AudioID audio_item_id,
		Audio::AudioInstanceIdentifier instance_identifier, float max_dist_sq) noexcept
		:
		m_object(object), m_item_id(audio_item_id), m_identifier(instance_identifier),
		m_max_dist_sq(max_dist_sq), m_last_valid(0U)
	{
		m_obj_id = (object ? object->GetID() : 0);
	}

	// Default contructor (for STL)
	CMPINLINE AudioInstanceObjectBinding(void) noexcept
		:
	m_object((iObject*)NULL), m_obj_id(0L), m_item_id(0U), m_identifier(0U), m_max_dist_sq(0.0f), m_last_valid(0U) { }


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
	CMPINLINE Audio::AudioID							GetAudioItemID(void) const { return m_item_id; }
	CMPINLINE Audio::AudioInstanceIdentifier			GetInstanceIdentifier(void) const { return m_identifier; }
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
	Audio::AudioID										m_item_id;
	Audio::AudioInstanceIdentifier						m_identifier;

	// Distance beyond which the audio instance will be terminated
	float												m_max_dist_sq;

	// Clock time at which this binding was last confirmed to be valid
	unsigned int										m_last_valid;
};