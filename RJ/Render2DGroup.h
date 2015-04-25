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
using namespace std;
using namespace std::tr1;


class Render2DGroup
{
public:
	// Component collections
	struct CG_STRUCT
	{
		RenderComponentGroup<Image2D*>					Image2D;
		RenderComponentGroup<Image2DRenderGroup*>		Image2DGroup;
		RenderComponentGroup<TextBlock*>				TextBlocks;
		RenderComponentGroup<MultiLineTextBlock*>		MultiLineTextBlocks;

		RenderComponentGroup<UIButton*>					Buttons;
		RenderComponentGroup<UITextBox*>				TextBoxes;
		RenderComponentGroup<UIComboBox*>				ComboBoxes;

		RenderComponentGroup<string>					RenderConstants;
		RenderComponentGroup<UIComponentGroup*>			ComponentGroups;
		RenderComponentGroup<RenderMouseEvent*>			MouseEvents;

	} Components;

public:

	typedef unordered_map<std::string, Image2D*> Image2DCollection;
	typedef unordered_map<std::string, Image2DRenderGroup*> Image2DRenderGroupCollection;
	typedef unordered_map<std::string, RenderMouseEvent*> MouseEventCollection;
	typedef unordered_map<std::string, TextBlock*> TextBlockCollection;
	typedef unordered_map<std::string, MultiLineTextBlock*> MultiLineTextBlockCollection;
	typedef unordered_map<std::string, UIButton*> ButtonCollection;
	typedef unordered_map<std::string, UITextBox*> TextBoxCollection;
	typedef unordered_map<std::string, UIComboBox*> ComboBoxCollection;


	typedef unordered_map<std::string, std::string> RenderConstantCollection;
	typedef unordered_map<std::string, UIComponentGroup*> ComponentGroup;

	typedef vector<iUIComponentRenderable*> RenderQueueCollection;

public:
	Render2DGroup(void);
	~Render2DGroup(void);

	CMPINLINE bool GetRenderActive(void) { return m_render; }
	void SetRenderActive(bool render);

	void ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse);

	Render2DManager *GetRenderManager(void);
	void SetRenderManager(Render2DManager *rm);

	// String description of this 2D rendering group
	CMPINLINE string GetDescription(void) { return m_description; }
	CMPINLINE void SetDescription(string text) { m_description = text; }

	// Methods to retrieve components or instances at a specific location
	Image2DRenderGroup::InstanceReference GetComponentInstanceAtLocation(INTVECTOR2 location, bool only_components_which_accept_mouse_input);

	// Shortcut methods to find and return a render constant with the specified key
	CMPINLINE bool HaveConstant(string key) { return (m_constants.count(key) > 0); }
	CMPINLINE string GetConstant(string key) { if (m_constants.count(key) != 0) return m_constants[key]; else return ""; }

	// Methods to execute the control-specific render method for each managed control in the render group, i.e. any more
	// complex rendering not already handled by the Render2DManager etc.
	void PerformManagedControlRendering(iUIControl *focuscontrol);

	// Method to search for a component by code, across all iUIComponent-derived component classes
	iUIComponent *FindUIComponent(string code, string key);

	// Render function, which will process each registered renderable component in turn
	void Render(D3DXMATRIX baseviewmatrix);

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
	string							m_description;

	Image2DCollection				m_components;
	Image2DRenderGroupCollection	m_componentgroups;
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