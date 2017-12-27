#pragma once

#include "RenderProcess.h"


class DeferredRenderProcess : public RenderProcess
{
public:

	DeferredRenderProcess(void);

	// Primary rendering method; executes all deferred rendering operations
	virtual void Render(void);


	DeferredRenderProcess(void);


private:



};
