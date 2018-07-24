#pragma once

#include <string>
#include "CompilerSettings.h"


class PostProcessComponent
{
public:

	// Constructors
	CMPINLINE PostProcessComponent(void) : m_code(""), m_description(""), m_active(true) { }
	CMPINLINE PostProcessComponent(const std::string & code, const std::string & desc) : m_code(code), m_description(desc), m_active(true) { }


	// Returns the (internal) string code for this post-processing component
	CMPINLINE std::string				GetCode(void) const { return m_code; }

	// Returns the string description of this post-processing component
	CMPINLINE std::string				GetDescription(void) const { return m_description; }

	// Virtual event which can be handled by subclasses; triggered when component is activated or deactivated
	CMPINLINE virtual void				ActiveStateChanged(bool is_active) { }

	// Determines whether the post-processing effect will be applied during rendering
	CMPINLINE bool						IsActive(void) const { return m_active; }
	CMPINLINE void						SetActive(bool active) 
	{ 
		if (active == m_active) return;

		m_active = active; 
		ActiveStateChanged(active);
	}
	CMPINLINE void						Activate(void) { SetActive(true); }
	CMPINLINE void						Deactivate(void) { SetActive(false); }

	// Response to a change in shader configuration or a reload of shader bytecode
	virtual void						ShadersReloaded(void) { }


private:

	std::string							m_code;
	std::string							m_description;
	bool								m_active;

};