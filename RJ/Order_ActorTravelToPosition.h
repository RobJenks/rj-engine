#pragma once

#ifndef __Order_ActorTravelToPositionH__
#define __Order_ActorTravelToPositionH__

#include <vector>
#include "Order.h"
#include "NavNetwork.h"
#include "NavNode.h"
#include "ComplexShip.h"

class Order_ActorTravelToPosition : public Order
{
public:
	// Specifies the class of order this object represents
	Order::OrderType GetType(void)				{ return Order::OrderType::ActorTravelToPosition; }


	// Constructor including main order parameters
	Order_ActorTravelToPosition(ComplexShip *environment, D3DXVECTOR3 startpos, D3DXVECTOR3 targetpos, float getwithin, bool run)
	{
		// Initialise parameters to default values
		PathNodes = NULL; 
		PathLength = PathIndex = 0;
			
		// Store the input parameters
		this->Parameters.Float3_1 = targetpos;
		this->Parameters.Float3_2.x = getwithin;
		this->Parameters.Flag_1 = run;
		this->m_startpos = startpos;
		this->m_env = environment;

		// Determine the path to be followed to reach the target position
		CalculateTravelPath();
	}

	// Calculates the path that should be followed in order to reach the target position
	void CalculateTravelPath(void)
	{
		// Parameter check
		if (!m_env || !m_env->GetNavNetwork()) return;
			
		// Find the nav nodes closest to our current (start) location, and the target (end) location
		NavNode *start = m_env->GetNavNetwork()->GetClosestNode(m_startpos);
		NavNode *end = m_env->GetNavNetwork()->GetClosestNode(Parameters.Float3_1);

		// If no node can be found for either the start or end of the path, quit now and generate no path.  Order will
		// then terminate on its first execution
		if (!start || !end) return;

		// Create a vector to hold the output nodes and request a path from the nav network
		std::vector<NavNode*> revpath;
		Result result = m_env->GetNavNetwork()->FindPath(start, end, &revpath);

		// If no path is possible then return now; order will terminate on first execution since it can generate no child nodes
		if (result != ErrorCodes::NoError) return;

		// Allocate space for the path.  There will be one additional position: the end point, which is likely different to navnode[n]
		PathLength = revpath.size() + 1;
		PathNodes = (INTVECTOR3*)malloc(sizeof(INTVECTOR3) * PathLength);

		// Add each point on the path in turn, using the reverse iterator to retrieve path nodes in turn
		PathIndex = 0;
		vector<NavNode*>::const_reverse_iterator it_end = revpath.rend();
		for (vector<NavNode*>::const_reverse_iterator it = revpath.rbegin(); it != it_end; ++it)
		{
			PathNodes[PathIndex] = (*it)->Position;
			++PathIndex;
		}

		// Add the final position in the path, which will be the target position itself.  We are also done with the node vector
		// Swap y & z since the nodes are held in element space, and our target position is in world space
		PathNodes[PathLength-1] = INTVECTOR3(Parameters.Float3_1.x, Parameters.Float3_1.z, Parameters.Float3_1.y);
		revpath.clear();

		// Reset the path index so that the actor will begin at the first node 
		PathIndex = 0;
	}

	// Default constructor / destructor
	//Order_ActorTravelToPosition(void) { }
	~Order_ActorTravelToPosition(void) { }

	
	// Maintain an array of positions that will be followed on the path, and an index specifying 
	// which position is the current move target
	INTVECTOR3 *					PathNodes;
	int								PathLength, PathIndex;



private:

	// Maintain a reference to the parent element-containing environment that the actor is within
	ComplexShip *			m_env;

	// The position that we started travelling from
	D3DXVECTOR3						m_startpos;
};



#endif