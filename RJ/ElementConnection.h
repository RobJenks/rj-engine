#pragma once

#ifndef __ElementConnectionH__
#define __ElementConnectionH__

#include <vector>
#include "Utility.h"

// Struct holding information on an element connection point within a complex ship
struct ElementConnection
{
	INTVECTOR3		Location;
	Direction		Connection;

	ElementConnection(void) { Location = INTVECTOR3(); Connection = Direction::_Count; }
	ElementConnection(INTVECTOR3 loc, Direction dir) { Location = loc; Connection = dir; }
};

// Standard collection of element connection objects
typedef std::vector<ElementConnection> ElementConnectionSet;


#endif