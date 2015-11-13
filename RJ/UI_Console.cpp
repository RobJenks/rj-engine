#include "DX11_Core.h"
#include "GameVarsExtern.h"
#include "GameInput.h"
#include "Render2DGroup.h"
#include "iUIControl.h"
#include "UITextBox.h"
#include "MultiLineTextBlock.h"
#include "GameConsole.h"
#include "GameConsoleCommand.h"
class UserInterface;

#include "UI_Console.h"

// Colours used for command outputs of each status
const XMFLOAT4 UI_Console::RESULT_COLOUR_NORMAL = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
const XMFLOAT4 UI_Console::RESULT_COLOUR_SUCCESS = XMFLOAT4(0.1176f, 1.0f, 0.1176f, 1.0f);
const XMFLOAT4 UI_Console::RESULT_COLOUR_WARNING = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
const XMFLOAT4 UI_Console::RESULT_COLOUR_FAILURE = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

// Default constructor
UI_Console::UI_Console(void)
{
	// Set default values
	m_input = NULL;
	m_history_text = NULL;
	m_selected_history_item = -1;
	m_history_clear_buffer = 0;
}

// Initialisation method, called once the UI component has been created
Result UI_Console::InitialiseController(Render2DGroup *render, UserInterface *ui)
{
	// Retrieve references to the key UI controls
	m_input = (UITextBox*)m_render->FindUIComponent("txt_input", NullString);
	m_history_text = (MultiLineTextBlock*)m_render->FindUIComponent("mltb_history", NullString);
	if (!m_input || !m_history_text) return ErrorCodes::CannotInitialiseAllRequiredConsoleComponents;


	// Return success
	return ErrorCodes::NoError;
}

// Method that is called when the UI controller becomes active
void UI_Console::Activate(void)
{
	// Perform an update of the displayed console history to make sure it is up-to-date
	UpdateConsoleHistoryDisplay();

	// Set focus to the console input field
	if (m_input) this->SetControlInFocus(m_input);
}

// Method that is called when the UI controller is deactivated
void UI_Console::Deactivate(void)
{

}

// Submits the current line of console input
void UI_Console::SubmitConsoleInput(void)
{
	// Decrement any remaining buffer from a previous wipe of the history display
	if (--m_history_clear_buffer < 0) m_history_clear_buffer = 0;

	// Attempt to retrieve the text that is being submitted
	if (!m_input) return;
	std::string input = m_input->GetText();
	if (input == NullString) return;
	
	// Build a raw text command from this input string
	GameConsoleCommand command;
	command.RawTextInput = input;
	
	// Submit to the game console.  The console will populate output parameters accordingly
	Game::Console.ProcessRawCommand(command);

	// Take action relating to this specific command, for example a sound or effect based on the command result

	// Now update the console output by displaying the last N items in the multi-line text block
	UpdateConsoleHistoryDisplay();

	// Clear the input text field once this command has been processed
	m_input->SetText("");
}

// Displays the game console history in the multi-line text block 
void UI_Console::UpdateConsoleHistoryDisplay(void)
{
	int line;
	std::string output;

	// Parameter check
	if (!m_history_text) return;

	// We want to display (LINE_COUNT / 2) items, so that the command & its result is displayed for each item
	int n = (m_history_text->GetLineCount() / 2);

	// Loop through each required item in turn
	for (int i = 0; i < n; ++i)
	{
		// For display reasons, this is line ((n-1) - i).  It will be displayed at (line*2) and ((line*2)+1)
		line = ((n - 1) - i) * 2;

		// Request this item from the console.  Subtract any buffer introduced as a result of clearing the 
		// history display.  A null item (Status == NotExecuted) will be returned by the Console if the item 
		// requested does not exist in the history
		const GameConsoleCommand & command = Game::Console.GetConsoleHistoryItem(i);
		if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted || 
			(i >= (n - m_history_clear_buffer)))
		{
			// Set both lines to blank if there is no history item
			m_history_text->SetText(line, NullString);
			m_history_text->SetText(line + 1, NullString);
		}
		else
		{
			// This is a valid command; first, set the command itself
			m_history_text->SetText(line, command.RawTextInput);

			// Now construct the result text, including the status & error code if applicable
			bool extra_info = !(command.OutputStatus == GameConsoleCommand::CommandResult::Success ||
								command.OutputResult == ErrorCodes::UnknownConsoleCommand);
			output = (extra_info ? concat(GameConsoleCommand::StatusToString(command.OutputStatus))(": ").str() : "");
			output = concat(output)(command.OutputString).str();
			if (extra_info) output = concat(output)(" [")(command.OutputResult)("]").str();
			
			// Set the line text, and also the line colour depending on status
			m_history_text->SetText(line + 1, output);
			m_history_text->SetColour(line + 1, GetStatusColour(command.OutputStatus));
		}			
	}
}

// Returns the number of lines of history that are displayed in the console
int UI_Console::GetHistoryLineCount(void) const
{
	return (m_history_text ? m_history_text->GetLineCount() : 0);
}

// Method to process user input into the active UI controller
void UI_Console::ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse)
{
	// Test for user input into the console
	if (keyboard->GetKey(DIK_RETURN))
	{
		// Submit the current input to the console
		SubmitConsoleInput();
		keyboard->LockKey(DIK_RETURN);
	}
	else if (keyboard->GetKey(DIK_UP))
	{
		MoveUpConsoleHistory();
		keyboard->LockKey(DIK_UP);
	}
	else if (keyboard->GetKey(DIK_DOWN))
	{
		MoveDownConsoleHistory();
		keyboard->LockKey(DIK_DOWN);
	}

	// Once all user input is processed, consume all other keys so that nothing filters through into the application
	keyboard->ConsumeAllKeys();
}

// Increment up (backwards in time) to the next history item
void UI_Console::MoveUpConsoleHistory(void)
{
	// Check that the input field text does match the history item at the current index; if not, 
	// the user has likely adjusted the text manually and so we want to reset the history
	if (m_input->GetText() != Game::Console.GetConsoleHistoryItem(m_selected_history_item).RawTextInput)
	{
		if (m_input->GetText() == NullString)		m_selected_history_item = -1;
		else										m_selected_history_item = -2;
	}

	// Move up (back in time) through the console history
	int history = (int)Game::Console.GetHistoryLength();
	if (++m_selected_history_item >= history)
		m_selected_history_item = (history - 1);
	PopulateWithConsoleHistoryItem(m_selected_history_item);
}

// Increment down (forwards in time) to the next history item
void UI_Console::MoveDownConsoleHistory(void)
{
	// Check that the input field text does match the history item at the current index; if not, 
	// the user has likely adjusted the text manually and so we want to reset the history
	if (m_input->GetText() != Game::Console.GetConsoleHistoryItem(m_selected_history_item).RawTextInput)
	{
		m_selected_history_item = 0;
	}

	// Move down (forward in time) through the console history
	if (--m_selected_history_item < -1) m_selected_history_item = -1;
	PopulateWithConsoleHistoryItem(m_selected_history_item);
	
}

// Populates the console input with the input data used for a previous command
void UI_Console::PopulateWithConsoleHistoryItem(int index)
{
	if (index <= -1)
	{
		m_input->SetText("");
	}
	else
	{
		const GameConsoleCommand & command = Game::Console.GetConsoleHistoryItem(index);
		if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)
		{
			m_input->SetText("");
		}
		else
		{
			m_input->SetText(command.RawTextInput);
		}
	}
}

// Virtual inherited method to accept a command from the console.  The console UI can accept commands from the
// console, to e.g. update the console data that is being displayed
bool UI_Console::ProcessConsoleCommand(GameConsoleCommand & command)
{
	if (command.InputCommand == "clear")
	{
		// Add a buffer of items that will not be displayed in the history, to simulate clearing of the
		// display.  Each time an item is submitted this buffer is decremented towards zero
		m_history_clear_buffer = m_history_text->GetLineCount() / 2;
		command.SetSuccessOutput("(Clearing history display)"); return true;
	}

	// Command not recognised
	return false;
}

// Event is triggered whenever a mouse click event occurs on a managed control, e.g. a button
void UI_Console::ProcessControlClickEvent(iUIControl *control)
{
	// Change focus to this control, if it is one that can accept focus
	if (control->CanAcceptFocus()) this->SetControlInFocus(control);
}

// Method to perform rendering of the UI controller components (excluding 2D render objects, which will be handled by the 2D render manager)
void UI_Console::Render(void)
{

}

// Terminates the UI layout and disposes of all relevant components
void UI_Console::Terminate(void)
{
	// Deactivate the UI layout, if it hasn't been deactivated already
	Deactivate();
}

// Default destructor
UI_Console::~UI_Console(void)
{

}