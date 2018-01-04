#pragma once

#include "CompilerSettings.h"
#include "Model.h"


class ModelRenderPredicate
{
public:

	/* Applies no filtering; all models will be rendered and predicate test should be inlined away */
	class RenderAll
	{
		CMPINLINE const bool operator()(const Model *model) const { return true; }
	};


};