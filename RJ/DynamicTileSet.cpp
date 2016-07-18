#include "ComplexShipTile.h"
#include "ComplexShipTileDefinition.h"

#include "DynamicTileSet.h"

// Default constructor
DynamicTileSet::DynamicTileSet(void)
	:
	m_code(NullString), m_default_option(NULL)
{
}

// Add a new option to this tile set
void DynamicTileSet::AddEntry(const DynamicTileRequirements & option)
{
	// Add to the vector of available options
	m_options.push_back(option);

	// Link the tile definition to this DTS, assuming it is non-null.  Note this is using a hack
	// to get around the const modifier on the tile definition which probably shouldn't be allowed
	if (option.TileDefinition)
	{
		ComplexShipTileDefinition *def = D::ComplexShipTiles.Get(option.TileDefinition->GetCode());
		if (def)
		{
			def->AddToDynamicTileSet(m_code);
		}
	}
}

// Set the default entry for this tile set
void DynamicTileSet::SetDefault(const ComplexShipTileDefinition * default_option)
{
	// Store the new default option
	m_default_option = default_option;

	// Link the tile definition to this DTS, assuming it is non-null.  Note this is using a hack
	// to get around the const modifier on the tile definition which probably shouldn't be allowed
	if (default_option)
	{
		ComplexShipTileDefinition *def = D::ComplexShipTiles.Get(default_option->GetCode());
		if (def)
		{
			def->AddToDynamicTileSet(m_code);
		}
	}
}

// Remove an option from this tile set
void DynamicTileSet::RemoveEntry(const ComplexShipTileDefinition * option)
{
	// Find all entries referring to the specified tile definition
	std::vector<DynamicTileRequirements>::iterator it = std::partition(m_options.begin(), m_options.end(),
		[&option](const DynamicTileRequirements & entry) { return (entry.TileDefinition != option); });

	// Process all matching results (if any)
	if (it != m_options.end())
	{
		// Remove the dynamic tile set link from each tile
		std::for_each(it, m_options.end(), [](DynamicTileRequirements & entry) 
		{ 
			if (entry.TileDefinition)
			{
				ComplexShipTileDefinition *def = D::ComplexShipTiles.Get(entry.TileDefinition->GetCode());
				if (def) def->RemoveLinkToDynamicTileSet();
			}
		});

		// Erase the set of options from the collection
		m_options.erase(it, m_options.end());
	}

}

// Clear all entries from this tile set
void DynamicTileSet::Clear(void)
{
	// Clear the option vector
	m_options.clear();
}

// Validate whether the specified tile still meets its selection criteria.  If not, selects another tile
// definition from the set which does meet the criteria (or default, if none).  Returns a pointer to the 
// correct tile definition for this scenario
DynamicTileSet::DynamicTileSetResult DynamicTileSet::GetMostAppropriateTileDefinition(ComplexShipTile *tile) const
{
	// Parameter check
	if (!tile) return DynamicTileSetResult(NULL, Rotation90Degree::Rotate0);

	// First, test whether the tile meets the criteria for its current definition
	const ComplexShipTileDefinition *current_def = tile->GetTileDefinition();
	Rotation90Degree current_rot = tile->GetRotation();
	std::vector<DynamicTileRequirements>::const_iterator current = std::find_if(m_options.begin(), m_options.end(), 
		[&current_def, &current_rot](const DynamicTileRequirements & entry) { return (entry.TileDefinition == current_def && entry.Rotation == current_rot); });
	if (current != m_options.end())
	{
		// We have found the tile option currently in use; if it is still valid, return it immediately
		const DynamicTileRequirements & req = (*current);
		if (TileMeetsCriteria(tile, req)) return DynamicTileSetResult(current_def, tile->GetRotation());
	}

	// The current tile definition is NOT valid.  We therefore need to iterate through all 
	// possible options and return the first one that meets our criteria
	std::vector<DynamicTileRequirements>::const_iterator it_end = m_options.end();
	for (std::vector<DynamicTileRequirements>::const_iterator it = m_options.begin(); it != it_end; ++it)
	{
		const DynamicTileRequirements & req = (*it);
		if (TileMeetsCriteria(tile, req)) return DynamicTileSetResult(req.TileDefinition, req.Rotation);
	}

	// We did not find any appropriate tile definition, so return the default (which may be NULL)
	return DynamicTileSetResult(m_default_option, Rotation90Degree::Rotate0);
}

// Tests whether the specified option is valid for the tile in question
bool DynamicTileSet::TileMeetsCriteria(ComplexShipTile *tile, const DynamicTileRequirements & criteria) const
{
	// Parameter check
	if (!tile) return false;

	// Test connection data 
	if (tile->Connections.Equals(criteria.Connections) == false) return false;

	// We have passed all tests, so return true
	return true;
}

// Default destructor
DynamicTileSet::~DynamicTileSet(void)
{
}