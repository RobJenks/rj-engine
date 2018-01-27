#pragma once

#include "Utility.h"
#include "ShaderDX11.h"
class DeferredRenderProcess;

class RenderProcess
{
public:

	virtual void					Render(void) = 0;


	// Static method which returns the name of a given render process
	template <class T>
	static constexpr const char * Name(void);

private:



};


// Static method which returns the name of a given render process; all render processes should be registered here
template <class T>		constexpr const char * RenderProcess::Name(void) { return "<unknown>"; }
template <>				constexpr const char * RenderProcess::Name<DeferredRenderProcess>(void) { return STRING(DeferredRenderProcess); }


