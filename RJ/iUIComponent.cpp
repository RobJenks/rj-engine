#include "iUIComponent.h"
#include "FastMath.h"
#include "DX11_Core.h"

iUIComponent::iUIComponent(void)
	:
	m_code(""), 
	m_position(0.0f, 0.0f), 
	m_size(0.0f, 0.0f), 
	m_render(true), 
	m_acceptsmouse(false), 
	m_control(NULL)
{
}

iUIComponent::iUIComponent(const std::string & code, XMFLOAT2 position, XMFLOAT2 size, bool render, bool acceptsmouse, iUIControl *control)
	:
	m_code(code),
	m_position(position),
	m_size(size),
	m_render(render),
	m_acceptsmouse(acceptsmouse),
	m_control(control)
{
}

iUIComponent::iUIComponent(const iUIComponent & other)
	:
	m_code(other.m_code), 
	m_position(other.m_position), 
	m_size(other.m_size), 
	m_render(other.m_render), 
	m_acceptsmouse(other.m_acceptsmouse), 
	m_control(other.m_control)
{
}

iUIComponent::iUIComponent(iUIComponent && other)
	:
	m_code(other.m_code),
	m_position(other.m_position),
	m_size(other.m_size),
	m_render(other.m_render),
	m_acceptsmouse(other.m_acceptsmouse),
	m_control(other.m_control)
{
}

iUIComponent & iUIComponent::operator=(const iUIComponent & other)
{
	m_code = other.m_code;
	m_position = other.m_position;
	m_size = other.m_size;
	m_render = other.m_render;
	m_acceptsmouse = other.m_acceptsmouse;
	m_control = other.m_control;

	return *this;
}

iUIComponent & iUIComponent::operator=(iUIComponent && other)
{
	m_code = other.m_code;
	m_position = other.m_position;
	m_size = other.m_size;
	m_render = other.m_render;
	m_acceptsmouse = other.m_acceptsmouse;
	m_control = other.m_control;

	return *this;
}


void iUIComponent::Move(XMFLOAT2 delta)
{
	SetPosition(Float2Add(GetPosition(), delta));
}

void iUIComponent::Resize(XMFLOAT2 delta)
{
	SetSize(Float2Add(GetSize(), delta));
}


iUIComponent::~iUIComponent(void)
{
}