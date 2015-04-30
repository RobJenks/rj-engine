#pragma once

#ifndef __iUIControllerH__
#define __iUIControllerH__

#include "Image2DRenderGroup.h"
class GameInputDevice;
class UserInterface;
class Render2DGroup;
class iUIControl;
class UIComboBox;

class iUIController
{

public:

	// Default constructor
	iUIController(void);

	// Method that is called when the UI controller is first created
	Result Initialise(Render2DGroup *render, UserInterface *ui);

	// Virtual initialisation method that subclasses must implement
	virtual Result InitialiseController(Render2DGroup *render, UserInterface *ui) = 0;

	// Method that is called when the UI controller becomes active
	virtual void Activate(void) = 0;

	// Method that is called when the UI controller is deactivated
	virtual void Deactivate(void) = 0;

	// Method to process user input into the active UI controller
	virtual void ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse) = 0;

	// Base methods to perform initial handling of mouse events.  These methods then pass control to the virtual 
	// methods implemented by subclasses below
	void ProcessMouseFirstDownEvent_Base(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component);
	void ProcessMouseUpEvent_Base(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component);
	void ProcessRightMouseFirstDownEvent_Base(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component);
	void ProcessRightMouseUpEvent_Base(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component);

	// Methods to accept mouse events from the UI manager
	virtual void ProcessMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) = 0;
	virtual void ProcessMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) = 0;
	virtual void ProcessMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component) = 0;

	virtual void ProcessRightMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) = 0;
	virtual void ProcessRightMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) = 0;
	virtual void ProcessRightMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component) = 0;

	// Methods to accept generic mouse click events at the specified location
	virtual void ProcessMouseClickAtLocation(INTVECTOR2 location) = 0;
	virtual void ProcessRightMouseClickAtLocation(INTVECTOR2 location) = 0;

	// Methods to accept the processed mouse click events for particular components
	virtual void ProcessMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation) = 0;
	virtual void ProcessRightMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation) = 0;

	// Methods to accept the processed mouse click events for managed components, e.g. buttons
	virtual void ProcessControlClickEvent(iUIControl *control) = 0;
	virtual void ProcessControlRightClickEvent(iUIControl *control) = 0;

	// Method to accept mouse move events, and also mouse hover events for specific components
	virtual void ProcessMouseMoveEvent(INTVECTOR2 location) = 0;
	virtual void ProcessMouseHoverEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, bool lmb, bool rmb) = 0;

	// Methods to process specific events raised from individual controls, and routed through the UserInterface
	virtual void ComboBox_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex) = 0;

	// Returns a reference to the render group assigned to this UI controller
	CMPINLINE Render2DGroup *			GetRenderGroup(void) { return m_render; }

	// Methods to retrieve and set the control (derived from iUIControl) that is currently in focus
	CMPINLINE iUIControl *				GetControlInFocus(void) { return m_focuscontrol; }
	CMPINLINE void						SetControlInFocus(iUIControl *control) { m_focuscontrol = control; }

	// Method to perform rendering of the UI controller components (excluding 2D render objects, which will be handled by the 2D render manager)
	virtual void Render(void) = 0;

	// Default destructor
	~iUIController(void);

protected:

	// Store a reference to the UI controller render group
	Render2DGroup *							m_render;

	// Maintain a reference to the control currently in focus
	iUIControl *							m_focuscontrol;

	// Flags determining whether a mouse event is in progress
	bool									m_lmb_down;
	bool									m_rmb_down;

	// Store the location that the user began a mouse-down event
	INTVECTOR2								m_lmb_down_location;
	INTVECTOR2								m_rmb_down_location;

	// Also store the component that the mouse was hovering over when a mouse event began, or NULL if there wasn't one
	Image2DRenderGroup::Instance *			m_lmb_down_component;
	Image2DRenderGroup::Instance *			m_rmb_down_component;

};


#endif