#pragma once

#include "CompilerSettings.h"
#include "ModelBuffer.h"


class ModelRenderPredicate
{
public:

	/* Applies no filtering; all models will be rendered and predicate test should be inlined away */
	class RenderAll
	{
	public:
		CMPINLINE const bool operator()(const ModelBuffer *model) const { return true; }
	};

	/* Renders all non-transparent objects; transparent is defined as any model with alpha < 1.0 */
	// TODO: Should have a test on [initialisation?] of objects to ensure Material is always != NULL
	class RenderNonTransparent
	{
	public:
		CMPINLINE const bool operator()(const ModelBuffer *model) const { return !model->Material->IsTransparent(); }
	};


};