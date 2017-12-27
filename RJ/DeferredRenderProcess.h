#pragma once

#include "RenderProcess.h"
#include "DeferredGBuffer.h"


class DeferredRenderProcess : public RenderProcess
{
public:

	DeferredRenderProcess(void);

	// GBuffer holding all deferred rendering data and render targets
	DeferredGBuffer GBuffer;

	// Primary rendering method; executes all deferred rendering operations
	virtual void Render(void);


	DeferredRenderProcess(void);


private:



};
