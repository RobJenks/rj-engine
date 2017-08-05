#pragma once

#ifndef __UIComboBox__
#define __UIComboBox__

#include <string>
#include <time.h>
#include "iUIControl.h"
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Utility.h"
class Image2DRenderGroup;
class TextBlock;
class UIManagedControlDefinition;
class GameInputDevice;
using namespace std;


// This class has no special alignment requirements
class UIComboBox : public iUIControl
{
public:
	// Custom struct for the data stored per item in the combobox
	struct ComboBoxItem { 
		string Value; string Tag; 
		ComboBoxItem()							{ Value = ""; Tag = ""; }
		ComboBoxItem(string value)				{ Value = value; Tag = ""; }
		ComboBoxItem(string value, string tag)	{ Value = value; Tag = tag; }
	};

	// Constants determining defaults values and display parameters
	static const int DEFAULT_TEXT_CAPACITY;
	static const int DEFAULT_EXPAND_SIZE;
	static const int EXPAND_MIN_SIZE;
	static const int EXPAND_MAX_SIZE;
	static const float CONTROL_ACTIVE_AREA;
	static const int TEXT_OFFSET_X;
	static const int TEXT_OFFSET_Y;
	static const int TEXT_SEPARATION;
	static const int SCROLL_HANDLE_MIN_SIZE;
	static const float SCROLL_INTERVAL;

	// Flag determinig whether the control performs managed rendering
	CMPINLINE static bool						PerformsManagedRendering(void) { return true; }

	// Enumeration of different expand states for the control
	enum ExpandState { Unknown = 0, Contracted, Expanded /* Potential to extend to Expanding/Contracting for animation */ };

	// Returns the type of control
	CMPINLINE iUIControl::Type					GetType(void) { return iUIControl::Type::Combobox; }

	// Constructor that takes a definition as parameter
	UIComboBox(void);

	// Initialises the control from a supplied control definition
	Result Initialise(UIManagedControlDefinition *def, string code, int expandsize, int x, int y, float z, int width, int height, bool render);

	// Inline methods to get aspects of the control positioning
	INTVECTOR2			GetPosition(void)			{ return m_location; }
	INTVECTOR2			GetSize(void)				{ return m_size; }
	float				GetZOrder(void)				{ return m_zorder; }
	bool				GetRenderActive(void) const	{ return m_render; }
	
	// Methods to set aspects of the control positioning
	void				SetPosition(INTVECTOR2 location);
	void				SetZOrder(float zorder);
	void				SetSize(INTVECTOR2 size);
	void				SetRenderActive(bool render);

	// Methods to retrieve different state parameters of the control
	CMPINLINE iUIControl::ControlState				GetState(void) { return m_state; }
	CMPINLINE UIComboBox::ExpandState				GetExpandState(void) { return m_expandstate; }

	// Primary method to update the control, including when positioning is changed.  Recalculates all component properties accordigly
	void				UpdateControl(bool fullrefresh);
	void				UpdateControl(INTVECTOR2 location, INTVECTOR2 size, float zorder, bool render, bool fullrefresh);

	// Updates the text contents of the control
	void				UpdateControlContents(void);

	// Updates the visibility of control components following a change to the render state or expand state
	void				UpdateControlVisibility(void);
		
	// Managed rendering methods for the control
	void				RenderControl(void);
	void				RenderControlInFocus(void);

	// Sets the expand state of the control
	void				SetExpandState(UIComboBox::ExpandState state);

	// Scrolls the item list in the specified direction
	void				ScrollItemList(iUIControl::ScrollState scrolldirection);

	// Methods to register and unregister a control from a 2D render group
	void				RegisterWithRenderGroup(Render2DGroup *group);
	void				UnregisterFromRenderGroup(void);

	// Determines whether the supplied point lies within the control bounds; part of the iUIControl interface
	bool				WithinControlBounds(INTVECTOR2 point);

	// Returns the number of items stored in this combo box
	CMPINLINE std::vector<ComboBoxItem>::size_type			GetItemCount(void) { return m_items.size(); }

	// Returns a reference to a specific item, or to the item collection itself
	CMPINLINE vector<ComboBoxItem>*	GetItems(void) { return &m_items; }
	CMPINLINE string				GetItem(std::vector<ComboBoxItem>::size_type index) const		
	{ 
		if (index >= m_items.size()) return ""; else return m_items[index].Value; 
	}
	CMPINLINE string				GetItemTag(std::vector<ComboBoxItem>::size_type index) const
	{
		if (index >= m_items.size()) return ""; else return m_items[index].Tag;
	}
	
	// Returns the currently-selected item in the combo box
	CMPINLINE std::vector<ComboBoxItem>::size_type	GetSelectedIndex(void) const	{ return m_selectedindex; }
	CMPINLINE string								GetSelectedItem(void) const		{ return GetItem(m_selectedindex); }
	CMPINLINE string								GetSelectedItemTag(void) const	{ return GetItemTag(m_selectedindex); }

	// Methods to add, modify or remove an item
	CMPINLINE void				AddItem(string item)							{ AddItem(item, ""); }
	void						AddItem(string item, string tag);
	CMPINLINE void				InsertItem(std::vector<ComboBoxItem>::size_type index, string item)					{ InsertItem(index, item, ""); }
	void						InsertItem(std::vector<ComboBoxItem>::size_type index, string item, string tag);
	void						SetItem(std::vector<ComboBoxItem>::size_type index, string item)					{ SetItem(index, item, ""); }
	void						SetItem(std::vector<ComboBoxItem>::size_type index, string item, string tag);
	void						RemoveItem(std::vector<ComboBoxItem>::size_type index);
	void						Clear(void);

	// Returns the index of the item currently shown at the top of the combo box expand list
	CMPINLINE std::vector<ComboBoxItem>::size_type	GetScrollPosition(void) { return m_scrollposition; }
	float											GetScrollPercentage(void); 

	// Selects an item from the expanded item list
	void											SelectItem(std::vector<ComboBoxItem>::size_type item);

	// Returns the current state of the control
	CMPINLINE iUIControl::ControlState				GetControlState(void) { return m_state; }
	
	// Returns the number of drop-down items that can be displayed in the expanded combo box
	CMPINLINE std::vector<TextBlock*>::size_type	GetExpandItemCount(void) { return m_expandtext.size(); }

	// Toggles the expand state of this control
	void				ToggleExpandState(void);

	// Methods to hold and release updates to the layout of this control, e.g. when performing mass updates
	CMPINLINE void		SuspendControlLayoutUpdates(void) { m_suspendupdates = true; }
	CMPINLINE void		ReleaseHoldOnControlUpdates(void) { m_suspendupdates = false; UpdateControl(true); }

	// Flag determining whether this control can accept user input focus
	CMPINLINE bool								CanAcceptFocus(void) { return m_canacceptfocus; }	
	CMPINLINE void								SetCanAcceptFocus(bool b) { m_canacceptfocus = b; }	

	// Returns a collection of iUIComponents that make up the control
	CMPINLINE iUIControl::ControlComponentCollection *	GetComponents(void) { return &m_components; }

	// Flag determining whether this control can accept keyboard input when it is in focus
	CMPINLINE bool								CanAcceptKeyboardInput(void) { return true; }
	void										ProcessKeyboardInput(GameInputDevice *keyboard);

	// Methods to handle user input to the control
	void										HandleMouseHoverEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);
	void										HandleMouseDownEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);
	void										HandleRightMouseDownEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);
	void										HandleMouseUpEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);
	void										HandleRightMouseUpEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);

	// Methods to handle mouse clicks on the components making up this control
	void HandleMouseClickEvent(Image2DRenderGroup *componentgroup, Image2DRenderGroup::Instance *component,
							   INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation);
	void HandleMouseRightClickEvent(Image2DRenderGroup *componentgroup, Image2DRenderGroup::Instance *component,
									INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation);

	// Shutdown method to satisfy the interface requirement (only basic components, e.g. Image2Ds & text, need to be disposed of)
	void Shutdown(void) { }

	~UIComboBox(void);


private:

	// Size & position properties for this control
	INTVECTOR2													m_location;
	INTVECTOR2													m_size;
	float														m_zorder;
	
	// Render group that this control is registered with
	Render2DGroup *												m_rendergroup;

	// Vector storing the contents of this combobox
	vector<ComboBoxItem>										m_items;

	// Currently selected index within the combobox
	std::vector<ComboBoxItem>::size_type						m_selectedindex;

	// Index of the current scroll position within this combobox
	std::vector<ComboBoxItem>::size_type						m_scrollposition;

	// Flag that can be set to temporarily disable updates to the control, for example when performing a mass update
	bool														m_suspendupdates;

	// The components making up this control in default view, i.e. when not expanded
	Image2DRenderGroup *										m_mainback;
	TextBlock *													m_maintext;
	Image2DRenderGroup *										m_expand;
	
	// The additional controls shown when the control is expanded (but no scrolling required)
	Image2DRenderGroup *										m_expandback;
	vector<TextBlock*>											m_expandtext;

	// The additional controls shown when control is expanded and also requires scrolling to view all items
	Image2DRenderGroup *										m_scrollbarback;
	Image2DRenderGroup *										m_scrollbarupdown;
	Image2DRenderGroup *										m_scrollbarhandle;

	// Collection in which to store the component pointers, for more efficient parsing of all constituent components
	vector<iUIComponent*>										m_components;

	// Current state of the control (part of the iUIControl interface)
	iUIControl::ControlState									m_state;

	// Class-specific state variables for the control
	UIComboBox::ExpandState										m_expandstate;

	// Records the last time the control scrolled up or down; used to maintain a smooth scrolling speed across varying FPS
	clock_t														m_lastscrolltime;

	// The height of each item in the expanded list
	int															m_expanditemheight;
	
	// Determines whether this control can accept focus
	bool														m_canacceptfocus;

};


#endif