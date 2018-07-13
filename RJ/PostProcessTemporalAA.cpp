#include "PostProcessTemporalAA.h"
#include "DeferredRenderProcess.h"



PostProcessTemporalAA::PostProcessTemporalAA(void)
{
	// Only for default construction
}

PostProcessTemporalAA::PostProcessTemporalAA(DeferredRenderProcess * render_process)
	:
	PostProcessComponent("temporal-aa", "Temporal Anti-Aliasing")
{
}


