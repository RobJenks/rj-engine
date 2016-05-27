#include "RenderConfig.h"


// Default constructor
RenderConfig::RenderConfig(void)
{
	//
}

// Add an entry to the config vector.  Returns the ID of the entry that was added, or 0
// if no entry could be added
RenderConfig::RCID RenderConfig::AddConfigEntry(const RenderConfigEntry &entry)
{
	// We can add the entry as long as we are not at the limit
	if (m_entry_count < RenderConfig::CONFIG_LIMIT)
	{
		m_entries.push_back(entry);
		return RenderConfig::ENTRY_ID[++m_entry_count];
	}
	else
	{
		return NO_CONFIG;
	}
}

// Replaces the specified config entry with a new one (only if the specified entry
// already exists).  Returns a flag indicating whether the entry could be replaced
bool RenderConfig::ReplaceConfigEntry(RenderConfig::RCID id, const RenderConfigEntry &entry)
{

}

// Removes all entries from the config vector
void RenderConfig::ClearConfig(void)
{

}


// Retrieves the details for a particular config entry
const RenderConfigEntry & RenderConfig::RetrieveConfigDetails(RenderConfig::RCID id)
{

}