#pragma once

#ifndef __iUIComponentH__
#define __iUIComponentH__

#include <string>
using namespace std;

class iUIComponent
{

public:

	virtual string			GetCode(void) = 0;
	virtual void			SetCode(string code) = 0;

	virtual bool			GetRenderActive(void) = 0;
	virtual void			SetRenderActive(bool render) = 0;

	virtual void			Shutdown(void) = 0;

};



#endif