#include "GameVarsExtern.h"
#include "iSpaceObjectEnvironment.h"
#include "iEnvironmentObject.h"
#include "Terrain.h"

#include "EnvironmentTree.h"


// Initialise the static memory pool of node objects
MemoryPool<EnvironmentTree> * EnvironmentTree::_MemoryPool = new MemoryPool<EnvironmentTree>();

// Explicit instantiation of template methods
template bool EnvironmentTree::RemoveItem<iEnvironmentObject*>(iEnvironmentObject*);
template bool EnvironmentTree::RemoveItem<Terrain*>(Terrain*);
template void EnvironmentTree::ItemMoved<iEnvironmentObject*>(iEnvironmentObject *item);
template void EnvironmentTree::ItemMoved<Terrain*>(Terrain *item);

// Constructor for the root node
EnvironmentTree::EnvironmentTree(iSpaceObjectEnvironment *environment) 
{
	// Initialise as the root node; special case
	Initialise(environment);
}

// Constructor for non-root notes; take no action, since the node will be set up via Initialise()
EnvironmentTree::EnvironmentTree(void) 
{ 
}

// Initialise a node as the root, attached to the specified environment
void EnvironmentTree::Initialise(iSpaceObjectEnvironment *environment)
{
	// Store a pointer to the environment
	m_environment = environment;

	// Call the initialisation method with no parent pointer (this is the root) and bounds
	// which cover the entire environment
	Initialise(NULL, NULL_INTVECTOR3, environment->GetElementSize());
}

// Initialise a node to cover the specified portion of the environment
void EnvironmentTree::Initialise(EnvironmentTree *parent, const INTVECTOR3 & start, const INTVECTOR3 & size)
{
	// We should always have a parent, unless this is the root node
	m_parent = parent;
	if (m_parent)
	{
		// Pull any required data from the parent node
		m_environment = m_parent->GetEnvironment();
	}
	
	// Store the element bounds of this node
	m_elmin = start;
	m_elsize = size;
	m_elmax = (start + size) - ONE_INTVECTOR3;

	// Determine the 'centre' element; constrain so that the centre never subdivides us below the minimum node size
	INTVECTOR3 centre_offset = IntVector3Max(Game::C_ENVTREE_MIN_NODE_SIZE_V,
		INTVECTOR3(ENVTREE_CENTRE_ELEMENT(m_elsize.x), ENVTREE_CENTRE_ELEMENT(m_elsize.y), ENVTREE_CENTRE_ELEMENT(m_elsize.z)));
	m_elcentre = m_elmin + centre_offset;

	// Also store the local-position equivalent to these bounds and centre points
	m_min = Game::ElementLocationToPhysicalPosition(m_elmin);
	m_max = Game::ElementLocationToPhysicalPosition(m_elmax + ONE_INTVECTOR3);
	m_size = Game::ElementLocationToPhysicalPosition(m_elsize);
	m_centre = Game::ElementLocationToPhysicalPosition(m_elcentre);

	// Store float3 versions as well for more efficient per-component manipulation
	XMStoreFloat3(&m_fmin, m_min);
	XMStoreFloat3(&m_fmax, m_max);
	XMStoreFloat3(&m_fsize, m_size);
	XMStoreFloat3(&m_fcentre, m_centre);
	
	// Calculate the ACTUAL centre point within this node
	m_actualcentre = XMVectorLerp(m_min, m_max, 0.5f);

	// Determine an approximate bounding sphere radius for the node, for more efficient rendering
	m_bounding_radius = GetElementBoundingSphereRadius(max(max(m_elsize.x, m_elsize.y), m_elsize.z));

	// This node can be subdivided if any dimension is larger than the minimum allowable size
	m_can_be_subdivided = !(m_elsize <= Game::C_ENVTREE_MIN_NODE_SIZE_V);

	// Reset child storage
	m_childcount = 0;
	memset(m_children, 0, sizeof(EnvironmentTree*) * 8);

	// The node begins with no active children
	m_active_children.clear();
	m_active_children.reserve(8U);
	m_pruningflag = false;

	// Initialise item storage
	m_objects.clear(); m_terrain.clear();
	m_totalitemcount = m_objectcount = m_terraincount = 0;
	m_objects.reserve(Game::C_ENVTREE_MAX_NODE_ITEMS_PER_TYPE);
	m_terrain.reserve(Game::C_ENVTREE_MAX_NODE_ITEMS_PER_TYPE);
}

// Add an item to the tree.  Will handle tree traversal / subdivision as necessary
// Returns a pointer to the tree node in which the object was added, or NULL
// if it could not be added due to an error
template <typename T>
EnvironmentTree * EnvironmentTree::AddItem(T item)
{
	// Parameter check
	if (!item) return NULL;

	// Test whether this is the correct node to contain the item, based on its bounds
	XMVECTOR pos = item->GetEnvironmentPosition();
	if (!this->ContainsPoint(pos))
	{
		// This is not the correct node.  Pass up to our parent to retry.  If we don't have a parent we are the root, and so this 
		// item is not one that should be included within the octree at all
		if (!m_parent) return NULL;

		// We do have a parent, so pass control up one level.  TODO: Return NULL instead?  Only if we always add from the root node (and check parent==0 on add)
		return m_parent->AddItem(item);
	}

	// If we have no children...
	if (m_childcount == 0)
	{
		// ...and we are under capacity, OR we are at the minimum node size and so need to accept the object anyway, 
		// add the item to this node and quit
		if (ItemCount<T>() < Game::C_ENVTREE_MAX_NODE_ITEMS || !m_can_be_subdivided)
		{
			// Add to the item collection and increment the count of items in this node
			AddItemToCollection<T>(item);

			// Link the object back to this node, and return this pointer as the node at which the item was successfully added
			item->SetEnvironmentTreeNode(this);
			return this;
		}

		// Otherwise, we are over capacity and want to subdivide into child nodes 
		else
		{
			// Subdivide into all relevant child nodes
			Subdivide();

			// Reallocate all items into the relevant child node
			ReallocateAllItemsToChildren();
		}
	}

	/* At this point we know that no items exist in this node, all previous items are in the child nodes, and we need to
	assign the new item to a child node.  Determine that node now and add the item */

	// We use the same logic as before to determine the relevant node
	int index = GetRelevantChildNode(pos);
	if (m_children[index])
	{
		return m_children[index]->AddItem(item);
	}
	else
	{
#		ifdef _DEBUG
			throw "Environment tree tried to allocate new object to non-existent child node";
#		endif
		return NULL;
	}

	/* Method complete.  We now know that either (a) this node contains items including the new one, or (b) this node contains no
	items, and this node's children (or recursively-derived children) contain the new item */
}

// Removes an item from this node.  Node will automatically handle merging back up to the parent if the reduced number of items now makes this possible
template <class T>
bool EnvironmentTree::RemoveItem(T item)
{
	// Get references to the relevant collections
	std::vector<T> & items = ItemCollectionReference<T>();
	int & count_ref = ItemCountReference<T>();
	int ubound = (count_ref - 1);

	// Loop through each item in the collection and look for the target item
	for (int i = 0; i <= ubound; ++i)
	{
		if (items[i] == item)
		{
			// This is the item, so swap & pop it from the node and reduce our item count
			std::swap(items[i], items[ubound]);
			items.pop_back();
			--(count_ref);
			--m_totalitemcount;

			// Break the reverse link from the object to this node
			item->SetEnvironmentTreeNode(NULL);

			// Assuming we aren't the root, mark our parent as potentially eligible for the next pruning sweep.  If 
			// the total items now held in all its children are lower than the node maximum then it can then roll up a level
			if (m_parent) m_parent->MarkForPruningCheck();

			// Return here now that the item has been successfully removed
			return true;
		}
	}

	// We could not find the item to be removed
	return false;
}

// Removes an item from this node.  Node will automatically handle merging back up to the parent if the reduced number of items now makes this possible.
// Recursively searches down the tree until it finds a match, or exhausts all child nodes.  Returns a flag indicating whether an item was actually removed
template <class T>
bool EnvironmentTree::RemoveItemRecursive(T item)
{
	// If this is a leaf node, return attempt to remove and return the result
	if (m_childcount == 0)
	{
		return RemoveItem<T>(item);
	}
	else
	{
		// If this is a branch node, recurse into each child in turn to search for the object being removed
		for (int i = 0; i < m_childcount; ++i)
		{
			if (m_active_children[i]->RemoveItemRecursive(item) == true) return true;
		}
		
		// We could not remove the item from any of our child nodes, so return false
		return false;
	}
}

// Tests whether a moving item is still valid within this node, or whether it needs to be moved to another node in the tree
// Assumes that the item passed is indeed already part of this node.  'pos' is the new position of the item.
template <typename T>
void EnvironmentTree::ItemMoved(T item)
{
	// We can quit immediately if the item is still within an element that this node covers
	if (ContainsElement(item->GetElementLocation())) return;

	// The item no longer fits within this node; remove it
	RemoveItem<T>(item);

	// We now attempt to add the item to our parent, which will determine the new node in which to place the item.  However
	// if we don't have a parent (i.e. we are the root) then we must simply quit: the item has moved outside the bounds of
	// the tree and we can no longer manage it
	if (m_parent) m_parent->AddItem<T>(item);
}

// Returns the set of items within scope of this node.  If this is not a leaf then it will progress recursively downwards
template <class T>
void EnvironmentTree::GetItems(std::vector<T> & outResult)
{
	// If this is a leaf node, add all items that it contains
	if (m_childcount == 0)
	{
		// If we have no items then return immediately
		if (ItemCount<T>() == 0) return;

		// Otherwise append all elements to the end of the result vector
		std::vector<T> & items = ItemCollectionReference<T>();
		outResult.insert(outResult.end(), items.begin(), items.end());

		// Return after we have added all the items
		return;
	}
	else
	{
		// This is not a leaf node, so will have 0 items of its own.  Traverse down the tree to each our our child nodes in turn
		for (int i = 0; i < m_childcount; ++i)
		{
			m_active_children[i]->GetItems<T>(outResult);
		}
	}
}

// Returns the set of all types of item within scope of this node.  If this is not a leaf then it will progress recursively downwards
void EnvironmentTree::GetAllItems(std::vector<iEnvironmentObject*> & outObjects, std::vector<Terrain*> & outTerrain)
{
	// If this is a leaf node, add all items that it contains
	if (m_childcount == 0)
	{
		// Append all elements to the end of the result vector
		if (m_objectcount != 0)		outObjects.insert(outObjects.end(), m_objects.begin(), m_objects.end());
		if (m_terraincount != 0)	outTerrain.insert(outTerrain.end(), m_terrain.begin(), m_terrain.end());

		// Return after we have added all the items
		return;
	}
	else
	{
		// This is not a leaf node, so will have 0 items of its own.  Traverse down the tree to each our our child nodes in turn
		for (int i = 0; i < m_childcount; ++i)
		{
			m_active_children[i]->GetAllItems(outObjects, outTerrain);
		}
	}
}

// Performs a pruning check at this node.  If the check is successful and we do roll up a level, set
// the pruning flag on our parent node for inclusion in the next check.  In this way any nodes that are
// no longer needed get gradually rolled back up towards the root
void EnvironmentTree::PerformPruningCheck(void)
{
	// Make sure we actually have leaves that could be pruned
	if (m_childcount == 0) return;

	// Check whether our children are leaf nodes.  If not there is nothing to prune: we cannot prune nodes from within
	// the middle of the tree, only the leaves.  In that case, move recursively down the tree
	bool haveonlyleaves = true;
	for (int i = 0; i < m_childcount; ++i)
	{
		if (m_active_children[i]->IsBranchNode())				// If this child node has its own leaves then we cannot prune here
		{
			m_active_children[i]->PerformPruningCheck();		// Move recursively to this child node instead
			haveonlyleaves = false;								// Set the flag to indicate that we cannot prune this node
		}
	}

	// If this node has only leaves below it then we can attempt to prune it
	if (haveonlyleaves)
	{
		// If we are not flagged for the pruning check then do not waste any further time on comparisons and simply quit here
		if (!m_pruningflag) return;

		// Remove the pruning flag since we are now processing this node
		m_pruningflag = false;

		// Iterate through our children and keep a note of any items that are found.  If we exceed the maximum
		// that can be stored within one node then we can quit immediately, since we could not roll all the 
		// child items up to be stored in this one
		std::vector<iEnvironmentObject*> objects; std::vector<Terrain*> terrain;
		for (int i = 0; i < m_childcount; ++i)
		{
			// Call the GetAllItems method to add this child's items to the vector.  We call on each child in turn (rather
			// than just calling on this node and having it recurse) since this allows us to stop between each call if 
			// we exceed the node item limit
			m_active_children[i]->GetAllItems(objects, terrain);
			if ((int)(objects.size() + terrain.size()) > Game::C_ENVTREE_MAX_NODE_ITEMS) return;
		}

		// If we reached this point then our children contain fewer than the node limit worth of items, so we can prune the 
		// child nodes and roll up to this one.  We have the items already collected in the items vector.  

		// Deallocate the child nodes first
		for (int i = 0; i < 8; ++i) m_children[i]->ShutdownSingleNode();
		memset(m_children, 0, sizeof(EnvironmentTree*) * 8);
		m_active_children.clear();
		m_childcount = 0;

		// Now add the items to this node.  Update the count of all items being added to the node
		m_objectcount = (int)objects.size(); 
		m_terraincount = (int)terrain.size();
		m_totalitemcount = m_objectcount + m_terraincount;

		// Directly assign the collated objects to this node
		m_objects = objects;
		m_terrain = terrain;

		// Set the node pointer of each item to point to this node now
		for (int i = 0; i < m_objectcount; ++i)		m_objects[i]->SetEnvironmentTreeNode(this);
		for (int i = 0; i < m_terraincount; ++i)	m_terrain[i]->SetEnvironmentTreeNode(this);

		// Finally, set the pruning flag on our parent in case we can roll up another level in the next check
		if (m_parent) m_parent->MarkForPruningCheck();
	}
}

// Returns a reference to this node's ultimate parent, i.e. the root node at the top of its tree
EnvironmentTree * EnvironmentTree::GetUltimateParent(void)
{
	int failcounter = 0;
	EnvironmentTree * ptr = this;

	// Move up the tree until we find the root.  Maintain the fail counter in case of infinite loops
	while (ptr->GetParent() && ++failcounter < 100)
		ptr = ptr->GetParent();

	// Return the pointer to the root node
	return ptr;
}

// Utility method to determine how deep in the tree this item is.  Force-terminates after 100 levels to avoid infinite loops/stack failure
int	EnvironmentTree::DetermineTreeDepth(void)
{
	// We will traverse up the tree using the node pointer, recording the depth as we go
	EnvironmentTree *ptr = m_parent;
	int depth = 0;

	// Keep looping until we reach a node with parent=NULL, i.e. the root node.  Each time we add one to the depth counter
	while (ptr && depth < 100)
	{
		++depth;
		ptr = ptr->GetParent();
	}

	// Return the depth of this node in the tree
	return depth;
}

// Shutdown method.  Moves recursively down the tree, deallocating resources as it goes
void EnvironmentTree::Shutdown(void)
{
	// If we have child nodes then recursively deallocate each in turn
	if (m_children[0]) m_children[0]->Shutdown();
	if (m_children[1]) m_children[1]->Shutdown();
	if (m_children[2]) m_children[2]->Shutdown();
	if (m_children[3]) m_children[3]->Shutdown();
	if (m_children[4]) m_children[4]->Shutdown();
	if (m_children[5]) m_children[5]->Shutdown();
	if (m_children[6]) m_children[6]->Shutdown();
	if (m_children[7]) m_children[7]->Shutdown();

	// Zero out the child collection; we no longer have any active children
	memset(m_children, 0, sizeof(EnvironmentTree*) * 8);
	m_active_children.clear();
	m_childcount = 0;

	// Now deallocate this node by returning it to the central memory pool
	EnvironmentTree::_MemoryPool->ReturnItem(this);
}

// Alternative shutdown method.  Deallocates only a single node without checking children.  Use only
// when node is known to be a leaf node
void EnvironmentTree::ShutdownSingleNode(void)
{
	// Deallocate the node by returning it to the central memory pool
	EnvironmentTree::_MemoryPool->ReturnItem(this);
}

// Default destructor; shouldn't be used at runtime since we are maintaining a memory pool of nodes
EnvironmentTree::~EnvironmentTree(void)
{
}

// Debug method to generate a string output of an EnvironmentTree and its recursively-defined children
std::string EnvironmentTree::DebugOutput(void)
{
	// Add details on this node to the output string
	int depth = DetermineTreeDepth();
	std::ostringstream s;
	for (int i = 0; i < depth; ++i) s << "\t";
	s << "Node " << std::hex << this << std::dec << "(Depth=" << depth << ", Items=" << m_totalitemcount
		<< ", Size=" << m_elsize.ToString() << ", Bounds=[" << m_elmin.ToString() << "-" << m_elmax.ToString() << ")\n";

	// Now process each child in turn, if we have children
	for (int i = 0; i < m_childcount; ++i)
			s << m_active_children[i]->DebugOutput();

	// Return the output from this section of the tree
	return s.str();
}


// Returns a numeric value indicating which child of this node is relevant for the specified point.  Based on offset
// about the 3D centre point
int EnvironmentTree::GetRelevantChildNode(const FXMVECTOR point)
{
	// Maintain constant array of indices
	const int nodes[2][2][2] = {
		{
			{
				ENVTREE_NW_DOWN, ENVTREE_SW_DOWN			// 000 (-x, -y, -z) and 001 (-x, -y, +z)
			},
			{
				ENVTREE_NW_UP, ENVTREE_SW_UP				// 010 (-x, +y, -z) and 011 (-x, +y, +z)
			}
		},
		{
			{
				ENVTREE_NE_DOWN, ENVTREE_SE_DOWN			// 100 (+x, -y, -z) and 101 (+x, -y, +z)
			},
			{
				ENVTREE_NE_UP, ENVTREE_SE_UP				// 110 (x, +y, -z) and 111 (+x, +y, +z)
			}
		}
	};

	// Test whether point is less than the centre point in each dimension
	XMVECTOR test = XMVectorLess(point, m_centre);

	// Store control values as uint
	uint32_t ui[3];
	XMStoreInt3(&ui[0], test);

	// +1 so that
	//		True (point < centre) is	(0U-1U) + 1		= 0
	//		False (point >= centre) is	(0U) + 1		= 1
	// and use as array index to quickly return the correct value
	return nodes[(uint32_t)ui[0] + 1U][(uint32_t)ui[1] + 1U][(uint32_t)ui[2] + 1];
}

// Returns a numeric value indicating which child of this node is relevant for the specified point.  Based on offset
// about the 3D centre point
int EnvironmentTree::GetRelevantChildNode(const XMFLOAT3 & point)
{
	if (point.x < m_fcentre.x) {						// West of centre
		if (point.y < m_fcentre.y) {					// Below centre
			if (point.z < m_fcentre.z) {				// South of centre
				return ENVTREE_NW_DOWN;					// --> We want the SW-down node
			}
			else {								// North of centre
				return ENVTREE_SW_DOWN;					// --> We want the NW-down node
			}
		}
		else {									// Above centre
			if (point.z < m_fcentre.z) {				// South of centre
				return ENVTREE_NW_UP;					// --> We want the SW-up node
			}
			else {									// North of centre
				return ENVTREE_SW_UP;					// --> We want the NW-up node
			}
		}
	}
	else {										// East of centre
		if (point.y < m_fcentre.y) {					// Below centre
			if (point.z < m_fcentre.z) {				// South of centre
				return ENVTREE_NE_DOWN;					// --> We want the SE-down node
			}
			else {								// North of centre
				return ENVTREE_SE_DOWN;					// --> We want the NE-down node
			}
		}
		else {									// Above centre
			if (point.z < m_fcentre.z) {				// South of centre
				return ENVTREE_NE_UP;					// --> We want the SE-up node
			}
			else {								// North of centre
				return ENVTREE_SE_UP;					// --> We want the NE-up node
			}
		}
	}
}

// Returns a numeric value indicating which child of this node is relevant for the specified element.  Based on offset
// about the 3D centre element division
int EnvironmentTree::GetRelevantChildNode(const INTVECTOR3 & element)
{
	// Test based on position around centre element; NOTE: y/z are in element space, i.e. z is Z-Up/|Z-Down
	if (element.x < m_elcentre.x) {
		if (element.y < m_elcentre.y) {
			return ((element.z < m_elcentre.z) ? ENVTREE_NW_DOWN : ENVTREE_NW_UP);		// -/-
		}
		else {
			return ((element.z < m_elcentre.z) ? ENVTREE_SW_DOWN : ENVTREE_SW_UP);		// -/+
		}
	}
	else {
		if (element.y < m_elcentre.y) {
			return ((element.z < m_elcentre.z) ? ENVTREE_NE_DOWN : ENVTREE_NE_UP);		// +/-
		}
		else {
			return ((element.z < m_elcentre.z) ? ENVTREE_SE_DOWN : ENVTREE_SE_UP);		// +/+
		}
	}
}

// Returns a pointer to the node containing the specified point.  Returns NULL if point is not within the tree bounds
EnvironmentTree * EnvironmentTree::GetNodeContainingPoint(const FXMVECTOR point)
{
	// First make sure this node actually contains the point; if not, traverse up to parents as far as possible
	EnvironmentTree *node = this;
	while (node && !node->ContainsPoint(point))
	{
		node = node->GetParent();
	}

	// If we traverse off the top of the parent hierarchy it means the point does not exist anywhere in the Octree
	// We don't need to test for NULL here since the downward traversal below will handle a NULL input (and return NULL)

	// We know that 'node' contains the point; continue moving down through child nodes until we reach the most
	// appropriate leaf node
	while (node && node->IsBranchNode())
	{
		node = node->GetChildNode(node->GetRelevantChildNode(point));
	}

	// Return the leaf node containing this point
	return node;
}

// Returns a pointer to the node containing the specified point.  Returns NULL if point is not within the tree bounds
EnvironmentTree * EnvironmentTree::GetNodeContainingElement(const INTVECTOR3 & el)
{
	// First make sure this node actually contains the point; if not, traverse up to parents as far as possible
	EnvironmentTree *node = this;
	while (node && !node->ContainsElement(el))
	{
		node = node->GetParent();
	}

	// If we traverse off the top of the parent hierarchy it means the point does not exist anywhere in the Octree
	// We don't need to test for NULL here since the downward traversal below will handle a NULL input (and return NULL)

	// We know that 'node' contains the point; continue moving down through child nodes until we reach the most
	// appropriate leaf node
	while (node && node->IsBranchNode())
	{
		node = node->GetChildNode(node->GetRelevantChildNode(el));
	}

	// Return the leaf node containing this point
	return node;
}

// Reallocate all objects from this node into the relevant child node (which should already have been created via SubDivide())
void EnvironmentTree::ReallocateAllItemsToChildren(void)
{
	// Reallocate from each collection into the relevant child nodes
	PerformReallocationOfItemsToChildren(m_objects);
	PerformReallocationOfItemsToChildren(m_terrain);

	// We should now have no items in this node
	m_objects.clear(); m_objectcount = 0;
	m_terrain.clear(); m_terraincount = 0;
	m_totalitemcount = 0;
}

// Reallocate all items of the specified type into the relevant child node
template <typename T>
void EnvironmentTree::PerformReallocationOfItemsToChildren(std::vector<T> & items)
{
	T item; int index;

	// Process each item in turn
	int n = ItemCount<T>();
	for (int i = 0; i < n; ++i)
	{
		// Make sure the item is valid
		item = items[i];
		if (!item) continue;

		// Determine the relevant child node and add to it
		index = GetRelevantChildNode(item->GetElementLocation());
		if (m_children[index])
		{
			m_children[index]->AddItem<T>(item);
		}
#		ifdef _DEBUG
		else
		{
			int tmp = 123;
			throw "Environment tree tried to reallocate into non-existent child node";
		}
#		endif
	}
}

// Internal method to add an item to the appropriate collection and recalculate derived statistics
template <typename T> 
void EnvironmentTree::AddItemToCollection(T item)
{
	ItemCollectionReference<T>().push_back(item);
	++(ItemCountReference<T>());
	++m_totalitemcount;
}

// Subdivides the node into the set of all relevant child nodes; maximum 8, or less based on element size
/*
NWD = true
NED = >x
SWD = >y
SED = >x, >y

NWU = >z
NEU = >z, >x
SWU = >z, >y
SEU = >z, >x, >y
*/
void EnvironmentTree::Subdivide(void)
{
	// Test whether we have space to subdivide in each dimension, and store for efficiency
	bool divx = (m_elsize.x > Game::C_ENVTREE_MIN_NODE_SIZE);
	bool divy = (m_elsize.y > Game::C_ENVTREE_MIN_NODE_SIZE);
	bool divxy = (divx && divy);

	// We will maintain a record of all active children
	m_active_children.clear();

	// Size of the +ve child nodes (where relevant) will be the element size minus the centre element index
	// 'Negative' node will be	{ start = m_elmin, size = (m_elcentre - m_elmin) }
	// 'Positve' node will be	{ start = m_elcentre, size = (m_elmax - m_elcentre) + (1,1,1) }
	const INTVECTOR3 & neg_start = m_elmin;
	INTVECTOR3 neg_size = (m_elcentre - m_elmin);
	const INTVECTOR3 & pos_start = m_elcentre;
	INTVECTOR3 pos_size = (m_elmax - m_elcentre) + ONE_INTVECTOR3;

	// NWD: At a minimum, we always have the -/-/- node
	m_children[ENVTREE_NW_DOWN] = EnvironmentTree::_MemoryPool->RequestItem();
	m_children[ENVTREE_NW_DOWN]->Initialise(this, neg_start, neg_size);
	m_active_children.push_back(m_children[ENVTREE_NW_DOWN]);
	m_childcount = 1;

	// NED: +/-/-
	if (divx)
	{
		m_children[ENVTREE_NE_DOWN] = EnvironmentTree::_MemoryPool->RequestItem();
		m_children[ENVTREE_NE_DOWN]->Initialise(this, INTVECTOR3(pos_start.x, neg_start.y, neg_start.z), INTVECTOR3(pos_size.x, neg_size.y, neg_size.z));
		m_active_children.push_back(m_children[ENVTREE_NE_DOWN]);
		++m_childcount;
	}

	// SWD: -/+/-
	if (divy)
	{
		m_children[ENVTREE_SW_DOWN] = EnvironmentTree::_MemoryPool->RequestItem();
		m_children[ENVTREE_SW_DOWN]->Initialise(this, INTVECTOR3(neg_start.x, pos_start.y, neg_start.z), INTVECTOR3(neg_size.x, pos_size.y, neg_size.z));
		m_active_children.push_back(m_children[ENVTREE_SW_DOWN]);
		++m_childcount;
	}

	// SED: +/+/-
	if (divxy)
	{
		m_children[ENVTREE_SE_DOWN] = EnvironmentTree::_MemoryPool->RequestItem();
		m_children[ENVTREE_SE_DOWN]->Initialise(this, INTVECTOR3(pos_start.x, pos_start.y, neg_start.z), INTVECTOR3(pos_size.x, pos_size.y, neg_size.z));
		m_active_children.push_back(m_children[ENVTREE_SE_DOWN]);
		++m_childcount;
	}

	// Check all Z-Up nodes in one comparison
	if (m_elsize.z > Game::C_ENVTREE_MIN_NODE_SIZE)
	{
		// NWU: -/-/+
		m_children[ENVTREE_NW_UP] = EnvironmentTree::_MemoryPool->RequestItem();
		m_children[ENVTREE_NW_UP]->Initialise(this, INTVECTOR3(neg_start.x, neg_start.y, pos_start.z), INTVECTOR3(neg_size.x, neg_size.y, pos_size.z));
		m_active_children.push_back(m_children[ENVTREE_NW_UP]);
		++m_childcount;

		// NEU: +/-/+
		if (divx)
		{
			m_children[ENVTREE_NE_UP] = EnvironmentTree::_MemoryPool->RequestItem();
			m_children[ENVTREE_NE_UP]->Initialise(this, INTVECTOR3(pos_start.x, neg_start.y, pos_start.z), INTVECTOR3(pos_size.x, neg_size.y, pos_size.z));
			m_active_children.push_back(m_children[ENVTREE_NE_UP]);
			++m_childcount;
		}

		// SWD: -/+/+
		if (divy)
		{
			m_children[ENVTREE_SW_UP] = EnvironmentTree::_MemoryPool->RequestItem();
			m_children[ENVTREE_SW_UP]->Initialise(this, INTVECTOR3(neg_start.x, pos_start.y, pos_start.z), INTVECTOR3(neg_size.x, pos_size.y, pos_size.z));
			m_active_children.push_back(m_children[ENVTREE_SW_UP]);
			++m_childcount;
		}

		// SED: +/+/+
		if (divxy)
		{
			m_children[ENVTREE_SE_UP] = EnvironmentTree::_MemoryPool->RequestItem();
			m_children[ENVTREE_SE_UP]->Initialise(this, pos_start, pos_size);
			m_active_children.push_back(m_children[ENVTREE_SE_UP]);
			++m_childcount;
		}
	}
}
