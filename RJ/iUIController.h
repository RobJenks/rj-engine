#pragma once

#ifndef __iUIControllerH__
#define __iUIControllerH__

#include "Image2DRenderGroup.h"
class GameInputDevice;
class UserInterface;
class Render2DGroup;
class iUIComponent;
class iUIControl;
class UIComboBox;

// This class has no special alignment requirements
class iUIController
{

public:

	// Default constructor
	iUIController(void);

	// Returns the string code which identifies this controller
	CMPINLINE std::string GetCode(void) const { return m_code; }

	// Method that is called when the UI controller is first created
	Result Initialise(const std::string & state_code, Render2DGroup *render, UserInterface *ui);

	// Virtual initialisation method that subclasses must implement
	virtual Result InitialiseController(Render2DGroup *render, UserInterface *ui) = 0;

	// Method that is called when the UI controller becomes active
	virtual void Activate(void) = 0;

	// Method that is called when the UI controller is deactivated
	virtual void Deactivate(void) = 0;

	// Shut down the controller	
	CMPINLINE void Shutdown(void)						{ m_awaiting_termination = true; }

	// Flag which indicates whether the controller is awaiting shutdown
	CMPINLINE bool AwaitingTermination(void) const		{ return m_awaiting_termination; }

	// Reset the flag that requests controller shutdown
	CMPINLINE void ResetTerminationFlag(void)			{ m_awaiting_termination = false; }

	// Method to process user input into the active UI controller
	virtual void ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse) = 0;

	// Base methods to perform initial handling of mouse events.  These methods then pass control to the virtual 
	// methods implemented by subclasses below
	void ProcessMouseFirstDownEvent_Base(INTVECTOR2 location, iUIComponent *component);
	void ProcessMouseUpEvent_Base(INTVECTOR2 location, INTVECTOR2 startlocation, iUIComponent *component);
	void ProcessRightMouseFirstDownEvent_Base(INTVECTOR2 location, iUIComponent *component);
	void ProcessRightMouseUpEvent_Base(INTVECTOR2 location, INTVECTOR2 startlocation, iUIComponent *component);

	// Methods to accept mouse events from the UI manager
	virtual void ProcessMouseDownEvent(INTVECTOR2 location, iUIComponent *component) = 0;
	virtual void ProcessMouseFirstDownEvent(INTVECTOR2 location, iUIComponent *component) = 0;
	virtual void ProcessMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, iUIComponent *component) = 0;

	virtual void ProcessRightMouseDownEvent(INTVECTOR2 location, iUIComponent *component) = 0;
	virtual void ProcessRightMouseFirstDownEvent(INTVECTOR2 location, iUIComponent *component) = 0;
	virtual void ProcessRightMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, iUIComponent *component) = 0;

	// Methods to accept generic mouse click events at the specified location
	virtual void ProcessMouseClickAtLocation(INTVECTOR2 location) = 0;
	virtual void ProcessRightMouseClickAtLocation(INTVECTOR2 location) = 0;

	// Methods to accept the processed mouse click events for particular components
	virtual void ProcessMouseClickEvent(iUIComponent *component, INTVECTOR2 location, INTVECTOR2 startlocation) = 0;
	virtual void ProcessRightMouseClickEvent(iUIComponent *component, INTVECTOR2 location, INTVECTOR2 startlocation) = 0;

	// Methods to accept the processed mouse click events for managed components, e.g. buttons
	virtual void ProcessControlClickEvent(iUIControl *control) = 0;
	virtual void ProcessControlRightClickEvent(iUIControl *control) = 0;
	
	// Methods to accept other managed control events
	virtual void ProcessTextboxChangedEvent(iUIControl *control) = 0;

	// Method to accept mouse move events, and also mouse hover events for specific components
	virtual void ProcessMouseMoveEvent(INTVECTOR2 location) = 0;
	virtual void ProcessMouseHoverEvent(iUIComponent *component, INTVECTOR2 location, bool lmb, bool rmb) = 0;

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

	// The string state code for this UI controller
	std::string								m_code;

	// Store a reference to the UI controller render group
	Render2DGroup *							m_render;

	// Maintain a reference to the control currently in focus
	iUIControl *							m_focuscontrol;

	// Flag indicating that the controller wishes to shut down in the next UI cycle
	bool									m_awaiting_termination;

	// Flags determining whether a mouse event is in progress
	bool									m_lmb_down;
	bool									m_rmb_down;

	// Store the location that the user began a mouse-down event
	INTVECTOR2								m_lmb_down_location;
	INTVECTOR2								m_rmb_down_location;

	// Also store the component that the mouse was hovering over when a mouse event began, or NULL if there wasn't one
	iUIComponent *							m_lmb_down_component;
	iUIComponent *							m_rmb_down_component;

};


#endif
