#pragma once

#ifndef __NavNodeH__
#define __NavNodeH__

#include "Utility.h"
class ComplexShipElement;
class NavNode;

// Struct holding details of a connection between two nav nodes
// This class has no special alignment requirements
struct NavNodeConnection
{
	NavNode *				Target;				// The nav node we are connecting to
	int						ConnectionCost;		// The cost of traversing this connection.  Generally equivalent to the
												// straight-line distance from this node to the target.  Int for efficiency.
	
	CMPINLINE NavNodeConnection(NavNode *target, int cost) { Target = target; ConnectionCost = cost; }
	CMPINLINE void operator=(const NavNodeConnection & rhs) { Target = rhs.Target; ConnectionCost = rhs.ConnectionCost; }
};

// Struct holding data on a navigation node, used by actors to route around the interior of ships/stations
// This class has no special alignment requirements
class NavNode
{
public:
	int						Index;				// Index of the node within the network.  Allows tracing back if necessary

	INTVECTOR3	 			Position;			// Position of the node in 3D ship space (relative to parent ship)
	ComplexShipElement *	Element;			// Pointer to the element containing this nav node

	int						F;					// F score: for the pathfinding algorithm; defined as G + H
	int						G;					// G score: cost of the path to this point.  Calculated by taking
												//			the G-score of the previous node and adding our node cost
	int						H;					// H score: estimated remaining distance along the path from here
												//			to the goal.  Using the Manhattan distance as the heuristic

	float					NodeCost;			// Cost modifier of traversing the nav node itself.  e.g. higher for 
												// a ladder than a flat corridor.  Default is 1.0.  Must be > 0.0
	
	NavNode *				PathParent;			// Parent path element, i.e. the node before this one in the path
	int						OnList;				// Indicates which list (open / closed) this node sits on.  For efficiency.

	NavNodeConnection *		Connections;		// Array of connections from this node to other nav nodes
	int						NumConnections;		// The number of connections from this node to others

	// Constructors
	CMPINLINE NavNode(int _index, ComplexShipElement *_el, INTVECTOR3 _position, float _cost)
	{
		Index = _index; Element = _el; Position = _position; NodeCost = _cost;
	}

};











#endif