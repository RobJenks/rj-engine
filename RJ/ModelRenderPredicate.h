#pragma once

#include "CompilerSettings.h"
#include "ModelBuffer.h"


class ModelRenderPredicate
{
public:

	/* Applies no filtering; all models will be rendered and predicate test should be inlined away */
	class RenderAll
	{
		CMPINLINE const bool operator()(const ModelBuffer *model) const { return true; }
	};


};