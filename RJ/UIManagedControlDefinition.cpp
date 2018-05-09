#include "FileSystem.h"
#include "Utility.h"
#include "Logging.h"
#include "iUIControl.h"
#include "CoreEngine.h"
#include "RenderAssetsDX11.h"

#include "UIManagedControlDefinition.h"

UIManagedControlDefinition::UIManagedControlDefinition(void)
{
	// Initialise members to default values
	m_code = "";
	m_class = iUIControl::Type::Unknown;
}

bool UIManagedControlDefinition::ValidateDefinition(void)
{
	std::string val;

	// Make sure we have a valid code and control class
	if (m_code == NullString || m_class == iUIControl::Type::Unknown)
	{
		Game::Log << LOG_WARN << "UI control \"" << m_code << "\" (type:" << m_class << ") failed validation\n";
		return false;
	}

	// Now loop through each component in turn
	UIManagedControlDefinition::DefinitionComponentCollection::const_iterator it_end = m_components.end();
	for (UIManagedControlDefinition::DefinitionComponentCollection::const_iterator it = m_components.begin(); it != it_end; ++it)
	{
		// Make sure the specified file exists
		if (it->second == NullString)
		{
			Game::Log << LOG_WARN << "UI control \"" << m_code << "\" failed validation with null component \"" << it->first << "\"\n";
			return false;
		}
		else if (!Game::Engine->GetAssets().AssetExists<TextureDX11>(it->second))
		{
			Game::Log << LOG_WARN << "UI control \"" << m_code << "\" failed validation with missing component \"" << it->first << "\"\n";
			return false;
		}
	}

	// We have passed all validations so return success
	return true;
}

UIManagedControlDefinition::~UIManagedControlDefinition(void)
{
}
