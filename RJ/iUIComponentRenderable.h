#pragma once

#ifndef __iUIComponentRenderableH__
#define __iUIComponentRenderableH__

#include "iUIComponent.h"

// Represents a UI component that can also be rendered directly.  This is primarily image/imagegroup rendering objects, 
// rather than higher-level objects such as buttons which are themselves composed of renderable image objects
class iUIComponentRenderable : public iUIComponent
{

public:

	// Renderable objects must expose a render method
	virtual void						Render(void) = 0;
	
	// Objects must expose a Z value so the render manager can decide in which order to render them
	virtual float						GetZOrder(void) = 0;
	virtual void						SetZOrder(float z) = 0;

	// Objects must also expose methods to provide rendering info to the shader
	virtual int							GetIndexCount(void) = 0;
	virtual ID3D11ShaderResourceView *	GetTexture(void) = 0;
}; 


#endif