#pragma once

#include <string>
#include <iostream>
#include <sstream>


class TransformerComponent
{
public:

#	define TRANSFORM_INFO std::cout << "Info [" << GetName() << "]: " 
#	define TRANSFORM_ERROR std::cerr << "Error [" << GetName() << "]: " << ReportError()

	// Default constructor
	TransformerComponent(void)
		:
		m_has_errors(false)
	{}

	// Implemented by each transformer component subclass
	virtual std::string							GetName(void) const = 0;

	// Records whether the transformer component is in a valid state.  Updated following each evaluation of 
	// the component.  A value of 'false' indicates the last evaluation encountered some errors
	inline bool									HasErrors(void) const { return m_has_errors; }

	// Reset the component ready for evaluation
	inline void									Reset(void) const { m_has_errors = false; }

protected:

	// Flags a failure during this evaluation; added as no-op during error logging to make this a minimal change
	inline std::string							ReportError(void) const { m_has_errors = true; return ""; }

private:

	// Record of any failures encounterd during the evalution
	mutable bool								m_has_errors;

};