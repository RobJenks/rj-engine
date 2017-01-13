#include "MemDebug.h"
#include "MemDebugCheck.h"


// Constructor; begins heap verification on every operation
MemDebugCheck::MemDebugCheck(void)
{
	// Checkpoint the previous debug state
	MemDebug::CheckpointCRTHeapDebugState();

	// Begin per-operation heap verification
	MemDebug::SetHeapIntegrityVerificationState(MemDebug::HeapValidationState::VerifyOnEveryOperation);
}


// Destructor; ends heap verification and restores to previous state
MemDebugCheck::~MemDebugCheck(void)
{
	// Restore previous heap verification state
	MemDebug::RestoreCRTHeapDebugState();
}

