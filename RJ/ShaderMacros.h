#pragma once

#include <string>
#include <vector>
#include <map>
#include "DX11_Core.h"

class ShaderMacros
{
public:

	typedef std::map<std::string, std::string>		MacroData;

	static const MacroData NONE;

	ShaderMacros(void);
	~ShaderMacros(void);

	// Add a new macro to the collection, overwriting any existing macro with the same name if applicable
	void											AddMacro(const std::string & name, const std::string & definition);
	
	// Returns a macro definition, or an empty string if the given macro does not exist
	std::string										GetMacroDefinition(const std::string & name) const;

	// Return the set of all macro definitions
	const MacroData & 								GetMacros(void) const;

	// Returns data on any currently-defined macros
	MacroData::size_type							GetDefinedMacroCount(void) const;
	bool											HasDefinedMacros(void) const;

	// Replace the entire set of shader macros with the given collection
	void											ReplaceMacros(const MacroData & macros);

	// Remove the macro with the given name, if one exists
	void											RemoveMacro(const std::string & name);

	// Clears all macro data
	void											ClearMacros(void);

	// Return a reference to the compiled macro set.  Will recompile if any changes have been made since the last compilation
	const D3D_SHADER_MACRO *						GetCompiledData(void);


private:

	MacroData										m_macros;
	std::vector<D3D_SHADER_MACRO>					m_macros_compiled;

	bool											m_isdirty;
};