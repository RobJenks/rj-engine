/* NOTE: NOT CURRENTLY IN USE.  Replaced with simpler std collection for the immediate region particles.  To continue using
   for other purposes, require an efficient way to remove & replace items.  To remove the item we need to locate it
   in every linked list in turn, so we can remove each of the nodes and relink it's neighbours.  Perhaps require pointers
   from the element in the main collection to each of its x/y/z list nodes */


#pragma once

#ifndef __SpatiallyLinkedCollectionH__
#define __SpatiallyLinkedCollectionH__

#include <cstdlib>
#include <vector>
using namespace std;

// This class has no special alignment requirements
template <typename T> class SpatiallyLinkedCollection
{
	typedef int COORD;

public:

	// Struct that holds the object data in memory, along with coordinates
	struct Element
	{
		T *item;
		COORD x, y, z;

		Element(T *item, COORD x, COORD y, COORD z) { this->item = item; this->x = x; this->y = y; this->z = z; }
	};

	// The node used to construct the spatial linked lists
	struct Node
	{	
		Element *item;		// The item this node is referencing
		COORD coord;		// Position in this dimension

		Node *prev;			// Link to previous item in this spatial dimension
		Node *next;			// Link to next item in this spatial dimension

		// Constructor for this node struct
		Node(Element *e, COORD coord, Node *prev, Node *next)
		{
			this->item = e;
			this->coord = coord;
			this->prev = prev;
			this->next = next;
		}
	};

private:
	typedef std::vector<Element*>			ITEM_STORE;
	ITEM_STORE								m_items;		// The collection of objects; stores a vector of pointers

	Node *									m_xlist;		// The linked list for the x dimension
	Node *									m_ylist;		// The linked list for the y dimension
	Node *									m_zlist;		// The linked list for the z dimension

	Node *									m_xend;			// Pointer to the end of the x list
	Node *									m_yend;			// Pointer to the end of the y list
	Node *									m_zend;			// Pointer to the end of the z list



/*******************************************************/
/*************** Function definitions ******************/
/*******************************************************/

public:

	// Adds a new element to a linked list.  Returns a pointer to the linked list, in case the first element is changed
	Node *AddToLinkedList(Element *e, Node *list, Node *end, COORD coord)
	{
		// Locate the previous node in the list, i.e. the item we want to insert this one after
		Node *prevnode = FindPositionInList(list, coord);

		if (!prevnode)	// If NULL, we want to add to the start of the list
		{
			Node *n = new Node(e, coord, NULL, list);			// New node will be the first in the list
			if (list) 
				list->prev = n;									// Assign list previous node to be this new one, assuming it does exist
			else
				end = n;										// If !list, i.e. this is the first addition, then also make this the end node

			return n;											// Return a pointer to the NEW OBJECT, since this is the new first element
		}
		else			// Otherwise, we add immediately after "prevnode"
		{
			Node *n = new Node(e, coord, prevnode, prevnode->next);	// Node will go between prevnove and prevnode's 'next' pointer
			if (prevnode->next) 
				(prevnode->next)->prev = n;							// Next node's "prev" is set to us, if that node does exist
			else
				end = n;											// If there is no next node, we are the end, so assign the end pointer

			prevnode->next = n;										// Previous node's "next" now points to us
			return list;											// Return pointer to the list
		}
	}


	// Traverses the list to locate the correct position for an item with coord x.  Returns a pointer to the item that will
	// become its new "next" node, i.e. the item after the position we want to insert at
	Node *FindPositionInList(Node *list, COORD coord)
	{
		// Traverse the list one node at a time
		while (list)
		{
			// If the new item is located before the current one, then we want to add it at this position
			// Return the item *BEFORE* the point we want to add at; will therefore return NULL to add to the start of the list
			if (coord < list->coord) return list->prev;

			// If this is the last item in the list then return it, to signify that we want to add at the end
			if (!list->next) return list;

			// Move to the next item in the linked list
			list = list->next;
		}

		// We only get here if the initial list was NULL; if so, return NULL to signify that we should add as the first element
		return NULL;
	}

	// Adds an item with the specified coordinates
	void AddItem(Element *e)
	{
		// Parameter check 
		if (!e) return;

		// Push into the main collection
		m_items.push_back(e);

		// Also add to all linked lists
		AddItemToLinkedLists(e);
	}

	CMPINLINE void AddItemToLinkedLists(Element *e)
	{
		// Allocate to the correct position in each linked list
		m_xlist = AddToLinkedList(e, m_xlist, m_xend, e->x);
		m_ylist = AddToLinkedList(e, m_ylist, m_yend, e->y);
		m_zlist = AddToLinkedList(e, m_zlist, m_zend, e->z);
	}

	Element *RemoveItem(Element *e)
	{
		
	}

	Element *RemoveItem(int x, int y, int z)
	{

	}

	// Finds an item, using one of the three dimension linked lists as a search key depending on the value we are looking for
	Element *FindItem(int x, int y, int z)
	{
		if (x < y)
			if (x < z)
				return FindItemInList(m_xlist, x, x, y, z);		// Use x dimension as primary search list
			else
				return FindItemInList(m_zlist, z, x, y, z);		// Use z dimension as primary search list
		else
			if (y < z)
				return FindItemInList(m_ylist, y, x, y, z);		// Use y dimension as primary search list
			else
				return FindItemInList(m_zlist, z, x, y, z);		// Use z dimension as primary search list

	}

	// Finds an item, using the specified linked list as the primary search criteria
	Element *FindItemInList(Node *list, int coord, int x, int y, int z)
	{
		// Process the x-list
		Node *n = list;

		// Loop while we have nodes remaining, and while we have not gone past the search value
		while (n && !(n->coord > coord))
		{
			// Test all coords and return (the first) match
			if (n->item->x == x && n->item->y == y && n->item->z == z)
				return n->item;

			// Move to the next node
			n = n->next;
		}

		// Otherwise we did not locate the item
		return NULL;
	}


	// Outputs the state of this object in string format, for debugging purposes
	string OutputState(void)
	{
		// Build a string to represent this collection
		std::ostringstream s;
		s << "SpatiallyLinkedCollection (" << m_items.size() << " items)\n\n";

		// Output the structure of the xlist
		s << "X-list = { " << OutputListState(m_xlist) << "}\n";
		s << "Y-list = { " << OutputListState(m_ylist) << "}\n";
		s << "Z-list = { " << OutputListState(m_zlist) << "}\n\n";
		
		return s.str();
	}

	// Outputs the state of a particular linked list
	string OutputListState(Node *list)
	{
		std::ostringstream s;
		Node *node = list;

		while (node) {
			s << "(" << node->item->x << "," << node->item->y << "," << node->item->z << ") ";
			node = node->next;
		}

		return s.str();
	}

	// Clears the collection of all items
	void ClearCollection(bool DeallocateObjects)
	{
		// For each item in the collection
		ITEM_STORE::iterator it_end = m_items.end();
		for (ITEM_STORE::iterator it = m_items.begin(); it != it_end; ++it)
		{
			// If we need to deallocate the underlying objects then do so here
			if (DeallocateObjects) delete ((Element*)*it)->item;

			// Delete the Element object represented by this iterator
			delete (*it);
		}

		// Now also deallocate any memory used by the linked lists, by following the pointers and deallocating as we go
		ClearLinkedList(m_xlist);
		ClearLinkedList(m_ylist);
		ClearLinkedList(m_zlist);
	
	}

	// Deallocates any memory used by the linked lists, by following the pointers and deallocating as we go.  Note that
	// this method relies on the integrity of the doubly-linked list.  If the integrity is compromised the method will 
	// likely fail, or fail to clear all nodes correctly.
	void ClearLinkedList(Node *list)
	{
		if (!list) return;

		// While we still have a next node, move to the next node and deallocate the previous one
		while (list->next != NULL) 
		{
			list = list->next;
			delete (list->prev);
			list->prev = NULL;
		}

		// Deallocate the final node to complete
		delete list; 
		list = NULL;
	}

	CMPINLINE Node *GetXList() { return m_xlist; }
	CMPINLINE Node *GetXEnd() { return m_xend; }
	CMPINLINE Node *GetYList() { return m_ylist; }
	CMPINLINE Node *GetYEnd() { return m_yend; }
	CMPINLINE Node *GetZList() { return m_zlist; }
	CMPINLINE Node *GetZEnd() { return m_zend; }


	// Constructor
	SpatiallyLinkedCollection<T>::SpatiallyLinkedCollection(void)
	{
		this->m_xlist = NULL;
		this->m_ylist = NULL;
		this->m_zlist = NULL;
	}

	// Destructor
	SpatiallyLinkedCollection<T>::~SpatiallyLinkedCollection(void)
	{
	}



};








#endif