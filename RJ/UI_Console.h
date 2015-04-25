#pragma once

#ifndef __UI_ConsoleH__
#define __UI_ConsoleH__

#include "DX11_Core.h"
#include "iUIController.h"
#include "iAcceptsConsoleCommands.h"

class UI_Console : public iUIController, public iAcceptsConsoleCommands 
{
public:
	// Default constructor
	UI_Console(void);

	// Initialisation method, called once the UI component has been created
	Result InitialiseController(Render2DGroup *render, UserInterface *ui);

	// Method that is called when the UI controller becomes active
	void Activate(void);

	// Method that is called when the UI controller is deactivated
	void Deactivate(void);

	// Submits the current line of console input
	void SubmitConsoleInput(void);

	// Displays the game console history in the multi-line text block 
	void UpdateConsoleHistoryDisplay(void);

	// Method to process user input into the active UI controller
	void ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse);

	// Method to perform rendering of the UI controller components (excluding 2D render objects, which will be handled by the 2D render manager)
	void Render(void);

	// Returns the number of lines of history that are displayed in the console
	int GetHistoryLineCount(void) const;

	// Increment up (backwards in time) to the next history item
	void MoveUpConsoleHistory(void);

	// Increment down (forwards in time) to the next history item
	void MoveDownConsoleHistory(void);

	// Populates the console input with the input data used for a previous command
	void PopulateWithConsoleHistoryItem(int index);

	// Virtual inherited method to accept a command from the console.  The console UI can accept commands from the
	// console, to e.g. update the console data that is being displayed
	bool ProcessConsoleCommand(GameConsoleCommand & command);

	// Terminates the UI layout and disposes of all relevant components
	void Terminate(void);

	// Colours used for command outputs of each status
	static const D3DXVECTOR4 RESULT_COLOUR_NORMAL;
	static const D3DXVECTOR4 RESULT_COLOUR_SUCCESS;
	static const D3DXVECTOR4 RESULT_COLOUR_WARNING;
	static const D3DXVECTOR4 RESULT_COLOUR_FAILURE;

	// Methods to accept mouse events from the UI manager
	void ProcessMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component) { }

	void ProcessRightMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessRightMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessRightMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component) { }

	// Methods to accept generic mouse click events at the specified location
	void ProcessMouseClickAtLocation(INTVECTOR2 location) { }
	void ProcessRightMouseClickAtLocation(INTVECTOR2 location) { }

	// Methods to accept the processed mouse click events for particular components
	void ProcessMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation) { }
	void ProcessRightMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation) { }

	// Methods to accept the processed mouse click events for managed components, e.g. buttons
	void ProcessControlClickEvent(iUIControl *control);
	void ProcessControlRightClickEvent(iUIControl *control) { }

	// Method to accept mouse move events, and also mouse hover events for specific components
	void ProcessMouseMoveEvent(INTVECTOR2 location) { }
	void ProcessMouseHoverEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, bool lmb, bool rmb) { }

	// Methods to process specific events raised from individual controls, and routed through the UserInterface
	void ComboBox_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex) { }

	// Returns the colour to be used for rendering a command with specified status
	CMPINLINE const D3DXVECTOR4 & GetStatusColour(GameConsoleCommand::CommandResult status)
	{
		switch (status)
		{
		case GameConsoleCommand::CommandResult::Success:	return RESULT_COLOUR_SUCCESS;
		case GameConsoleCommand::CommandResult::Warning:	return RESULT_COLOUR_WARNING;
		case GameConsoleCommand::CommandResult::Failure:	return RESULT_COLOUR_FAILURE;
		default:											return RESULT_COLOUR_NORMAL;
		}
	}

	// Default destructor
	~UI_Console(void);

protected:

	// References to key components
	UITextBox *				m_input;
	MultiLineTextBlock *	m_history_text;

	// Store a reference to the history item we are selecting via the arrow keys
	int						m_selected_history_item;

	// Use a buffer of empty items to simulate clearing of the history display
	int						m_history_clear_buffer;

};




#endif