#include "FileSystem.h"
#include "Utility.h"
#include "iUIControl.h"

#include "UIManagedControlDefinition.h"

UIManagedControlDefinition::UIManagedControlDefinition(void)
{
	// Initialise members to default values
	m_code = "";
	m_class = iUIControl::Type::Unknown;
}

bool UIManagedControlDefinition::ValidateDefinition(void)
{
	string val;

	// Make sure we have a valid code and control class
	if (m_code == NullString || m_class == iUIControl::Type::Unknown) return false;

	// Now loop through each component in turn
	UIManagedControlDefinition::DefinitionComponentCollection::const_iterator it_end = m_components.end();
	for (UIManagedControlDefinition::DefinitionComponentCollection::const_iterator it = m_components.begin(); it != it_end; ++it)
	{
		// Make sure the specified file exists
		if (it->second == NullString || !FileSystem::FileExists(it->second.c_str())) return false;
	}

	// We have passed all validations so return success
	return true;
}

UIManagedControlDefinition::~UIManagedControlDefinition(void)
{
}
