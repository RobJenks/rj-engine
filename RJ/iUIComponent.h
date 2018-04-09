#pragma once

#ifndef __iUIComponentH__
#define __iUIComponentH__

#include <string>
#include "CompilerSettings.h"
#include "DX11_Core.h"
class iUIControl;

class iUIComponent
{

public:

	iUIComponent(void);
	iUIComponent(const std::string & code, XMFLOAT2 position, XMFLOAT2 size, bool render = true, bool acceptsmouse = false, iUIControl *control = NULL);

	iUIComponent(const iUIComponent & other);
	iUIComponent(iUIComponent && other);
	iUIComponent & operator=(const iUIComponent & other);
	iUIComponent & operator=(iUIComponent && other);


	CMPINLINE std::string	GetCode(void) const						{ return m_code; }
	CMPINLINE void			SetCode(const std::string & code)		{ m_code = code; SetCodeEx(code); }
	
	CMPINLINE virtual void	SetCodeEx(const std::string & code)
	{
		// Virtual method which subclasses can override to take further action if required
	}

	CMPINLINE XMFLOAT2		GetPosition(void) const					{ return m_position; }
	CMPINLINE XMFLOAT2		GetSize(void) const						{ return m_size; }
	CMPINLINE void			SetPosition(XMFLOAT2 position)			{ m_position = position; }
	CMPINLINE void			SetSize(XMFLOAT2 size)					{ m_size = size; }
	void					Move(XMFLOAT2 delta);
	void					Resize(XMFLOAT2 delta);


	CMPINLINE virtual bool	GetRenderActive(void) const				{ return m_render; }
	CMPINLINE virtual void	SetRenderActive(bool render)			{ m_render = render; }

	CMPINLINE bool			AcceptsMouseInput(void)					{ return m_acceptsmouse; }
	CMPINLINE void			SetAcceptsMouseInput(bool flag)			{ m_acceptsmouse = flag; }

	CMPINLINE iUIControl *	GetParentControl(void) const			{ return m_control; }
	CMPINLINE void			SetParentControl(iUIControl *control)	{ m_control = control; }

	virtual void			Shutdown(void)
	{
		// Nothing required in base interface; subclasses should override as required
	}

	~iUIComponent(void);

protected:

	std::string				m_code;
	XMFLOAT2				m_position;
	XMFLOAT2				m_size;
	bool					m_render;
	bool					m_acceptsmouse;
	iUIControl *			m_control;

};



#endif