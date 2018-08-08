#include "ShaderMacros.h"
#include "Utility.h"
#include "DX11_Core.h"


const ShaderMacros::MacroData ShaderMacros::NONE = { };


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

// Returns data on any currently-defined macros
ShaderMacros::MacroData::size_type ShaderMacros::GetDefinedMacroCount(void) const
{
	return m_macros.size();
}

// Returns data on any currently-defined macros
bool ShaderMacros::HasDefinedMacros(void) const
{
	return !m_macros.empty();
}

// Replace the entire set of shader macros with the given collection
void ShaderMacros::ReplaceMacros(const MacroData & macros)
{
	// Shortcut to catch the usual case
	if (macros.empty() && m_macros.empty()) return;

	// Store the new macros and flag as dirty, regardless of contents
	m_macros = macros;
	m_isdirty = true;
}

// Remove the macro with the given name, if one exists
void ShaderMacros::RemoveMacro(const std::string & name)
{
	MacroData::const_iterator it = m_macros.find(name);
	if (it != m_macros.end())
	{
		m_macros.erase(it);
		m_isdirty = true;
	}
}

// Clears all macro data
void ShaderMacros::ClearMacros(void)
{
	if (HasDefinedMacros())
	{
		m_macros.clear();
		m_isdirty = true;
	}
}


// Return a reference to the compiled macro set.  Will recompile if any changes have been made since the last compilation
const D3D_SHADER_MACRO * ShaderMacros::GetCompiledData(void)
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

		// Array of shader tuples must be terminated with a NULL ENTRY at the end, per the API, otherwise DX will 
		// not know where to stop and will throw access violations during shader compilation.  We only need to do
		// this if the collection contains >0 item, since an empty collection .data() will just return null anyway
		if (!m_macros_compiled.empty()) m_macros_compiled.push_back( { NULL, NULL } );

		m_isdirty = false;
	}

	return m_macros_compiled.data();
}