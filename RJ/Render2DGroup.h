#pragma once

#ifndef __Render2DGroupH__
#define __Render2DGroupH__

#include <string>
#include <vector>
#include <unordered_map>
#include "CompilerSettings.h"
#include "Image2D.h"
#include "Image2DRenderGroup.h"
#include "RenderComponentGroup.h"
class TextBlock;
class MultiLineTextBlock;
class UIButton;
class UITextBox;
class UIComboBox;
class GameInputDevice;
class Render2DManager;
class RenderMouseEvent;
class UIComponentGroup;


// This class has no special alignment requirements
class Render2DGroup
{
public:
	// Component collections
	struct CG_STRUCT
	{
		RenderComponentGroup<Image2D*>					Image2D;
		RenderComponentGroup<TextBlock*>				TextBlocks;
		RenderComponentGroup<MultiLineTextBlock*>		MultiLineTextBlocks;

		RenderComponentGroup<UIButton*>					Buttons;
		RenderComponentGroup<UITextBox*>				TextBoxes;
		RenderComponentGroup<UIComboBox*>				ComboBoxes;

		RenderComponentGroup<std::string>				RenderConstants;
		RenderComponentGroup<UIComponentGroup*>			ComponentGroups;
		RenderComponentGroup<RenderMouseEvent*>			MouseEvents;

	} Components;

public:

	typedef std::unordered_map<std::string, Image2D*> Image2DCollection;
	typedef std::unordered_map<std::string, RenderMouseEvent*> MouseEventCollection;
	typedef std::unordered_map<std::string, TextBlock*> TextBlockCollection;
	typedef std::unordered_map<std::string, MultiLineTextBlock*> MultiLineTextBlockCollection;
	typedef std::unordered_map<std::string, UIButton*> ButtonCollection;
	typedef std::unordered_map<std::string, UITextBox*> TextBoxCollection;
	typedef std::unordered_map<std::string, UIComboBox*> ComboBoxCollection;


	typedef std::unordered_map<std::string, std::string> RenderConstantCollection;
	typedef std::unordered_map<std::string, UIComponentGroup*> ComponentGroup;

	typedef std::vector<iUIComponentRenderable*> RenderQueueCollection;

public:
	Render2DGroup(void);
	~Render2DGroup(void);

	CMPINLINE bool GetRenderActive(void) { return m_render; }
	void SetRenderActive(bool render);

	void ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse);

	Render2DManager *GetRenderManager(void);
	void SetRenderManager(Render2DManager *rm);

	// String description of this 2D rendering group
	CMPINLINE std::string GetDescription(void) { return m_description; }
	CMPINLINE void SetDescription(std::string text) { m_description = text; }

	// Methods to retrieve components or instances at a specific location
	iUIComponent * GetComponentAtLocation(XMFLOAT2 location, bool only_components_which_accept_mouse_input);

	// Shortcut methods to find and return a render constant with the specified key
	CMPINLINE bool HaveConstant(std::string key) { return (m_constants.count(key) > 0); }
	CMPINLINE std::string GetConstant(std::string key) { if (m_constants.count(key) != 0) return m_constants[key]; else return ""; }

	// Methods to execute the control-specific render method for each managed control in the render group, i.e. any more
	// complex rendering not already handled by the Render2DManager etc.
	void PerformManagedControlRendering(iUIControl *focuscontrol);

	// Method to search for a component by code, across all iUIComponent-derived component classes
	iUIComponent *FindUIComponent(const std::string & code);

	// Render function, which will process each registered renderable component in turn
	void RJ_XM_CALLCONV Render(void);

	// Methods to add and remove specific renderable components, so they are handled correctly by the render queue
	bool RegisterRenderableComponent(iUIComponentRenderable *component);
	bool UnregisterRenderableComponent(iUIComponentRenderable *component);
	bool UpdateRenderableComponent(iUIComponentRenderable *component);

	// Terminates the render group
	void Shutdown(void);
	
private:

	// Private method to perform managed control rendering for a specific control class
	void PerformManagedControlRenderingOnControlClass(iUIControl *focuscontrol, RenderComponentGroup<iUIControl*> *controlgroup);


private:
	Render2DManager *				m_rendermanager;

	bool							m_render;
	std::string						m_description;

	Image2DCollection				m_components;
	MouseEventCollection			m_mouseevents;
	TextBlockCollection				m_textblocks;
	MultiLineTextBlockCollection	m_multilinetextblocks;
	ButtonCollection				m_buttons;
	TextBoxCollection				m_textboxes;
	ComboBoxCollection				m_comboboxes;
	
	RenderConstantCollection		m_constants;
	ComponentGroup					m_groups;

	RenderQueueCollection			m_renderqueue;

};


#endif