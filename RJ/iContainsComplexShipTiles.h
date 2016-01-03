#pragma once

#ifndef __iContainsComplexShipTilesH__
#define __iContainsComplexShipTilesH__

#include <vector>
#include "CompilerSettings.h"
#include "GameDataExtern.h"
class ComplexShipTile;
using namespace std;

// Class has no special alignment requirements
class iContainsComplexShipTiles
{

public:

	// Define the standard collection of ship tiles, and an iterator over the collection
	typedef __declspec(align(16)) vector<__declspec(align(16)) ComplexShipTile*> ComplexShipTileCollection;
	typedef ComplexShipTileCollection::const_iterator ConstTileIterator;

	// Default constructor
	iContainsComplexShipTiles(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void										InitialiseCopiedObject(iContainsComplexShipTiles *source);


	// Standard methods to add/remove/retrieve the tiles that are contained within this parent object
	CMPINLINE ComplexShipTileCollection *	GetTileCollection(void)		{ return &(m_tiles[0]); }
	CMPINLINE ComplexShipTile **			GetTiles(void)				{ return &((m_tiles[0])[0]); }
	CMPINLINE ComplexShipTile *				GetTile(void)				{ return (m_tiles[0])[0]; }
	CMPINLINE ComplexShipTile *				GetTile(int index)			{ return (m_tiles[0])[index]; }
	CMPINLINE int							GetTileCount(void)			{ return m_tilecount[0]; }
	CMPINLINE bool							HasTiles(void)				{ return m_hastiles; }
	CMPINLINE bool							HasMultipleTiles(void)		{ return m_multitile; }
	
	// More complex methods for interacting with ship tiles 
	int										FindShipTile(ComplexShipTile *tile);
	int										FindShipTile(ComplexShipTile *tile, D::TileClass type);
	void									AddShipTile(ComplexShipTile *tile, bool PrimaryTileLimit);
	void									RemoveShipTile(ComplexShipTile *tile);
	void									RemoveAllShipTiles(void);
	void									RecalculateShipTileData(void);

	// Collection of ship tile pointers, by tile type, for easy access to specific types of tile.  tiles[Unknown/0] is the full collection
	CMPINLINE std::vector<ComplexShipTile*> & GetTilesOfType(D::TileClass type)			{ return m_tiles[(int)type]; }
	CMPINLINE int							  GetTileCountOfType(D::TileClass type)		{ return m_tilecount[(int)type]; }

	// Subclass methods that are triggered when a ship tile is added or removed
	virtual void							ShipTileAdded(ComplexShipTile *tile) = 0;
	virtual void							ShipTileRemoved(ComplexShipTile *tile) = 0;

	// Methods to get/set the flag determining whether we suspend tile-based refreshes.  Used to allow all tiles to
	// be added on ship creation without refreshing at every step
	bool									IsTileRecalculationSuspended(void)			{ return m_tilecalcsuspended; }
	void									SuspendTileRecalculation(void)				{ m_tilecalcsuspended = true; }
	void									ReactivateTileRecalculation(void)			
	{
		m_tilecalcsuspended = false;
		RecalculateShipTileData();
	}

	// Iterator functions for the tile collection
	CMPINLINE ConstTileIterator 		GetTileIteratorStart(void) const	{ return m_tiles[0].begin(); }
	CMPINLINE ConstTileIterator 		GetTileIteratorEnd(void) const		{ return m_tiles[0].end(); }
	
	// Returns the number of tiles of a particular type
	CMPINLINE ComplexShipTileCollection::size_type		GetTileCountOfType(D::TileClass tileclass) const
	{
		if ((int)tileclass > 0 && (int)tileclass < D::TileClass::_COUNT)
			return m_tiles[(int)tileclass].size();
		else return 0;
	}

	// Returns a reference to the first tile we are linked to of the specified type
	CMPINLINE ComplexShipTile *iContainsComplexShipTiles::GetFirstTileOfType(D::TileClass tileclass)
	{
		// Either return the first tile of this type, or NULL if none exist
		if (GetTileCountOfType(tileclass) != 0)		return m_tiles[(int)tileclass].at(0);
		else										return NULL;
	}

	// Indicates whether we contain any tiles of the specified type
	CMPINLINE bool iContainsComplexShipTiles::HaveTileOfType(D::TileClass tileclass)
	{
		return (GetTileCountOfType(tileclass) != 0);
	}
	
	// Methods to test and retrieve the primary tile in this collection, if it exists
	CMPINLINE ComplexShipTile *				GetPrimaryTile(void)			{ return m_primarytile; }
	CMPINLINE bool							HasPrimaryTile(void)			{ return (m_primarytile != NULL); }

	// Shuts down, deletes & deallocates all tile data in this object.  This performs a controlled unlinking
	// of the tiles from all their parents to prevent floating pointers from other parent objects.  Unlink_all flag
	// determines whether the method will trace back to parent objects and remove their link.  Should be set to true
	// for ad-hoc shutdown, but false for full game shutdown since in that case all objects are deallocated together
	// from the space object register.  This means the parent ship/section could be deallocated before this method.
	void									ShutdownAllTileData(bool deallocate, bool unlink_all);



protected:

	// Fields for managing the ship tiles linked to this parent object
	ComplexShipTileCollection		m_tiles[D::TileClass::_COUNT];			// The collection of tiles linked to this parent; [0] is the full collection, [1]-[n-1] are collections by class
	int								m_tilecount[D::TileClass::_COUNT];		// The number of tiles that are linked to this parent (precalc)
	bool							m_hastiles;								// Flag to indicate whether this parent contains any tiles (precalc)
	bool							m_multitile;							// Flag to indicate whether this parent contains >1 tiles (precalc)

	// Maintain a link to the 'primary' tile in this collection.  Most collections can only contain one primary tile
	ComplexShipTile *				m_primarytile;

	// Determines whether tile-based calculation is enabled
	bool							m_tilecalcsuspended;

private:

};



#endif