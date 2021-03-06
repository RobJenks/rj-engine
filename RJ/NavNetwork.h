#pragma once

#ifndef __NavNetworkH__
#define __NavNetworkH__

#include <vector>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Utility.h"
#include "BinaryHeap.h"
class iSpaceObjectEnvironment;
class ComplexShipElement;
class NavNode;


// This class has no special alignment requirements
class NavNetwork
{
public:
	// Default constructor
	NavNetwork(void);

	// Initialises the nav node network for a specific entity containing CS elements
	Result						InitialiseNavNetwork(iSpaceObjectEnvironment *parent);

	// Returns a value indicating whether the network has been successfully initialised
	CMPINLINE bool				IsInitialised(void)		{ return m_initialised; }

	// Finds a path from one node to another, populating the result vector as a set of nav nodes
	Result						FindPath(NavNode *start, NavNode *end, std::vector<NavNode*> & outPathReverse);

	// Finds a path from one element position ot another, populating the result vector as a set of nav nodes
	Result						FindPath(const FXMVECTOR start, const FXMVECTOR end, std::vector<NavNode*> & outPathReverse);

	// Finds the navigation node closest to the specified position
	CMPINLINE NavNode *			GetClosestNode(const FXMVECTOR pos)
	{
		XMStoreFloat3(&m_float3, pos);
		return GetClosestNode(m_float3);
	}

	// Finds the navigation node closest to the specified position
	NavNode *					GetClosestNode(const XMFLOAT3 & pos);

	// Identifies the best node to connect in a direction from the specified element.  Will preferentially
	// look for an element that "wants" to connect in that direction.  If none exists, it will identify
	// the node closest to the relevant edge.  Returns NULL if no applicable nodes are available
	NavNode *					GetNodeForConnectionToAdjancentElement(	ComplexShipElement *element, NavNode **element_nodes, 
																		int element_nodecount, Direction direction);

	// Macro which returns the index for an element at the specified coordinates
#	define						NAV_LAYOUT_INDEX(x,y,z,size) (x + (y * size.x) + (z * size.x * size.y))

	// Returns the number of nodes in this network
	CMPINLINE int				GetNodeCount(void) const					{ return m_nodecount; }

	// Returns a pointer to the immutable node collection for this network
	CMPINLINE const NavNode *	GetNodes(void) const						{ return m_nodes; }

	// Outputs a string representation of the network
	std::string					OutputAsString(void);

	// Shutdown method to deallocate any resources assigned to this navigation network
	void						Shutdown(void);

	// Default destructor
	~NavNetwork(void);


private:

	// Pointer to the element-containing owner of this network
	iSpaceObjectEnvironment *							m_parent;

	// Bounds of the network in elements
	INTVECTOR3											m_elementsize;

	// Array of nav nodes in this network
	NavNode *											m_nodes;
	int													m_nodecount;

	// Flag indicating whether the network has been initialised based on a parent object that contains elements
	bool												m_initialised;

	// Open list is maintained as a binary heap for efficiency
	BinaryHeap<int, NavNode*>							m_openlist;

	// Flags that indicate which list a node is on.  Resets after a long time and wraps around.  For efficiency
	int													OPEN_LIST;
	int													CLOSED_LIST;

	// Selects one node from an array based on its proximity to the edge of its element
	NavNode *					GetNodeNearestToEdge(NavNode **nodes, int nodecount, Direction edge);


	// Temporary structure used to hold connection data while the network is being built
	struct tmpconndata { 
		NavNode *src; NavNode *tgt; int cost;
		tmpconndata(NavNode *_src, NavNode *_tgt, int _cost) { src = _src; tgt = _tgt; cost = _cost; }
	};

	// Temporary storage for local float representations of vector data
	XMFLOAT3											m_float3;
};



#endif