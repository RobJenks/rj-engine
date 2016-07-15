#pragma once

#ifndef __TileAdjacencyH__
#define __TileAdjacencyH__

#include "Utility.h"
class ComplexShipTile;


// Structure holding data on a tile adjacency
struct TileAdjacency 
{
	INTVECTOR3 Location;
	Direction AdjDirection; 
	ComplexShipTile *Tile;

	// Default constructor
	TileAdjacency(void) : Location(NULL_INTVECTOR3), AdjDirection(Direction::_Count), Tile(NULL) { }

	// Constructor
	TileAdjacency(const INTVECTOR3 & location, Direction direction, ComplexShipTile *tile) 
		: 
		Location(location), AdjDirection(direction), Tile(tile) { }

};


#endif