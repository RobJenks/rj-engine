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

	// Determines whether the post-processing effect will be applied during rendering
	CMPINLINE bool						IsActive(void) const { return m_active; }
	CMPINLINE void						SetActive(bool active) { m_active = active; }
	CMPINLINE void						Activate(void) { SetActive(true); }
	CMPINLINE void						Deactivate(void) { SetActive(false); }



private:

	std::string							m_code;
	std::string							m_description;
	bool								m_active;

};