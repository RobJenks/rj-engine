#include "RenderMouseEvent.h"

// Constructor; initialises all components to null
RenderMouseEvent::RenderMouseEvent(std::string code)
{
	m_code = code;
	m_render = true;
	m_state = MOUSE_EVENT_TYPE::None;		// Ensures that first trigger will update to default

	m_mousedefaultid = ""; 
	m_mousehoverid = "";
	m_mousedownid = "";

	m_mousedefault = NULL;
	m_mousehover = NULL;
	m_mousedown = NULL;
}

// Constructor; the string ID of event components can be passed in as parameters.  If a pointer to the component
// index is provided it will also seek to derive the object references at the same time
RenderMouseEvent::RenderMouseEvent(std::string code, std::string mouse_default, std::string mouse_hover, std::string mouse_down,
								   Render2DGroup::Image2DCollection *componentindex)
{
	// Set properties based on supplied parameters
	m_code = code;
	m_render = true;
	m_mousedefaultid = mouse_default;
	m_mousehoverid = mouse_hover;
	m_mousedownid = mouse_down;

	// Initialise component pointers to NULL
	m_mousedefault = NULL;
	m_mousehover = NULL;
	m_mousedown = NULL;

	// Also attempt to resolve these component IDs if we have been provided an index
	if (componentindex)
		ResolveAllComponents(componentindex);
}

// Activates a particular state for this event
void RenderMouseEvent::ActivateEvent(MOUSE_EVENT_TYPE state)
{
	switch (state)
	{
		case MOUSE_EVENT_TYPE::Default:
			ActivateMouseDefaultEvent();
		case MOUSE_EVENT_TYPE::Hover:
			ActivateMouseHoverEvent();
		case MOUSE_EVENT_TYPE::Down:
			ActivateMouseDownEvent();
	}	
}

void RenderMouseEvent::ActivateMouseDefaultEvent(void)
{
	if (m_mousedefault)		m_mousedefault->SetRenderActive(true);
	if (m_mousehover)		m_mousehover->SetRenderActive(false);
	if (m_mousedown)		m_mousedown->SetRenderActive(false);

	m_state = MOUSE_EVENT_TYPE::Default;
}

void RenderMouseEvent::ActivateMouseHoverEvent(void)
{
	if (m_mousehover)		m_mousehover->SetRenderActive(true);
	else					return;

	if (m_mousedefault)		m_mousedefault->SetRenderActive(false);
	if (m_mousedown)		m_mousedown->SetRenderActive(false);

	m_state = MOUSE_EVENT_TYPE::Hover;
}

void RenderMouseEvent::ActivateMouseDownEvent(void)
{
	if (m_mousedown)		m_mousedown->SetRenderActive(true);
	else					return;

	if (m_mousedefault)		m_mousedefault->SetRenderActive(false);
	if (m_mousehover)		m_mousehover->SetRenderActive(false);

	m_state = MOUSE_EVENT_TYPE::Down;
}


// Sets the component for the specified mouse event condition
void RenderMouseEvent::SetMouseComponent(MOUSE_EVENT_TYPE type, std::string component, Render2DGroup::Image2DCollection *componentindex)
{
	switch (type)
	{
		case MOUSE_EVENT_TYPE::Default:
			SetMouseDefault(component, componentindex);			break;
		case MOUSE_EVENT_TYPE::Hover:
			SetMouseHover(component, componentindex);			break;
		case MOUSE_EVENT_TYPE::Down:
			SetMouseDown(component, componentindex);			break;
	}
}

// Methods to set each specific component pointer.  If a pointer to the component index is provided it will also seek
// to derive the objects references at the same time
void RenderMouseEvent::SetMouseDefault(std::string component, Render2DGroup::Image2DCollection *componentindex)
{
	// Set the component ID
	m_mousedefaultid = component;

	// If we have been provided an index then also attempt to resolve the component here
	if (componentindex && m_mousedefaultid != NullString) m_mousedefault = ResolveComponentID(m_mousedefaultid, componentindex);
}
void RenderMouseEvent::SetMouseHover(std::string component, Render2DGroup::Image2DCollection *componentindex)
{
	// Set the component ID
	m_mousehoverid = component;

	// If we have been provided an index then also attempt to resolve the component here
	if (componentindex && m_mousehoverid != NullString) m_mousehover = ResolveComponentID(m_mousehoverid, componentindex);
}
void RenderMouseEvent::SetMouseDown(std::string component, Render2DGroup::Image2DCollection *componentindex)
{
	// Set the component ID
	m_mousedownid = component;

	// If we have been provided an index then also attempt to resolve the component here
	if (componentindex && m_mousedownid != NullString) m_mousedown = ResolveComponentID(m_mousedownid, componentindex);
}

// Method that attempts to locate all component IDs in the supplied index, and populates the component references if found
void RenderMouseEvent::ResolveAllComponents(Render2DGroup::Image2DCollection *componentindex)
{
	// Simply call each method to set component IDs, passing the component index, to force each to update in turn
	SetMouseDefault(m_mousedefaultid, componentindex);
	SetMouseHover(m_mousehoverid, componentindex);
	SetMouseDown(m_mousedownid, componentindex);
}


// Method that attempts to locate the specified component ID in the supplied index and returns it if found
Image2D *RenderMouseEvent::ResolveComponentID(std::string component, Render2DGroup::Image2DCollection *componentindex)
{
	// Attempt to retrieve this component from the collection
	if (componentindex && component != NullString)
	{
		// Cast the index pointer to a reference lvalue, allowing us to use the overloaded operators
		// without creating a temporary copy via full dereferencing
		Render2DGroup::Image2DCollection &index = *componentindex;

		// If the component exists within the index then return a reference to it now
		if (index.count(component) > 0)
			return index[component];
	}

	// If no match is found then return NULL
	return NULL;
}

// Set the bounds of this event region
void RenderMouseEvent::SetBounds(int x, int y, int width, int height)
{
	SetX(x);
	SetY(y);
	SetWidth(width);
	SetHeight(height);
}

std::string RenderMouseEvent::GetMouseComponentID(MOUSE_EVENT_TYPE type)
{
	switch (type)
	{
		case MOUSE_EVENT_TYPE::Default:
			return GetMouseDefaultComponentID();
		case MOUSE_EVENT_TYPE::Hover:
			return GetMouseHoverComponentID();
		case MOUSE_EVENT_TYPE::Down:
			return GetMouseDownComponentID();
		default:
			return "";
	}
}

Image2D* RenderMouseEvent::GetMouseComponent(MOUSE_EVENT_TYPE type)
{
	switch (type)
	{
		case MOUSE_EVENT_TYPE::Default:
			return GetMouseDefault();
		case MOUSE_EVENT_TYPE::Hover:
			return GetMouseHover();
		case MOUSE_EVENT_TYPE::Down:
			return GetMouseDown();
		default:
			return NULL;
	}
}

// Destructor
RenderMouseEvent::~RenderMouseEvent(void)
{
	// Do nothing; we do not want to deallocate the component references.  Simply unassign the component pointers to
	// avoid any potential garbage collection (will this happen?)
	m_mousedefault = NULL;
	m_mousehover = NULL;
	m_mousedown = NULL;
}
