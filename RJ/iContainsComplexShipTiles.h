#pragma once

#ifndef __iContainsComplexShipTilesH__
#define __iContainsComplexShipTilesH__

#include <vector>
#include "CompilerSettings.h"
#include "GameDataExtern.h"
#include "ALIGN16.h"
#include "GameConsoleCommand.h"
class ComplexShipTile;

// 16-bit aligned to allow use of SIMD instruction set
class iContainsComplexShipTiles : public ALIGN16 <iContainsComplexShipTiles>
{

public:

	// Define the standard collection of ship tiles, and an iterator over the collection
	typedef __declspec(align(16)) ComplexShipTile AComplexShipTile;
	typedef __declspec(align(16)) struct AComplexShipTile_P_T {
		__declspec(align(16)) AComplexShipTile *value;
		AComplexShipTile_P_T(ComplexShipTile *tile) : value(tile) { }
	} AComplexShipTile_P;

	typedef __declspec(align(16)) std::vector<AComplexShipTile_P> ComplexShipTileCollection;
	typedef ComplexShipTileCollection::const_iterator ConstTileIterator;

	// Default constructor
	iContainsComplexShipTiles(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void									InitialiseCopiedObject(iContainsComplexShipTiles *source);


	// Standard methods to add/remove/retrieve the tiles that are contained within this parent object
	CMPINLINE ComplexShipTileCollection &	GetTiles(void)				{ return m_tiles[0]; }
	CMPINLINE ComplexShipTile *				GetTile(int index)			{ return (m_tiles[0])[index].value; }
	CMPINLINE int							GetTileCount(void) const	{ return m_tilecount[0]; }
	CMPINLINE bool							HasTiles(void) const		{ return m_tilecount[0] != 0; }
	
	// Methods to search for particular tiles in the collection
	int												FindShipTileIndex(ComplexShipTile *tile) const;
	CMPINLINE ComplexShipTileCollection::iterator	TileNotFound(void) { return m_tiles[0].end(); }
	

	// Collection of ship tile pointers, by tile type, for easy access to specific types of tile.  tiles[Unknown/0] is the full collection
	CMPINLINE ComplexShipTileCollection &	  GetTilesOfType(D::TileClass type)			{ return m_tiles[(int)type]; }
	CMPINLINE int							  GetTileCountOfType(D::TileClass type) const		
	{ 
		return (static_cast<int>(type) > 0 && static_cast<int>(type) < static_cast<int>(D::TileClass::_COUNT) ? m_tilecount[static_cast<int>(type)] : 0);
	}

	// Attempts to locate a tile at the specified location.  Only searches for tiles with an ElementLocation specifically
	// at this location; will not return a tile that is at a different location but extends over the target location
	ComplexShipTile *						FindTileWithSpecificLocation(const INTVECTOR3 & element_location);

	// Attempts to locate a tile that spans the specified location.  Returns the first matching tile found
	ComplexShipTile *						FindTileAtLocation(const INTVECTOR3 & element_location);

	// Return a tile based on its unique ID, or NULL if no such tile exists
	ComplexShipTile *						FindTileByID(Game::ID_TYPE tile_id);

	// Return a tile based on its unique ID, or NULL if no such tile exists.  Searches only within the subset of 
	// tiles with the given type, allowing for a more efficient search
	ComplexShipTile *						FindTileByID(Game::ID_TYPE tile_id, D::TileClass type);
	
	// Apply a fade effect to all ship tiles in this environment
	void									FadeAllTiles(float time, float alpha, bool ignore_pause);
	CMPINLINE void							FadeAllTiles(float time, float alpha)						{ FadeAllTiles(time, alpha, false); }

	// Methods to get/set the flag determining whether we suspend tile-based refreshes.  Used to allow all tiles to
	// be added on ship creation without refreshing at every step
	void									RecalculateShipTileData(void);
	bool									IsTileRecalculationSuspended(void)			{ return m_tilecalcsuspended; }
	void									SuspendTileRecalculation(void)				{ m_tilecalcsuspended = true; }
	void									ReactivateTileRecalculation(void)			
	{
		m_tilecalcsuspended = false;
		RecalculateShipTileData();
	}

	// Shuts down and (optionally) deallocates all tile data in this object
	void									ShutdownAllTileData(bool deallocate);

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void									ProcessDebugCommand(GameConsoleCommand & command);


protected:

	// Fields for managing the ship tiles linked to this parent object
	ComplexShipTileCollection		m_tiles[static_cast<int>(D::TileClass::_COUNT)];			// The collection of tiles linked to this parent; [0] is the full collection, [1]-[n-1] are collections by class
	int								m_tilecount[static_cast<int>(D::TileClass::_COUNT)];		// The number of tiles that are linked to this parent (precalc)
	
	// Determines whether tile-based calculation is enabled
	bool							m_tilecalcsuspended;

	// Protected method to find and return an iterator to the supplied tile, or m_tiles[0].end() if it does not exist in the collection
	CMPINLINE ComplexShipTileCollection::iterator FindShipTile(ComplexShipTile *tile)
	{
		return (std::find_if(m_tiles[0].begin(), m_tiles[0].end(),
			[&tile](const AComplexShipTile_P & element) { return (element.value == tile); }));
	}

	// Methods to update the tile collection data.  Protected; implementing classes should expose this logic
	void									AddShipTile(ComplexShipTile *tile);
	void									RemoveShipTile(ComplexShipTile *tile);
	void									RemoveAllShipTiles(void);



};



#endif