#include <math.h>
#include <time.h>
#include "DX11_Core.h"

#include "GameDataExtern.h"
#include "CoreEngine.h"
#include "UserInterface.h"
#include "Render2DGroup.h"
#include "Texture.h"
#include "UIManagedControlDefinition.h"
#include "Image2D.h"
#include "TextBlock.h"

#include "UIComboBox.h"

// Constant values
const int UIComboBox::DEFAULT_TEXT_CAPACITY = 256;
const int UIComboBox::DEFAULT_EXPAND_SIZE = 5;
const int UIComboBox::EXPAND_MIN_SIZE = 3;
const int UIComboBox::EXPAND_MAX_SIZE = 256;
const float UIComboBox::CONTROL_ACTIVE_AREA = 0.85f;
const int UIComboBox::TEXT_OFFSET_X = 16;
const int UIComboBox::TEXT_OFFSET_Y = 4;
const int UIComboBox::TEXT_SEPARATION = 4;
const int UIComboBox::SCROLL_HANDLE_MIN_SIZE = 5;
const float UIComboBox::SCROLL_INTERVAL = 0.5f;

UIComboBox::UIComboBox(void)
{
	// Set default positioning properties
	m_location = INTVECTOR2(0, 0);
	m_size = INTVECTOR2(10, 10);
	m_zorder = 0.0f;
	m_render = true;
	
	// Set default contents for this control
	m_rendergroup = NULL;
	m_items.clear();
	m_selectedindex = 0;
	m_scrollposition = 0;
	m_state = iUIControl::ControlState::Default;
	m_expandstate = UIComboBox::ExpandState::Unknown;
	m_lastscrolltime = 0;
	m_expanditemheight = 16;
	m_canacceptfocus = true;
	m_suspendupdates = false;

	// Initialise pointers to null
	m_mainback = NULL;
	m_maintext = NULL;
	m_expand = NULL;
	m_expandback = NULL;
	m_expandtext.clear();
	m_scrollbarback = NULL;
	m_scrollbarup = NULL;
	m_scrollbardown = NULL;
	m_scrollbarhandle = NULL;
}

Result UIComboBox::Initialise(UIManagedControlDefinition *def, std::string code, int expandsize, int x, int y, float z, int width, int height, bool render)
{
	// Make sure we have valid parameters
	if (!def) return ErrorCodes::CannotCreateControlFromInvalidDefinition;
	expandsize = min(max(expandsize, UIComboBox::EXPAND_MIN_SIZE), UIComboBox::EXPAND_MAX_SIZE);

	// Make sure the definition is complete
	if ( (def->HaveComponent("mainback") && def->HaveComponent("expand") &&
		  def->HaveComponent("expandback") && def->HaveComponent("scrollbarback") && 
		  def->HaveComponent("scrollbarupdown") && def->HaveComponent("scrollbarhandle")) == false)
				return ErrorCodes::CannotCreateControlFromInvalidDefinition;

	// Store the new control code; this is mandatory for creating a combobox based on a definition
	m_code = code;

	// Create each image component in turn, based on the definition
	m_mainback = InitialiseImageComponentDefault(def, "mainback", z);
	m_expand = InitialiseImageComponentDefault(def, "expand", z);
	m_expandback = InitialiseImageComponentDefault(def, "expandback", z);
	m_scrollbarback = InitialiseImageComponentDefault(def, "scrollbarback", z);
	m_scrollbarup = InitialiseImageComponentDefault(def, "scrollbarupdown", z);
	m_scrollbardown = InitialiseImageComponentDefault(def, "scrollbarupdown", z);
	m_scrollbarhandle = InitialiseImageComponentDefault(def, "scrollbarhandle", z);

	// There are two components for the scroll bar up/down; rotate one by 180 degrees so they are aligned correctly
	m_scrollbardown->SetRotation(PI);

	// Now create the main text block
	m_maintext = D::UI->CreateTextBlock(concat(m_code)(".maintext").str(), "", 256, 0, INTVECTOR2(0,0), 1.0f, XMFLOAT4(1.0, 1.0f, 1.0f, 1.0f), true);
	if (!m_maintext) return ErrorCodes::CouldNotCreateTextComponentForManagedControl;

	// Create text blocks for each of the entries potentially visible in the expanded drop-down 
	for (int i=0; i<expandsize; i++)
	{
		// Create a new text block; note that all start with rendering disabled
		TextBlock *t = D::UI->CreateTextBlock(	concat(m_code)(".expandtext.")(i).str(), "", UIComboBox::DEFAULT_TEXT_CAPACITY, 0, 
												INTVECTOR2(0, 0), 1.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), false);

		// Make sure we could create the text block
		if (!t) return ErrorCodes::CouldNotCreateTextComponentForManagedControl;

		// Add to the expand text collection
		m_expandtext.push_back(t);
	}
	
	// Store pointers to each component in the control components collection for ease of traversal
	m_components.push_back(m_mainback);
	m_components.push_back(m_maintext);
	m_components.push_back(m_expand);
	m_components.push_back(m_expandback);
	m_components.push_back(m_scrollbarback);
	m_components.push_back(m_scrollbarup);
	m_components.push_back(m_scrollbardown);
	m_components.push_back(m_scrollbarhandle);

	for (int i = 0; i < (int)m_expandtext.size(); i++)
	{
		m_components.push_back(m_expandtext.at(i));
	}

	// Now set the location & size of this control, which will cause a recalculation of all component properties and set up the control
	UpdateControl(INTVECTOR2(x, y), INTVECTOR2(width, height), z, render, true);

	// Combo box controls will always begin contracted by default
	SetExpandState(UIComboBox::ExpandState::Contracted);

	// Return success
	return ErrorCodes::NoError;
}

// Performs a complete update of the control layout 
void UIComboBox::UpdateControl(bool fullrefresh)
{
	// Simply call the overloaded function with the current positioning parameters to force a refresh
	UpdateControl(m_location, m_size, m_zorder, m_render, fullrefresh);
}

// Sets the size & position of this control, and recalculates all component properties accordingly
void UIComboBox::UpdateControl(INTVECTOR2 location, INTVECTOR2 size, float zorder, bool render, bool fullrefresh)
{
	INTVECTOR2 textpos = INTVECTOR2(0, 0);

	// Store the new positioning parameters
	m_location = location;
	m_size = size;
	m_zorder = zorder;
	m_render = render;

	// Retrieve key values used in rendering this control
	std::vector<TextBlock*>::size_type expandcount = GetExpandItemCount();
	std::vector<ComboBoxItem>::size_type itemcount = GetItemCount();

	// Set position of main component back panel first
	if (m_mainback && fullrefresh) {
		m_mainback->SetPosition(location.ToFloat());
		m_mainback->SetSize(INTVECTOR2((int)floor((float)size.x * UIComboBox::CONTROL_ACTIVE_AREA), size.y).ToFloat());
		m_mainback->SetZOrder(zorder);

		// Main text block is displayed within this back panel
		m_maintext->UpdateTextBlock( m_maintext->GetText().c_str(), location.x + UIComboBox::TEXT_OFFSET_X, 
									 (int)(location.y + ((m_mainback->GetSize().y / 2.0f) - (m_maintext->GetTextHeight() / 2.0f)) + 2.0f),
									 m_maintext->GetRenderActive(),
									 m_maintext->GetTextColour(), m_maintext->GetSize());
	}

	// Expand button is set just to the side of the main back panel
	if (m_expand && m_mainback && fullrefresh) {
		m_expand->SetPosition(INTVECTOR2(location.x + (int)m_mainback->GetSize().x, location.y).ToFloat());
		m_expand->SetSize(INTVECTOR2(size.x - (int)m_mainback->GetSize().x, size.y).ToFloat());
		m_expand->SetZOrder(zorder);
	}

	// We now want to create the expand panel and text items; we can only do this if the prior components were successfully created
	if (m_mainback && m_expand && fullrefresh)
	{
		// Position the expanded text blocks before the frame itself, to determine how much room is required.  Determine positioning first
		textpos = INTVECTOR2(location.x + UIComboBox::TEXT_OFFSET_X, location.y + (int)m_mainback->GetSize().y + UIComboBox::TEXT_OFFSET_Y);
		
		// Calculate a height value for each item, based on the rasterised height of item #0
		m_expanditemheight = 16;
		if (m_expandtext.size() > 0) m_expanditemheight = ((int)m_expandtext[0]->GetTextHeight() + (2 * UIComboBox::TEXT_SEPARATION));

		// Now position each text item in the drop-down list in turn
		for (std::vector<TextBlock*>::size_type i = 0; i < expandcount; ++i)
		{
			// Update the position of this text block
			TextBlock *tb = m_expandtext.at(i);
			if (!tb) continue;
			tb->UpdateTextBlock(tb->GetText().c_str(), textpos.x, UIComboBox::TEXT_SEPARATION + textpos.y, 
								tb->GetRenderActive(), tb->GetTextColour(), tb->GetSize());

			// Update the position of the next text block to be offset below this one
			textpos = INTVECTOR2(textpos.x, textpos.y + m_expanditemheight);
		}
	}

	// We can now position the expand back panel; height can be derived from the main panel size and the rasterised height of text items
	if (m_expandback && m_mainback && fullrefresh) {
		m_expandback->SetPosition(INTVECTOR2(location.x, location.y + (int)m_mainback->GetSize().y).ToFloat());
		m_expandback->SetSize(XMFLOAT2(m_mainback->GetSize().x, (float)textpos.y - ((float)location.y + m_mainback->GetSize().y)));
		m_expandback->SetZOrder(zorder);
	}

	// The scroll bar back panel is positioned alongside the expand panel, with same width as the expand button
	if (m_scrollbarback && m_expand && m_expandback && fullrefresh) {
		m_scrollbarback->SetPosition(INTVECTOR2((int)m_expand->GetPosition().x, location.y + (int)m_expand->GetSize().y).ToFloat());
		m_scrollbarback->SetSize(XMFLOAT2(m_expand->GetSize().x, m_expandback->GetSize().y));
		m_scrollbarback->SetZOrder(zorder);
	}

	// Scroll bar buttons are positioned at each end of the scroll bar panel, and are square in size
	if (m_scrollbarup && m_scrollbardown && m_scrollbarback && fullrefresh) 
	{
		XMFLOAT2 size(m_scrollbarback->GetSize().x, m_scrollbarback->GetSize().x);
		m_scrollbarup->SetSize(size);
		m_scrollbardown->SetSize(size);
		m_scrollbarup->SetPosition(m_scrollbarback->GetPosition());
		m_scrollbardown->SetPosition(XMFLOAT2(m_scrollbarback->GetPosition().x, m_scrollbarback->GetPosition().y + (m_scrollbarback->GetSize().y - m_scrollbardown->GetSize().y)));
		m_scrollbarup->SetZOrder(zorder);
		m_scrollbardown->SetZOrder(zorder);

		// Reduce the size of the scroll bar back panel so that it does not overlap the up/down buttons.  Also move it back in the z order
		m_scrollbarback->Move(XMFLOAT2(0.0f, m_scrollbarup->GetSize().y));
		m_scrollbarback->Resize(XMFLOAT2(0.0f, -(m_scrollbarup->GetSize().y + m_scrollbardown->GetSize().y)));
		m_scrollbarback->AdjustZOrder(+0.001f);
	}

	// Scroll bar handle is positioned at the % equivalent to selection in the list, and has (bounded) size 
	// relative to #items vs #displayed.  This is part of the lite-refresh (i.e. fullrefresh == false)
	if (m_scrollbarhandle && m_scrollbarback && m_scrollbarup && m_scrollbardown) 
	{
		// Calculate the active area of scroll bar (i.e. the extent between the two control buttons)
		float scrollbararea = (m_scrollbarback->GetSize().y);

		// Determine the scroll handle height
		float handlesize;
		if (itemcount == 0 || itemcount <= expandcount)		
			handlesize = scrollbararea;
		else
			handlesize = (scrollbararea * (max(min(((float)expandcount / (float)itemcount), 1.0f), 0.0f)));

		// Set the scroll handle size and position
		m_scrollbarhandle->SetSize(XMFLOAT2(m_scrollbarback->GetSize().x, handlesize));
		m_scrollbarhandle->SetZOrder(zorder);
		m_scrollbarhandle->SetPosition(XMFLOAT2(m_scrollbarback->GetPosition().x,
											m_scrollbarback->GetPosition().y + 
											((float)m_scrollposition / (float)itemcount) * scrollbararea));

		// Perform bounds checking on the handle size and adjust its position if necessary
		if ((m_scrollbarhandle->GetPosition().y + m_scrollbarhandle->GetSize().y) > (m_scrollbarback->GetPosition().y + scrollbararea))
		{
			m_scrollbarhandle->Move(XMFLOAT2(0.0f, m_scrollbarback->GetPosition().y + scrollbararea - m_scrollbarhandle->GetSize().y));
		}
	}

	// Now update both control render state and control contents
	UpdateControlVisibility();
	UpdateControlContents();
}

// Sets the expand state of the control
void UIComboBox::SetExpandState(UIComboBox::ExpandState state)
{
	// Store the new expand state
	m_expandstate = state;

	// Update component visibility based on this new state
	UpdateControlVisibility();
}

// Updates the text contents of the control
void UIComboBox::UpdateControlContents(void)
{
	// Make sure the currently-selected index within the control is valid; if not (e.g. items have been removed) then reset it
	if (m_selectedindex < 0) m_selectedindex = 0;
	if (m_selectedindex >= (int)m_items.size()) m_selectedindex = max(0, (int)m_items.size()-1);

	// Make sure the current scroll position within the control is accurate; if not, reset it
	if (m_scrollposition < 0) m_scrollposition = 0;
	if (m_scrollposition >= (int)(m_items.size() - m_expandtext.size())) m_scrollposition = max(0, (int)(m_items.size()-m_expandtext.size()));

	// Update the main text to reflect the currently-selected item
	m_maintext->SetText(GetItem(m_selectedindex));

	// Now update each of the drop-down items with the relevant item (GetItem logic will deal with null items etc)
	for (int i=0; i<(int)m_expandtext.size(); i++)
		m_expandtext[i]->SetText(GetItem(m_scrollposition + i));
}

// Updates the visibility of control components following a change to the render state or expand state
void UIComboBox::UpdateControlVisibility(void)
{
	// Default components will simply take the render state; expanded components need to also consider the expand state
	bool expandrender = (m_render && m_expandstate == UIComboBox::ExpandState::Expanded);

	// Scroll-related components also take into account the items & capacity of the control
	bool scrollrender = (expandrender && (m_items.size() > m_expandtext.size()));

	// Set the render state of each component accordingly; first, the default components
	if (m_mainback) m_mainback->SetRenderActive(m_render);
	if (m_expand) m_expand->SetRenderActive(m_render);
	if (m_maintext) m_maintext->SetRenderActive(m_render);

	// Now the expanded components
	if (m_expandback) m_expandback->SetRenderActive(expandrender);
	if (m_scrollbarback) m_scrollbarback->SetRenderActive(scrollrender);
	if (m_scrollbarup) m_scrollbarup->SetRenderActive(scrollrender);
	if (m_scrollbardown) m_scrollbardown->SetRenderActive(scrollrender);
	if (m_scrollbarhandle) m_scrollbarhandle->SetRenderActive(scrollrender);

	// Iterate over each item in the expanded list and apply the same render state
	int n = (int)m_expandtext.size();
	for (int i = 0; i < n; ++i)
	{
		if (m_expandtext[i]) m_expandtext[i]->SetRenderActive(expandrender);
	}
}

void UIComboBox::RenderControl(void) 
{
	// There is no additional rendering required for comboboxes that are out of focus; will not even be called by the UI manager
}

void UIComboBox::RenderControlInFocus(void)
{	
	// There is no additional rendering required for comboboxes that are in focus;  will not even be called by the UI manager
}

// Method to register a control from a 2D render group
void UIComboBox::RegisterWithRenderGroup(Render2DGroup *group)
{
	// Make sure this is a valid render group. If it is, store a reference to it
	if (!group) return;
	m_rendergroup = group;

	// Register the control itself
	group->Components.ComboBoxes.AddItem(m_code, this);

	// Now add each image component to the render group in turn
	group->Components.Image2D.AddItem(m_mainback->GetCode(), m_mainback);
	group->Components.Image2D.AddItem(m_expand->GetCode(), m_expand);
	group->Components.Image2D.AddItem(m_expandback->GetCode(), m_expandback);
	group->Components.Image2D.AddItem(m_scrollbarback->GetCode(), m_scrollbarback);
	group->Components.Image2D.AddItem(m_scrollbarup->GetCode(), m_scrollbarup);
	group->Components.Image2D.AddItem(m_scrollbardown->GetCode(), m_scrollbardown);
	group->Components.Image2D.AddItem(m_scrollbarhandle->GetCode(), m_scrollbarhandle);
	
	// Add each image component to the group render queue
	group->RegisterRenderableComponent(m_mainback);
	group->RegisterRenderableComponent(m_expand);
	group->RegisterRenderableComponent(m_expandback);
	group->RegisterRenderableComponent(m_scrollbarback);
	group->RegisterRenderableComponent(m_scrollbarup);
	group->RegisterRenderableComponent(m_scrollbardown);
	group->RegisterRenderableComponent(m_scrollbarhandle);
	
	// Now add each of the control text components in turn
	group->Components.TextBlocks.AddItem(m_maintext->GetCode(), m_maintext);
	for (int i=0; i<(int)m_expandtext.size(); i++)
		group->Components.TextBlocks.AddItem(m_expandtext[i]->GetCode(), m_expandtext[i]);	
}

// Method to unregister a control from a 2D render group
void UIComboBox::UnregisterFromRenderGroup(void)
{
	// Make sure we are actually assigned to a render group; if not, do nothing
	if (!m_rendergroup) return;

	// Remve the control itself
	m_rendergroup->Components.ComboBoxes.RemoveItem(m_code);

	// First remove each image component from the group render queue
	m_rendergroup->UnregisterRenderableComponent(m_mainback);
	m_rendergroup->UnregisterRenderableComponent(m_expand);
	m_rendergroup->UnregisterRenderableComponent(m_expandback);
	m_rendergroup->UnregisterRenderableComponent(m_scrollbarback);
	m_rendergroup->UnregisterRenderableComponent(m_scrollbarup);
	m_rendergroup->UnregisterRenderableComponent(m_scrollbardown);
	m_rendergroup->UnregisterRenderableComponent(m_scrollbarhandle);

	// Now remove the image components themselves from the group
	m_rendergroup->Components.Image2D.RemoveItem(m_mainback->GetCode());
	m_rendergroup->Components.Image2D.RemoveItem(m_expand->GetCode());
	m_rendergroup->Components.Image2D.RemoveItem(m_expandback->GetCode());
	m_rendergroup->Components.Image2D.RemoveItem(m_scrollbarback->GetCode());
	m_rendergroup->Components.Image2D.RemoveItem(m_scrollbarup->GetCode());
	m_rendergroup->Components.Image2D.RemoveItem(m_scrollbardown->GetCode());
	m_rendergroup->Components.Image2D.RemoveItem(m_scrollbarhandle->GetCode());

	// Now remove all text components from the group
	m_rendergroup->Components.TextBlocks.RemoveItem(m_maintext->GetCode());
	for (int i=0; i<(int)m_expandtext.size(); i++)
		m_rendergroup->Components.TextBlocks.RemoveItem(m_expandtext[i]->GetCode());

	// Remove our link to the render group 
	m_rendergroup = NULL;
}


// Returns the current scroll percentage for the expanded list of items
float UIComboBox::GetScrollPercentage(void)
{
	std::vector<ComboBoxItem>::size_type itemcount = GetItemCount();
	std::vector<TextBlock*>::size_type expandcount = GetExpandItemCount();

	// If there are not enough items for scrolling to be relevant then return 0% as a default
	if (itemcount <= expandcount) return 0.0f;

	// Otherwise calculate the relevant scroll percentage through the list
	return max(0.0f, min(1.0f, ( (float)m_scrollposition / (float)(itemcount - (expandcount - 1)) ) ));
}

// Methods to set aspects of the control positioning
void UIComboBox::SetPosition(INTVECTOR2 location)	{ UpdateControl(location, m_size, m_zorder, m_render, true); }
void UIComboBox::SetSize(INTVECTOR2 size)			{ UpdateControl(m_location, size, m_zorder, m_render, true); }
void UIComboBox::SetZOrder(float zorder)			{ UpdateControl(m_location, m_size, zorder, m_render, true); }
void UIComboBox::SetRenderActive(bool render)		{ UpdateControl(m_location, m_size, m_zorder, render, true); }


// Method to handle user keyboard input to the control
void UIComboBox::ProcessKeyboardInput(GameInputDevice *keyboard)
{
}

// Adds a new item to the control
void UIComboBox::AddItem(std::string item, std::string tag)
{
	// Add a new item
	m_items.push_back(ComboBoxItem(item, tag));

	// Perform an update of the control, as long as we are not holding the refresh as part of a mass update
	if (!m_suspendupdates) UpdateControl(true);
}

// Inserts a new item to the control at the specified index
void UIComboBox::InsertItem(std::vector<ComboBoxItem>::size_type index, std::string item, std::string tag)
{
	// If the index provided is not valid then simply push onto the items vector
	if (index >= m_items.size() || m_items.size() == 0) 
		m_items.push_back(ComboBoxItem(item, tag));
	else
	{
		// We know the collection has at least one item, so can apply the same method to all cases.  
		// First, add a new item at the end of the vector
		m_items.push_back(ComboBoxItem());

		// Now move the post-index vector contents across by one element, to overwrite the new element
		memmove(&(m_items[index+1]), &(m_items[index]), sizeof(ComboBoxItem) * (m_items.size() - index - 1));

		// Now set the value of the element at index
		m_items[index] = ComboBoxItem(item, tag);

		// If our currently-selected item is after this index, adjust the selected item to compensate for the indices moving by one
		if (m_selectedindex >= index) m_selectedindex++;
	}
		
	// Perform an update of the control, as long as we are not holding the refresh as part of a mass update
	if (!m_suspendupdates) UpdateControl(true);
		
}

void UIComboBox::SetItem(std::vector<ComboBoxItem>::size_type index, std::string item, std::string tag)
{
	// Make sure the index is valid
	if (index >= m_items.size()) return;

	// Set the item to the new supplied value
	m_items[index] = ComboBoxItem(item, tag);

	// Perform an update of the control, as long as we are not holding the refresh as part of a mass update
	if (!m_suspendupdates) UpdateControl(true);
}

void UIComboBox::RemoveItem(std::vector<ComboBoxItem>::size_type index)
{
	// Make sure the index is valid
	std::vector<ComboBoxItem>::size_type size = m_items.size();
	if (index >= size) return;

	// If we are attempting to remove the last item then simply pop it from the end of the vector now
	if (index == (size - 1)) 
	{ 
		m_items.pop_back(); 
	}
	else
	{
		// Otherwise we want to copy the extent of vector past 'index' back by one element, to overwrite the removed item
		memmove(&(m_items[index]), &(m_items[index + 1]), sizeof(ComboBoxItem) * (size - index - 1));

		// Remove the item at the end of the vector which is now redundant
		m_items.pop_back();

		// If our currently-selected item is after this index, adjust the selected item to compensate for the indices moving by one
		if (m_selectedindex >= index) --m_selectedindex;
	}

	// Perform an update of the control, as long as we are not holding the refresh as part of a mass update
	if (!m_suspendupdates) UpdateControl(true);
}

// Clear all items from the combobox
void UIComboBox::Clear(void)
{
	// Simply clear the item vector and refresh the control
	m_items.clear();
	if (!m_suspendupdates) UpdateControl(true);
}

// Scrolls the item list in the specified direction
void UIComboBox::ScrollItemList(iUIControl::ScrollState scrolldirection)
{
	// Make sure we are not still within the scroll interval
	clock_t t = clock();
	if (t - m_lastscrolltime < UIComboBox::SCROLL_INTERVAL) return;

	// Store the new scroll time
	m_lastscrolltime = t;

	// Now take action depending on scroll direction
	if (scrolldirection == iUIControl::ScrollState::ScrollUp)
		m_scrollposition--;
	else
		m_scrollposition++;

	// Update the control layout; this will also perform validation of the new scroll position
	UpdateControl(false);
}

// Selects an item from the expanded item list
void UIComboBox::SelectItem(std::vector<ComboBoxItem>::size_type item)
{
	// Make sure a valid item has been specified, based on the expand list capacity
	if (item >= m_expandtext.size()) return;

	// Determine the actual item selected based on current scroll position, and again determine whether this is a valid index
	std::vector<ComboBoxItem>::size_type index = (item + m_scrollposition);
	if (index >= m_items.size()) return;

	// Change the selected item and close the drop-down list
	std::vector<ComboBoxItem>::size_type previousindex = m_selectedindex;
	m_selectedindex = index;
	m_expandstate = UIComboBox::ExpandState::Contracted;
	
	// Request a full update of the control so that these changes are reflected
	UpdateControl(true);

	// Raise an event to signal that the selected item has been changed
	D::UI->ComboBox_SelectedIndexChanged(this, (int)m_selectedindex, (int)previousindex);
}

// Process mouse down events on the control	
void UIComboBox::HandleMouseDownEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
	if (!component) return;

	// Test which component we have the mouse over
	if (component == m_scrollbarup)
	{
		ScrollItemList(iUIControl::ScrollState::ScrollUp);
	}
	else if (component == m_scrollbardown)
	{
		ScrollItemList(iUIControl::ScrollState::ScrollDown);
	}

}

// Methods to handle user mouse input to the control
void UIComboBox::HandleMouseHoverEvent(iUIComponent *component, INTVECTOR2 mouselocation) { }
void UIComboBox::HandleRightMouseDownEvent(iUIComponent *component, INTVECTOR2 mouselocation) { }
void UIComboBox::HandleMouseUpEvent(iUIComponent *component, INTVECTOR2 mouselocation) { }
void UIComboBox::HandleRightMouseUpEvent(iUIComponent *component, INTVECTOR2 mouselocation) { }

// Event handler for left mouse clicks on the components making up this control
void UIComboBox::HandleMouseClickEvent(iUIComponent *component, INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation)
{
	if (!component) return;

	// Test which part of the control has been clicked
	if (component == m_expand)
	{
		// If the user has clicked on the expand button then we want to toggle expand state
		ToggleExpandState();
	}
	else if (component == m_expandback)
	{
		// If the user has clicked within the expanded list then work out which item has been selected
		int y = (mouselocation.y - (int)m_expandback->GetPosition().y) - UIComboBox::TEXT_OFFSET_Y;
		int item = (int)floor((float)y / (float)(m_expanditemheight));

		// Change the selection to this new item
		SelectItem(item);
	}
}

// Event handler for right mouse clicks on the components making up this control
void UIComboBox::HandleMouseRightClickEvent(iUIComponent *component, INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation)
{
}

// Toggles the expand state of this control
void UIComboBox::ToggleExpandState(void)
{
	// Take action depending on the current state
	if (m_expandstate == UIComboBox::ExpandState::Contracted)
	{
		// If the control is currently in contracted state then expand it now
		SetExpandState(UIComboBox::ExpandState::Expanded);
	}
	else if (m_expandstate == UIComboBox::ExpandState::Expanded)
	{
		// If the control is currently in expanded state then contract it now
		SetExpandState(UIComboBox::ExpandState::Contracted);
	}
	else
	{
		// If the control is in an unknown or invalid state then return it to default contracted layout
		SetExpandState(UIComboBox::ExpandState::Contracted);
	}
}

// Determines whether the supplied point lies within the control bounds; part of the iUIControl interface
bool UIComboBox::WithinControlBounds(INTVECTOR2 point)
{
	return (point.x >= m_location.x && point.y >= m_location.y && 
			point.x <= (m_location.x + m_size.x) && 
			point.y <= (m_location.y + m_size.y) );
}


UIComboBox::~UIComboBox(void)
{
}
