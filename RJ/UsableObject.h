#pragma once

#include "CompilerSettings.h"
#include "AudioParameters.h"
class iObject;


class UsableObject
{
public:
	
	// Default constructor
	UsableObject(void);

	// Event raised when a usable object is used; virtual method to be implemented by subclasses, does nothing if not implemented
	// Should return a flag indicating whether the object was 'successfully' used; by default, when this does nothing, the method returns false
	virtual bool									OnUsed(iObject *user);

	// Return default audio to be played on interaction, if any
	CMPINLINE AudioParameters						GetSuccessfulInteractionAudio(void) const				{ return m_successful_interaction_audio; }
	CMPINLINE AudioParameters						GetFailedInteractionAudio(void) const					{ return m_failed_interaction_audio; }

	// Indicates whether the object has defined default audio parameters for player interaction
	CMPINLINE bool									HasDefinedSuccessfulInteractionAudio(void) const		{ return (m_successful_interaction_audio.Exists()); }
	CMPINLINE bool									HasDefinedFailedInteractionAudio(void) const			{ return (m_failed_interaction_audio.Exists()); }

	// Set default audio to be played on interaction, if any
	CMPINLINE void									SetSuccessfulInteractionAudio(AudioParameters audio)	{ m_successful_interaction_audio = audio; }
	CMPINLINE void									SetFailedInteractionAudio(AudioParameters audio)		{ m_failed_interaction_audio = audio; }


protected:

	// Audio feedback for successful or failed player interaction
	AudioParameters									m_successful_interaction_audio;
	AudioParameters									m_failed_interaction_audio;

};





