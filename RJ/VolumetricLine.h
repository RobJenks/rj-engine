#pragma once

#ifndef __VolumetricLineH__
#define __VolumetricLineH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "FastMath.h"
class MaterialDX11;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
struct VolumetricLine : public ALIGN16<VolumetricLine>
{
	// Default values for additional parameter vector
	static const XMFLOAT4 DEFAULT_PARAMS;

	// Line endpoints P1 and P2
	AXMVECTOR				P1;
	AXMVECTOR				P2;

	// Line colour and alpha component
	XMFLOAT4				Colour;

	// Texture (if applicable; can be NULL for pure volumetric rendering)
	MaterialDX11 *			RenderMaterial;

	// Additional parameters.  x = line radius
	XMFLOAT4				Params;

	// Default constructor
	VolumetricLine(void) 
		:
		P1(NULL_VECTOR), P2(NULL_VECTOR), Colour(ONE_FLOAT4), RenderMaterial(NULL), Params(VolumetricLine::DEFAULT_PARAMS)
	{ 
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2)
		:
		P1(_P1), P2(_P2), Colour(ONE_FLOAT4), RenderMaterial(NULL), Params(VolumetricLine::DEFAULT_PARAMS)
	{
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2, const XMFLOAT4 & colour)
		:
		P1(_P1), P2(_P2), Colour(colour), RenderMaterial(NULL), Params(VolumetricLine::DEFAULT_PARAMS)
	{
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2, MaterialDX11 *texture)
		:
		P1(_P1), P2(_P2), Colour(ONE_FLOAT4), RenderMaterial(texture), Params(VolumetricLine::DEFAULT_PARAMS)
	{
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2, const XMFLOAT4 & colour, MaterialDX11 *texture)
		:
		P1(_P1), P2(_P2), Colour(colour), RenderMaterial(texture), Params(VolumetricLine::DEFAULT_PARAMS)
	{
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2, const XMFLOAT4 & colour, float radius, MaterialDX11 *texture)
		:
		P1(_P1), P2(_P2), Colour(colour), RenderMaterial(texture), Params(XMFLOAT4(radius, 0.0f, 0.0f, 0.0f))
	{
	}

	// Set all additional parameters in one call
	CMPINLINE void								SetParameters(const XMFLOAT4 & params)	{ Params = params; }

	// Retrieve or set the line radius
	CMPINLINE float								GetLineRadius(void) const				{ return Params.x; }
	CMPINLINE void								SetLineRadius(float radius)				{ Params.x = radius; }
};


#endif