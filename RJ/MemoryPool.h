#pragma once

#ifndef __MemoryPoolH__
#define __MemoryPoolH__

#include <vector>
#include "CompilerSettings.h"

template <class Octree> class MemoryPool;

// This class has no special alignment requirements
template <typename T> class MemoryPool
{
public:

	// Custom type to hold pool instances of type T
	typedef std::vector<T*> PoolItemCollection;

	// Define starting size of each memory pool; pool will double in size from this point whenever required
	static const typename PoolItemCollection::size_type POOL_STARTING_CAPACITY = 32;

	// Constructor.  Creates the initial pool of objects ready for use
	MemoryPool<T>::MemoryPool(void)
	{
		// Initialise key variables
		m_itemcount = 0;

		// We will start the process by setting the initial capacity, pretending that is our current state, requesting more space, 
		// then finally reverting the capacity back (e.g. initial capacity = 32, allocating will *=2 = 64, so we then revert back to 32)
		m_capacity = POOL_STARTING_CAPACITY;
		ExtendPool();
		m_capacity /= 2;
	}

	// Extends the size of the memory pool by doubling its capacity and allocating memory for all new objects via the default constructor
	void MemoryPool::ExtendPool(void)
	{
		// Add new items to the pool to double the existing capacity.  TODO: Can this be done in bulk more efficiently?
		m_items.reserve(m_capacity * 2);
		for (PoolItemCollection::size_type i = 0; i < m_capacity; ++i)
		{
			m_items.push_back(new T());
		}

		// Increase the count of available items, plus the overall capacity, to reflect the addition of these new items
		m_itemcount = m_items.size();			// This should always be equivalent to m_itemcount += m_capacity
												// However this adds a safety net for unexpected changes to the vector
		m_capacity *= 2;
	}

	// Requests a new item from the pool.  If the pool is now empty it is extended and then a new item is returned
	T *MemoryPool::RequestItem(void)
	{
		// If the pool is empty then we need to extend it
		if (m_itemcount == 0) ExtendPool();

		// We should now definitely have an item available.  Return the next item from the pool vector
		T *item = m_items.back();
		m_items.pop_back();
		--m_itemcount;
		return item;
	}

	// Returns an item to the pool once it is no longer required
	void MemoryPool::ReturnItem(T *item)
	{
		// Make sure this is a valid item
		if (item == NULL) return;

		// Return item to the pool and increase the item count accordingly
		m_items.push_back(item);
		++m_itemcount;
	}

	// Method to return the number of items currently checked out to all other processes
	CMPINLINE typename std::vector<T*>::size_type MemoryPool::NumberOfItemsRequested(void)	{ return (m_capacity - m_itemcount); }

	// Destructor
	MemoryPool<T>::~MemoryPool(void)
	{
	}

	// Deallocate all items currently in the pool (this will not deallocate any items checked out to other processes)
	void MemoryPool<T>::Shutdown(void)
	{
		// Process each item in the pool in turn
		PoolItemCollection::size_type n = m_items.size();
		for (PoolItemCollection::size_type i = 0; i < n; ++i)
		{
			if (m_items[i]) 
			{
				delete m_items[i];
				m_items[i] = NULL;
			}
		}

		// Reset counters (although this isn't really required since the pool is being deallocated anyway)
		m_itemcount = m_capacity = 0;

		// Clear the memory pool vector now that it contains only invalid pointers
		m_items.clear();
	}

private:

	// The pool of available items
	PoolItemCollection							m_items;

	// The number of items currently held in the pool and available for use
	typename PoolItemCollection::size_type		m_itemcount;

	// The current capacity of the pool, i.e. the total number of objects available in the pool plus those being actively used elsewhere
	typename PoolItemCollection::size_type		m_capacity;
};


#endif