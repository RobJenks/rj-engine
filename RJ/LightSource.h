#pragma once

#ifndef __LightSourceH__
#define __LightSourceH__

#include "ALIGN16.h"
#include "iObject.h"
#include "Light.h"

class LightSource : public ALIGN16<LightSource>, public iObject
{
public:

	// Specify the correct aligned allocators to use for this object type
	USE_ALIGN16_ALLOCATORS(LightSource)

	// Default constructor
	LightSource(void);

	// Return or set the light data for this light source
	CMPINLINE const Light &								GetLight(void) const				{ return m_light; }
	void												SetLight(const Light & data);

	// Importance of the light for use when rendering needs to be prioritised
	CMPINLINE int										GetPriority(void) const				{ return m_priority; }
	CMPINLINE void										SetPriority(int priority)			{ m_priority = priority; }


protected:

	// Light data
	Light									m_light;

	// Importance of the light for use when rendering needs to be prioritised
	int										m_priority;

};






#endif






