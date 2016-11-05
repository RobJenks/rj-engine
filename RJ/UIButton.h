#pragma once

#ifndef __UIButtonH__
#define __UIButtonH__

#include <string>
#include "Utility.h"
#include "iUIControl.h"
#include "Image2DRenderGroup.h"
class TextBlock;
class GameInputDevice;
using namespace std;


// This class has no special alignment requirements
class UIButton : public iUIControl
{

public:
	
	// Static class method that determines whether this control type peforms any additional managed rendering
	CMPINLINE static bool						PerformsManagedRendering(void) { return false; }

	// Constructor
	UIButton(string code,
					 Image2DRenderGroup::InstanceReference upcomponent, 
					 Image2DRenderGroup::InstanceReference downcomponent, 
					 TextBlock *textcomponent,
					 INTVECTOR2 pos, INTVECTOR2 size, bool render);

	// Destructor
	~UIButton(void);

	// Methods to retrieve key properties
	INTVECTOR2									GetPosition(void) { return m_position; }
	INTVECTOR2									GetSize(void) { return m_size; }
	bool										GetRenderActive(void) const { return m_render; }
	INTVECTOR2									GetTextOffset(void) { return m_textoffset; }

	// Methods to retrieve components that make up the button control
	CMPINLINE const Image2DRenderGroup::InstanceReference *	GetUpComponent(void) { return &m_upcomponent; }
	CMPINLINE const Image2DRenderGroup::InstanceReference *	GetDownComponent(void) { return &m_downcomponent; }
	CMPINLINE TextBlock *									GetTextComponent(void) { return m_textcomponent; }

	// Methods to change properties, which will also propogate to component parts as required
	void										SetPosition(INTVECTOR2 pos);
	void										SetSize(INTVECTOR2 size);
	void										SetRenderActive(bool render);
	void										CalculateTextOffset(void);
	void										SetTextOffset(INTVECTOR2 offset);
	void										CentreTextInControl(void);

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

	// Flag determining whether this control can accept keyboard input when it is in focus
	CMPINLINE bool								CanAcceptKeyboardInput(void) { return false; }
	CMPINLINE void								ProcessKeyboardInput(GameInputDevice *keyboard) { }		// No action required

	// Sets the value of the flag that determines whether this control can accept focus
	CMPINLINE void								SetCanAcceptFocus(bool b) { m_canacceptfocus = b; }		

	// Render management methods; one called if the control is in focus, one if it is not (for efficiency; non-focus method can often be compiled out)
	void										RenderControl(void);
	void										RenderControlInFocus(void);

	// Returns a collection of iUIComponents that make up the control
	CMPINLINE iUIControl::ControlComponentCollection *	GetComponents(void) { return &m_components; }

	// Returns the type of control
	CMPINLINE iUIControl::Type					GetType(void) { return iUIControl::Type::Button; }

	// Returns the current state of the control
	CMPINLINE iUIControl::ControlState			GetControlState(void) { return m_state; }

	// Shutdown method to satisfy the interface requirement (only basic components, e.g. Image2Ds & text, need to be disposed of)
	void Shutdown(void) { }


private:

	// Current state of the control
	iUIControl::ControlState					m_state;

	// The components that make up this button
	Image2DRenderGroup::InstanceReference		m_upcomponent;
	Image2DRenderGroup::InstanceReference		m_downcomponent;
	TextBlock *									m_textcomponent;

	// Collection in which to store the component pointers, for more efficient parsing of all constituent components
	vector<iUIComponent*>						m_components;

	// Button properties
	INTVECTOR2									m_position;
	INTVECTOR2									m_size;
	INTVECTOR2									m_textoffset;

	// Determines whether the control can accept focus
	bool										m_canacceptfocus;

};



#endif