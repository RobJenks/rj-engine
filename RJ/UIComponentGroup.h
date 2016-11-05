#pragma once

#ifndef __UIComponentGroupH__
#define __UIComponentGroupH__

#include <vector>
#include <string>
#include "CompilerSettings.h"
#include "iUIComponent.h"
using namespace std;


class UIComponentGroup : public iUIComponent
{

public:

	struct Item 
	{
		iUIComponent *		component;
		bool				previousrenderstate;

		Item(void) { component = NULL; previousrenderstate = true; }
		Item(iUIComponent *item) { component = item; previousrenderstate = true; }
	};
	typedef vector<UIComponentGroup::Item> ComponentGroupItems;


public:

	// Add/remove/retrieve items from the collection
	void						AddItem(iUIComponent *item);
	void						RemoveItem(int index);
	void						RemoveItem(iUIComponent *item);
	int							FindItem(string code);
	int							FindItem(iUIComponent *item);
	iUIComponent *				GetItem(int index);
	ComponentGroupItems *		GetItems(void);

	// Method to query or change the render state 
	CMPINLINE bool				IsGroupRenderingEnabled(void) const { return m_render; }
	void						DisableGroupRendering();
	void						EnableGroupRendering(bool ignore_previous_state);

	// Constructor / destructor
	UIComponentGroup(void);
	~UIComponentGroup(void);

	// Shutdown method
	void						Shutdown(void);

	// Methods to satisfy the iUIComponent interface and thereby allow nested component groups
	CMPINLINE bool				GetRenderActive(void) const  { return IsGroupRenderingEnabled(); }
	void						SetRenderActive(bool render);


private:

	// The collection of all items within this component group
	ComponentGroupItems		m_items;
	
};



#endif