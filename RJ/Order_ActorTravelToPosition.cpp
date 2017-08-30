#include "NavNetwork.h"
#include "NavNode.h"
#include "ComplexShip.h"

#include "Order_ActorTravelToPosition.h"

// Constructor including main order parameters
Order_ActorTravelToPosition::Order_ActorTravelToPosition(	iSpaceObjectEnvironment *environment, CXMVECTOR startpos, CXMVECTOR targetpos,
															float getwithin, float followdistance, bool run)
	:
	Environment(environment),
	StartPosition(startpos),
	TargetPosition(targetpos),
	CloseDistance(getwithin),
	CloseDistanceSq(getwithin * getwithin),
	FollowDistance(followdistance), 
	FollowDistanceSq(followdistance * followdistance),
	Run(run),
	PathNodes(NULL),
	PathLength(0),
	PathIndex(0)
{
	// All order subclasses must set their order type on construction
	m_ordertype = Order::OrderType::ActorTravelToPosition;

	// Determine the path to be followed to reach the target position
	CalculateTravelPath();
}

// Calculates the path that should be followed in order to reach the target position
void Order_ActorTravelToPosition::CalculateTravelPath(void)
{
	// Parameter check
	iSpaceObjectEnvironment *env = Environment();
	if (!env || !env->GetNavNetwork()) return;

	// Find the nav nodes closest to our current (start) location, and the target (end) location
	NavNode *start = env->GetNavNetwork()->GetClosestNode(StartPosition);
	NavNode *end = env->GetNavNetwork()->GetClosestNode(TargetPosition);

	// If no node can be found for either the start or end of the path, quit now and generate no path.  Order will
	// then terminate on its first execution
	if (!start || !end) return;

	// We shouldn't have any existing path data, but check and deallocate in case to avoid potential memory leaks
	if (PathNodes) SafeDeleteArray(PathNodes);

	// Create a vector to hold the output nodes and request a path from the nav network
	std::vector<NavNode*> revpath;
	Result result = env->GetNavNetwork()->FindPath(start, end, revpath);

	// If no path is possible then return now; order will terminate on first execution since it can generate no child nodes
	if (result != ErrorCodes::NoError) return;

	// Allocate space for the path.  There will be one additional position: the end point, which is likely different to navnode[n]
	PathLength = (int)revpath.size() + 1;
	PathNodes = new INTVECTOR3[PathLength];

	// Add each point on the path in turn, using the reverse iterator to retrieve path nodes in turn
	PathIndex = 0;
	std::vector<NavNode*>::const_reverse_iterator it_end = revpath.rend();
	for (std::vector<NavNode*>::const_reverse_iterator it = revpath.rbegin(); it != it_end; ++it)
	{
		PathNodes[PathIndex] = (*it)->Position;
		++PathIndex;
	}

	// Add the final position in the path, which will be the target position itself.  We are also done with the node vector
	// Swap y & z since the nodes are held in element space, and our target position is in world space
	Vector3ToIntVectorSwizzleYZ(TargetPosition, PathNodes[PathLength - 1]);
	revpath.clear();

	// Reset the path index so that the actor will begin at the first node 
	PathIndex = 0;
}


// Default destructor
Order_ActorTravelToPosition::~Order_ActorTravelToPosition(void)
{ 
	// Deallocate any memory currently owned by this object
	if (PathNodes) SafeDeleteArray(PathNodes);
}