#pragma once

#ifndef __BinaryHeapH__
#define __BinaryHeapH__

#include <memory.h>
#include <malloc.h>
#include <algorithm>
#include "CompilerSettings.h"

// This class has no special alignment requirements
template <typename TVal, typename TItem>
class BinaryHeap
{
public:
	// Struct holding the value and object data together
	struct Element 
	{
		TVal	Value;
		TItem	Item;

		Element(TVal value, TItem item) { Value = value; Item = item; }
	};

	// Heap size limit to prevent overallocation
	static const int HEAP_SIZE_LIMIT = 50000;

	// Array of elements in the heap
	Element *	Items;
	
	// Store the size and capacity of the heap
	int			Size;
	int			Capacity;

	/**************************************************************************
	  Default constructor - initialises all data to NULL
	 **************************************************************************/
	BinaryHeap(void)
	{
		Items = NULL;
		Capacity = Size = 0;
	}

	/**************************************************************************
	  Initialise: Sets up the heap and initialises it to a specific size
	 **************************************************************************/
	Result Initialise(int size)
	{
		// If there is already any space allocated then deallocate it now
		if (Items) { free(Items); Items = NULL; Capacity = Size = 0; }

		// Make sure this is a valid size
		if (size < 0 || size > HEAP_SIZE_LIMIT) { Items = NULL; Capacity = Size = 0; return ErrorCodes::CouldNotAllocateBinaryHeap; }

		// Allocate space for the heap and zero it by default.  We will use a 1-base array since this is more efficient
		// for tracing back to parent elements by dividing by two.  It means that element[0] will always be unused & zero.
		Capacity = size;
		Items = (Element*)malloc(sizeof(Element) * (Capacity + 1));
		memset(Items, 0, sizeof(Element) * (Capacity + 1));

		// No items exist in the heap to start with
		Size = 0;
		return ErrorCodes::NoError;
	}


	/**************************************************************************
	  AddItem: Add a new item to the heap, inserting it into the correct location based on its value
	 **************************************************************************/
	void AddItem(TVal value, TItem item)
	{
		int indexBy2;

		// Add to the end of the heap
		Items[++Size] = BinaryHeap::Element(value, item);
		
		// Now we need to move the element to the correct position in the heap.  Loop while the 
		// element has not reached the top of the heap (index == 1)
		int index = Size;
		while (index != 1)
		{
			// Check whether this element has better value than its parent
			indexBy2 = index / 2;
			if (Items[index].Value <= Items[indexBy2].Value)
			{
				// If it does, swap it with the parent now
				std::swap(Items[index], Items[indexBy2]);

				// Move up a level and see if the element (now at its parent's position) needs to move further
				index = indexBy2;
			}
			else
			{
				// If the child is not better than its parent then it is in the right place, so quit here
				return;
			}
		}
	}

	/**************************************************************************
	  RemoveTopItem: Removes the item from the top of the heap, resorting the heap in response
	 **************************************************************************/
	void RemoveTopItem(void)
	{
		// Swap the last element with the top element, and reduce the size of the heap by one
		Items[1] = Items[Size];
		--Size;

		// Now resort the heap.  Compare the new top element with its children, and if it has
		// worse value then swap it with the better of the two children.  Repeat until the correct
		// place is found
		int u, u2; int v = 1;
		while (true)
		{
			// Set u (parent) equal to v.  Initially, v = 1 = the top element.  In subsequent loops v will become
			// the best child element, which the parent is swapped into repeatedly until it reaches the right place
			u = v; u2 = 2*u;

			// Take different action depending on the number of child elements
			if (u2+1 <= Size)
			{
				// Both children exist, so compare against each and select the lowest of all three (parent + 2 children)
				if (Items[u].Value >= Items[u2].Value)		v = u2;
				if (Items[v].Value >= Items[u2+1].Value)	v = u2+1;
			}
			else if (u2 <= Size)
			{
				// Only one child exists, so compare against just that item
				if (Items[u].Value >= Items[u2].Value)		v = u2;
			}

			// Now check if we need to make a swap.  If v was changed from its starting value of u (the index
			// of the parent element) then we need to swap element u (the parent) & element v (the best child)
			if (u != v)
			{
				std::swap(Items[u], Items[v]);		// Swap the parent with its best child and repeat
			}
			else
			{
				return;								// Parent is in the right place, so quit here
			}
		}
	}

	/**************************************************************************
	  ClearHeap: Resets the heap to empty.  Elements are not cleared; simply resets pointers
	 **************************************************************************/
	CMPINLINE void ClearHeap(void)
	{
		// Setting the size to zero is equivalent to clearing the heap.  No other changes needed
		Size = 0;
	}
	
	/**************************************************************************
	  ReorderElement: Reorders an existing element, if required due to a change in its value
	 **************************************************************************/
	CMPINLINE void ReorderElement(TItem item)
	{
		for (int i = 0; i < Size; i++)
		{
			if (Items[i].Item == item)
			{
				ReorderElement(i);
				return;
			}
		}
	}

	/**************************************************************************
	  ReorderElement: Reorders an existing element, if required due to a change in its value
	 **************************************************************************/
	void ReorderElement(int index)
	{
		int indexBy2;

		// We will bubble the item up towards the root node as far as required
		while (index != 1)
		{
			indexBy2 = index / 2;
			if (Items[index].Value < Items[indexBy2].Value)
			{
				// Child value (the element in question) is better than its parent, so swap
				// the two now.  Repeat until in the right place or at the root node
				std::swap(Items[index], Items[indexBy2]);

				// Move to the parent (which is now this element) and repeat the test
				index = indexBy2;
			}
			else
			{
				// Child is not better than the parent, so it is in the right place.  Quit now.
				return;
			}
		}
	}

	/**************************************************************************
	  Default destructor.  Deallocates all memory assigned for the binary heap
	 **************************************************************************/
	~BinaryHeap(void)
	{
		// Deallocate the memory allocated for this heap
		if (Items) free(Items);
		Capacity = Size = 0;
	}


};






#endif