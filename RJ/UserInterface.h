#pragma once

#ifndef __UserInterfaceH__
#define __UserInterfaceH__

#include <string>
#include "ErrorCodes.h"
#include "TextManager.h"
#include "Image2DRenderGroup.h"
#include "RenderComponentGroup.h"
#include "UIManagedControlDefinition.h"
#include "iAcceptsConsoleCommands.h"
#include "SentenceType.h"
class CoreEngine;
class Render2DGroup;
class TextBlock;
class UIComboBox;
class Image2D;
class GameInputDevice;
class iUIController;
class UI_Console;
class UI_ShipDesigner;
class UI_ModelBuilder;
class UI_ShipBuilder;
using namespace std;

// This class has no special alignment requirements
class UserInterface  : public iAcceptsConsoleCommands 
{
public:

	UserInterface(void);
	~UserInterface(void);

	// Static constants
	static const int CLICK_TOLERANCE = 4;
	static const int DEBUG_STRING_MAX_LENGTH = 512;

	// Collection of text strings that are required for the UI
	struct TextStringData
	{
		SentenceType *				S_DBG_FPSCOUNTER;										// For displaying the current FPS
		char						C_DBG_FPSCOUNTER		[DEBUG_STRING_MAX_LENGTH];

		SentenceType *				S_DBG_FLIGHTINFO_1;										// Line 1 of debug flight information
		SentenceType *				S_DBG_FLIGHTINFO_2;										// Line 2 of debug flight information
		SentenceType *				S_DBG_FLIGHTINFO_3;										// Line 3 of debug flight information
		SentenceType *				S_DBG_FLIGHTINFO_4;										// Line 4 of debug flight information
		char						C_DBG_FLIGHTINFO_1		[DEBUG_STRING_MAX_LENGTH];	
		char						C_DBG_FLIGHTINFO_2		[DEBUG_STRING_MAX_LENGTH];	
		char						C_DBG_FLIGHTINFO_3		[DEBUG_STRING_MAX_LENGTH];	
		char						C_DBG_FLIGHTINFO_4		[DEBUG_STRING_MAX_LENGTH];

	} TextStrings;

	// Initialises the user interface
	Result					Initialise(void);

	// Builds UI layouts, once all required data has been loaded from game data files
	Result					BuildUILayouts(void);

	// Allocates memory and prepares text string objects for the UI
	Result					InitialiseUITextComponents(void);

	// Initialises a new text block.  These are the wrapper classes used for manipulation of string data in the interface
	TextBlock *				CreateTextBlock(string code, const char *text, int maxlength, int font, INTVECTOR2 pos, float size, const XMFLOAT4 & col, bool render);

	// Initialises a UI text string.  TextBuffer is an optional parameter which will also be initialised to 0 if set
	Result					InitialiseTextString(TextManager *tm, SentenceType **sentence, char *textbuffer, int fontID, 
										   int x, int y, int maxlength, float size, const XMFLOAT4 & colour, bool render);

	// Creates a new component, for addition to one of the UI component sets
	Image2D *				NewComponent(string code, const char *filename, int x, int y, float z, int width, int height);

	// Initialise the different components sets used for rendering the UI in different scenarios
	Result					InitialiseUIComponentSets(void);

	// Initialisation method for UI controllers
	Result					InitialiseShipDesignerUI(void);
	Result					InitialiseModelBuilderUI(void);
	Result					InitialiseShipBuilderUI(void);
	Result					InitialiseConsoleUI(void);

	// Accessor method for UI controllers
	CMPINLINE UI_ShipDesigner *		ShipDesignerUI(void)		{ return m_shipdesigner; }
	CMPINLINE UI_ModelBuilder *		ModelBuilderUI(void)		{ return m_modelbuilder; }
	CMPINLINE UI_ShipBuilder *		ShipBuilderUI(void)			{ return m_shipbuilder; }
	CMPINLINE UI_Console *			ConsoleUI(void)				{ return m_console; }

	// Activates or deactivates particular UI states
	void					ActivateUIState(const std::string & state);
	void					DeactivateUIState(const std::string & state);
	void					DeactivateAllUIComponents(void);

	// Returns the active UI controller (if relevant)
	CMPINLINE iUIController *	GetActiveUIController(void) const		{ return m_controller; }
	std::string					GetActiveUIControllerCode(void) const;

	// Termaintes the UI controller which is currently active (if applicable)
	void					TerminateActiveUIController(void);

	// Activate or deactivate the game console
	void					ActivateConsole(void);
	void					DeactivateConsole(void);
	CMPINLINE void			ToggleConsole(void) { if (m_console_active) DeactivateConsole(); else ActivateConsole(); }

	// Invokes rendering of the user interface
	void					Render(void);

	// Virtual inherited method to accept a command from the console
	bool					ProcessConsoleCommand(GameConsoleCommand & command);

	// Returns a pointer to the UI controller responsible for the specified state, if one exists
	iUIController *			GetUIController(string state);

	// Processes user interface events from the main game loop
	void					ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse);

	// Broadcasts a mouse up event to any control not currently in its default state
	void					BroadcastMouseUpEvent(bool lmb, bool rmb, Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);

	// User control events from each control type; can be handled by the UI, but generally passed directly to the controller
	void					ComboBox_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex);

	// Determines whether FPS info is displayed on screeen
	CMPINLINE void			SetFPSCounterDisplay(bool display) { TextStrings.S_DBG_FPSCOUNTER->render = true; }
	CMPINLINE bool			IsFPSCounterDisplayed(void) { return TextStrings.S_DBG_FPSCOUNTER->render; }

	// Methods for handling the set of managed control definitions
	CMPINLINE bool							HaveManagedControlDefinition(string key) 
											{ return (m_managedcontroldefs.count(key) > 0 && m_managedcontroldefs[key] != NULL); }
	CMPINLINE UIManagedControlDefinition*	GetManagedControlDefinition(string key) { return m_managedcontroldefs[key]; }
	CMPINLINE void							AddManagedControlDefinition(string key, UIManagedControlDefinition *def) 
											{ m_managedcontroldefs[key] = def; }
	CMPINLINE void							RemoveManagedControlDefinition(string key) { m_managedcontroldefs[key] = NULL; }

	// Determines whether debug flight information is displayed on screen
	void					SetDebugFlightInfoDisplay(bool display);

	// Terminates the UI and all UI layouts that have been created
	void					Terminate(void);
	void					ReleaseManagedControlDefinitions(void);


	// Debug output options
	void					WriteDebugOutput(int line, const char *text);
	void					WriteDebugOutput(int line, string text);

private:

	// Primary UI objects, for those layouts which have their own defined functionality
	UI_Console *			m_console;
	UI_ShipDesigner *		m_shipdesigner;
	UI_ModelBuilder *		m_modelbuilder;
	UI_ShipBuilder *		m_shipbuilder;

	// Collection of PPs to UI controllers, for instances where we need to parse all of them
	std::vector<iUIController**>	m_ui_controllers;

	// Pointer to the currently-active UI controller, or NULL if none are active
	iUIController *			m_controller;

	// Store the controller (if any) that was active before the console was activated, for purposes of reverting control afterwards
	iUIController *			m_pre_console_controller;
	bool					m_console_active;

	// Mouse control data
	INTVECTOR2				m_mouselocation;
	bool					m_lmb, m_rmb;

	// Cached data on the last mouse event, for efficiency in subsequent cycles where the mouse hasn't moved
	INTVECTOR2									m_mousepreviouslocation;
	Image2DRenderGroup::InstanceReference		m_mousecurrenthovercomponent;

	// Collection of managed control definitions
	typedef unordered_map<string, UIManagedControlDefinition*> ManagedControlDefinitionCollection;
	ManagedControlDefinitionCollection			m_managedcontroldefs;

public:
	// String keys for the primary UI layouts 
	static const string		UI_INFLIGHT;					
	static const string		UI_MAINMENU;					
	static const string		UI_CONSOLE;
	static const string		UI_SHIPDESIGNER;				
	static const string		UI_MODELBUILDER;
	static const string		UI_SHIPBUILDER;


private:
	
	// Broadcasts a mouse up event within a class of iUIControl-derived components.  Internal private method.
	void BroadcastMouseUpEventToClass(RenderComponentGroup<iUIControl*> *cclass, bool lmb, bool rmb, Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);

};


#endif