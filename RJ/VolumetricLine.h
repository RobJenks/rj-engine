#pragma once

#ifndef __VolumetricLineH__
#define __VolumetricLineH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "FastMath.h"
class Texture;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
struct VolumetricLine : public ALIGN16<VolumetricLine>
{
	// Default values for additional parameter vector
	static const AXMVECTOR DEFAULT_PARAMS;

	// Line endpoints P1 and P2
	AXMVECTOR				P1;
	AXMVECTOR				P2;

	// Line colour and alpha component
	AXMVECTOR				Colour;

	// Texture (if applicable; can be NULL for pure volumetric rendering)
	Texture *				RenderTexture;

	// Additional parameters.  x = line radius
	AXMVECTOR				Params;

	// Default constructor
	VolumetricLine(void) 
		:
		P1(NULL_VECTOR), P2(NULL_VECTOR), Colour(ONE_VECTOR), RenderTexture(NULL), Params(VolumetricLine::DEFAULT_PARAMS)
	{ 
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2)
		:
		P1(_P1), P2(_P2), Colour(ONE_VECTOR), RenderTexture(NULL), Params(VolumetricLine::DEFAULT_PARAMS)
	{
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2, const FXMVECTOR colour)
		:
		P1(_P1), P2(_P2), Colour(colour), RenderTexture(NULL), Params(VolumetricLine::DEFAULT_PARAMS)
	{
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2, Texture *texture)
		:
		P1(_P1), P2(_P2), Colour(ONE_VECTOR), RenderTexture(texture), Params(VolumetricLine::DEFAULT_PARAMS)
	{
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2, const FXMVECTOR colour, Texture *texture)
		:
		P1(_P1), P2(_P2), Colour(colour), RenderTexture(texture), Params(VolumetricLine::DEFAULT_PARAMS)
	{
	}

	// Constructor
	VolumetricLine(const FXMVECTOR _P1, const FXMVECTOR _P2, const FXMVECTOR colour, float radius, Texture *texture)
		:
		P1(_P1), P2(_P2), Colour(colour), RenderTexture(texture), Params(XMVectorSetX(NULL_VECTOR, radius))
	{
	}

	// Set all additional parameters in one call
	CMPINLINE void								SetParameters(const FXMVECTOR params)	{ Params = params; }

	// Retrieve or set the line radius
	CMPINLINE float								GetLineRadius(void) const				{ return XMVectorGetX(Params); }
	CMPINLINE void								SetLineRadius(float radius)				{ Params = XMVectorSetX(Params, radius); }
};


#endif