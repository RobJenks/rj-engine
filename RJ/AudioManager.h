#pragma once

#include <Audio.h>
#include "ErrorCodes.h"

class AudioManager
{
public:

	// Default constructor
	AudioManager(void);

	// Initialise all audio manager resources
	Result Initialise(void);

	// Per-frame update method
	void Update(void);

	// Indicates whether the audio engine is currently in a failure state
	CMPINLINE bool IsInErrorState(void) const			{ return m_in_error_state; }



	// Shutdown method to deallocate all audio manager resources
	Result Shutdown(void);

	// Default destructor
	~AudioManager(void);


private:

	// Core audio engine component
	DirectX::AudioEngine *								m_engine;

	// Flag indicating whether we are currently in an error state
	bool												m_in_error_state;


};






