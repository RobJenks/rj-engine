#pragma once

#ifndef __UITextBoxH__
#define __UITextBoxH__

#include <string>
#include "iUIControl.h"
#include "Utility.h"
#include "Image2DRenderGroup.h"
class TextBlock;
class GameInputDevice;
using namespace std;


class UITextBox : public iUIControl
{
public:
	// Static class method that determines whether this control type peforms any additional managed rendering
	// The textbox control does perform additional managed rendering, to switch between normal and focus appearances
	CMPINLINE static bool						PerformsManagedRendering(void) { return true; }

	// The margin on the right hand edge of the textbox
	static const int TEXTBOX_RIGHT_MARGIN = 24;

	// Constructor
	UITextBox(	string code, 
				Image2DRenderGroup::InstanceReference framecomponent, 
				Image2DRenderGroup::InstanceReference framefocuscomponent, 
				TextBlock * textcomponent,
				INTVECTOR2 pos, INTVECTOR2 size, bool render );

	// Destructor
	~UITextBox(void);

	// Methods to retrieve key properties
	CMPINLINE INTVECTOR2						GetPosition(void) { return m_position; }
	CMPINLINE INTVECTOR2						GetSize(void) { return m_size; }
	const std::string & 						GetText(void);
	void										SetText(std::string text);
	CMPINLINE bool								GetRenderActive(void) const { return m_render; }
	CMPINLINE INTVECTOR2						GetTextOffset(void) { return m_textoffset; }

	// Methods to retrieve text box components
	CMPINLINE const Image2DRenderGroup::InstanceReference *	GetFrameComponent(void) { return &m_framecomponent; }
	CMPINLINE const Image2DRenderGroup::InstanceReference *	GetFrameFocusComponent(void) { return &m_framefocuscomponent; }
	CMPINLINE TextBlock *									GetTextComponent(void) { return m_textcomponent; }

	// Methods to change properties, which will also propogate to component parts as required
	void										SetPosition(INTVECTOR2 pos);
	void										SetSize(INTVECTOR2 size);
	void										SetRenderActive(bool render);
	void										SetTextOffset(INTVECTOR2 offset);

	// Core control methods, implemented to satisfy the iUIControl interface
	bool										WithinControlBounds(INTVECTOR2 point);
	void										HandleMouseHoverEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);
	void										HandleMouseDownEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);
	void										HandleRightMouseDownEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);
	void										HandleMouseUpEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);
	void										HandleRightMouseUpEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation);

	// Methods to handle mouse clicks on the components making up this control
	void HandleMouseClickEvent(Image2DRenderGroup *componentgroup, Image2DRenderGroup::Instance *component,
							   INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation);
	void HandleMouseRightClickEvent(Image2DRenderGroup *componentgroup, Image2DRenderGroup::Instance *component,
									INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation);

	// Flag determining whether this control can accept user input focus
	CMPINLINE bool								CanAcceptFocus(void) { return m_canacceptfocus; }			

	// Sets the value of the flag that determines whether this control can accept focus
	CMPINLINE void								SetCanAcceptFocus(bool b) { m_canacceptfocus = b; }		

	// Flag determining whether this control can accept keyboard input when it is in focus
	CMPINLINE bool								CanAcceptKeyboardInput(void) { return true; }
	void										ProcessKeyboardInput(GameInputDevice *keyboard);

	// Render management methods; one called if the control is in focus, one if it is not (for efficiency; non-focus method can often be compiled out)
	void										RenderControl(void);
	void										RenderControlInFocus(void);

	// Returns a collection of iUIComponents that make up the control
	CMPINLINE iUIControl::ControlComponentCollection *	GetComponents(void) { return &m_components; }

	// Shutdown method to satisfy the interface requirement (only basic components, e.g. Image2Ds & text, need to be disposed of)
	void Shutdown(void) { }


	// Returns the type of control
	CMPINLINE iUIControl::Type					GetType(void) { return iUIControl::Type::Button; }

	// Returns the current state of the control
	CMPINLINE iUIControl::ControlState			GetControlState(void) { return m_state; }

private:
	
	// Current state of the control
	iUIControl::ControlState					m_state;

	// The components that make up this text box
	Image2DRenderGroup::InstanceReference		m_framecomponent;
	Image2DRenderGroup::InstanceReference		m_framefocuscomponent;
	TextBlock *									m_textcomponent;
	TextBlock *									m_caretcomponent;

	// Collection in which to store the component pointers, for more efficient parsing of all constituent components
	vector<iUIComponent*>						m_components;

	// Text box properties
	INTVECTOR2									m_position;
	INTVECTOR2									m_size;
	INTVECTOR2									m_textoffset;

	// Determines whether the control can accept focus
	bool										m_canacceptfocus;

};



#endif