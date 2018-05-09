#include "FastMath.h"
#include "iUIController.h"
class UserInterface;

// Default constructor
iUIController::iUIController(void)
{
	// Set all fields to default values
	m_code = "";
	m_render = NULL;
	m_focuscontrol = NULL;
	m_awaiting_termination = false;
	m_lmb_down = m_rmb_down = false;
	m_lmb_down_location = m_rmb_down_location = NULL_INTVECTOR2;
	m_lmb_down_component = m_rmb_down_component = NULL;
}

// Method that is called when the UI controller is first created
Result iUIController::Initialise(const std::string & state_code, Render2DGroup *render, UserInterface *ui)
{
	// Store the state code for this controller
	m_code = state_code;

	// Store a reference to the parent render group
	m_render = render;

	// Call the subclass initialisation method and return the result
	return InitialiseController(render, ui);
}


// Handles the first mouse-down event with the LMB.  
void iUIController::ProcessMouseFirstDownEvent_Base(INTVECTOR2 location, iUIComponent *component)
{
	// Store details on the mouse down event
	m_lmb_down = true;
	m_lmb_down_location = location;

	// Also store the UI component that we began the movement over, in case it is relevant later
	m_lmb_down_component = component;

	// Pass control to the subclass-implemented method
	ProcessMouseFirstDownEvent(location, component);
}

// Handles the first mouse-down event with the RMB.  
void iUIController::ProcessRightMouseFirstDownEvent_Base(INTVECTOR2 location, iUIComponent *component)
{
	// Store details on the mouse down event
	m_rmb_down = true;
	m_rmb_down_location = location;

	// Also store the UI component that we began the movement over, in case it is relevant later
	m_rmb_down_component = component;

	// Pass control to the subclass-implemented method
	ProcessRightMouseFirstDownEvent(location, component);
}

// Handles the LMB mouse-up event that generally signals the end of a mouse click and/or drag
void iUIController::ProcessMouseUpEvent_Base(INTVECTOR2 location, INTVECTOR2 startlocation, iUIComponent *component)
{
	// Pass control to the subclass-implemented method first, before we reset the mouse down flags
	ProcessMouseUpEvent(location, startlocation, component);

	// Clear the mouse down flags
	m_lmb_down = false;
	m_lmb_down_location = NULL_INTVECTOR2;
	m_lmb_down_component = NULL;
}

// Handles the RMB mouse-up event that generally signals the end of a mouse click and/or drag
void iUIController::ProcessRightMouseUpEvent_Base(INTVECTOR2 location, INTVECTOR2 startlocation, iUIComponent *component)
{
	// Pass control to the subclass-implemented method first, before we reset the mouse down flags
	ProcessRightMouseUpEvent(location, startlocation, component);

	// Clear the mouse down flags
	m_rmb_down = false;
	m_rmb_down_location = NULL_INTVECTOR2;
	m_rmb_down_component = NULL;
}

// Default destructor
iUIController::~iUIController(void)
{

}
