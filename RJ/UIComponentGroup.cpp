#include <vector>
#include "iUIComponent.h"

#include "UIComponentGroup.h"
using namespace std;


UIComponentGroup::UIComponentGroup(void)
{
	m_render = true;
}

// Disables rendering of all items in the group, and stores the prior render state for restoring later
void UIComponentGroup::DisableGroupRendering()
{
	// Only proceed if the group is currently active
	if (m_render == false) return;

	// Store the new group render state
	m_render = false;

	// Loop through the collection and disable rendering at the item level
	ComponentGroupItems::size_type n = m_items.size();
	for (ComponentGroupItems::size_type i = 0; i < n; ++i)
	{
		m_items[i].previousrenderstate = m_items[i].component->GetRenderActive();
		m_items[i].component->SetRenderActive(false);
	}
}

// Restores the render state of the group, or optionally ignores the previous state and makes all items visible
void UIComponentGroup::EnableGroupRendering(bool ignore_previous_state)
{
	// Only proceed if the group is not already active
	if (m_render) return;

	// Store the new group render state
	m_render = true;

	// Loop through the collection and restore rendering at the item level
	ComponentGroupItems::size_type n = m_items.size();
	for (ComponentGroupItems::size_type i = 0; i < n; ++i)
	{
		m_items[i].component->SetRenderActive( (ignore_previous_state ? true : m_items[i].previousrenderstate) );
	}
}

// Returns the index of an item in the collection if it exists, or -1 if it does not
int UIComponentGroup::FindItem(string code)
{
	// Loop through the collection and try to locate the item
	ComponentGroupItems::size_type n = m_items.size();
	for (ComponentGroupItems::size_type i = 0; i < n; ++i)
	{
		if (m_items[i].component->GetCode() == code) return (int)i;
	}

	return -1;
}

// Returns the index of an item in the collection if it exists, or -1 if it does not
int UIComponentGroup::FindItem(iUIComponent *item)
{
	// Loop through the collection and try to locate the item
	ComponentGroupItems::size_type n = m_items.size();
	for (ComponentGroupItems::size_type i = 0; i < n; ++i)
	{
		if (m_items[i].component == item) return (int)i;
	}

	return -1;
}

// Adds an item to the collection, if it doesn't already exist
void UIComponentGroup::AddItem(iUIComponent *item)
{
	if (item && FindItem(item) == -1)
		m_items.push_back(item);
}
	
// Removes an item from the collection, if it does exist in the collection
void UIComponentGroup::RemoveItem(iUIComponent *item)
{
	// Attempt to locate the item in the collection, then call the overloaded function that takes a vector index
	int index = FindItem(item);
	RemoveItem(index);
}

// Removes an item from the collection, if it does exist in the collection
void UIComponentGroup::RemoveItem(int index)
{
	// Parameter check
	int size = (int)m_items.size();
	if (index < 0 || index >= size) return;

	// Perform a swap-and-pop to remove this item from the collection
	std::swap(m_items[index], m_items[size-1]);
	m_items.pop_back();
}

// Returns a reference to the item collection
UIComponentGroup::ComponentGroupItems *UIComponentGroup::GetItems(void)
{
	return &m_items;
}

// Returns a reference to a specific item in the collection
iUIComponent *UIComponentGroup::GetItem(int index)
{
	if (index < 0 || index >= (int)m_items.size())
		return NULL;
	else
		return m_items.at(index).component;
}

UIComponentGroup::~UIComponentGroup(void)
{
}

// Implemented to satisfy the iUIComponent interface.  Simply makes a call to the relevant component group method
void UIComponentGroup::SetRenderActive(bool render)
{
	// Call the relevant group rendering function depending on the render flag specified
	if (render)
		EnableGroupRendering(false);			// Default enable mode; don't use the option to ignore previous component render states
	else
		DisableGroupRendering();				// Disable rendering of all components in the grop
}

// Shuts down the component group
void UIComponentGroup::Shutdown(void)
{
	// This object simply groups components together; it does not 'own' them.  We therefore simply want
	// to clear the component vector without deleting objects when shutting down
	m_items.clear();
}