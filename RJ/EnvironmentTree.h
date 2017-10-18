#pragma once

#ifndef __EnvironmentTree__
#define __EnvironmentTree__

#include "ALIGN16.h"
#include "CompilerSettings.h"
#include "AlignedAllocator.h"
#include "MemoryPool.h"
#include "Utility.h"
template <class EnvironmentTree> class MemoryPool;
class iSpaceObjectEnvironment;
class iEnvironmentObject;
class Terrain;
using namespace DirectX;

// The index into child node pointers for each of the eight possible subdivision directions
#define ENVTREE_NW_UP		0
#define ENVTREE_NE_UP		1
#define ENVTREE_SE_UP		2
#define ENVTREE_SW_UP		3
#define ENVTREE_NW_DOWN		4
#define ENVTREE_NE_DOWN		5
#define ENVTREE_SE_DOWN		6
#define ENVTREE_SW_DOWN		7

// Formula to determine the 'centre' element in a set.  The 'centre' element is the first in the 'rightmost' 
// subset of elements.  E.g. with 4 elements, 0-3, the centre element is 2 (so the two subsets are 01 and 23)
// Formula is not defined where the set size is 0 or 1
#define ENVTREE_CENTRE_ELEMENT(n) ((n & 1) ? (n / 2 + 1) : (n / 2))

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class EnvironmentTree : public ALIGN16<EnvironmentTree>
{
public:

	// Constructor for the root tree node
	EnvironmentTree(iSpaceObjectEnvironment *parent);

	// Default constructor for all non-root nodes
	EnvironmentTree(void);

	// Initialise a node as the root, attached to the specified environment
	void										Initialise(iSpaceObjectEnvironment *environment);

	// Initialise a node to cover the specified portion of the environment
	void										Initialise(EnvironmentTree *parent, const INTVECTOR3 & start, const INTVECTOR3 & size);

	// Add an item to the tree.  Will handle tree traversal / subdivision as necessary
	// Returns a pointer to the tree node in which the object was added, or NULL
	// if it could not be added due to an error
	template <class T>
	EnvironmentTree *							AddItem(T item);

	// Removes an item from this node.  Node will automatically handle merging back up to the parent if the reduced 
	// number of items now makes this possible.  Returns a flag indicating whether an item was successfully removedd
	template <class T>
	bool										RemoveItem(T item);

	// Removes an item from this node.  Node will automatically handle merging back up to the parent if the reduced number of items now makes this possible.
	// Recursively searches down the tree until it finds a match, or exhausts all child nodes.  Returns a flag indicating whether an item was actually removed
	template <class T>
	bool										RemoveItemRecursive(T item);

	// Tests whether a moving item is still valid within this node, or whether it needs to be moved to another node in the tree
	// Assumes that the item passed is indeed already part of this node
	template <class T>
	void										ItemMoved(T item);

	// Returns the set of items within scope of this node.  If this is not a leaf then it will progress recursively downwards
	template <class T>
	void										GetItems(std::vector<T> & outResult);

	// Returns the set of all types of item within scope of this node.  If this is not a leaf then it will progress recursively downwards
	void										GetAllItems(std::vector<iEnvironmentObject*> & outObjects, 
															std::vector<Terrain*> & outTerrain);

	// Returns a pointer to the environment that this tree covers
	iSpaceObjectEnvironment *					GetEnvironment(void) const		{ return m_environment; }

	// Return data on the node position
	CMPINLINE XMVECTOR							GetCentrePoint(void) const		{ return m_centre; }
	CMPINLINE XMFLOAT3							GetCentrePointF(void) const		{ return m_fcentre; }
	CMPINLINE INTVECTOR3						GetElementCentre(void) const	{ return m_elcentre; }
	CMPINLINE INTVECTOR3						GetElementMin(void) const		{ return m_elmin; }
	CMPINLINE INTVECTOR3						GetElementMax(void) const		{ return m_elmax; }
	CMPINLINE XMVECTOR							GetMin(void) const				{ return m_min; }
	CMPINLINE XMVECTOR							GetMax(void) const				{ return m_max; }

	// Returns the ACTUAL centre point of this node (distinct from the node "centre" which is the 
	// element-aligned position around which the node may be subdivided)
	CMPINLINE XMVECTOR							GetActualCentrePoint(void) const { return m_actualcentre; }

	// Determines whether the node contains the specified point
	CMPINLINE bool								ContainsPoint(const FXMVECTOR point) const
	{
		return (XMVector3GreaterOrEqual(point, m_min) && XMVector3Less(point, m_max));
	}

	// Determines whether the node contains the specified area; node is treated as half-open range [min max) as per default behaviour
	CMPINLINE bool								ContainsArea(const FXMVECTOR area_min, const FXMVECTOR area_max)
	{
		return (XMVector3GreaterOrEqual(area_min, m_min) && XMVector3Less(area_max, m_max));
	}

	// Determines whether the node contains the specified area, with node treated as fully-open range [min max] as special casee
	CMPINLINE bool								ContainsAreaFullyInclusive(const FXMVECTOR area_min, const FXMVECTOR area_max)
	{
		return (XMVector3GreaterOrEqual(area_min, m_min) && XMVector3LessOrEqual(area_max, m_max));
	}

	// Determines whether the node intersects the specified area at all; node is treated as half-open range [min max) as per default behaviour
	CMPINLINE bool								IntersectsArea(const FXMVECTOR area_min, const FXMVECTOR area_max)
	{
		// Min < node-max && Max >= node-min
		return (XMVector3Less(area_min, m_max) && XMVector3GreaterOrEqual(area_max, m_min));
	}

	// Determines whether the node intersects the specified area at all; with node treated as fully -open range [min max) as special case
	CMPINLINE bool								IntersectsAreaFullyInclusive(const FXMVECTOR area_min, const FXMVECTOR area_max)
	{
		// Min < node-max && Max >= node-min
		return (XMVector3LessOrEqual(area_min, m_max) && XMVector3GreaterOrEqual(area_max, m_min));
	}

	// Determines whether the node contains the specified element
	CMPINLINE bool								ContainsElement(const INTVECTOR3 & el) const
	{
		return ((el >= m_elmin) && (el <= m_elmax));
	}

	// Determines whether the node covers any part of the specified element range
	CMPINLINE bool								CoversElementRange(const INTVECTOR3 & emin, const INTVECTOR3 & emax) const 
	{
		return (m_elmin <= emax && m_elmax >= emin);
	}

	// Returns the number of children below this node
	CMPINLINE int								GetChildCount(void) const { return m_childcount; }

	// Return the specified child (use ENVTREE_*_* as indices)
	CMPINLINE EnvironmentTree *					GetChildNode(int child_index)		{ return m_children[child_index]; }

	// Returns the specified active child node, of which there will always be zero to eight (and exactly ChildCount)
	CMPINLINE EnvironmentTree *					GetActiveChildNode(int index) const { return m_active_children[index]; }

	// Returns a numeric value indicating which child of this node is relevant for the specified point.  Based on offset
	// about the 3D centre point
	int											GetRelevantChildNode(const FXMVECTOR point);

	// Returns a numeric value indicating which child of this node is relevant for the specified point.  Based on offset
	// about the 3D centre point
	int											GetRelevantChildNode(const XMFLOAT3 & point);

	// Returns a numeric value indicating which child of this node is relevant for the specified element.  Based on offset
	// about the 3D centre element division
	int											GetRelevantChildNode(const INTVECTOR3 & element);

	// Returns a pointer to the node containing the specified point.  Returns NULL if point is not within the tree bounds
	EnvironmentTree *							GetNodeContainingPoint(const FXMVECTOR point);

	// Returns a pointer to the node containing the specified element.  Returns NULL if point is not within the tree bounds
	EnvironmentTree *							GetNodeContainingElement(const INTVECTOR3 & el);

	// Returns the approximate radius of a bounding sphere that encompasses this node
	CMPINLINE float								GetBoundingSphereRadius(void) const { return m_bounding_radius; }

	// Returns a reference to the children below this node
	CMPINLINE EnvironmentTree **					GetChildNodes(void)					{ return m_children; }
	CMPINLINE const std::vector<EnvironmentTree*> &	GetActiveChildNodes(void) const		{ return m_active_children; }

	// Subdivides the node into the set of all relevant child nodes; maximum 8, or less based on element size
	void										Subdivide(void);

	// Reallocate all items into the relevant child node
	template <typename T>
	void										ReallocateAllItemsToChildren(std::vector<T> & items);

	// Shutdown method.  Moves recursively down the tree, deallocating resources as it goes
	void										Shutdown(void);

	// Alternative shutdown method.  Deallocates only a single node without checking children.  Use only
	// when node is known to be a leaf node
	void										ShutdownSingleNode(void);

	// Inline accessor methods for key properties
	CMPINLINE EnvironmentTree *					GetParent(void) const					{ return m_parent; }
	CMPINLINE AXMVECTOR							GetSize(void) const						{ return m_size; }
	CMPINLINE XMFLOAT3							GetSizeF(void) const					{ return m_fsize; }
	CMPINLINE INTVECTOR3						GetElementSize(void) const				{ return m_elsize; }
	CMPINLINE int								GetObjectCount(void) const				{ return m_objectcount; }
	CMPINLINE int								GetTerrainCount(void) const				{ return m_terraincount; }
	CMPINLINE int								GetTotalItemCount(void) const			{ return m_totalitemcount; }

	// Inline methods for setting and checking the pruning check flag
	CMPINLINE void								MarkForPruningCheck(void)				{ m_pruningflag = true; }
	CMPINLINE void								UnmarkForPruningCheck(void)				{ m_pruningflag = false; }
	CMPINLINE bool								IsEligibleForPruningCheck(void) const	{ return m_pruningflag; }

	// Flag which determines whether this node can be further subdivided, or has reached the minimum allowable level
	CMPINLINE bool								CanBeSubdivided(void) const				{ return m_can_be_subdivided; }

	// Reallocate all objects from this node into the relevant child node (which should already have been created via SubDivide())
	void										ReallocateAllItemsToChildren(void);

	// Performs a pruning check at this node.  If the check is successful and we do roll up a level, set
	// the pruning flag on our parent node for inclusion in the next check.  In this way any nodes that are
	// no longer needed get gradually rolled back up towards the root
	void										PerformPruningCheck(void);

	// Returns a reference to the objects within this node
	CMPINLINE const std::vector<iEnvironmentObject*> &		GetNodeObjects(void) const	{ return m_objects; }
	CMPINLINE const std::vector<Terrain*> &			GetNodeTerrain(void) const	{ return m_terrain; }

	// Returns a reference to this node's ultimate parent, i.e. the root node at the top of its tree
	EnvironmentTree *							GetUltimateParent(void);

	// Utility method to determine how deep in the tree this item is.  Force-terminates after 100 levels to avoid infinite loops/stack failure
	int											DetermineTreeDepth(void);

	// Utility method to determine the size of the tree below this node.  Runs recursively so can result in deep stack calls.
	int											DetermineTreeSize(void);

	// Returns a value indicating whether this is a leaf node (i.e. has no child nodes below it)
	CMPINLINE bool								IsLeafNode(void) const					{ return (m_childcount == 0); }

	// Returns a value indicating whether this is a branch node (i.e. has no items, and has child nodes below it)
	CMPINLINE bool								IsBranchNode(void) const				{ return (m_childcount != 0); }

	// Debug method to generate a string output of an EnvironmentTree and its recursively-defined children
	std::string									DebugOutput(void);

	// Default destructor
	~EnvironmentTree(void);

protected:

	// Items held within the node
	std::vector<iEnvironmentObject*>			m_objects;
	std::vector<Terrain*>					m_terrain;

	// Item counts
	int											m_objectcount, m_terraincount, m_totalitemcount;

	// Pointer to the environment that this tree is assigned to
	iSpaceObjectEnvironment *					m_environment;

	// Pointers to parent and child nodes
	EnvironmentTree *							m_parent;
	EnvironmentTree *							m_children[8];
	int											m_childcount;

	// Minimum and maximum extents of the node in position- and element-space
	AXMVECTOR									m_min, m_max;
	XMFLOAT3									m_fmin, m_fmax;
	INTVECTOR3									m_elmin, m_elmax;

	// Centre point of the node, used to determine which child node should be selected.  This is not the actual
	// centre; rather this is the point around which the node may be subdivided.  It falls on an x/y/z boundary
	AXMVECTOR									m_centre;
	XMFLOAT3									m_fcentre;

	// Actual centre point of the node, calculated as (min + max) / 2
	XMVECTOR									m_actualcentre;

	// 'Centre' element is the one in the top-top-left of the '+ve' child nodes, e.g. in a 4x4 node, from 10-13 in each dimension, 
	// the 'centre' element is (12,12,12).  Has no meaning for a one-element distance in any dimension - in that case the centre point
	// is top left
	INTVECTOR3									m_elcentre;

	// Size of the node in each dimension
	AXMVECTOR									m_size;
	XMFLOAT3									m_fsize;
	INTVECTOR3									m_elsize;

	// Approximate size of the bounding sphere that encompasses this node, for more efficient rendering
	float										m_bounding_radius;

	// Vector of pointers to active child nodes; can be between 0 and 8 noddes
	std::vector<EnvironmentTree*>				m_active_children;

	// Flag which determines whether this node can be further subdivided, or has reached the minimum allowable level
	bool										m_can_be_subdivided;

	// Flag that determines whether this node should be checked in the next pruning cycle.  Set when one of our children has an item removed
	bool										m_pruningflag;

	// Internal methods to retrieve item count for a particular type
	template <typename T> CMPINLINE int			ItemCount(void) const { throw "EnvironmentTree cannot support this object type"; }
	template <> CMPINLINE int					ItemCount<iEnvironmentObject*>(void) const { return m_objectcount; }
	template <> CMPINLINE int					ItemCount<Terrain*>(void) const { return m_terraincount; }

	// Internal methods to return a reference to the item collection or its item counts
	template <typename T> CMPINLINE std::vector<T> &			ItemCollectionReference(void) { throw "EnvironmentTree cannot support this object type"; }
	template <> CMPINLINE std::vector<iEnvironmentObject*> &	ItemCollectionReference<iEnvironmentObject*>(void) { return m_objects; }
	template <> CMPINLINE std::vector<Terrain*> &			ItemCollectionReference<Terrain*>(void) { return m_terrain; }
	template <typename T> CMPINLINE int &						ItemCountReference(void) { throw "EnvironmentTree cannot support this object type"; }
	template <> CMPINLINE int &									ItemCountReference<iEnvironmentObject*>(void) { return m_objectcount; }
	template <> CMPINLINE int &									ItemCountReference<Terrain*>(void) { return m_terraincount; }

	// Internal methods to add an item to the appropriate collection and recalculate derived statistics
	template <typename T> void					AddItemToCollection(T item);

	// Reallocate all items of the specified type into the relevant child node
	template <typename T> void					PerformReallocationOfItemsToChildren(std::vector<T> & items);


public:

	// Static memory pool of Octree objects, for more efficient allocation/deallocation during execution
	static MemoryPool<EnvironmentTree> * 		_MemoryPool;

	// Static method to shut down the memory pool and release any allocated resources *still within the pool*.  Any items requested
	// and taken by other processes are the responsibility of the other process to deallocate
	static void									ShutdownMemoryPool(void)
	{
		if (_MemoryPool)
		{
			_MemoryPool->Shutdown();
			SafeDelete(_MemoryPool);
		}
	}

};




#endif