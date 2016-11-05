#pragma once

#ifndef __iUIComponentH__
#define __iUIComponentH__

#include <string>
#include "CompilerSettings.h"

class iUIComponent
{

public:

	iUIComponent(void)		: m_code(""), m_render(true) { }

	CMPINLINE std::string	GetCode(void) const						{ return m_code; }
	CMPINLINE void			SetCode(const std::string & code)		{ m_code = code; SetCodeEx(code); }
	
	CMPINLINE virtual void	SetCodeEx(const std::string & code)
	{
		// Virtual method which subclasses can override to take further action if required
	}


	CMPINLINE virtual bool	GetRenderActive(void) const				= 0;
	CMPINLINE virtual void	SetRenderActive(bool render)			= 0;

	virtual void			Shutdown(void) = 0;

protected:

	std::string				m_code;
	bool					m_render;

};



#endif