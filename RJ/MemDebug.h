#pragma once

#ifndef __MemDebugH__
#define __MemDebugH__

// This functionality is only relevant in debug mode
#ifdef _DEBUG

#include <crtdbg.h>
#include "CompilerSettings.h"
#include "Utility.h"
#include "MemDebugCheck.h"

class MemDebug
{

public:

	enum HeapValidationState { NoValidation = 0, VerifyEvery16, VerifyEvery128, VerifyEvery1024, VerifyOnEveryOperation };

	// Enables validation of heap integrity on every heap allocation.  Major performance impact so 
	// only for use when debugging heap corruption issues
	CMPINLINE static void SetHeapIntegrityVerificationState(HeapValidationState validationstate)
	{
		// Retrieve the current state of the CRT debug memory flag
		OutputDebugString("Changing debug heap verification state");
		int state = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

		// Take different action depending on desired state
		switch (validationstate)
		{
			case HeapValidationState::VerifyEvery16:
				state &= ~_CRTDBG_CHECK_ALWAYS_DF;							// Disable the every-operation flag
				state = (state & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_16_DF;	// Clear the upper 16 bits and OR in the desired frequency
				break;

			case HeapValidationState::VerifyEvery128:
				state &= ~_CRTDBG_CHECK_ALWAYS_DF;							// Disable the every-operation flag
				state = (state & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_128_DF;	// Clear the upper 16 bits and OR in the desired frequency
				break;

			case HeapValidationState::VerifyEvery1024:
				state &= ~_CRTDBG_CHECK_ALWAYS_DF;							// Disable the every-operation flag
				state = (state & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_1024_DF;	// Clear the upper 16 bits and OR in the desired frequency
				break;

			case HeapValidationState::VerifyOnEveryOperation:
				state = (state & 0x0000FFFF) | _CRTDBG_CHECK_DEFAULT_DF;	// Clear the upper 16 bits and OR with zero, i.e. no checks
				state |= _CRTDBG_CHECK_ALWAYS_DF;							// Enable heap verification on every operation
				break;

			default:
				state = (state & 0x0000FFFF) | _CRTDBG_CHECK_DEFAULT_DF;	// Clear the upper 16 bits and OR with zero, i.e. no checks
				state &= ~_CRTDBG_CHECK_ALWAYS_DF;							// Disable the every-operation flag
				break;
		}

		// Store the altered CRT debug memory state
		_CrtSetDbgFlag(state);
	}

	// Records a snapshot of the current CRT debug state, which can be restored later 
	// with RestoreCRTHeapDebugState()
	CMPINLINE static void CheckpointCRTHeapDebugState(void)
	{
		MemDebug::_state_checkpoint = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		OutputDebugString("Taking checkpoint of debug heap verification state");
	}

	// Restores a previously-recorded checkpoint of CRT debug state.  Behaviour is undefined
	// (and likely very bad) if invoked without a prior call to CheckpointCRTHeapDebugState()
	CMPINLINE static void RestoreCRTHeapDebugState(void)
	{
		_CrtSetDbgFlag(MemDebug::_state_checkpoint);
		OutputDebugString("Restoring debug heap verification state\n");
	}

	// Performs spot-validation of all heap operations.  Based on lifetime of created object so should
	// be called within appropriate scope (and never heap-allocated...)
	CMPINLINE static MemDebugCheck AnalyseLocalHeapOperations(void)
	{
		return MemDebugCheck();
	}


	
private:

	// Record of the prior CRT debug state, recorded using the checkpoint method
	static int _state_checkpoint;

};


#endif

#endif