#pragma once

#ifndef __Image2DRenderGroupInstanceH__
#define __Image2DRenderGroupInstanceH__

#include <string>
#include "CompilerSettings.h"
#include "Image2DRenderGroup.h"
#include "iUIComponent.h"

class Image2DRenderGroupInstance : public iUIComponent
{
public:
	Image2DRenderGroupInstance(void);
	//Image2DRenderGroupInstance(Image2DRenderGroup *rg, string key);

	~Image2DRenderGroupInstance(void);

	// Methods to get and set the render group pointer
	CMPINLINE Image2DRenderGroup *	GetRenderGroup(void) { return m_rendergroup; }
	CMPINLINE void					SetRenderGroup(Image2DRenderGroup *rg) { m_rendergroup = rg; }
	
	// Methods to get and set the string index into the render group for this instance
	CMPINLINE string				GetKey(void) { return m_key; }	
	CMPINLINE void					SetKey(string key) { m_key = key; }

	// Method to test the render active flag for this instance
	CMPINLINE bool					GetRenderActive(void) const
	{
		Image2DRenderGroup::Instance *instance = m_rendergroup->GetInstanceByCode(m_key);
		if (instance && instance->render) return true; else return false;
	}

	// Method to set the render active flag for this instance
	void							SetRenderActive(bool active)
	{
		Image2DRenderGroup::Instance *instance = m_rendergroup->GetInstanceByCode(m_key);
		if (instance) instance->render = active;
	}

private:
	Image2DRenderGroup *		m_rendergroup;
	string						m_key;

};


#endif