#include <vector>
#include "DX11_Core.h"

#include "Utility.h"
#include "FastMath.h"
#include "GameVarsExtern.h"
#include "GameInput.h"
#include "CoreEngine.h"
#include "RenderComponentGroup.h"
#include "RenderMouseEvent.h"
#include "iUIComponentRenderable.h"
#include "TextManager.h"
#include "UIButton.h"
#include "UITextBox.h"
#include "UIComboBox.h"
#include "TextBlock.h"
#include "MultiLineTextBlock.h"
#include "Render2DGroup.h"
#include "TextureShader.h"

Render2DGroup::Render2DGroup(void)
{
	m_rendermanager = NULL;
	m_render = true;

	// Link render groups to the relevant item collections
	Components.Image2D.LinkToCollection(&m_components);
	Components.Image2DGroup.LinkToCollection(&m_componentgroups);
	Components.MouseEvents.LinkToCollection(&m_mouseevents);
	Components.TextBlocks.LinkToCollection(&m_textblocks);
	Components.MultiLineTextBlocks.LinkToCollection(&m_multilinetextblocks);
	Components.Buttons.LinkToCollection(&m_buttons);
	Components.TextBoxes.LinkToCollection(&m_textboxes);
	Components.ComboBoxes.LinkToCollection(&m_comboboxes);

	Components.RenderConstants.LinkToCollection(&m_constants);
	Components.ComponentGroups.LinkToCollection(&m_groups);
}

// Executes the control-specific render method for each managed control in the render group, i.e. any more
// complex rendering not already handled by the Render2DManager etc.
void Render2DGroup::PerformManagedControlRendering(iUIControl *focuscontrol)
{
	// Execute rendering for each control class in turn, if required
	if (UIButton::PerformsManagedRendering())
		PerformManagedControlRenderingOnControlClass(focuscontrol, (RenderComponentGroup<iUIControl*>*)&( Components.Buttons ) );

	if (UITextBox::PerformsManagedRendering())
		PerformManagedControlRenderingOnControlClass(focuscontrol, (RenderComponentGroup<iUIControl*>*)&( Components.TextBoxes ) );

	if (UIComboBox::PerformsManagedRendering())
		PerformManagedControlRenderingOnControlClass(focuscontrol, (RenderComponentGroup<iUIControl*>*)&( Components.ComboBoxes ) );

	/* .... */

	// Finally, execute managed rendering on the control currently in focus
	if (focuscontrol) focuscontrol->RenderControlInFocus();
}

// Performs managed control rendering for a specific class of iUIControl-derived control
void Render2DGroup::PerformManagedControlRenderingOnControlClass(iUIControl *focuscontrol, RenderComponentGroup<iUIControl*> *controlgroup)
{
	RenderComponentGroup<iUIControl*>::ItemCollection::const_iterator it_end = controlgroup->Items()->end();
	for (RenderComponentGroup<iUIControl*>::ItemCollection::const_iterator it = controlgroup->Items()->begin(); it != it_end; ++it)
	{
		// Call the control rendering function, as long as it is not the control in focus
		if (it->second && it->second != focuscontrol) it->second->RenderControl();
	}
}


// Searches the render group for the item identified by 'code' (and potentially indexed by 'key'), 
// across all iUIComponent-derived classes
iUIComponent *Render2DGroup::FindUIComponent(string code, string key)
{
	iUIComponent *item;

	// Try each component group in turn, in the order of likelihood that it will be the relevant group (for efficiency)
	// First of all, try the I2D render groups which are a special case because they are also indexed by a string key per instance
	Image2DRenderGroup *group = Components.Image2DGroup.GetItem(code);
	if (group && key != NullString) {
		item = group->GetInstanceByCode(key);
		if (item) return item;
	}

	// Now try each of the other simple (non-indexed groups) in turn, again in order of likelihood
	item = (iUIComponent*)Components.Buttons.GetItem(code);				if (item) return item;
	item = (iUIComponent*)Components.TextBoxes.GetItem(code);			if (item) return item;
	item = (iUIComponent*)Components.ComboBoxes.GetItem(code);			if (item) return item;
	item = (iUIComponent*)Components.TextBlocks.GetItem(code);			if (item) return item;
	item = (iUIComponent*)Components.MultiLineTextBlocks.GetItem(code);	if (item) return item;
	item = (iUIComponent*)Components.ComponentGroups.GetItem(code);		if (item) return item;
	item = (iUIComponent*)Components.Image2D.GetItem(code);				if (item) return item;
	item = (iUIComponent*)Components.MouseEvents.GetItem(code);			if (item) return item;

	// We could not locate the item in any collection, so return NULL and quit
	return NULL;
}

// Renders the render group by processing its queue of renderable components in sequence
void Render2DGroup::Render(D3DXMATRIX baseviewmatrix)
{
	D3DXMATRIX baseworld, ortho;
	TextureShader *tshader; 
	iUIComponentRenderable *component;
	ID3D11DeviceContext *devicecontext;

	// Get a reference to the device context, texture shader & matrices _we will use for rendering
	devicecontext = Game::Engine->GetDeviceContext();
	tshader = Game::Engine->GetTextureShader();
	if (!devicecontext || !tshader) return;

	// Get the matrices we will use for rendering
	Game::Engine->GetRenderOrthographicMatrix(ortho);
	baseworld = ID_MATRIX;
	
	// Now process each item registered for rendering in turn
	RenderQueueCollection::const_iterator it_end = m_renderqueue.end();
	for (RenderQueueCollection::const_iterator it = m_renderqueue.begin(); it != it_end; ++it)
	{
		// Get a reference to this component and make sure it should be rendered this frame
		component = (*it);
		if (!component || !component->GetRenderActive()) continue;

		// Render the component vertex buffer to the pipeline
		component->Render();

		// Now render these vertices using the engine texture shader
		tshader->Render(devicecontext, component->GetIndexCount(), baseworld, baseviewmatrix, ortho, component->GetTexture());
	}
}


// Processes the current user input state and triggers any required events
void Render2DGroup::ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse)
{
	RenderMouseEvent *ev;
	int x, y;
	RenderMouseEvent::MOUSE_EVENT_TYPE state;
	BOOL *buttons;
	BOOL lmb;

	// Get mouse position data
	x = (int)mouse->GetX();
	y = (int)mouse->GetY();
	buttons = mouse->GetButtons();
	lmb = buttons[0];

	// Process all defined mouse events
	MouseEventCollection::const_iterator it_end = m_mouseevents.end();
	for (MouseEventCollection::const_iterator it = m_mouseevents.begin(); it != it_end; ++it)
	{
		// Skip this event if it is not set to be active
		if (!it->second->GetRenderActive()) continue;

		// Get mouse event data
		ev = it->second;
		state = ev->GetEventState();
		int ex = ev->GetX();
		int ey = ev->GetY();
		
		// Check whether the mouse pointer is within this mouse event's region
		if (x >= ex && y >= ey && x <= (ex+ev->GetWidth()) && y <= (ey+ev->GetHeight()))
		{
			// Test whether the mouse button is depressed
			if (lmb) {
				// If the left mouse button is depressed then activate the mousedown event
				if (state != RenderMouseEvent::MOUSE_EVENT_TYPE::Down) 
				{
					// Activate the mouse down event, and also broadcast this click message back to the UI
					ev->ActivateMouseDownEvent();
				}
			} else {
				// If no mouse button is pressed then activate the hover event
				if (state != RenderMouseEvent::MOUSE_EVENT_TYPE::Hover)
					ev->ActivateMouseHoverEvent();
			}
		}
		else {
			// If the mouse is not within this event region then activate the default event
			if (state != RenderMouseEvent::MOUSE_EVENT_TYPE::Default)
				ev->ActivateMouseDefaultEvent();
		}
	}

	// Process all keyboard events (to be added)
}

// Returns a pointer to any component group instance that contains this point within its bounds.
// NOTE: returns the first instance found, and ignores any subsequent (i.e. overlapping) instances
// Allows us to specify that only those components accepting mouse input should be considered
Image2DRenderGroup::InstanceReference Render2DGroup::GetComponentInstanceAtLocation(INTVECTOR2 location, bool only_components_which_accept_mouse_input)
{
	Image2DRenderGroup::Instance *instance;

	// Iterate over the component collection to see if any contains the point above within its bounds
	Image2DRenderGroupCollection::const_iterator it_end = m_componentgroups.end();
	for (Image2DRenderGroupCollection::const_iterator it = m_componentgroups.begin(); it != it_end; ++it)
	{
		// Make sure this component group is being rendered, that it has components to be tested, and that it accepts mouse input (if relevant)
		int count = it->second->GetInstanceCount();
		if (!it->second->GetRenderActive() || count == 0) continue;
		if (only_components_which_accept_mouse_input && !it->second->AcceptsMouseInput()) continue;

		// Look at each instance in the collection in turn
		for (int i=0; i<count; i++)
		{
			instance = it->second->GetInstance(i);

			if ( (instance->render) && 
				 (location.x > instance->position.x) && (location.y > instance->position.y) &&
				 (location.x < (instance->position.x + instance->size.x)) &&
				 (location.y < (instance->position.y + instance->size.y)) )
			{
				// The point is within the bounds of this component, so return a reference now
				return Image2DRenderGroup::InstanceReference(instance, it->second, i, instance->code);
			}
		}
	}

	// If no match then return a NULL reference
	return Image2DRenderGroup::InstanceReference();
}

// Adds a renderable component to the group render queue, maintaining the ordering by Z value
bool Render2DGroup::RegisterRenderableComponent(iUIComponentRenderable *component)
{
	// Make sure we have been passed a valid component 
	if (!component) return false;
	float z = component->GetZOrder();

	// Loop through the render queue from start to end
	int n = m_renderqueue.size();
	for (int i=0; i<n; i++)
	{
		// Make sure this item doesn't already exist in the collection
		if (m_renderqueue[i] == component) return false;

		// If this item has a Z value greater than this item then we want to insert here (to maintain ascending order).  
		if (m_renderqueue[i]->GetZOrder() > z) 
		{
			// Insert into the collection and return true for success
			InsertIntoVector(m_renderqueue, component, i);
			return true;
		}
	}

	// We did not find an element to insert this before, so simply add to the end
	InsertIntoVector(m_renderqueue, component, n);
	return true;
}

// Removes a renderable component from the render queue
bool Render2DGroup::UnregisterRenderableComponent(iUIComponentRenderable *component) 
{ 
	// Attempt to locate the item in the render queue; return false if it does not exist
	int index = FindInVector(m_renderqueue, component);
	if (index == -1) return false;

	// If it does exist, remove the item at this location
	RemoveFromVectorAtIndex(m_renderqueue, index);
	return true;
}

// Updates the status of a component in the render queue, primarily in order to maintain Z ordering
bool Render2DGroup::UpdateRenderableComponent(iUIComponentRenderable *component) 
{ 
	bool result;

	// Unregister from the render queue
	result = UnregisterRenderableComponent(component);
	if (!result) return false;

	// Now re-register in the queue.  Item will be placed in the correct position in the queue
	result = RegisterRenderableComponent(component);
	if (!result) return false;

	// Return success
	return true;
}

void Render2DGroup::Shutdown(void)
{
	// Shutdown, delete & deallocate each iUIComponent group in turn
	Components.Image2D.ShutdownUIComponentGroup();
	Components.Image2DGroup.ShutdownUIComponentGroup();
	Components.MouseEvents.ShutdownUIComponentGroup();
	Components.TextBlocks.ShutdownUIComponentGroup();
	Components.MultiLineTextBlocks.ShutdownUIComponentGroup();
	Components.Buttons.ShutdownUIComponentGroup();
	Components.TextBoxes.ShutdownUIComponentGroup();
	Components.ComboBoxes.ShutdownUIComponentGroup();
	Components.ComponentGroups.ShutdownUIComponentGroup();

	// Also delete & deallocate any groups that do not inherit from iUIComponent
	
}

// Changes the render state of the entire group.  Performs some component-specific notification as well
void Render2DGroup::SetRenderActive(bool render)
{
	// Store the new render state
	m_render = render;

	// Also notify all TEXT components, whose render state is managed by the text manager and so should be updated if necessary
	TextBlockCollection::const_iterator it_end = m_textblocks.end();
	for (TextBlockCollection::const_iterator it = m_textblocks.begin(); it != it_end; ++it) {
		it->second->ParentRenderStateChanged(m_render);
	}
}

Render2DManager *Render2DGroup::GetRenderManager(void)
{
	return m_rendermanager;
}

void Render2DGroup::SetRenderManager(Render2DManager *rm)
{
	m_rendermanager = rm;
}


Render2DGroup::~Render2DGroup(void)
{
}
