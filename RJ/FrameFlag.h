#pragma once

#include "CompilerSettings.h"
#include "GameVarsExtern.h"

// Class maintaining a flag that is automatically reset each frame.  Uses Game::PersistentTimeMs in order
// to maintain a last-updated point that is unique per frame (TODO: technically, relies on a guarantee
// that there will not be more than one frame with the same Ms time, i.e. we stay below 1000FPS.  Could
// make this an explicit guarantee in the render cycle if really necessary)
class FrameFlag
{
public:

	CMPINLINE FrameFlag(void) : m_flag_time(0U)		{ }

	// Indicates whether the flag is currently set
	CMPINLINE bool IsSet(void) const				{ return (m_flag_time == Game::PersistentClockMs); }

	// Indicates whether the flag was set in the previous frame (for e.g. render-based flags
	// where rendering typically takes place in frame N-1 before simulation & testing of the flag in frame N)
	CMPINLINE bool WasSetInPriorFrame(void) const { return (m_flag_time == Game::PreviousPersistentClockMs); }

	// Indicates whether the flag was set in this or the previous frame (for e.g. render-based flags
	// where rendering typically takes place in frame N-1 before simulation & testing of the flag in frame N)
	CMPINLINE bool WasSetSincePriorFrame(void) const { return (m_flag_time >= Game::PreviousPersistentClockMs); }

	// Sets the flag for this frame
	CMPINLINE void Set(void)						{ m_flag_time = Game::PersistentClockMs; }

	// Clears the flag for this frame
	CMPINLINE void Clear(void)						{ m_flag_time = 0U; }

private:

	unsigned int									m_flag_time;

};