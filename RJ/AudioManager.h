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





	// Shutdown method to deallocate all audio manager resources
	Result Shutdown(void);

	// Default destructor
	~AudioManager(void);


private:



};






