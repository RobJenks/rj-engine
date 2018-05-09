#pragma once

#ifndef __iUIComponentRenderableH__
#define __iUIComponentRenderableH__

#include "iUIComponent.h"

// Represents a UI component that can also be rendered directly.  This is primarily image/imagegroup rendering objects, 
// rather than higher-level objects such as buttons which are themselves composed of renderable image objects
class iUIComponentRenderable : public iUIComponent
{

public:

	// Constructor
	iUIComponentRenderable(void);
	iUIComponentRenderable(const std::string & code, XMFLOAT2 position, XMFLOAT2 size, float zorder, bool render = true, bool acceptsmouse = false, iUIControl *control = NULL);

	iUIComponentRenderable(const iUIComponentRenderable & other);
	iUIComponentRenderable(iUIComponentRenderable && other);
	iUIComponentRenderable & operator=(const iUIComponentRenderable & other);
	iUIComponentRenderable & operator=(iUIComponentRenderable && other);


	// Renderable objects must expose a render method
	virtual void						Render(void) = 0;
	
	// Objects must expose a Z value so the render manager can decide in which order to render them
	CMPINLINE float						GetZOrder(void) const { return m_zorder; }
	CMPINLINE void						SetZOrder(float z) { m_zorder = z; }
	void								AdjustZOrder(float delta);

	~iUIComponentRenderable(void);

protected:

	float								m_zorder;

}; 


#endif