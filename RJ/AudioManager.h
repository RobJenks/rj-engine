#pragma once

#include <Audio.h>
#include "ErrorCodes.h"

class AudioManager
{
public:

	// Static constants
	static const float					DEFAULT_VOLUME;
	static const float					DEFAULT_PITCH_SHIFT;
	static const float					DEFAULT_PAN;

	// Default constructor
	AudioManager(void);

	// Initialise all audio manager resources
	Result								Initialise(void);

	// Per-frame update method
	void								Update(void);

	// Indicates whether the audio engine is currently in a failure state
	CMPINLINE bool						IsInErrorState(void) const			{ return m_in_error_state; }

	// Play a one-shot sound effect based on the specified audio resource.  Less efficient than playing a
	// pre-loaded audio effect, so should be used sparingly
	// Volume: default = 1
	// PitchShift: In the range [-1 +1], default with no shift = 0
	// Pan: In the range [-1 = full left, +1 = full right], default with no panning = 0
	void								PlayOneShot(const wchar_t *filename, float volume, float pitch_shift, float pan) const;
	void								PlayOneShot(const std::string & filename, float volume, float pitch_shift, float pan) const;

	// Play a one-shot sound effect based on the specified audio resource.  Less efficient than playing a
	// pre-loaded audio effect, so should be used sparingly
	void								PlayOneShot(const wchar_t *filename);
	void								PlayOneShot(const std::string & filename);





	// Shutdown method to deallocate all audio manager resources
	Result								Shutdown(void);

	// Default destructor
	~AudioManager(void);


private:

	// Core audio engine component
	DirectX::AudioEngine *								m_engine;

	// Flag indicating whether we are currently in an error state
	bool												m_in_error_state;

	DirectX::SoundEffect *effect;
};






