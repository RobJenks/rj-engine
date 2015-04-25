#pragma once

#ifndef __UIComponentGroupH__
#define __UIComponentGroupH__

#include <vector>
#include <string>
#include "CompilerSettings.h"
#include "iUIComponent.h"
using namespace std;


class UIComponentGroup : iUIComponent
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

	// Methods to set/retrieve the group code
	CMPINLINE string			GetCode(void) { return m_code; }
	CMPINLINE void				SetCode(string code) { m_code = code; } 
	
	// Method to query or change the render state 
	CMPINLINE bool				IsGroupRenderingEnabled(void) { return m_render; }
	void						DisableGroupRendering();
	void						EnableGroupRendering(bool ignore_previous_state);

	// Constructor / destructor
	UIComponentGroup(void);
	~UIComponentGroup(void);

	// Shutdown method
	void						Shutdown(void);

	// Methods to satisfy the iUIComponent interface and thereby allow nested component groups
	CMPINLINE bool				GetRenderActive(void) { return IsGroupRenderingEnabled(); }
	void						SetRenderActive(bool render);


private:

	// Code that identifies this component group
	string					m_code;

	// The collection of all items within this component group
	ComponentGroupItems		m_items;
	
	// The current render state of the group
	bool					m_render;

};



#endif