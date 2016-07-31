#include <algorithm>
#include "iContainsComplexShipTiles.h"
#include "ComplexShipTile.h"


// Default constructor
iContainsComplexShipTiles::iContainsComplexShipTiles(void)
{
	// Initialise member variables with their default values
	m_tilecalcsuspended = false;

	// Perform an initial recalculation which will initialise all other data to its default starting point
	RecalculateShipTileData();
}


// Method to initialise fields back to defaults on a copied object.  Called by derived classes.
void iContainsComplexShipTiles::InitialiseCopiedObject(iContainsComplexShipTiles *source)
{

}

// Finds a ship tile in the collection of items linked to this object
int iContainsComplexShipTiles::FindShipTileIndex(ComplexShipTile *tile) const
{
	for (int i = 0; i < m_tilecount[0]; ++i)
		if (m_tiles[0][i].value == tile)
			return i;

	return -1;
}

// Attempts to locate a tile at the specified location.  Only searches for tiles with an ElementLocation specifically
// at this location; will not return a tile that is at a different location but extends over the target location
ComplexShipTile * iContainsComplexShipTiles::FindTileWithSpecificLocation(const INTVECTOR3 & element_location)
{
	ComplexShipTileCollection::const_iterator it = std::find_if(m_tiles[0].begin(), m_tiles[0].end(),
		[&element_location](const AComplexShipTile_P & element) { 
			return (element.value && element.value->GetElementLocation() == element_location); });

	if (it != m_tiles[0].end())		return (*it).value;
	else							return NULL;
}

// Attempts to locate a tile that spans the specified location.  Returns the first matching tile found
ComplexShipTile * iContainsComplexShipTiles::FindTileAtLocation(const INTVECTOR3 & element_location)
{
	ComplexShipTileCollection::const_iterator it_end = m_tiles[0].end();
	for (ComplexShipTileCollection::const_iterator it = m_tiles[0].begin(); it != it_end; ++it)
	{
		AComplexShipTile *tile = (*it).value;
		if (tile && (element_location >= tile->GetElementLocation()) &&
					(element_location < (tile->GetElementLocation() + tile->GetElementSize())))
		{
			return tile;
		}
	}

	return NULL;
}

// Links a new ship tile to this object
void iContainsComplexShipTiles::AddShipTile(ComplexShipTile *tile)
{
	// Attempt to locate this tile in the collection; if it already exists then stop now
	if (FindShipTile(tile) != TileNotFound()) return;

	// Add the item to the overall collection now that we know it isn't currently linked
	m_tiles[0].push_back(AComplexShipTile_P(tile));

	// Update based on this new tile, unless we have suspended updates.  This will populate the relevant class-specific tile collection
	if (!m_tilecalcsuspended) 
	{
		// Recalculate the statistics and cached values storing tile state
		RecalculateShipTileData();
	}
}

// Removes the link from a ship tile to this object
void iContainsComplexShipTiles::RemoveShipTile(ComplexShipTile *tile)
{
	// Attempt to locate this tile in the collection, and remove it if it exists
	ComplexShipTileCollection::iterator it = FindShipTile(tile);
	if (it != TileNotFound()) m_tiles[0].erase(it);
	
	// Recalculate tile data for this parent object
	if (!m_tilecalcsuspended) RecalculateShipTileData();
}

// Recalculate derived tile data based on the tiles currently linked to this object
void iContainsComplexShipTiles::RecalculateShipTileData(void)
{
	// Store the total number of tile currently linked to the object
	m_tilecount[0] = (int)m_tiles[0].size();

	// Clear all the class-specific collections before repopulating them
	for (int i = 1; i < (int)D::TileClass::_COUNT; ++i)
		m_tiles[i].clear();

	// Populate each class-specific collection for more efficient lookups
	ComplexShipTile *t; int type; int typecount = (int)D::TileClass::_COUNT;
	ComplexShipTileCollection::const_iterator it_end = m_tiles[0].end();
	for (ComplexShipTileCollection::const_iterator it = m_tiles[0].begin(); it != it_end; ++it)
	{
		t = (*it).value; if (!t) continue;
		type = (int)t->GetTileClass();

		// Add to the relevant collection, assuming this is a valid tile class
		if (type > 0 && type < typecount) m_tiles[type].push_back(t);
	}

	// Finally store the tile count per tile type, now that each subset vector has been populated
	for (int i = 1; i < typecount; ++i)
		m_tilecount[i] = (int)m_tiles[i].size();
}

// Removes all tiles from the collection.  More efficient than removing one-by-one and recalculating each time
void iContainsComplexShipTiles::RemoveAllShipTiles(void)
{
	m_tiles[0].clear();
	RecalculateShipTileData();
}

// Apply a fade effect to all ship tiles in this environment
void iContainsComplexShipTiles::FadeAllTiles(float time, float alpha, bool ignore_pause)
{
	// Iterate through all tiles in the collection and apply this fade value
	ComplexShipTile *t;
	ComplexShipTileCollection::const_iterator it_end = m_tiles[0].end();
	for (ComplexShipTileCollection::const_iterator it = m_tiles[0].begin(); it != it_end; ++it)
	{
		t = (*it).value; if (!t) continue;
		t->Fade.FadeToAlpha(time, alpha, ignore_pause);
	}
}

// Shuts down and (optionally) deallocates all tile data in this object
void iContainsComplexShipTiles::ShutdownAllTileData(bool deallocate)
{
	if (deallocate)
	{
		// If we want to deallocate tiles, call the delete/erase iteration method across the whole primary collection
		delete_erase_value<AComplexShipTile_P>(m_tiles[0], m_tiles[0].begin(), m_tiles[0].end());
	}

	// Now clear every tile pointer collection (incl the primary collection) and reset tile counts
	for (int i = 0; i < (int)D::TileClass::_COUNT; ++i)
	{
		m_tiles[i].clear();
		m_tilecount[i] = 0;
	}
}


