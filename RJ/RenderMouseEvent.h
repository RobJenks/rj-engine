#pragma once

#ifndef __RenderMouseEventH__
#define __RenderMouseEventH__

#include <string>
#include "CompilerSettings.h"
#include "Render2DGroup.h"
#include "iUIComponent.h"

class RenderMouseEvent : public iUIComponent
{
public:

	enum MOUSE_EVENT_TYPE { None = 0, Default, Hover, Down };

	// Constructor; initialises all components to null
	RenderMouseEvent	(std::string code);

	// Constructor; the string ID of event components can be passed in as parameters.  If a pointer to the component
	// index is provided it will also seek to derive the object references at the same time
	RenderMouseEvent	(std::string code, std::string mouse_default, std::string mouse_hover, std::string mouse_down,
						 Render2DGroup::Image2DCollection *componentindex);
	
	// Returns the current state of this mouse event
	CMPINLINE MOUSE_EVENT_TYPE	GetEventState(void) { return m_state; }

	// Methods to activate a specific scenario of this event
	void				ActivateEvent(MOUSE_EVENT_TYPE state);
	void				ActivateMouseDefaultEvent(void);
	void				ActivateMouseHoverEvent(void);
	void				ActivateMouseDownEvent(void);

	// Methods to set each specific component pointer.  If a pointer to the component index is provided it will also seek
	// to derive the objects references at the same time
	void				SetMouseComponent(MOUSE_EVENT_TYPE type, std::string component, Render2DGroup::Image2DCollection *componentindex);
	void				SetMouseDefault(std::string component, Render2DGroup::Image2DCollection *componentindex);
	void				SetMouseHover(std::string component, Render2DGroup::Image2DCollection *componentindex);
	void				SetMouseDown(std::string component, Render2DGroup::Image2DCollection *componentindex);

	// Method that attempts to locate all component IDs in the supplied index, and populates the component references if found
	void				ResolveAllComponents(Render2DGroup::Image2DCollection *componentindex);

	// Method that attempts to locate the specified component ID in the supplied index and returns it if found
	Image2D *			ResolveComponentID(std::string component, Render2DGroup::Image2DCollection &componentindex);

	
	// Methods for accessing/modifying whether or not this event is active
	CMPINLINE bool		GetRenderActive(void) const		{ return m_render; }
	CMPINLINE void		SetRenderActive(bool render)	{ m_render = render; }

	// Methods around region bounds
	CMPINLINE int		GetX(void) { return m_x; }
	CMPINLINE int		GetY(void) { return m_y; }
	CMPINLINE int		GetWidth(void) { return m_width; }
	CMPINLINE int		GetHeight(void) { return m_height; }

	CMPINLINE void		SetX(int x) { m_x = x; }
	CMPINLINE void		SetY(int y) { m_y = y; }
	CMPINLINE void		SetWidth(int width) { m_width = width; }
	CMPINLINE void		SetHeight(int height) { m_height = height; }
	void				SetBounds(int x, int y, int width, int height);

	// Accessor methods for all key properties
	CMPINLINE std::string	GetMouseComponentID(MOUSE_EVENT_TYPE type);
	CMPINLINE std::string	GetMouseDefaultComponentID(void) { return m_mousedefaultid; }
	CMPINLINE std::string	GetMouseHoverComponentID(void) { return m_mousehoverid; }
	CMPINLINE std::string	GetMouseDownComponentID(void) { return m_mousedownid; }
	
	CMPINLINE Image2D*	GetMouseComponent(MOUSE_EVENT_TYPE type);
	CMPINLINE Image2D*	GetMouseDefault(void) { return m_mousedefault; } 
	CMPINLINE Image2D*	GetMouseHover(void) { return m_mousehover; }
	CMPINLINE Image2D*	GetMouseDown(void) { return m_mousedown; }

	// Shutdown method to satisfy the interface requirement (only basic components, e.g. Image2Ds & text, need to be disposed of)
	void Shutdown(void) { }

	// Destructor
	~RenderMouseEvent(void);

private:
	
	// Method that attempts to locate the specified component ID in the supplied index and returns it if found
	Image2D *			ResolveComponentID(std::string component, Render2DGroup::Image2DCollection *componentindex);

	// Current event state of this item
	MOUSE_EVENT_TYPE	m_state;

	// Position & size of this event region
	int					m_x, m_y, m_width, m_height;

	// Flag determining whether this event is currently active
	bool				m_render;

	// Local instance variables storing component IDs and references
	std::string			m_mousedefaultid, m_mousehoverid, m_mousedownid;
	Image2D 			*m_mousedefault, *m_mousehover, *m_mousedown;



};


#endif