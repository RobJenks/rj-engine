#pragma once

#ifndef __HashIndexedVectorH__
#define __HashIndexedVectorH__

#include <type_traits>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include "DefaultValues.h"

template <typename T, typename TIndex>
class HashIndexedVector
{
public:

	typedef typename std::vector<typename T>							CollectionType;
	typedef typename CollectionType::size_type							Index;

	// Returns a reference to the underlying vector collection
	CMPINLINE CollectionType &											Items(void) { return m_items; }

	// Returns the item at the specified index
	CMPINLINE T 														Get(Index index) const { return m_items[index]; }
	
	// Returns a reference to the item at the specified index
	CMPINLINE T &														GetReference(Index index) { return m_items[index]; }

	// Returns an iterator to the item with the specified hash index, or items.end() if it does not exit
	CMPINLINE typename CollectionType::iterator							Get(TIndex index)
	{
		MapType::const_iterator it = m_map.find(index); 
		return (it == m_map.end() ? m_items.end() : (m_items.begin() + it->second));
	}

	// Attempts to locate the specified object in the collection.  Returns an iterator, or items.end() if the 
	// object cannot be found
	CMPINLINE typename CollectionType::iterator							Find(T item)
	{
		return (std::find(m_items.begin(), m_items.end(), item));
	}

	// Adds an item with the specified hash index
	CMPINLINE void														Add(T item, TIndex index)
	{
		m_items.push_back(item);
		m_map[index] = (m_items.size() - 1);
	}

	// Adds an item without specifying the hash index
	CMPINLINE void														Add(T item)
	{
		m_items.push_back(item);
	}

	// Link an existing item to the specified hash index
	CMPINLINE void														SetIndex(Index item, TIndex index)
	{
		m_map[index] = item;
	}
	
	// Link an existing item to the specified hash index
	CMPINLINE void														SetIndex(typename CollectionType::iterator item, TIndex index)
	{
		if (item == m_items.end()) return;
		m_map[index] = std::distance(m_items.begin(), item);
	}
	
	// Remove an item from the collection.  All indices and iterators beyond this item in the collection are invalidated
	CMPINLINE void														Remove(Index item)
	{
		if (index < 0 || index >= m_items.size()) return;
		m_items.erase(m_items.begin() + item);
		
		MapType::iterator it_end = m_map.end();
		for (MapType::iterator it = m_map.begin(); it != it_end; /* No increment */)
		{
			if (it->second == item)
				it = m_map.erase(it);
			else
			{
				if (it->second > item) it->second -= 1;
				++it;
			}
		}
	}

	// Remove an item from the collection.  All indices and iterators beyond this item in the collection are invalidated
	CMPINLINE void														Remove(typename CollectionType::iterator item)
	{
		if (item == m_items.end()) return;
		Index index = std::distance(m_items.begin(), item);
		m_items.erase(item);

		MapType::iterator it_end = m_map.end();
		for (MapType::iterator it = m_map.begin(); it != it_end; /* No increment */)
		{
			if (it->second == index)
				it = m_map.erase(it);
			else
			{
				if (it->second > index) it->second -= 1;
				++it;
			}
		}
	}

	// Clear the collection
	CMPINLINE void														Clear(void)
	{
		m_items.clear();
		m_map.clear();
	}

	// Returns the size of the collection
	CMPINLINE Index														Size(void) const		{ return m_items.size(); }

	

protected:

	// Vector collection of data
	CollectionType														m_items;

	// Hashed index into the data collection
	typedef std::unordered_map<typename TIndex, typename Index>			MapType;
	MapType																m_map;

};





#endif