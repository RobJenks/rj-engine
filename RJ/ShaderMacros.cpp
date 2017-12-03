#include "ShaderMacros.h"
#include "Utility.h"
#include "DX11_Core.h"


ShaderMacros::ShaderMacros(void)
	:
	m_isdirty(false)
{
}

ShaderMacros::~ShaderMacros(void)
{
}

// Add a new macro to the collection, overwriting any existing macro with the same name if applicable
void ShaderMacros::AddMacro(const std::string & name, const std::string & definition)
{
	m_macros[name] = definition;
	m_isdirty = true;
}

// Returns a macro definition, or an empty string if the given macro does not exist
std::string ShaderMacros::GetMacroDefinition(const std::string & name) const
{
	MacroData::const_iterator it = m_macros.find(name);
	return (it != m_macros.end() ? it->second : NullString);
}

// Return the set of all macro definitions
const ShaderMacros::MacroData & ShaderMacros::GetMacros(void) const
{
	return m_macros;
}

// Remove the macro with the given name, if one exists
void ShaderMacros::RemoveMacro(const std::string & name)
{
	MacroData::const_iterator it = m_macros.find(name);
	if (it != m_macros.end)
	{
		m_macros.erase(it);
		m_isdirty = true;
	}
}

// Return a reference to the compiled macro set.  Will recompile if any changes have been made since the last compilation
const ShaderMacros::CompiledMacroData & const ShaderMacros::GetCompiledData(void)
{
	if (m_isdirty)
	{
		// Recompile all macro data
		m_macros_compiled.clear();
		for (const auto & macro : m_macros)
		{
			std::string name = macro.first;
			std::string definition = macro.second;

			char* c_name = new char[name.size() + 1];
			char* c_definition = new char[definition.size() + 1];

			strncpy_s(c_name, name.size() + 1, name.c_str(), name.size());
			strncpy_s(c_definition, definition.size() + 1, definition.c_str(), definition.size());

			m_macros_compiled.push_back( { c_name, c_definition } );
		}

		m_isdirty = false;
	}

	return m_macros_compiled;
}