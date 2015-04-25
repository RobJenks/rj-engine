#pragma once

#ifndef __GameVarsH__
#define __GameVarsH__

#include <vector>
using namespace std;

// Standard data type definitions
typedef unsigned long hitpoints;

class Ship;

namespace Game {

	static const float GameSpeed = 1.0f;

	// Ship-specific data
	Ship*							player = NULL;			// Pointer to the player ship
	vector<Ship*>					activeships(0);			// All currently-active ships




}



#endif