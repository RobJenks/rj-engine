#pragma once

#include "CompilerSettings.h"
#include "Utility.h"
#include "ShaderFlags.h"
#include "RM_ModelDataCollection.h"


class ShaderRenderPredicate
{
public:

	/* Applies no filtering; all shaders will be rendered and predicate test should be inlined away */
	class RenderAll
	{
	public:
		CMPINLINE const bool operator()(const RM_ModelDataCollection & shader) const { return true; }
	};

	/* Renders only geometry; i.e. all input to the primary engine render process */
	class RenderGeometry
	{
	public:
		CMPINLINE const bool operator()(const RM_ModelDataCollection & shader) const { return CheckBit_Any(shader.Flags, static_cast<ShaderFlags>(ShaderFlag::ShaderTypeGeometry)); }
	};

	/* Renders only UI and other orthographic/textured quad data */
	class RenderUI
	{
	public:
		CMPINLINE const bool operator()(const RM_ModelDataCollection & shader) const { return CheckBit_Any(shader.Flags, static_cast<ShaderFlags>(ShaderFlag::ShaderTypeUI)); }
	};

};