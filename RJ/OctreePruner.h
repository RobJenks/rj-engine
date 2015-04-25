#pragma once

#ifndef __OctreePrunerH__
#define __OctreePrunerH__

#include <vector>
#include "Utility.h"
#include "ScheduledObject.h"
template <class T> class Octree;

template <typename T> 
class OctreePruner : public ScheduledObject
{
public:
	OctreePruner(void);
	~OctreePruner(void);

	// Override the scheduling methods to perform periodic pruning of octree items
	CMPINLINE void Update(void) { }					// No per-frame update required
	void UpdateInfrequent(void);

	// Methods to add or remove items to be pruned on a regular basis
	void AddNode(Octree<T> *node);
	void RemoveNode(Octree<T> *node);


private:

	// Vector of nodes in scope for pruning
	std::vector<Octree<T>*> m_items;

	// Temporary iterators for traversing the item vector
	typename std::vector<Octree<T>*>::iterator it, it_end;
};


// Default constructor
template <typename T>
OctreePruner<T>::OctreePruner(void)
{
}

// Override the scheduling methods to perform periodic pruning of octree items
template <typename T>
void OctreePruner<T>::UpdateInfrequent(void)
{
	it_end = m_items.end();
	for (it = m_items.begin(); it != it_end; ++it)
	{
		(*it)->PerformPruningCheck();
	}
}

// Add a node to be recursively pruned on a regular basis
template <typename T>
void OctreePruner<T>::AddNode(Octree<T> *node)
{
	// Push onto the vector if this is a valid node
	if (node) m_items.push_back(node);
}

// Remove a node from the pruning vector
template <typename T>
void OctreePruner<T>::RemoveNode(Octree<T> *node)
{
	// If the node is valid, remove it from the vector
	if (node) RemoveFromVector<Octree<T>*>(m_items, node);
}

// Default destructor
template <typename T>
OctreePruner<T>::~OctreePruner(void)
{
}





#endif