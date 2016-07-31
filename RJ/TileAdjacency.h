#pragma once

#ifndef __TileAdjacencyH__
#define __TileAdjacencyH__

#include "Utility.h"
class ComplexShipTile;


// Structure holding data on a tile adjacency
struct TileAdjacency 
{
	INTVECTOR3 Location;			// The element in question
	Direction AdjDirection;			// The direction of the neighbouring tile
	INTVECTOR3 AdjLocation;			// The element in the adjacent location (which is within the neighbouring tile)

	ComplexShipTile *AdjTile;		// NOTE: This is a pointer to the tile at the time the adjacency test was performed.  
									// This tile pointer may not remain valid if tiles are subsequently modified or deallocated

	// Default constructor
	TileAdjacency(void) : Location(NULL_INTVECTOR3), AdjDirection(Direction::_Count), AdjLocation(NULL_INTVECTOR3), AdjTile(NULL) { }

	// Constructor
	TileAdjacency(const INTVECTOR3 & location, Direction direction, const INTVECTOR3 adj_location, ComplexShipTile *adjtile)
		: 
		Location(location), AdjDirection(direction), AdjLocation(adj_location), AdjTile(adjtile) { }

};


#endif