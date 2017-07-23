#include "MemDebug.h"

// This functionality is only relevant in debug mode
#ifdef _DEBUG

	// Initialise static fields
	int MemDebug::_state_checkpoint = 0;

#endif