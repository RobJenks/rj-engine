#pragma once

#include "Utility.h"
#include "ShaderDX11.h"
class DeferredRenderProcess;

class RenderProcess
{
public:

	// Constructor
	RenderProcess(void);

	// Virtual render method; must be implemented by all derived render processess
	virtual void					Render(void) = 0;

	// Perform any initialisation that cannot be completed on construction, e.g. because it requires
	// data that is read in from disk during the data load process
	virtual void					PerformPostDataLoadInitialisation(void) = 0;

	// Return the name of this render process instance
	std::string						GetName(void) const { return m_name; }

	// Static method which returns the name of a given render process class
	template <class T>
	static constexpr const char *	Name(void);


protected:

	std::string						m_name;

	CMPINLINE void					SetName(const std::string & name) { m_name = name; }

};


// Static method which returns the name of a given render process; all render processes should be registered here
template <class T>		constexpr const char * RenderProcess::Name(void) { return "<unknown>"; }
template <>				constexpr const char * RenderProcess::Name<DeferredRenderProcess>(void) { return STRING(DeferredRenderProcess); }


