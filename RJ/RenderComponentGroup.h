#pragma once

#ifndef __RenderComponentGroupH__
#define __RenderComponentGroupH__

#include <string>
#include <unordered_map>
#include "CompilerSettings.h"
using namespace std;
using namespace std::tr1;


template <typename T> class RenderComponentGroup
{
public:
	typedef unordered_map<string, T> ItemCollection;

	// Public methods to manipulate the component collection
	CMPINLINE ItemCollection *Items(void) { return m_collection; }
	CMPINLINE T GetItem(string key) { return __GetItem(*m_collection, key); }
	CMPINLINE void RemoveItem(string key) { __RemoveItem(*m_collection, key); }
	CMPINLINE void AddItem(string key, T item) { __AddItem(*m_collection, key, item); }

	// Links this component manager to a component collection in the render group
	void LinkToCollection(ItemCollection *items) { m_collection = items; }

	// Constructor
	RenderComponentGroup<T>::RenderComponentGroup(void) { m_collection = NULL; }
	
	// Destructor
	RenderComponentGroup<T>::~RenderComponentGroup(void) { }

	// Deletes & deallocates the entire collection.  Only implemented for components deriving from iUIComponent
	void ShutdownUIComponentGroup(void)
	{
		ItemCollection::iterator it_end = m_collection->end();
		for (ItemCollection::iterator it = m_collection->begin(); it != it_end; ++it)
		{
			// Get a reference to this component
			iUIComponent *component = (iUIComponent*)it->second;
			if (!component) continue;

			// Call the component shutdown method and delete the object 
			component->Shutdown();
			delete component;
		}

		// Finally clear the collection now that it no longer contains any valid component pointers
		m_collection->clear();
	}

	// Deletes & deallocates the entire collection.  Used for classes not deriving from iUIComponent, 
	// and not requiring the execution of its shutdown method
	void ShutdownOtherComponentGroup(void)
	{
		ItemCollection::iterator it_end = m_collection->end();
		for (ItemCollection::iterator it = m_collection->begin(); it != it_end; ++it)
		{
			// Get a reference to this component
			T component = it->second;
			if (!component) continue;

			// Delete the object 
			delete component;
		}

		// Finally clear the collection now that it no longer contains any valid component pointers
		m_collection->clear();
	}

	

private:

	// Reference to the collection of items being managed
	ItemCollection *					m_collection;
	
	// Private method to return an item from the collection (via reference indirection)
	CMPINLINE T __GetItem(ItemCollection &__collection, string key) { 
		if (__collection.count(key) != 0) return __collection[key]; else return NULL; 
	}

	// Private method to remove an item from the collection (via reference indirection)
	CMPINLINE void __RemoveItem(ItemCollection &__collection, string key) { 
		if (__collection.count(key) != 0) __collection[key] = NULL; 
	}

	// Private method to add an item to the collection (via reference indirection)
	CMPINLINE void __AddItem(ItemCollection &__collection, string key, T item) {
		if (__collection.count(key) == 0) 
			__collection[key] = item;		// Add to the collection
	}
};

#endif