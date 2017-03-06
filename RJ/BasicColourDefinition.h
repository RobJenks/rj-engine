#pragma once

#ifndef __BasicColourDefinitionH__
#define __BasicColourDefinitionH__

#include <string>
#include "DX11_Core.h"
#include "FastMath.h"

struct BasicColourDefinition 
{
	XMFLOAT4 colour; std::string name;

	BasicColourDefinition(void) : colour(NULL_FLOAT4), name("") { }
	BasicColourDefinition(const XMFLOAT4 & _colour, const std::string & _name) : colour(_colour), name(_name) { }
};


#endif