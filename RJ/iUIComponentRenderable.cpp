#include "iUIComponentRenderable.h"

// Constructor
iUIComponentRenderable::iUIComponentRenderable(void)
	:
	iUIComponent(), 
	m_zorder(0.0f)
{
}

// Constructor
iUIComponentRenderable::iUIComponentRenderable(const std::string & code, XMFLOAT2 position, XMFLOAT2 size, float zorder, bool render, bool acceptsmouse, iUIControl *control)
	:
	iUIComponent(code, position, size, render, acceptsmouse, control), 
	m_zorder(zorder)
{
}

iUIComponentRenderable::iUIComponentRenderable(const iUIComponentRenderable & other)
	:
	iUIComponent(other), 
	m_zorder(other.m_zorder)
{
}

iUIComponentRenderable::iUIComponentRenderable(iUIComponentRenderable && other)
	:
	iUIComponent(other),
	m_zorder(other.m_zorder)
{
}

iUIComponentRenderable & iUIComponentRenderable::operator=(const iUIComponentRenderable & other)
{
	iUIComponent::operator=(other);
	m_zorder = other.m_zorder;

	return *this;
}

iUIComponentRenderable & iUIComponentRenderable::operator=(iUIComponentRenderable && other)
{
	iUIComponent::operator=(other);
	m_zorder = other.m_zorder;

	return *this;
}



void iUIComponentRenderable::AdjustZOrder(float delta)
{
	SetZOrder(GetZOrder() + delta);
}


iUIComponentRenderable::~iUIComponentRenderable(void)
{
}
