#pragma once

#ifndef __RenderConfigH__
#define __RenderConfigH__

#include <vector>
#include "RenderConfigEntry.h"

class RenderConfig
{
public:
	// Config IDs will be represented by bits in a bitstring, so they can be efficiently combined 
	// and only transferred once to the shader.  
	typedef unsigned long long					RCID;

	// Maximum number of render configurations supported (must be <= (log2(RCID::Max) - 1))
	static const int							CONFIG_LIMIT = 32;

	// Define ID of 2^CONFIG_LIMIT (i.e. 0xFFFFF...FF) == no config, also used as "error"/default code
	static const RCID							NO_CONFIG = (RCID)(0 - 1);

	// Default constructor
	RenderConfig(void);

	// Add an entry to the config vector.  Returns the ID of the entry that was added, or 0
	// if no entry could be added
	RCID										AddConfigEntry(const RenderConfigEntry &entry);

	// Replaces the specified config entry with a new one (only if the specified entry
	// already exists).  Returns a flag indicating whether the entry could be replaced
	bool										ReplaceConfigEntry(RCID id, const RenderConfigEntry &entry);

	// Removes all entries from the config vector
	void										ClearConfig(void);

	// Retrieves the details for a particular config entry
	const RenderConfigEntry &					RetrieveConfigDetails(RCID id);


protected:

	// Vector of config data
	std::vector<RenderConfigEntry>				m_entries;

	// Total number of config entries currently registered
	std::vector<RenderConfigEntry>::size_type	m_entry_count;

	// Array that translates 



};





#endif







