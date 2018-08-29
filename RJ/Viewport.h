#pragma once

#include "CompilerSettings.h"
#include "DX11_Core.h"


class Viewport
{
public:

	// Constructor
	CMPINLINE Viewport(float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f)
		: 
		X(x),
		Y(y),
		Width(width),
		Height(height),
		MinDepth(minDepth),
		MaxDepth(maxDepth)
	{
	}

	// Viewport data
	float X;
	float Y;
	float Width;
	float Height;
	float MinDepth;
	float MaxDepth;

	// Generate a compiled version of this viewport
	void CompileViewport(D3D11_VIEWPORT & outCompiledViewport) const;
	D3D11_VIEWPORT CompileViewport(void) const;

	// Destructor
	CMPINLINE ~Viewport() { }

};