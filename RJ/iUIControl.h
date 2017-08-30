#pragma once

#ifndef __iUIControlH__
#define __iUIControlH__

#include <vector>
#include "Utility.h"
#include "iUIComponent.h"
#include "Image2DRenderGroup.h"
class UIManagedControlDefinition;
class GameInputDevice;

class iUIControl : public iUIComponent
{

public:

	enum Type { Unknown = 0, Button, Textbox, Combobox };
	enum ControlState { Default = 0, LMBDown, RMBDown };

	// Enumeration of different scrolling states for a control
	enum ScrollState { None = 0, ScrollUp, ScrollDown, ScrollLeft, ScrollRight };

	// Standard collection of components that make up a control
	typedef std::vector<iUIComponent*> ControlComponentCollection;

	virtual bool WithinControlBounds(INTVECTOR2 point) = 0;

	virtual Type GetType(void) = 0;
	virtual ControlState GetControlState(void) = 0;

	virtual bool CanAcceptFocus(void) = 0;			// Flag determining whether this control can accept user input focus
	virtual void SetCanAcceptFocus(bool b) = 0;		// Sets the value of this focus flag

	virtual void RenderControl(void) = 0;				// Called to render each control that is not in focus
	virtual void RenderControlInFocus(void) = 0;		// Called to render the control that is currently in focus

	// Methods to handle mouse events on the components making up this control
	virtual void HandleMouseHoverEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation) = 0;
	virtual void HandleMouseDownEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation) = 0;
	virtual void HandleRightMouseDownEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation) = 0;
	virtual void HandleMouseUpEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation) = 0;
	virtual void HandleRightMouseUpEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 mouselocation) = 0;

	// Methods to handle mouse clicks on the components making up this control
	virtual void HandleMouseClickEvent(Image2DRenderGroup *componentgroup, Image2DRenderGroup::Instance *component,
									   INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation) = 0;
	virtual void HandleMouseRightClickEvent(Image2DRenderGroup *componentgroup, Image2DRenderGroup::Instance *component,
											INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation) = 0;

	// Methods for processing UI input
	virtual bool CanAcceptKeyboardInput(void) = 0;
	virtual void ProcessKeyboardInput(GameInputDevice *keyboard) = 0;

	// Returns a collection of iUIComponents that make up the control
	virtual ControlComponentCollection *GetComponents(void) = 0;

	// Derives a control type from its string representation
	static iUIControl::Type DeriveType(std::string typestring);

	// Generates a new unique control ID
	static int iUIControl::GenerateControlID(void);

protected:

	// Method to initialise an image component for this control with default parameters
	Result InitialiseImageComponentDefault(UIManagedControlDefinition *def, Image2DRenderGroup **component,
		std::string componentname, int instancecount, float zorder);

};




#endif