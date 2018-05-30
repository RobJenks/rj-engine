#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "Utility.h"
#include "Logging.h"
#include "CoreEngine.h"
#include "RJMain.h"
#include "TextManager.h"
#include "TextBlock.h"
#include "Fonts.h"
#include "GameDataExtern.h"
#include "Render2DManager.h"
#include "Render2DGroup.h"
#include "iUIController.h"
#include "iUIComponent.h"
#include "iUIControl.h"
#include "UI_Console.h"
#include "UI_ShipDesigner.h"
#include "UI_ModelBuilder.h"
#include "UI_ShipBuilder.h"
#include "SentenceType.h"
class GameInputDevice;

#include "UserInterface.h"

// Constant string IDs of the primary UI layouts
const std::string		UserInterface::UI_INFLIGHT					= "UI_INFLIGHT";
const std::string		UserInterface::UI_MAINMENU					= "UI_MAINMENU";
const std::string		UserInterface::UI_CONSOLE					= "UI_CONSOLE";
const std::string		UserInterface::UI_SHIPDESIGNER				= "UI_SHIPDESIGNER";
const std::string		UserInterface::UI_MODELBUILDER				= "UI_MODELBUILDER";
const std::string		UserInterface::UI_SHIPBUILDER				= "UI_SHIPBUILDER";

// Default constructor
UserInterface::UserInterface(void)
{
	// Initialise key pointers to NULL
	m_console = NULL;
	m_shipdesigner = NULL;
	m_modelbuilder = NULL;
	m_shipbuilder = NULL;
	m_controller = NULL;
	m_mouselocation.x = m_mouselocation.y = 0;
	m_mousepreviouslocation.x = m_mousepreviouslocation.y = 0;
	m_mousecurrenthovercomponent = NULL;
	m_console_active = false;
	m_pre_console_controller = NULL;
	m_show_fps = false;

	m_lmb = m_rmb = false;
}

Result UserInterface::Initialise(void)
{
	Result result;

	// Initialise the text strings that will be used in the UI
	result = InitialiseUITextComponents();
	if (result != ErrorCodes::NoError) return result;

	// Initialise the render groups used for 2D rendering in each primary game scenario
	InitialiseUIComponentSets();
	
	// Return success if all initialisation was successful
	return ErrorCodes::NoError;
}

Result UserInterface::BuildUILayouts(void)
{
	Result result, overallresult = ErrorCodes::NoError;

	/* Ship designer UI is currently disabled; expected to be replaced by the ShipBuilder UI*/
	// Perform post-load initialisation of the ship designer UI
	/*result = InitialiseShipDesignerUI();
	if (result != ErrorCodes::NoError)
	{
		overallresult = result; 
		Game::Log << LOG_ERROR << "Error building ship designer UI layout\n";
	}*/

	// Initialise the model builder UI
	result = InitialiseModelBuilderUI();
	if (result != ErrorCodes::NoError)
	{
		overallresult = result;
		Game::Log << LOG_ERROR << "Error building model builder UI layout\n";
	}

	// Initialise the model builder UI
	result = InitialiseShipBuilderUI();
	if (result != ErrorCodes::NoError)
	{
		overallresult = result;
		Game::Log << LOG_ERROR << "Error building ship builder UI layout\n";
	}

	// Initialise the game console UI
	result = InitialiseConsoleUI();
	if (result != ErrorCodes::NoError)
	{
		overallresult = result;
		Game::Log << LOG_ERROR << "Error building console UI layout\n";
	}

	// Return the overall result
	return overallresult;
}


Result UserInterface::InitialiseUITextComponents(void)
{
	// TODO [textrender]: Update for new text rendering component
	// Get a handle to the engine text manager
/*	if (!Game::Engine || !Game::Engine->GetTextManager()) return ErrorCodes::CannotInitialiseUIWithoutGameEngineComponents;
	TextManager *tm = Game::Engine->GetTextManager();

	// Standard text colours for use in the text rendering
	XMFLOAT4 colYellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.75f);

	// FPS counter
	InitialiseTextString(tm, &TextStrings.S_DBG_FPSCOUNTER, TextStrings.C_DBG_FPSCOUNTER, Game::Fonts::FONT_BASIC1, 
						     20, 20, DEBUG_STRING_MAX_LENGTH, 1.0f, colYellow, false);

	// Debug flight info
	InitialiseTextString(tm, &TextStrings.S_DBG_FLIGHTINFO_1, TextStrings.C_DBG_FLIGHTINFO_1, Game::Fonts::FONT_BASIC1, 
							 20, 60, DEBUG_STRING_MAX_LENGTH, 1.0f, colYellow, false);
	InitialiseTextString(tm, &TextStrings.S_DBG_FLIGHTINFO_2, TextStrings.C_DBG_FLIGHTINFO_2, Game::Fonts::FONT_BASIC1, 
							 20, 80, DEBUG_STRING_MAX_LENGTH, 1.0f, colYellow, false);
	InitialiseTextString(tm, &TextStrings.S_DBG_FLIGHTINFO_3, TextStrings.C_DBG_FLIGHTINFO_3, Game::Fonts::FONT_BASIC1, 
							 20, 100, DEBUG_STRING_MAX_LENGTH, 1.0f, colYellow, false);
	InitialiseTextString(tm, &TextStrings.S_DBG_FLIGHTINFO_4, TextStrings.C_DBG_FLIGHTINFO_4, Game::Fonts::FONT_BASIC1,
							 20, 120, DEBUG_STRING_MAX_LENGTH, 1.0f, colYellow, false);

	// Return success, for now, although TODO: we have not checked the return values for each sentence
	*/
	return ErrorCodes::NoError;
}

// Initialises a text block.  These are the managed wrapper objects for manipulating text strings in the interface
TextBlock *UserInterface::CreateTextBlock(const std::string & code, const std::string & text, Font::ID font, INTVECTOR2 pos, float font_size, const XMFLOAT4 & col, bool render)
{	
	TextBlock *tb = new TextBlock(code);
	tb->SetText(text);
	tb->SetFont(font);
	tb->SetColour(col);
	tb->SetPosition(pos.ToFloat());
	tb->SetFontSize(font_size);
	tb->SetRenderActive(render);

	return tb;
}


Result UserInterface::InitialiseUIComponentSets(void)
{
	// Start with all component sets disabled
	Game::Engine->Get2DRenderManager()->DeactivateAllGroups();

	// Return success
	return ErrorCodes::NoError;
}

Result UserInterface::InitialiseShipDesignerUI(void)
{
	/*Result result;

	// Make sure this ship designer UI was loaded with the game data
	Render2DGroup *ui = Game::Engine->Get2DRenderManager()->GetRenderGroup(UserInterface::UI_SHIPDESIGNER);
	if (ui == NULL) return ErrorCodes::CannotInitialiseUIRenderGroupAsNotLoaded;

	// If it was, create the ship designer UI object
	m_shipdesigner = new UI_ShipDesigner();
	if (!m_shipdesigner) return ErrorCodes::CouldNotCreateShipDesignerUIController;

	// Attempt initialisation of the model builder
	result = m_shipdesigner->Initialise(UserInterface::UI_SHIPDESIGNER, ui, this);
	if (result != ErrorCodes::NoError) return result;

	// Add to the collection of controllers
	m_ui_controllers.push_back((iUIController**)&m_shipdesigner);*/

	// Return succeess
	return ErrorCodes::NoError;
}

Result UserInterface::InitialiseModelBuilderUI(void)
{
	Result result;

	// Make sure this UI was loaded with the game data
	Render2DGroup *ui = Game::Engine->Get2DRenderManager()->GetRenderGroup(UserInterface::UI_MODELBUILDER);
	if (ui == NULL) return ErrorCodes::CannotInitialiseUIRenderGroupAsNotLoaded;

	// If it was, create the model builder UI object
	m_modelbuilder = new UI_ModelBuilder();
	if (!m_modelbuilder) return ErrorCodes::CouldNotCreateModelBuilderUIController;

	// Attempt initialisation of the model builder
	result = m_modelbuilder->Initialise(UserInterface::UI_MODELBUILDER, ui, this);
	if (result != ErrorCodes::NoError) return result;

	// Add to the collection of controllers
	m_ui_controllers.push_back((iUIController**)&m_modelbuilder);

	// Return succeess
	return ErrorCodes::NoError;
}

Result UserInterface::InitialiseShipBuilderUI(void)
{
	Result result;

	// Make sure this UI was loaded with the game data
	Render2DGroup *ui = Game::Engine->Get2DRenderManager()->GetRenderGroup(UserInterface::UI_SHIPBUILDER);
	if (ui == NULL) return ErrorCodes::CannotInitialiseUIRenderGroupAsNotLoaded;

	// If it was, create the ship builder UI object
	m_shipbuilder = new UI_ShipBuilder();
	if (!m_shipbuilder) return ErrorCodes::CouldNotCreateShipBuilderUIController;

	// Attempt initialisation of the ship designer
	result = m_shipbuilder->Initialise(UserInterface::UI_SHIPBUILDER, ui, this);
	if (result != ErrorCodes::NoError) return result;

	// Add to the collection of controllers
	m_ui_controllers.push_back((iUIController**)&m_shipbuilder);

	// Return succeess
	return ErrorCodes::NoError;
}

Result UserInterface::InitialiseConsoleUI(void)
{
	Result result;

	// Make sure this UI was loaded with the game data
	Render2DGroup *ui = Game::Engine->Get2DRenderManager()->GetRenderGroup(UserInterface::UI_CONSOLE);
	if (ui == NULL) return ErrorCodes::CannotInitialiseUIRenderGroupAsNotLoaded;

	// If it was, create the console UI object
	m_console = new UI_Console();
	if (!m_console) return ErrorCodes::CouldNotCreateConsoleUIController;

	// Attempt initialisation of the console
	result = m_console->Initialise(UserInterface::UI_CONSOLE, ui, this);
	if (result != ErrorCodes::NoError) return result;

	// Add to the collection of controllers
	m_ui_controllers.push_back((iUIController**)&m_console);

	// Return succeess
	return ErrorCodes::NoError;
}

void UserInterface::Render(void)
{
	// We only need to perform additional rendering (beyond e.g. the 2D render manager) if we have a primary UI controller active 
	if (m_controller) 
	{
		// Execute the control-specific render behaviour for managed controls in this UI controller
		m_controller->GetRenderGroup()->PerformManagedControlRendering(m_controller->GetControlInFocus());

		// Pass control to the UI controller render method
		m_controller->Render();
	}
}

// Returns the code of the active UI controller (or an empty string, if no controller is active)
std::string UserInterface::GetActiveUIControllerCode(void) const
{
	if (m_controller)	return m_controller->GetCode();
	else				return NullString;
}

// Activates a UI state based on its uniquely-identifying code
void UserInterface::ActivateUIState(const std::string & state)
{
	// If we currently have a UI controller active then notify it that it is being deactivated
	if (m_controller) m_controller->Deactivate();

	// Deactivate all UI render groups
	Game::Engine->Get2DRenderManager()->DeactivateAllGroups();

	// Request that the specified group be activated
	Game::Engine->Get2DRenderManager()->ActivateGroup(state);
	
	// Also enable the relevant UI controller, if there is one for this state
	m_controller = GetUIController(state);	

	// If there is a controller for this new state, notify it that it is becoming active
	if (m_controller) m_controller->Activate();
}

// Activate the game console
void UserInterface::ActivateConsole(void)
{
	// Only take action if the console is not already active
	if (m_console_active) return;

	// Record the fact that the console is now active
	m_console_active = true;

	// Store a reference to the UI controller that is currently active (if any) for purposes of reverting afterwards
	m_pre_console_controller = m_controller;

	// Activate the console, then directly set it as the active controller
	Game::Engine->Get2DRenderManager()->ActivateGroup(UserInterface::UI_CONSOLE);
	m_console->Activate();
	m_controller = m_console;
}

// Deactivate the game console
void UserInterface::DeactivateConsole(void)
{
	// Only take action if the console is already active
	if (!m_console_active) return;

	// Record the fact that the console is no longer active
	m_console_active = false;

	// Deactivate the console
	m_console->Deactivate();
	Game::Engine->Get2DRenderManager()->DeactivateGroup(UserInterface::UI_CONSOLE);

	// Restore the previous UI controller (if any)
	m_controller = m_pre_console_controller;
	m_pre_console_controller = NULL;
}


// Returns a pointer to the primary UI layout corresponding to this state, or NULL if not applicable
iUIController *UserInterface::GetUIController(std::string state)
{
	// Attempt to locate a controller matching the desired state code
	std::vector<iUIController**>::iterator it_end = m_ui_controllers.end();
	for (std::vector<iUIController**>::iterator it = m_ui_controllers.begin(); it != it_end; ++it)
	{
		if ((*it) && (**it))
		{
			if ((**it)->GetCode() == state) return (**it);
		}
	}

	// We did not find a matching controller
	return NULL;
}

// Deactivates a single UI state
void UserInterface::DeactivateUIState(const std::string & state)
{
	// Pass this request directly to the 2D render manager
	Game::Engine->Get2DRenderManager()->DeactivateGroup(state);

	// Deactivate the current UI controller, if one exists and if it matches this state
	if (m_controller && m_controller->GetCode() == state)
	{
		m_controller->Deactivate();
		m_controller = NULL;
	}
}

void UserInterface::DeactivateAllUIComponents(void)
{
	// Pass this request directly to the 2D render manager
	Game::Engine->Get2DRenderManager()->DeactivateAllGroups();

	// Reactivate the console render group, if the console is active, since we want this to always remain on top
	if (m_console_active) Game::Engine->Get2DRenderManager()->ActivateGroup(UserInterface::UI_CONSOLE);

	// Deactivate the current UI controller, if one exists
	if (m_controller)
	{
		m_controller->Deactivate();
		m_controller = NULL;
	}
}

void UserInterface::Terminate(void)
{
	// Terminate all major UI components in turn
	//if (m_shipdesigner)		{ m_shipdesigner->Terminate(); SafeDelete(m_shipdesigner); }
	if (m_modelbuilder)		{ m_modelbuilder->Terminate(); SafeDelete(m_modelbuilder); }
	if (m_console)			{ m_console->Terminate(); SafeDelete(m_console); }

	// Deactivate the active primary UI controller
	m_controller = NULL;

	// Also dispose of all managed control definitions
	ReleaseManagedControlDefinitions();
}

void UserInterface::ReleaseManagedControlDefinitions(void)
{
	// Loop through each control definition in turn
	ManagedControlDefinitionCollection::iterator it_end = m_managedcontroldefs.end();
	for (ManagedControlDefinitionCollection::iterator it = m_managedcontroldefs.begin(); it != it_end; ++it)
	{
		// Get a handle to this definition and skip it if the object has already been disposed of
		UIManagedControlDefinition *def = it->second;
		if (!def) continue;

		// Call the shutdown method for this definition
		def->Shutdown();

		// Delete & dispose of the object
		delete def;
		it->second = NULL;
	}

	// Also clear the collection at the very end to remove any lingering pointers
	m_managedcontroldefs.clear();
}

void UserInterface::SetDebugFlightInfoDisplay(bool display)
{
	// Set the render flag on all UI debug flight info components
	// TODO [textrender]: Update for new text rendering component
	/*TextStrings.S_DBG_FLIGHTINFO_1->render = 
	TextStrings.S_DBG_FLIGHTINFO_2->render = 
	TextStrings.S_DBG_FLIGHTINFO_3->render = 
	TextStrings.S_DBG_FLIGHTINFO_4->render = display;*/
}

// Virtual inherited method to accept a command from the console
bool UserInterface::ProcessConsoleCommand(GameConsoleCommand & command)
{
	if (command.InputCommand == "debug_flight_info")
	{
		bool activate = (command.Parameter(0) == "1");
		SetDebugFlightInfoDisplay(activate);
		command.SetSuccessOutput(concat((activate ? "Activating" : "Deactivating"))(" debug flight info display").str()); return true;
	}
	else if (command.InputCommand == "activate_ui")
	{
		ActivateUIState(command.Parameter(0));							
		command.SetSuccessOutput(concat("Attempting to activate UI state \"")(command.Parameter(0))("\"").str()); return true;
	}
	else if (command.InputCommand == "deactivate_ui")
	{
		DeactivateAllUIComponents();									
		command.SetSuccessOutput("Deactivating all UI components"); return true;
	}

	// We do not recognise the command
	return false;
}

// Termaintes the UI controller which is currently active (if applicable)
void UserInterface::TerminateActiveUIController(void)
{
	// Make sure a controller is actually active
	if (m_controller)
	{
		// Reset the termination flag on the controller since we are now processing that request
		m_controller->ResetTerminationFlag();

		// Deactivate the UI state.  This will also call iUIController::Deactivate() and clear
		// the controller pointer since the controller code matches the state being deactivated
		DeactivateUIState(m_controller->GetCode());
	}
}

void UserInterface::ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse)
{
	// Store this data for use in other methods
	m_mouselocation = mouse->GetCursor();
	m_lmb = mouse->LMB();
	m_rmb = mouse->RMB();

	// Run these user events through the Render2DManager, which will determine any impact on the UI and change its state accordingly
	Game::Engine->Get2DRenderManager()->ProcessUserEvents(keyboard, mouse);

	// Also pass these events to the active UI controller, if applicable
	if (m_controller) 
	{
		// Test whether the controller is attempting to shut down, and process that request here if necessary
		if (m_controller->AwaitingTermination())
		{
			TerminateActiveUIController();
		}
		else
		{
			// Pass keyboard input to the current focus control, if there is one, it is visible & can accept keyboard input
			iUIControl *focus = m_controller->GetControlInFocus();
			if (focus && focus->CanAcceptKeyboardInput() && focus->GetRenderActive()) focus->ProcessKeyboardInput(keyboard);

			// Pass all mouse & keyboard data to the generic method before generating specific events
			m_controller->ProcessUserEvents(keyboard, mouse);

			// The primary controller user event method has the potential to shut down the controller itself.  Test for this (rare)
			// condition and just early-exit here if it occurs
			if (!m_controller) return;

			// First, generate mouse hover event handlers
			// See whether the mouse has actually moved since last cycle
			if ((m_mouselocation.x != m_mousepreviouslocation.x) || (m_mouselocation.y != m_mousepreviouslocation.y))
			{
				// If it has, generate a mouse move event and pass to the UI controller
				m_controller->ProcessMouseMoveEvent(m_mouselocation);

				// Also test whether this is a component at this new location
				m_mousecurrenthovercomponent = m_controller->GetRenderGroup()->GetComponentAtLocation(m_mouselocation.ToFloat(), true);
			}

			// If there IS a component at this location (either cached or new) then raise a mouse hover event for the component
			if (m_mousecurrenthovercomponent)
			{
				m_controller->ProcessMouseHoverEvent(m_mousecurrenthovercomponent, m_mouselocation, m_lmb, m_rmb);

				// Also pass control to any managed component to update its appearance based on a mouse hover event
				if (m_mousecurrenthovercomponent->GetParentControl())
					m_mousecurrenthovercomponent->GetParentControl()->HandleMouseHoverEvent(m_mousecurrenthovercomponent, m_mouselocation);
			}

			// Now generate LMB events
			if (m_lmb)
			{
				// If the mouse button is currently down then generate a mouse down event
				m_controller->ProcessMouseDownEvent(m_mouselocation, m_mousecurrenthovercomponent);

				// If the mouse is over a managed component then also pass it control to update its own appearance
				if (m_mousecurrenthovercomponent && m_mousecurrenthovercomponent->GetParentControl())
					m_mousecurrenthovercomponent->GetParentControl()->HandleMouseDownEvent(m_mousecurrenthovercomponent, m_mouselocation);

				// Also report whether this is the first press of the button
				if (mouse->LMBFirstDown())
					m_controller->ProcessMouseFirstDownEvent_Base(m_mouselocation, m_mousecurrenthovercomponent);
			}
			else
			{
				// If the mouse button is no longer down, we have a mouse-up event
				if (mouse->LMBFirstUp())
				{
					// Raise a new mouse up event
					INTVECTOR2 startloc = mouse->GetLMBStartPosition();
					m_controller->ProcessMouseUpEvent_Base(m_mouselocation, startloc, m_mousecurrenthovercomponent);

					// If we start & end the mouse click within a certain tolerance, generate a generic click event at the end position
					if ((abs(startloc.x - m_mouselocation.x) < UserInterface::CLICK_TOLERANCE) && (abs(startloc.y - m_mouselocation.y) < UserInterface::CLICK_TOLERANCE))
						m_controller->ProcessMouseClickAtLocation(m_mouselocation);

					// Also test whether this qualifies as a mouse click event on a specific component, if the mouse start and 
					// end location are both within a particular control. 
					// First, test whether the current (mouse up) location is within a component, and whether began the mouse event within the same component
					if (m_mousecurrenthovercomponent && 
						PointWithinBounds(startloc, INTVECTOR2(m_mousecurrenthovercomponent->GetPosition()), INTVECTOR2(m_mousecurrenthovercomponent->GetSize())))
					{
						// If it did, raise a mouse click event for this component
						m_controller->ProcessMouseClickEvent(m_mousecurrenthovercomponent, m_mouselocation, startloc);

						// Also raise a control-click method (both at the controller and the control itself) if this is a managed control
						if (m_mousecurrenthovercomponent->GetParentControl())
						{
							m_controller->ProcessControlClickEvent(m_mousecurrenthovercomponent->GetParentControl());
							m_mousecurrenthovercomponent->GetParentControl()->HandleMouseClickEvent(
								m_mousecurrenthovercomponent, m_mouselocation, startloc);
						}
						else
						{
							// We were clicking entirely within a component, but that component is not part of a focus-able control
							m_controller->SetControlInFocus(NULL);
						}
					}
					else
					{
						// We were not clicking on an component, or the down/up events did not take place within the same component.  Deselect any control
						m_controller->SetControlInFocus(NULL);
					}

					// Pass a mouse up notification to any managed control that requires it to revert from a mouse down state
					BroadcastMouseUpEvent(true, false, m_mousecurrenthovercomponent, m_mouselocation);
				}
			}

			// Now generate RMB events
			if (m_rmb)
			{
				// If the mouse button is currently down then generate a mouse down event
				m_controller->ProcessRightMouseDownEvent(m_mouselocation, m_mousecurrenthovercomponent);

				// If the mouse is over a managed component then also pass it control to update its own appearance
				if (m_mousecurrenthovercomponent && m_mousecurrenthovercomponent->GetParentControl())
					m_mousecurrenthovercomponent->GetParentControl()->HandleRightMouseDownEvent(m_mousecurrenthovercomponent, m_mouselocation);

				// Also report whether this is the first press of the button
				if (mouse->RMBFirstDown())
					m_controller->ProcessRightMouseFirstDownEvent_Base(m_mouselocation, m_mousecurrenthovercomponent);
			}
			else
			{
				// If the mouse button is no longer down, we have a mouse-up event
				if (mouse->RMBFirstUp())
				{
					// Raise a new mouse up event
					INTVECTOR2 startloc = mouse->GetRMBStartPosition();
					m_controller->ProcessRightMouseUpEvent_Base(m_mouselocation, startloc, m_mousecurrenthovercomponent);

					// If we start & end the mouse click within a certain tolerance, generate a generic click event at the end position
					if ((abs(startloc.x - m_mouselocation.x) < UserInterface::CLICK_TOLERANCE) && (abs(startloc.y - m_mouselocation.y) < UserInterface::CLICK_TOLERANCE))
						m_controller->ProcessRightMouseClickAtLocation(m_mouselocation);

					// Also test whether this qualifies as a mouse click event on a specific component, if the mouse start and 
					// end location are both within a particular control. 
					// First, test whether the current (mouse up) location is within a component
					if (m_mousecurrenthovercomponent) {

						// If the mouse IS within a component then test whether it also began the mouse event within the same component
						if (PointWithinBounds(startloc, INTVECTOR2(m_mousecurrenthovercomponent->GetPosition()), INTVECTOR2(m_mousecurrenthovercomponent->GetSize())))
						{
							// If it did, raise a right mouse click event for this component
							m_controller->ProcessRightMouseClickEvent(m_mousecurrenthovercomponent, m_mouselocation, startloc);

							// Also raise a control-right-click method if this is a managed control
							if (m_mousecurrenthovercomponent->GetParentControl())
							{
								m_controller->ProcessControlRightClickEvent(m_mousecurrenthovercomponent->GetParentControl());
								m_mousecurrenthovercomponent->GetParentControl()->HandleMouseRightClickEvent(
									m_mousecurrenthovercomponent, m_mouselocation, startloc);
							}
						}
					}

					// Pass a mouse up notification to any managed control that requires it to revert from a mouse down state
					BroadcastMouseUpEvent(false, true, m_mousecurrenthovercomponent, m_mouselocation);
				}
			}
		}
	}

	// Store the previous mouse location once all events are processed, for efficiency next cycle
	m_mousepreviouslocation = m_mouselocation;
}

// Broadcasts a mouse up event to any control not currently in its default state
void UserInterface::BroadcastMouseUpEvent(bool lmb, bool rmb, iUIComponent *component, INTVECTOR2 mouselocation)
{
	// Get a reference to the current render group.  NOTE: assumes that m_controller and the render group are valid
	Render2DGroup *group = m_controller->GetRenderGroup();

	// Broadcast this mouse up event to each class of managed control in turn
	BroadcastMouseUpEventToClass((RenderComponentGroup<iUIControl*>*)&(group->Components.Buttons), lmb, rmb, component, mouselocation);
}

// Broadcasts a mouse up event within a class of iUIControl-derived components.  Internal private method.
void UserInterface::BroadcastMouseUpEventToClass(RenderComponentGroup<iUIControl*> *cclass, bool lmb, bool rmb, iUIComponent *component, INTVECTOR2 mouselocation)
{
	RenderComponentGroup<iUIControl*>::ItemCollection::const_iterator it_end = cclass->Items()->end();
	for (RenderComponentGroup<iUIControl*>::ItemCollection::const_iterator it = cclass->Items()->begin(); it != it_end; ++it)
	{
		// Make sure this is a valid control
		if (!it->second) continue;

		// Pass a mouse up event to any control that requires it to clear its current mouse-down state
		if (lmb && it->second->GetControlState() == iUIControl::ControlState::LMBDown) it->second->HandleMouseUpEvent(component, mouselocation);
		if (rmb && it->second->GetControlState() == iUIControl::ControlState::RMBDown) it->second->HandleRightMouseUpEvent(component, mouselocation);
	}
}

// User control events from each control type; can be handled by the UI, but generally passed directly to the controller
void UserInterface::ComboBox_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex) 
{ 
	if (m_controller) m_controller->ComboBox_SelectedIndexChanged(control, selectedindex, previousindex); 
}

// Writes a c string to the debug output
void UserInterface::WriteDebugOutput(int line, const char *text)
{
	// Make sure the string doesn't exceed maximum length
	if (strlen(text) > UserInterface::DEBUG_STRING_MAX_LENGTH) return;

	// Copy this string data into the debug string
	// TODO [textrender]: Update for new text rendering component
	/*
	if (line == 1)
	{
		strcpy(D::UI->TextStrings.C_DBG_FLIGHTINFO_1, text);
		Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_1, D::UI->TextStrings.C_DBG_FLIGHTINFO_1, 1.0f);
	}
	else if (line == 2)
	{
		strcpy(D::UI->TextStrings.C_DBG_FLIGHTINFO_2, text);
		Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_2, D::UI->TextStrings.C_DBG_FLIGHTINFO_2, 1.0f);
	}
	else if (line == 3)
	{
		strcpy(D::UI->TextStrings.C_DBG_FLIGHTINFO_3, text);
		Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_3, D::UI->TextStrings.C_DBG_FLIGHTINFO_3, 1.0f);
	}
	else if (line == 4)
	{
		strcpy(D::UI->TextStrings.C_DBG_FLIGHTINFO_4, text);
		Game::Engine->GetTextManager()->SetSentenceText(D::UI->TextStrings.S_DBG_FLIGHTINFO_4, D::UI->TextStrings.C_DBG_FLIGHTINFO_4, 1.0f);
	}
	*/
}

// Writes a string to the debug output
void UserInterface::WriteDebugOutput(int line, std::string text)
{
	const char *c = text.c_str();
	WriteDebugOutput(line, c);
}


UserInterface::~UserInterface(void)
{
}
