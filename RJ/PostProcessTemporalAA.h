#pragma once

#include <vector>
#include "DX11_Core.h"
#include "PostProcessComponent.h"
class DeferredRenderProcess;


class PostProcessTemporalAA : public PostProcessComponent
{
public:

	// Initialise the postprocess and all required resources
	PostProcessTemporalAA(void);
	PostProcessTemporalAA(DeferredRenderProcess * render_process);



private:




};