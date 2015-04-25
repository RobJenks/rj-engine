#pragma once

#ifndef __OctreeH__
#define __OctreeH__

#include <string>
#include "CompilerSettings.h"
#include "MemoryPool.h"
//#include "iSpaceObject.h"
#include "OverlayRenderer.h"
#include "ScheduledObject.h"
#include "CoreEngine.h"
template <class Octree> class MemoryPool;

// The maximum number of items that can be stored in a node before it will subdivide
#define OCTREE_MAX_ITEMS 4

// The index into child node pointers for each of the eight possible subdivision directions
#define OCTREE_NW_UP		0
#define OCTREE_NE_UP		1
#define OCTREE_SE_UP		2
#define OCTREE_SW_UP		3
#define OCTREE_NW_DOWN		4
#define OCTREE_NE_DOWN		5
#define OCTREE_SE_DOWN		6
#define OCTREE_SW_DOWN		7

template <typename T> 
class Octree
{
public:

	// Constructor for the root node.  Params  specifies the position, and length of each edge of the covered area.  
	// 'areasize' must be a power of 2 (TBC)
	Octree(D3DXVECTOR3 position, float areasize);

	// Default constructor; used for creating nodes that will ultimately be initialised as non-root nodes
	Octree(void);

	// Initialisation method for non-root nodes.  Specifies the parent, plus the area of space which this node covers
	void							Initialise(Octree<T> *parent, float x0, float x1, float y0, float y1, float z0, float z1);

	// Adds an item to this node.  Node will automatically handle subdivision if necessary to remain under item limit
	Octree<T> *						AddItem(T item, const D3DXVECTOR3 & pos);

	// Removes an item from this node.  Node will automatically handle merging back up to the parent if the reduced number of items now makes this possible
	void							RemoveItem(T item);

	// Removes an item from this node.  Node will automatically handle merging back up to the parent if the reduced number of items now makes this possible.
	// Recursively searches down the tree until it finds a match, or exhausts all child nodes.  Returns a flag indicating whether an item was actually removed
	bool							RemoveItemRecursive(T item);

	// Tests whether a moving item is still valid within this node, or whether it needs to be moved to another node in the tree
	// Assumes that the item passed is indeed already part of this node.  'pos' is the new position of the item
	void							ItemMoved(T item, D3DXVECTOR3 pos);

	// Returns the set of items within scope of this node.  If this is not a leaf then it will progress recursively downwards
	void							GetItems(std::vector<T> & outResult);

	// Determines whether this node contains the specified point.  Simple comparison to node bounds
	CMPINLINE bool					ContainsPoint(const D3DXVECTOR3 & point);

	// Returns a numeric value indicating which child of this node is relevant for the specified point.  Based on offset
	// about the 3D centre point
	CMPINLINE int					GetRelevantChildNode(const D3DXVECTOR3 & point);

	// Returns a pointer to the node containing the specified point.  Returns NULL if point is not within the tree bounds
	CMPINLINE Octree<T> *			GetNodeContainingPoint(const D3DXVECTOR3 & point);

	// Shutdown method.  Moves recursively down the tree, deallocating resources as it goes
	void							Shutdown(void);

	// Alternative shutdown method.  Deallocates only a single node without checking children.  Use only
	// when node is known to be a leaf node
	void							ShutdownSingleNode(void);

	// Inline accessor methods for key properties
	CMPINLINE Octree<T> *			GetParent(void) const		{ return m_parent; }
	CMPINLINE float					GetAreaSize(void) const		{ return m_areasize; }
	CMPINLINE int					GetItemCount(void) const	{ return m_itemcount; }

	// Inline methods for setting and checking the pruning check flag
	CMPINLINE void					MarkForPruningCheck(void)				{ m_pruningflag = true; }
	CMPINLINE void					UnmarkForPruningCheck(void)				{ m_pruningflag = false; }
	CMPINLINE bool					IsEligibleForPruningCheck(void) const	{ return m_pruningflag; }

	// Performs a pruning check at this node.  If the check is successful and we do roll up a level, set
	// the pruning flag on our parent node for inclusion in the next check.  In this way any nodes that are
	// no longer needed get gradually rolled back up towards the root
	void							PerformPruningCheck(void);

	// Returns a reference to this node's ultimate parent, i.e. the root node at the top of its tree
	Octree<T> *						GetUltimateParent(void);

	// Utility method to determine how deep in the tree this item is.  Force-terminates after 100 levels to avoid infinite loops/stack failure
	int								DetermineTreeDepth(void);

	// Utility method to determine the size of the tree below this node.  Runs recursively so can result in deep stack calls.
	int								DetermineTreeSize(void);

	// Returns a value indicating whether this is a leaf node (i.e. has no child nodes below it)
	CMPINLINE bool					IsLeafNode(void)					{ return (m_children[0] == NULL); }

	// Debug method to generate a string output of an Octree and its recursively-defined children
	string							DebugOutput(void);

	// Constructs and returns a vector holding the min or max bounds for this node
	D3DXVECTOR3						ConstructMinBoundsVector(void)		{ return D3DXVECTOR3(m_xmin, m_ymin, m_zmin); }
	D3DXVECTOR3						ConstructMaxBoundsVector(void)		{ return D3DXVECTOR3(m_xmax, m_ymax, m_zmax); }

	// Debug method to render node bounds in 3D space
	void							DebugRender(bool include_children);

	// Default destructor
	~Octree(void);

public:			// We will allow public access to member variables for efficiency.  Only ever accessed by core internal methods.

	// The items within this node, plus the current count of objects stored
	T											m_items[OCTREE_MAX_ITEMS];
	int											m_itemcount;

	// Pointers to our parent and child nodes
	Octree<T> *									m_parent;
	Octree<T> *									m_children[8];

	// Store the minimum and maximum bounds of this node in world space.  Min bounds are inclusive, max bounds exclusive.
	// i.e. an item with pos 'x' is valid in this node if, for each dimension,  min <= x < max
	float										m_xmin, m_xmax;
	float										m_ymin, m_ymax;
	float										m_zmin, m_zmax;

	// Precalculated centre point of the node, based on bounds.  Used to determine which subdivision is relevant for a particular position
	float										m_xcentre, m_ycentre, m_zcentre;

	// We will also store the size of the grid handled by this octree.  Set in the root, propogated down to all children.  Must be cubic.
	float										m_areasize;

	// Flag that determines whether this node should be checked in the next pruning cycle.  Set when one of our children has an item removed
	bool										m_pruningflag;

public:

	// Static memory pool of Octree objects, for more efficient allocation/deallocation during execution
	static MemoryPool<Octree<T>> * 				_MemoryPool;

	// Static method to shut down the memory pool and release any allocated resources *still within the pool*.  Any items requested
	// and taken by other processes are the responsibility of the other process to deallocate
	static void									ShutdownMemoryPool(void) { _MemoryPool->Shutdown(); delete _MemoryPool; _MemoryPool = NULL; }
};


// (Begin cpp file)


// Initialise the static memory pool of Octree nodes
template <typename T> 
MemoryPool<Octree<T>> * Octree<T>::_MemoryPool = new MemoryPool<Octree<T>>();

// Constructor for the root node.  Params specify the position, and length of each edge of the covered area.
template <typename T> 
Octree<T>::Octree(D3DXVECTOR3 position, float areasize) :	m_areasize(areasize),
															m_parent(0),
															m_xmin(position.x), m_ymin(position.y), m_zmin(position.z),
															m_xmax(position.x + areasize), 
															m_ymax(position.y + areasize), 
															m_zmax(position.z + areasize)
{
	// Make sure we have been given a valid size parameter; if not, this is an unrecoverable error
	if (areasize <= 0) throw 1;

	// Precalculate the node centre point for future subdivisions
	m_xcentre = (m_xmin + m_xmax) / 2.0f;
	m_ycentre = (m_ymin + m_ymax) / 2.0f;
	m_zcentre = (m_zmin + m_zmax) / 2.0f;

	// Initialise the storage and pointers to null
	memset(m_children, 0, sizeof(Octree<T>*) * 8);
	memset(m_items, 0, sizeof(T) * OCTREE_MAX_ITEMS);
	m_itemcount = 0;
	m_pruningflag = false;
}

// Default constructor; used for creating nodes that will ultimately be initialised as non-root nodes
template <typename T> 
Octree<T>::Octree(void)
{
	
}

// Initialisation method for non-root nodes.  Specifies the parent, plus the area of space which this node covers
template <typename T> 
void Octree<T>::Initialise(Octree<T> *parent, float x0, float x1, float y0, float y1, float z0, float z1)
{
	// Make sure we have a valid parent.  If not, we cannot recover from this error
	if (!parent) throw 0;

	// Store the parent and bounds provided as parameters
	m_parent = parent;
	m_xmin = x0; m_ymin = y0; m_zmin = z0;
	m_xmax = x1; m_ymax = y1; m_zmax = z1;

	// Propogate the total area size from our parent, which in turn ultimately received it from the root node
	m_areasize = m_parent->GetAreaSize();

	// Precalculate the node centre point for future subdivisions
	m_xcentre = (m_xmin + m_xmax) / 2.0f;
	m_ycentre = (m_ymin + m_ymax) / 2.0f;
	m_zcentre = (m_zmin + m_zmax) / 2.0f;

	// Initialise the storage and pointers to null	
	memset(m_children, 0, sizeof(Octree<T>*) * 8);
	memset(m_items, 0, sizeof(T) * OCTREE_MAX_ITEMS);
	m_itemcount = 0;
	m_pruningflag = false;
}

// Adds an item to this node.  Node will automatically handle subdivision if necessary to remain under item limit
// Returns a pointer to the node that now contains the item (it may not be this one)
template <typename T> 
Octree<T> * Octree<T>::AddItem(T item, const D3DXVECTOR3 & pos)
{
	// Parameter check
	if (!item) return NULL;

	// Test whether this is the correct node to contain the item, based on its bounds
	if (!this->ContainsPoint(pos))
	{
		// This is not the correct node.  Pass up to our parent to retry.  If we don't have a parent we are the root, and so this 
		// item is not one that should be included within the octree at all
		if (!m_parent) return NULL;

		// We do have a parent, so pass control up one level.  TODO: Return NULL instead?  Only if we always add from the root node (and check parent==0 on add)
		return m_parent->AddItem(item, pos);
	}

	// If we have no children...
	if (!m_children[0])
	{
		// ...and we are under capacity, we can simply add the item to this node and quit
		if (m_itemcount < OCTREE_MAX_ITEMS)
		{
			// Find the next empty index and insert the item there
			int i = 0;
			while (m_items[i] && i < OCTREE_MAX_ITEMS) ++i;
			m_items[i] = item;

			// Increment the count of items in this node
			++m_itemcount;

			// Link the object back to this node, and return this pointer as the node at which the item was successfully added
			item->SetSpatialTreeNode(this);
			return this;
		}

		// Otherwise, we are over capacity and so need to subdivide into child nodes now
		else
		{
			
			// Special boundary case: if the size of this node is already at 1.0f then we will not subdivide further; return NULL as an error
			if ((m_xmax - m_xmin) < 1.0f) return NULL;

			// We need to partition the tree node to avoid going over the per-node item limit
			// We will request a new node for each child from the memory pool and assign its parameters accordingly
			m_children[OCTREE_NW_DOWN] = Octree::_MemoryPool->RequestItem();
			m_children[OCTREE_NW_DOWN]->Initialise(this, m_xmin, m_xcentre, m_ymin, m_ycentre, m_zcentre, m_zmax);
			m_children[OCTREE_NE_DOWN] = Octree::_MemoryPool->RequestItem();
			m_children[OCTREE_NE_DOWN]->Initialise(this, m_xcentre, m_xmax, m_ymin, m_ycentre, m_zcentre, m_zmax);
			m_children[OCTREE_SE_DOWN] = Octree::_MemoryPool->RequestItem();
			m_children[OCTREE_SE_DOWN]->Initialise(this, m_xcentre, m_xmax, m_ymin, m_ycentre, m_zmin, m_zcentre);
			m_children[OCTREE_SW_DOWN] = Octree::_MemoryPool->RequestItem();
			m_children[OCTREE_SW_DOWN]->Initialise(this, m_xmin, m_xcentre, m_ymin, m_ycentre, m_zmin, m_zcentre);
			m_children[OCTREE_NW_UP] = Octree::_MemoryPool->RequestItem();
			m_children[OCTREE_NW_UP]->Initialise(this, m_xmin, m_xcentre, m_ycentre, m_ymax, m_zcentre, m_zmax);
			m_children[OCTREE_NE_UP] = Octree::_MemoryPool->RequestItem();
			m_children[OCTREE_NE_UP]->Initialise(this, m_xcentre, m_xmax, m_ycentre, m_ymax, m_zcentre, m_zmax);
			m_children[OCTREE_SE_UP] = Octree::_MemoryPool->RequestItem();
			m_children[OCTREE_SE_UP]->Initialise(this, m_xcentre, m_xmax, m_ycentre, m_ymax, m_zmin, m_zcentre);
			m_children[OCTREE_SW_UP] = Octree::_MemoryPool->RequestItem();
			m_children[OCTREE_SW_UP]->Initialise(this, m_xmin, m_xcentre, m_ycentre, m_ymax, m_zmin, m_zcentre);

			// We now need to reallocate our current items to these new child nodes
			D3DXVECTOR3 itempos;
			for (int i = 0; i < OCTREE_MAX_ITEMS; i++)
			{
				// Make sure the item is valid, then get its position
				if (!m_items[i]) continue;
				itempos = m_items[i]->GetPosition();

				// Now determine the relevant child node and add the item to it
				m_children[GetRelevantChildNode(itempos)]->AddItem(m_items[i], itempos);			
			}
			
			// We can now clear the item storage and count for this node, since all items have been reallocated to our children
			memset(m_items, 0, sizeof(T) * OCTREE_MAX_ITEMS);
			m_itemcount = 0;
		}
	}

	/* At this point we know that no items exist in this node, all previous items are in the child nodes, and we need to 
	   assign the new item to a child node.  Determine that node now and add the item */

	// We use the same logic as before to determine the relevant node
	return m_children[GetRelevantChildNode(pos)]->AddItem(item, pos);			
	
	/* Method complete.  We now know that either (a) this node contains items including the new one, or (b) this node contains no 
	   items, and this node's children (or recursively-derived children) contain the new item */
}

// Removes an item from this node.  Node will automatically handle merging back up to the parent if the reduced number of items now makes this possible.
template <typename T> 
void Octree<T>::RemoveItem(T item)
{
	// Loop through the item in this node and remove this item if it is one of them
	for (int i = 0; i < OCTREE_MAX_ITEMS; i++)
	{
		if (m_items[i] == item)
		{
			// This is the item, so remove it from the node and reduce our item count
			m_items[i] = NULL;
			--m_itemcount;

			// Break the reverse link from the object to this node
			item->SetSpatialTreeNode(NULL);

			// Assuming we aren't the root, mark our parent as potentially eligible for the next pruning sweep.  If 
			// the total items now held in all its children are lower than the node maximum then it can then roll up a level
			if (m_parent) m_parent->MarkForPruningCheck();

			// Return here now that the item has been successfully removed
			return;
		}
	}
}

// Removes an item from this node.  Node will automatically handle merging back up to the parent if the reduced number of items now makes this possible.
// Recursively searches down the tree until it finds a match, or exhausts all child nodes.  Returns a flag indicating whether an item was actually removed
template <typename T>
bool Octree<T>::RemoveItemRecursive(T item)
{
	// Take different action depending on whether this is a leaf or a branch node
	if (!m_children[0])
	{
		// This is a leaf.  Loop through the item in this node and remove this item if it is one of them
		for (int i = 0; i < OCTREE_MAX_ITEMS; i++)
		{
			if (m_items[i] == item)
			{
				// This is the item, so remove it from the node and reduce our item count
				m_items[i] = NULL;
				--m_itemcount;

				// Break the reverse link from the object to this node
				item->SetSpatialTreeNode(NULL);

				// Assuming we aren't the root, mark our parent as potentially eligible for the next pruning sweep.  If 
				// the total items now held in all its children are lower than the node maximum then it can then roll up a level
				if (m_parent) m_parent->MarkForPruningCheck();

				// Return here now that the item has been successfully removed
				return true;
			}
		}
	}
	else
	{
		// If this is a branch node, recurse into each child in turn to search for the object being removed
		for (int i = 0; i < 8; i++)
		{
			if (m_children[i]->RemoveItemRecursive(item) == true) return true;
		}
	}

	// We didn't find the object, so return false
	return false;
}


// Tests whether a moving item is still valid within this node, or whether it needs to be moved to another node in the tree
// Assumes that the item passed is indeed already part of this node.  'pos' is the new position of the item.
template <typename T> 
void Octree<T>::ItemMoved(T item, D3DXVECTOR3 pos)
{
	// We can quit immediately if the item still fits within our bounds (the overwhelmingly likely case)
	if (pos.x >= m_xmin && pos.x < m_xmax && pos.y >= m_ymin && pos.y < m_ymax && pos.z >= m_zmin && pos.z < m_zmax) return;

	// The item no longer fits within this node; remove it
	RemoveItem(item);

	// We now attempt to add the item to our parent, which will determine the new node in which to place the item.  However
	// if we don't have a parent (i.e. we are the root) then we must simply quit: the item has moved outside the bounds of
	// the octree and we can no longer manage it
	if (m_parent) m_parent->AddItem(item, pos);	
}

// Returns the set of items within scope of this node.  If this is not a leaf then it will progress recursively downwards
template <typename T> 
void Octree<T>::GetItems(std::vector<T> & outResult)
{
	// If this is a leaf node then we want to add our items and return
	if (!m_children[0])
	{
		// If we have no items then return immediately
		if (m_itemcount == 0) return;

		// Loop through our item collection and any any non-NULL items
		T *p = &m_items[0];
		for (int i=0; i<OCTREE_MAX_ITEMS; i++, p++)
		{
			if (*p) outResult.push_back(*p);
		}

		// Return after we have added all the items
		return;
	}
	else
	{
		// This is not a leaf node, so will have 0 items of its own.  Traverse down the tree to each our our child nodes in turn
		for (int i=0; i<8; i++) m_children[i]->GetItems(outResult);
	}
}

// Shutdown method.  Moves recursively down the tree, deallocating resources as it goes
template <typename T> 
void Octree<T>::Shutdown(void)
{
	// If we have child nodes then recursively deallocate each in turn
	for (int i = 0; i < 8; i++)
	{
		if (m_children[i]) 
		{
			m_children[i]->Shutdown();
			m_children[i] = NULL;
		}
	}

	// Now deallocate this node by returning it to the central memory pool
	Octree<T>::_MemoryPool->ReturnItem(this);
}

// Alternative shutdown method.  Deallocates only a single node without checking children.  Use only
// when node is known to be a leaf node
template <typename T>
void Octree<T>::ShutdownSingleNode(void)
{
	// Deallocate the node by returning it to the central memory pool
	Octree<T>::_MemoryPool->ReturnItem(this);
}

// Default destructor; shouldn't be used at runtime since we are maintaining a memory pool of nodes
template <typename T> 
Octree<T>::~Octree(void)
{
}

// Debug method to generate a string output of an Octree and its recursively-defined children
template <typename T> 
string Octree<T>::DebugOutput(void)
{
	// Add details on this node to the output string
	std::ostringstream s;
	s << "Node " << std::hex << this << std::dec << "(Depth=" << this->DetermineTreeDepth() << ", Items=" << m_itemcount 
	  << ", Size=" << m_xmax-m_xmin << ", Bounds=[" << m_xmin << "," << m_ymin << "," << m_zmin << "]-[" << m_xmax << "," 
	  << m_ymax << "," << m_zmax << "])\n";

	// Now process each child in turn, if we have children
	if (m_children[0])
		for (int i = 0; i < 8; i++)
			s << m_children[i]->DebugOutput();

	// Return the output from this section of the tree
	return s.str();
}

// Debug method to render node bounds in 3D space
template <typename T> 
void Octree<T>::DebugRender(bool include_children)
{
	// Create a world matrix that will translate the rendered box into position
	D3DXMATRIX world;
	D3DXMatrixTranslation(&world, m_xmin, m_ymin, m_zmin);

	// Send a request to the overlay renderer to render this node of the tree
	Game::Engine->GetOverlayRenderer()->RenderBox(world, 
												 (m_itemcount == 0 ? OverlayRenderer::RenderColour::RC_Red : OverlayRenderer::RenderColour::RC_Green),
												 (m_itemcount == 0 ? 1.0f : 3.0f), (m_xmax-m_xmin), (m_ymax-m_ymin), (m_zmax-m_zmin) );

	// Now also render children if required
	if (include_children) 
		for (int i = 0; i < 8; i++)
			if (m_children[i]) m_children[i]->DebugRender(true);

}

// Returns a reference to this node's ultimate parent, i.e. the root node at the top of its tree
template <typename T>
Octree<T> * Octree<T>::GetUltimateParent(void)
{
	int failcounter = 0;
	Octree<T> * ptr = this;

	// Move up the tree until we find the root.  Maintain the fail counter in case of infinite loops
	while (ptr->GetParent() && ++failcounter < 100)
		ptr = ptr->GetParent();

	// Return the pointer to the root node
	return ptr;
}

// Utility method to determine how deep in the tree this item is.  Force-terminates after 100 levels 
// to avoid infinite loops/stack failure
template <typename T> 
int Octree<T>::DetermineTreeDepth(void)
{
	// We will traverse up the tree using the node pointer, recording the depth as we go
	Octree<T> *ptr = m_parent;
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

// Utility method to determine the size of the tree below this node.  Runs recursively so can result in deep stack calls.
template <typename T> 
int Octree<T>::DetermineTreeSize(void)
{
	// If we have no children then return 1, to account for this node only
	if (m_children[0] == NULL) return 1;

	// Otherwise recursively sum the total number of nodes under each child.  Start at 1 to account for this node
	int count = 1;
	for (int i = 0; i < 8; i++)
		if (m_children[i])
			count += m_children[i]->DetermineTreeSize();

	// Return the count of all nodes below & including this one
	return count;
}

// Performs a pruning check at this node.  If the check is successful and we do roll up a level, set
// the pruning flag on our parent node for inclusion in the next check.  In this way any nodes that are
// no longer needed get gradually rolled back up towards the root
template <typename T>
void Octree<T>::PerformPruningCheck(void)
{
	// Make sure we actually have children that could be pruned
	if (!m_children[0]) return;

	// Check whether our children are leaf nodes.  If not there is nothing to prune: we cannot prune nodes from within
	// the middle of the tree, only the leaves.  In that case, move recursively down the tree
	bool haveonlyleaves = true;
	for (int i = 0; i < 8; i++)
	{
		if (!m_children[i]->IsLeafNode())				// If this child node has its own leaves then we cannot prune here
		{
			m_children[i]->PerformPruningCheck();		// Move recursively to this child node instead
			haveonlyleaves = false;						// Set the flag to indicate that we cannot prune this node
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
		vector<T> items;
		for (int i = 0; i < 8; i++)
		{
			// Call the GetItems method to add this child's items to the vector.  We call on each child in turn (rather
			// than just calling on this node and having it recurse) since this allows us to stop between each call if 
			// we exceed the node item limit
			m_children[i]->GetItems(items);
			if (items.size() > OCTREE_MAX_ITEMS) return;
		}

		// If we reached this point then our children contain fewer than the node limit worth of items, so we can prune the 
		// child nodes and roll up to this one.  We have the items already collected in the items vector.  
		
		// Deallocate the child nodes first
		for (int i = 0; i < 8; i++) m_children[i]->ShutdownSingleNode();
		memset(m_children, 0, sizeof(Octree<T>*) * 8);

		// Now add the items to this node.  Change the node link from those items so that it now points to this node
		m_itemcount = items.size();
		for (int i = 0; i < m_itemcount; i++)
		{
			m_items[i] = items[i];
			m_items[i]->SetSpatialTreeNode(this);
		}

		// Finally, set the pruning flag on our parent in case we can roll up another level in the next check
		if (m_parent) m_parent->MarkForPruningCheck();
	}
}

// Determines whether this node contains the specified point.  Simple comparison to node bounds
template <typename T>
CMPINLINE bool Octree<T>::ContainsPoint(const D3DXVECTOR3 & point)
{
	return !(point.x < m_xmin || point.x >= m_xmax || point.y < m_ymin || point.y >= m_ymax || point.z < m_zmin || point.z >= m_zmax);
}

// Returns a numeric value indicating which child of this node is relevant for the specified point.  Based on offset
// about the 3D centre point
template <typename T>
CMPINLINE int Octree<T>::GetRelevantChildNode(const D3DXVECTOR3 & point)
{
	if (point.x < m_xcentre) {						// West of centre
		if (point.y < m_ycentre) {					// Below centre
			if (point.z < m_zcentre) {				// South of centre
				return OCTREE_SW_DOWN;					// --> We want the SW-down node
			} else {								// North of centre
				return OCTREE_NW_DOWN;					// --> We want the NW-down node
			} 
		} else {									// Above centre
			if (point.z < m_zcentre) {				// South of centre
				return OCTREE_SW_UP;					// --> We want the SW-up node
			}	
			else {									// North of centre
				return OCTREE_NW_UP;					// --> We want the NW-up node
			}
		} 
	} else {										// East of centre
		if (point.y < m_ycentre) {					// Below centre
			if (point.z < m_zcentre) {				// South of centre
				return OCTREE_SE_DOWN;					// --> We want the SE-down node
			} else {								// North of centre
				return OCTREE_NE_DOWN;					// --> We want the NE-down node
			} 
		} else {									// Above centre
			if (point.z < m_zcentre) {				// South of centre
				return OCTREE_SE_UP;					// --> We want the SE-up node
			} else {								// North of centre
				return OCTREE_NE_UP;					// --> We want the NE-up node
			} 
		} 
	}
}

// Returns a pointer to the node containing the specified point.  Returns NULL if point is not within the tree bounds
template <typename T>
CMPINLINE Octree<T> * Octree<T>::GetNodeContainingPoint(const D3DXVECTOR3 & point)
{
	// Traverse the hierarchy from the invoking node downwards, recursively looking for the most appropriate node
	if (m_children[0])
	{
		// If we have children then identify the most appropriate child node for this point and move recursively into it
		return m_children[GetRelevantChildNode(point)]->GetNodeContainingPoint(point);
	}
	else
	{
		// If we have no children then this is the most specific & appropriate node for the point, so return it
		return this;
	}
}


#endif




