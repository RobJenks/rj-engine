#include "ErrorCodes.h"
#include "GameVarsExtern.h"
#include "ComplexShipElement.h"
#include "iSpaceObjectEnvironment.h"
#include "NavNode.h"
#include "BinaryHeap.h"

#include "NavNetwork.h"

NavNetwork::NavNetwork(void)
{
	// Initialise fields to their default values upon creation
	m_parent = NULL;
	OPEN_LIST = 10; CLOSED_LIST = 11;
	m_nodes = NULL;
	m_nodecount = 0;
	m_elementsize = NULL_INTVECTOR3;

	// The network is uninitialised upon creation
	m_initialised = false;
}


// Initialises the nav node network for a specific entity containing CS elements
Result NavNetwork::InitialiseNavNetwork(iSpaceObjectEnvironment *parent)
{

	ComplexShipElement *el, *elsrc, *eltgt;
	ComplexShipElement::NavNodePos *navpos;
	ComplexShipElement::NavNodeConnection *navconn;
	NavNode *nsrc, *ntgt;
	int elementnavcount, elementconncount, index;
	int csrc, ctgt, layoutindex;
	int targetindex, targetnodecount; INTVECTOR3 targetpos;
	bool *validconn = 0; int vc_capacity = 0;		// Indicates whether a connection is valid, and the memory that has been allocated so far for this
	INTVECTOR3 node_layout_size; int node_layout_count;
	NavNode ***node_layout;							// Maintain a temporary array of node pointers, by element.  
													// node_layout[x + y*(size.x) + z*(size.x*size.y)] = NavNode*[count]

	// Shutdown the network before we recreate it here.  We will only consider the network 
	// initialised if we reach the end of the method succesfully
	Shutdown();

	// Make sure the parent object is valid, and store a reference if it is
	if (!parent) return ErrorCodes::CannotGenerateNavNetworkForNullParentEntity;
	m_parent = parent;

	// Store required data from the parent object
	m_elementsize = parent->GetElementSize();
	if (m_elementsize.x < 1 || m_elementsize.y < 1 || m_elementsize.z < 1) 
		return ErrorCodes::CannotGenerateNavNetworkWithInvalidSize;

	// Allocate space for a temporary array of node pointers, arranged by element, for efficient lookup during network build
	node_layout_size = parent->GetElementSize();
	node_layout_count = (node_layout_size.x * node_layout_size.y * node_layout_size.z);
	node_layout = new NavNode**[node_layout_count]; // (NavNode***)malloc(sizeof(NavNode**) * node_layout_count);
	memset(node_layout, 0, sizeof(NavNode**) * node_layout_count);

	// Iterate through the full possible extent of elements in this parent entity
	int nodecount = 0;
	for (int x = 0; x < m_elementsize.x; x++)
	{
		for (int y = 0; y < m_elementsize.y; y++)
		{
			for (int z = 0; z < m_elementsize.z; z++)
			{
				// Get this element.  If it doesn't exist, or it does but is not 
				// walkable, then skip it since there will be no nodes here
				el = parent->GetElement(x, y, z);
				if (!el || !el->IsWalkable()) continue;

				if (x == 2 && y == 8 && z == 0)
				{
					int sdfse = 34;
				}

				/* 0. If this element has no manually-specified nav data, we will deallocate any auto-generated data and regenerate */
				if (el->HasCustomNavData() == false)
				{
					el->DeallocateNavPointPositionData();
					el->DeallocateNavPointConnectionData();
				}

				/* 1. Nav node positions.  If we don't already have any specified, generate defaults now */
				elementnavcount = el->GetNavPointPositionCount();
				if (elementnavcount <= 0)
				{
					// Generate a new nav node position and give it default values
					el->AllocateNavPointPositionData(1);
					navpos = el->GetNavPointPositionData();		// Get a pointer to element 0, i.e. &(elements[0])
					navpos->Position = INTVECTOR3((int)(x * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT_INT, 
												  (int)(y * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT_INT, 
												  (int)(z * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT_INT);
					navpos->CostModifier = 1.0f;
					
					// Determine which connections are possible from this nav point
					int numconns = el->DetermineNumberOfWalkableConnections();
					if (numconns > 0)
					{
						// Allocate space for the connections out of this node
						el->AllocateNavPointConnectionData(numconns);
						navpos->NumConnections = numconns;

						// Create each connection in turn
						int conn = 0;
						for (int i = 1; i <= DirectionCount; i++)
						{
							if (el->ConnectsInDirection(DirectionToBS((Direction)i)))
							{
								// We are connecting in this direction so add a new nav node connection
								el->GetNavPointConnectionData()[conn].Source = 0;
								el->GetNavPointConnectionData()[conn].IsDirection = true;
								el->GetNavPointConnectionData()[conn].Target = (Direction)i;
								el->GetNavPointConnectionData()[conn].Cost = (IsDiagonalDirection((Direction)i) ? Game::C_NAVNETWORK_TRAVERSE_COST_DIAG
																												: Game::C_NAVNETWORK_TRAVERSE_COST);
								++conn;
								if (conn == numconns) break;
							}
						}
					}
					else navpos->NumConnections = 0;
				}

				/* 1b. Add to the total number of nav nodes required, and also store in the temporary node layout array */
				elementnavcount = el->GetNavPointPositionCount();
				nodecount += elementnavcount;

				layoutindex = NAV_LAYOUT_INDEX(x, y, z, node_layout_size); 
				node_layout[layoutindex] = new NavNode*[elementnavcount]; // (NavNode**)malloc(sizeof(NavNode*) * elementnavcount);
				memset(node_layout[layoutindex], 0, sizeof(NavNode*) * elementnavcount);
								
			} // z
		} // y
	} // x


	/* 2. Nav nodes.  Now we have nav node positions, allocate space and generate a new node for each one in this element */

	// Record the total number of nodes in this network and allocate space to hold them
	m_nodecount = nodecount;
	m_nodes = new NavNode[m_nodecount];
	if (!m_nodes) return ErrorCodes::CouldNotAllocateSpaceForNavNetworkNodes;
	memset(m_nodes, 0, sizeof(NavNode) * m_nodecount);
	int nodeindex = 0;
	
	// Iterate through each set of data and create the node
	for (int x = 0; x < m_elementsize.x; x++)
	{
		for (int y = 0; y < m_elementsize.y; y++)
		{
			for (int z = 0; z < m_elementsize.z; z++)
			{
				// Get this element.  If it doesn't exist, or it does but is not 
				// walkable, then skip it since there will be no nodes here
				el = parent->GetElement(x, y, z);
				if (!el || !el->IsWalkable()) continue;

				// Derive the layout array index for this node
				layoutindex = NAV_LAYOUT_INDEX(x, y, z, node_layout_size);

				// Clear the current collection of nodes associated to the element.  These will be repopulated below
				el->NavNodes.clear();

				// Iterate over the nodes within this element				
				elementnavcount = el->GetNavPointPositionCount();
				{
					int i; for (i = 0, navpos = el->GetNavPointPositionData(); i < elementnavcount; i++, navpos++)
					{
						// Add this node to the network and set its properties
						m_nodes[nodeindex].Index = nodeindex;
						m_nodes[nodeindex].Element = el;
						m_nodes[nodeindex].Position = navpos->Position;
						m_nodes[nodeindex].NodeCost = navpos->CostModifier;
						//m_nodes[nodeindex].NumConnections = 0;					// This will be set later when connections are created.  
																				// Must be initialised to 0 here. [Already memset to 0]

						// Add a pointer to this node in the temporary element layout array
						node_layout[layoutindex][i] = &(m_nodes[nodeindex]);

						// Push a pointer to the node to the element itself, for more efficient traceback from element>nodes
						el->NavNodes.push_back(&(m_nodes[nodeindex]));

						// Increment the index of the next node to be created
						++nodeindex;
					}
				} // i

			} // z
		} // y
	} // x

	/* 3. Nav node connections.  We do need to iterate through the set of elements again to create these (due 
		  to a dependency on creation of the nodes first)															  */
	
	// Allocate temporary space to hold connection data while it is created
	std::vector<tmpconndata> conn_data;				
	int ccount_capacity = 20;
	int *ccounts = new int[ccount_capacity];

	// Iterate through the full possible extent of elements in this parent entity
	for (int x = 0; x < m_elementsize.x; x++)
	{
		for (int y = 0; y < m_elementsize.y; y++)
		{
			for (int z = 0; z < m_elementsize.z; z++)
			{
				// Get this element.  If it doesn't exist, or isn't walkable, then skip it now
				el = parent->GetElement(x, y, z);
				if (!el || !el->IsWalkable()) continue;

				// Get the number of nodes in this element
				elementnavcount = el->GetNavPointPositionCount();
				if (elementnavcount == 0) continue;

				// Prepare the temporary storage
				if (elementnavcount > ccount_capacity) 
				{ 
					ccount_capacity = elementnavcount * 2;
					delete[] ccounts;
					ccounts = new int[ccount_capacity];
				}
				for (int i = 0; i < elementnavcount; i++) ccounts[i] = 0;
				layoutindex = NAV_LAYOUT_INDEX(x, y, z, node_layout_size);
			
				// Now look at each potential connection in turn
				{
					elementconncount = el->GetNavPointConnectionCount();
					int i; for (i = 0, navconn = el->GetNavPointConnectionData(); i < elementconncount; i++, navconn++)
					{
						// Validate the connection
						if (!navconn->IsDirection)
						{
							// This is an internal connection.  Make sure it involves valid nodes within the element
							csrc = navconn->Source; ctgt = navconn->Target;
							if (csrc < 0 || csrc >= elementnavcount || ctgt < 0 || ctgt >= elementnavcount || csrc == ctgt) continue;
							nsrc = node_layout[layoutindex][csrc]; ntgt = node_layout[layoutindex][ctgt];
							if (!nsrc || !ntgt) continue;

							// This is valid so include the connection for creation now
							++ccounts[csrc]; 
							conn_data.push_back(tmpconndata(nsrc, ntgt, navconn->Cost));
						}
						else
						{
							// This is an external connection.  Connection will only be created if we can locate a suitable neighbouring element/node
							csrc = navconn->Source; ctgt = navconn->Target;
							if (csrc < 0 || csrc >= elementnavcount || ctgt <= 0 || ctgt > DirectionCount) continue;	// Note: we want direction 1-10, not 0-9

							// Attempt to get the relevant neighbouring element
							nsrc = node_layout[layoutindex][csrc];			if (!nsrc) continue;
							elsrc = nsrc->Element;							if (!elsrc) continue;
							index = elsrc->GetNeighbour((Direction)ctgt);	if (index == ComplexShipElement::NO_ELEMENT) continue;
							eltgt = &parent->GetElements()[index];

							// Make sure the neighbour is a valid walkable target element that connects in our direction
							Direction oppositedirection = GetOppositeDirection((Direction)ctgt);
							if (oppositedirection == Direction::_Count) continue;
							if (!eltgt->IsWalkable() || !eltgt->ConnectsInDirection(DirectionToBS(oppositedirection))) continue;

							// Determine the index of this element in the node array based on the target element location
							targetpos = eltgt->GetLocation();
							targetindex = targetpos.x + (targetpos.y * node_layout_size.x) + (targetpos.z * node_layout_size.x * node_layout_size.y);
							
							// If the element has no nodes we just quit here.  If it has one, take a shortcut and select this one immediately.  Otherwise search for the best
							targetnodecount = eltgt->GetNavPointPositionCount();
							if (targetnodecount == 0)	continue;
							if (targetnodecount == 1)	ntgt = node_layout[targetindex][0];
							else						ntgt = GetNodeForConnectionToAdjancentElement(	eltgt, node_layout[targetindex], 
																										targetnodecount, oppositedirection);
							
							// Make sure we were able to retrieve a valid target node;							
							if (!ntgt) continue;
 
							// We have a valid connection so store it now							
							++ccounts[csrc]; 
							conn_data.push_back(tmpconndata(nsrc, ntgt, navconn->Cost));
						}
					}
				}		// For each connection in this element

				/* 3b. Allocate space for connections from each node in this element while we are here, to save iterating again */
				for (int i = 0; i < elementnavcount; i++)
				{
					if (ccounts[i] < 1)
						node_layout[layoutindex][i]->Connections = NULL;
					else
						node_layout[layoutindex][i]->Connections = new NavNodeConnection[ccounts[i]]; // (NavNodeConnection*)malloc(sizeof(NavNodeConnection) * ccounts[i]);
				}

			} // z
		} // y
	} // x

	/* 4. Finally, we can create the validated nav connections and populate them in the pre-allocated space within each node */
	vector<tmpconndata>::const_iterator it_end = conn_data.end();
	for (vector<tmpconndata>::const_iterator it = conn_data.begin(); it != it_end; ++it)
	{
		it->src->Connections[it->src->NumConnections].Target = it->tgt;
		it->src->Connections[it->src->NumConnections].ConnectionCost = it->cost;
		++it->src->NumConnections;
	}

	/* 5. Free the memory that was allocated temporarily during build of the nav network */
	for (int i = 0; i < node_layout_count; i++) if (node_layout[i]) SafeDeleteArray(node_layout[i]);
	SafeDeleteArray(node_layout);
	SafeDeleteArray(ccounts);

	/* 6. Allocate a sufficiently-large binary heap for use as the open list in pathfinding calls */
	m_openlist.Initialise(m_nodecount);

	// We have successfully generated the nav network
	m_initialised = true;
	return ErrorCodes::NoError;
}

// Finds the navigation node closest to the specified position
NavNode * NavNetwork::GetClosestNode(const XMFLOAT3 & pos)
{
	// Parameter & initialisation checks
	if (!m_parent) return NULL;

	// Determine which element this position sits in
	INTVECTOR3 elpos = Game::PhysicalPositionToElementLocation(pos);
	ComplexShipElement *el = m_parent->GetElement(elpos);

	// We should always be within an element.  However if we are not for some reason, clamp between 0 & the element size and attempt to continue
	if (!el)
	{
		elpos.x = max(0, min(elpos.x, m_elementsize.x - 1));
		elpos.y = max(0, min(elpos.y, m_elementsize.y - 1));
		elpos.z = max(0, min(elpos.z, m_elementsize.z - 1));

		el = m_parent->GetElement(elpos);
	}

	// If the element is valid and walkable, we can return a node from within it.  Note that this does mean that a
	// suboptimal route will sometimes be taken since a node in an adjacent element could actually be closer, if the position
	// is on the edge of an element boundary.  However for now we will continue with this method for efficiency
	if (el && el->IsWalkable() && el->NavNodes.size() > 0)
	{
		// Shortcut: If the element only contains one node then return it immediately
		std::vector<NavNode*>::size_type count = el->NavNodes.size();
		if (count == 1) return el->NavNodes[0];

		// Otherwise, we need to perform more work to find the best node 
		// ** Switch y & z coordinates since we are moving from world to element space **
		INTVECTOR3 tgt = INTVECTOR3(pos.x, pos.z, pos.y);		

		// Determine the distance to the first node, as our initial best choice
		NavNode *best = el->NavNodes[0]; NavNode *node;
		INTVECTOR3 diff = INTVECTOR3(best->Position.x - tgt.x, best->Position.y - tgt.y, best->Position.z - tgt.z);
		int dist = (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z);
		int bestdist = dist;

		// Consider each node (after the first node) in turn and return the one closest to pos
		for (std::vector<NavNode*>::size_type i = 1; i < count; ++i)
		{
			// Calculate the squared distance to this node
			node = el->NavNodes[i];
			diff = INTVECTOR3(node->Position.x - tgt.x, node->Position.y - tgt.y, node->Position.z - tgt.z);
			dist = (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z);

			// If the node is closest then record it as the new best node
			if (dist < bestdist) { best = node; bestdist = dist; }
		}

		// Return the best node that we found
		return best;

	} // (If the node is valid, walkable, and contains at least one nav node)
	else
	{
		// The node is not valid, walkable, or doesn't contain any nav nodes.  We will therefore traverse out from
		// the centre and look for an element that does contain nodes.  First that we find will become the best node
		static const int MAX_SEARCH_DIST = 3;		// We will search a maximum of three elements from the current node in each direction
		INTVECTOR3 location;

		// Loop through each search level in turn, getting further from the centre if no nodes are found
		for (int searchlevel = 1; searchlevel <= MAX_SEARCH_DIST; searchlevel++)
		{
			// We will search outwards in both x and y dimensions (not z)
			for (int x = -searchlevel; x <= searchlevel; x++)
			{
				for (int y = -searchlevel; y <= searchlevel; y++)
				{
					// We only need to consider locations where one of x or y is == searchlevel.  If not, the location 
					// has already been considered in a prior search level
					if (x != -searchlevel && x != searchlevel && y != -searchlevel && y != searchlevel) continue;

					// Get the element at this location
					location = INTVECTOR3(elpos.x + x, elpos.y + y, elpos.z);
					el = m_parent->GetElement(location);

					// If the element is valid, walkable, and has any nav nodes, we will take one and return it 
					// immediately.  May not always be optimal but will be efficient.  Can be improved later if required.
					if (el && el->IsWalkable() && el->NavNodes.size() > 0) return el->NavNodes[0];
				}
			}
		}

		// If we reached this point there is no node within the search distance, so return NULL
		return NULL;

	} // (if the node is not valid, so we have to search outwards)
}

// Identifies the best node to connect in a direction from the specified element.  Will preferentially
// look for an element that "wants" to connect in that direction.  If none exists, it will identify
// the node closest to the relevant edge.  Returns NULL if no applicable nodes are available
NavNode * NavNetwork::GetNodeForConnectionToAdjancentElement(ComplexShipElement *element, NavNode **element_nodes, int element_nodecount, Direction direction)
{
	// Parameter check
	if (!element || !element_nodes || element_nodecount <= 0) return NULL;
	int iDir = (int)direction;

	// Loop through each possible connection from the element in turn
	ComplexShipElement::NavNodeConnection *conns = element->GetNavPointConnectionData();
	if (conns)
	{
		int nconn = element->GetNavPointConnectionCount();
		for (int i = 0; i < nconn; ++i)
		{
			if (conns[i].IsDirection && conns[i].Target == iDir && conns[i].Source >= 0 && conns[i].Source < element_nodecount)
			{
				// Return this node as a good connection point, as long as it is valid
				NavNode *node = element_nodes[conns[i].Source];
				if (node) return node;
			}
		}
	}

	// There are no noes which specifically try to connect in this direction.  Return the closest node
	// to the relevant edge as an alternative
	return GetNodeNearestToEdge(element_nodes, element_nodecount, direction);
}

// Selects one node from an array based on its proximity to the edge of its element
NavNode * NavNetwork::GetNodeNearestToEdge(NavNode **nodes, int nodecount, Direction edge)
{
	int best;
	
	// Parameter check
	if (!nodes || nodecount <= 0) return NULL;

	// Looks inelegant, but this saves a lot more comparisons than having the loop as the outer construct
	// For now we will also link diagonal elements to a node satisfying the horizontal condition, for efficiency & to save true distance calculations
	NavNode *node = nodes[0]; 
	switch (edge)
	{
		case Direction::Up:
			best = node->Position.y;
			for (int i = 1; i < nodecount; i++)	if (nodes[i]->Position.y < best) { node = nodes[i]; best = node->Position.y; } break;

		case Direction::Left:
		case Direction::UpLeft:
		case Direction::DownLeft:
			best = node->Position.x;
			for (int i = 1; i < nodecount; i++)	if (nodes[i]->Position.x < best) { node = nodes[i]; best = node->Position.x; } break;

		case Direction::Down:
			best = node->Position.y;
			for (int i = 1; i < nodecount; i++)	if (nodes[i]->Position.y > best) { node = nodes[i]; best = node->Position.y; } break;

		case Direction::Right:
		case Direction::UpRight:
		case Direction::DownRight:
			best = node->Position.x;
			for (int i = 1; i < nodecount; i++)	if (nodes[i]->Position.x > best) { node = nodes[i]; best = node->Position.x; } break;

		case Direction::ZUp:
			best = node->Position.z;
			for (int i = 1; i < nodecount; i++)	if (nodes[i]->Position.z < best) { node = nodes[i]; best = node->Position.z; } break;

		case Direction::ZDown:
			best = node->Position.z;
			for (int i = 1; i < nodecount; i++)	if (nodes[i]->Position.z > best) { node = nodes[i]; best = node->Position.z; } break;

		default:
			return NULL;
	}

	// Return the best node that we could locate
	return node;
}



// Shutdown method to deallocate any resources assigned to this navigation network
void NavNetwork::Shutdown(void)
{
	// Deallocate the memory allocated for connections between nodes
	if (m_nodes)
	{
		for (int i = 0; i < m_nodecount; ++i)
		{
			if (m_nodes[i].Connections) SafeDeleteArray(m_nodes[i].Connections);
		}
	}

	// Release all space allocated for nodes in this network
	SafeDeleteArray(m_nodes);

	// Clear related fields
	m_nodecount = 0;
	m_initialised = false;
}

// Outputs a string representation of the network
std::string NavNetwork::OutputAsString(void)
{
	// Output header data for the network
	NavNode *n;
	std::string s = concat("Nav network: ")(m_nodecount)(" nodes\n\n").str();

	// Now add data for each node in turn
	for (int i = 0; i < m_nodecount; i++)
	{
		// Node position
		n = &(m_nodes[i]);
		s = concat(s)("Node ")(n->Index)(": Pos = ")(IntVectorToString(&(n->Position))).str();
		
		// Element reference
		if (n->Element) s = concat(s)(", El = ")(IntVectorToString(&(n->Element->GetLocation()))).str();
		
		// Node cost and pathfinding field
		s = concat(s)(", Cost = ")(n->NodeCost)(", F = ")(n->F)(", G = ")(n->G)(", H = ")(n->H).str();

		// Path parent
		if (n->PathParent) s = concat(s)(", Path parent = ")(n->PathParent->Index).str();

		// Connection count
		s = concat(s)(", Num connections = ")(n->NumConnections).str();

		// Connection data
		if (n->NumConnections > 0)
		{
			s = concat(s)(", Connections = { ").str();
			for (int c = 0; c < n->NumConnections; c++)
			{
				if (n->Connections[c].Target) 
					s = concat(s)(" [")(n->Connections[c].Target->Index)(" | ")(n->Connections[c].ConnectionCost)("]").str();
			}
			s = concat(s)(" }\n").str();
		}
		else
		{
			s = concat(s)("\n").str();
		}
	}
	
	return s;
}

// Finds a path from one element position ot another, populating the result vector as a set of nav nodes
Result NavNetwork::FindPath(const FXMVECTOR start, const FXMVECTOR end, std::vector<NavNode*> & outPathReverse)
{
	// Attempt to locate the nearest nav node to each position, then find a path between them
	// The primary method will deal with situations where one or other node cannot be identified
	return FindPath(GetClosestNode(start), GetClosestNode(end), outPathReverse);
}

// Finds a path from one node to another, populating the result vector as a (reverse) set of navnodes
Result NavNetwork::FindPath(NavNode *start, NavNode *end, std::vector<NavNode*> & outPathReverse)
{
	NavNode *node, *next;
	int newG;

	// 1. Basic efficiency checks where pathfinding is not required
	if (!start || !end) return ErrorCodes::InvalidPathfindingParameters;
	if (start == end) { outPathReverse.push_back(end); return ErrorCodes::NoError; }

	// 2. If we have reached the max values for open/closed list counters, reset all nodes and wrap around
	if (CLOSED_LIST > 100000)
	{
		// Reset all nodes to a zero value in the list flag
		for (int i = 0; i < m_openlist.Capacity; i++) 
			if (m_openlist.Items[i].Item) m_openlist.Items[i].Item->OnList = 0;

		// Reset the list flag.  We will increment from here in each call to the find path method
		CLOSED_LIST = 10;
	}

	// 3. Increment the open/closed list pointers, to distinguish this run from previous ones
	CLOSED_LIST += 2; 
	OPEN_LIST = CLOSED_LIST - 1;

	// 4. Clear the open list, then add the starting node to the open list and initialise its values
	start->F = start->G = 0;
	start->OnList = OPEN_LIST;
	m_openlist.ClearHeap();
	m_openlist.AddItem(0, start);

	// 5. Now loop through the network nodes to build the path
	while (true)
	{
		// If the open list is not empty, we will take the top node (which has the best F score) and add
		// it to the closed list.  This will form part of the path.
		if (m_openlist.Size != 0)
		{
			// 6. Remove the top node from the open list and add it to the closed list
			node = m_openlist.Items[1].Item;
			node->OnList = CLOSED_LIST;
			m_openlist.RemoveTopItem();

			// 7. Iterate through each node connected to this one and test to see if it should be next in the path
			for (int i = 0; i < node->NumConnections; i++)
			{
				// Get a reference to the node.  Ignore it if the node is on the closed list
				next = node->Connections[i].Target;
				if (next->OnList == CLOSED_LIST) continue;

				// If this node is not already on the open list, calculate its values and add it to the list
				if (next->OnList != OPEN_LIST)
				{
					// Calculate the F, G and H cost for this node, and link to its parent
					next->G = node->G + node->Connections[i].ConnectionCost;	// G = parent node's G-cost plus cost of the new connection
					next->H = abs(next->Position.x - end->Position.x) +			// H = heuristic; linear distance to the target node
							  abs(next->Position.y - end->Position.y) + 
							  abs(next->Position.z - end->Position.z);
					next->F = next->G + next->H;								// F = G + H
					next->PathParent = node;									// Parent = the previous node we last added to the closed list

					// Add the node to the open list
					m_openlist.AddItem(next->F, next);
					next->OnList = OPEN_LIST;
				}
				else	/* Item is already on the open list */
				{
					// 8. If item is already on the open list, check to see if the current path to it is 
					// better than the previous one that was defined.  If so, recalculate value & parent
					newG = node->G + node->Connections[i].ConnectionCost;		// G = parent node's G-cost plus cost of the new connection			
					if (newG < next->G)
					{
						// This path is better, so update the parent value, F and G scores
						next->PathParent = node;								// Parent = the new better path parent value
						next->G = newG;											// G = the new, better G cost reflecting a shorter path from start
						next->F = next->G + next->H;							// F = G + H, so update based on the new G-score

						// This could change the node's position on the open list heap.  Reassess now based on its new F-score
						m_openlist.ReorderElement(next);
					}
				}
			}	// for each connection
		}		// if openlist.Size != 0
		else	// This means the open list is empty
		{
			// If the open list is empty then a valid path does not exist from start>end
			return ErrorCodes::PathDoesNotExist;
		}

		// 10. If the path target has now been added to the open list (i.e. is one step away from being added
		// to the closed list) then we have a complete path and can return it
		if (end->OnList == OPEN_LIST)
		{
			// Push the end element as the first element in the (reverse) output path
			outPathReverse.push_back(end);
			node = end;

			// Traverse the path back to the start, adding each node to the output path vector in turn
			while (node != start)
			{
				// Push this node onto the output vector and move to the next item in the list
				node = node->PathParent;
				outPathReverse.push_back(node);
			}

			// Return success to indicate that we found a path
			return ErrorCodes::NoError;
		}

	} // Repeat until the path is either found, or deemed non-existent

	// We should never get here.  Return error if we do 
	return ErrorCodes::UnknownPathfindingError;
}



// Default destructor; no action, deallocation is taken care of in the shutdown method
NavNetwork::~NavNetwork(void)
{
}



/* ******************************************
   *** Nav network initialisation process ***
   ******************************************

   Create temporary node_layout[] array of nav nodes

   1. for each x/y/z
		if no nav nodes,
			Generate one navposdata
			Generate connectiondata for each walkable m_connections directions
		Add element nodecount-sized array in node_layout for that element index

   2. Allocate space for all nav nodes in the network
		for each x/y/z
		for each navpos data,
			add a nav node in the network collection
			add a pointer to the node to the element itself

   3. Connections.  for each x/y/z
		if not a direction-conn, this is INTERNAL, so
			create temp connection to the tgt node ID
		else, direction-conn, so this is EXTERNAL, so
			create temp connection to the best node in the el in this direction, if applicable

   4. Create connections.  for each temp conn data,
		add conection to the source node that references the tgt node

*/
