#include "iContainsComplexShipTiles.h"
#include "ComplexShipTile.h"


// Default constructor
iContainsComplexShipTiles::iContainsComplexShipTiles(void)
{
	// Initialise member variables with their default values
	m_tilecount[0] = 0;
	m_hastiles = false;
	m_multitile = false;
	m_primarytile = NULL;
	m_tilecalcsuspended = false;
}


// Method to initialise fields back to defaults on a copied object.  Called by derived classes.
void iContainsComplexShipTiles::InitialiseCopiedObject(iContainsComplexShipTiles *source)
{

}

// Finds a ship tile in the collection of items linked to this object
int iContainsComplexShipTiles::FindShipTile(ComplexShipTile *tile)
{
	for (int i=0; i<m_tilecount[0]; ++i)
		if (m_tiles[0][i] == tile)
			return i;

	return -1;
}

// Finds a ship tile in the collection of items linked to this object.  Supplying the tile class makes the search more efficient
int iContainsComplexShipTiles::FindShipTile(ComplexShipTile *tile, D::TileClass type)
{
	for (int i = 0; i<m_tilecount[(int)type]; ++i)
		if (m_tiles[(int)type][i] == tile)
			return i;

	return -1;
}

// Links a new ship tile to this object
void iContainsComplexShipTiles::AddShipTile(ComplexShipTile *tile, bool PrimaryTileLimit)
{
	// Attempt to locate this tile in the collection; if it already exists then stop now
	int index = FindShipTile(tile, tile->GetTileClass());
	if (index != -1) return;

	// Certain tile containers (e.g. elements) will allow only one primary tile to be present.  Others (e.g. ships) 
	// do not care and do not track which is the current primary tile.  Accomodate both cases.
	if (PrimaryTileLimit)
	{
		// If this is a primary tile, and we already have a primary tile, then we cannot add another 
		bool isprimary = tile->IsPrimaryTile();
		if (isprimary && m_primarytile != NULL) return;

		// If this is a primary tile then record that fact within the collection
		if (isprimary) m_primarytile = tile;
	}

	// Add the item to the overall collection now that we know it isn't currently linked
	m_tiles[0].push_back(tile);

	// Update based on this new tile, unless we have suspended updates.  This will populate the relevant class-specific tile collection
	if (!m_tilecalcsuspended) 
	{
		// Recalculate the statistics and cached values storing tile state
		RecalculateShipTileData();

		// Update ourself based on the addition of this new tile (subclass method, depending on the implementing object: ship/section/element/...)
		ShipTileAdded(tile);
	}
}

// Removes the link from a ship tile to this object
void iContainsComplexShipTiles::RemoveShipTile(ComplexShipTile *tile)
{
	// Attempt to locate this tile in the collection; if it doesn't exist then stop now
	int index = FindShipTile(tile);
	if (index == -1) return;
	
	// If the item is linked to this object then swap/pop it now
	std::swap(m_tiles[0].at(index), m_tiles[0].at(m_tiles[0].size()-1));
	m_tiles[0].pop_back();

	// If this was the primary tile then also record the fact that we no longer have a primary
	if (tile->IsPrimaryTile()) m_primarytile = NULL;

	// Recalculate tile data for this parent object
	if (!m_tilecalcsuspended) RecalculateShipTileData();
}

// Recalculate derived tile data based on the tiles currently linked to this object
void iContainsComplexShipTiles::RecalculateShipTileData(void)
{
	// Store the total number of tile currently linked to the object
	m_tilecount[0] = m_tiles[0].size();

	// Precalculate flags to indicate the tile linkage to this object
	m_hastiles = (m_tilecount[0] > 0);
	m_multitile = (m_tilecount[0] > 1);

	// Clear all the class-specific collections before repopulating them
	for (int i = 1; i < (int)D::TileClass::_COUNT; ++i)
		m_tiles[i].clear();

	// Populate each class-specific collection for more efficient lookups
	ComplexShipTile *t;  int type;
	ConstTileIterator it_end = m_tiles[0].end();
	for (ConstTileIterator it = m_tiles[0].begin(); it != it_end; ++it)
	{
		// Get the type of this tile
		t = (*it); if (!t) continue;
		type = t->GetTileClass();

		// Add to the relevant collection, assuming this is a valid tile class
		if (type > 0 && type < (int)D::TileClass::_COUNT)
			m_tiles[(int)type].push_back(t);
	}

	// Finally store the tile count per tile type, now that each subset vector has been populated
	for (int i = 1; i < (int)D::TileClass::_COUNT; ++i)
		m_tilecount[i] = m_tiles[i].size();
}

// Removes all tiles from the collection.  More efficient than removing one-by-one and recalculating each time
void iContainsComplexShipTiles::RemoveAllShipTiles(void)
{
	m_tiles[0].clear();
	m_primarytile = NULL;
	RecalculateShipTileData();
}

// Shuts down, deletes & deallocates all tile data in this object.  This performs a controlled unlinking
// of the tiles from all their parents to prevent floating pointers from other parent objects.  Unlink_all flag
// determines whether the method will trace back to parent objects and remove their link.  Should be set to true
// for ad-hoc shutdown, but false for full game shutdown since in that case all objects are deallocated together
// from the space object register.  This means the parent ship/section could be deallocated before this method.
void iContainsComplexShipTiles::ShutdownAllTileData(bool deallocate, bool unlink_all)
{
	// Remove tiles from the ship until we have none left.  If any controlled unlinking fails 
	int remainingattempts = m_tiles[0].size() * 2;
	while (m_tiles[0].size() > 0 && (--remainingattempts) > 0)
	{
		// Get a handle to the ship tile and make sure it is a valid pointer
		ComplexShipTile *tile = m_tiles[0].back();
		if (!tile) 
		{
			// If the tile is invalid then remove the entry and continue
			m_tiles[0].pop_back();
			continue;
		}

		// Take different action depending on whether we want to unlink the tile
		if (unlink_all) 
		{
			// Perform a controlled unlinking of the tile from all its parent objects.  This will remove the tile
			// from this ship, reducing the size of the collection without any removal needed in this method
			tile->UnlinkFromParent();
		}
		else
		{
			// If we are not unlinking the tile then we just remove it from the collection here
			m_tiles[0].pop_back();
		}
		
		// Now delete the tile itself, if we have the parameter set to perform deallocation
		if (deallocate) { delete tile; tile = NULL; }
	}

	// Clear every tile pointer collection to be sure of no remaining pointers
	for (int i = 0; i < (int)D::TileClass::_COUNT; ++i)
	{
		m_tiles[i].clear();
		m_tilecount[i] = 0;
	}

	// We no longer have any tiles in the collection
	m_hastiles = false; m_multitile = false; m_primarytile = NULL;
}


