#pragma once

#ifndef __MemDebugCheckH__
#define __MemDebugCheckH__

// This functionality is only relevant in debug mode
#ifdef _DEBUG

#include "CompilerSettings.h"

class MemDebugCheck
{
public:

	// Constructor; begins heap verification on every operation
	MemDebugCheck(void);

	// Destructor; ends heap verification and restores to previous state
	~MemDebugCheck(void);

};


#endif

#endif