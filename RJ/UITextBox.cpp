#include "iUIControl.h"
#include "TextManager.h"
#include "Image2DRenderGroup.h"
#include "TextBlock.h"
#include "Utility.h"
#include "GameDataExtern.h"
#include "GameInput.h"
#include "UserInterface.h"
#include "iUIController.h"
#include "UITextBox.h"

UITextBox::UITextBox(std::string code,
						Image2D *framecomponent, 
						Image2D *framefocuscomponent, 
						TextBlock *textcomponent,
						INTVECTOR2 pos, INTVECTOR2 size, bool render )
{
	// Store references to these objects; 
	m_code = code;
	m_state = iUIControl::ControlState::Default;
	m_framecomponent = framecomponent;
	m_framefocuscomponent = framefocuscomponent;
	m_textcomponent = textcomponent;

	// Also populate the component collection with these pointers
	m_components.clear();
	m_components.push_back(m_framecomponent);
	m_components.push_back(m_framefocuscomponent);
	m_components.push_back(m_textcomponent);

	// Calculate the offset between textbox location and text location, to be maintained in any further position changes
	if (m_textcomponent && m_framecomponent)
		m_textoffset = (m_textcomponent->GetPosition() - INTVECTOR2(m_framecomponent->GetPosition()));
	else
		m_textoffset = INTVECTOR2(0, 0);

	// Now set properties of these components as per the default button state
	SetPosition(pos);
	SetSize(size);
	SetRenderActive(render);
	SetCanAcceptFocus(true);
}


void UITextBox::SetPosition(INTVECTOR2 pos)
{
	// Store the new position
	m_position = pos;

	// Set the position of all components to match
	if (m_framecomponent) m_framecomponent->SetPosition(pos.ToFloat());

	// Also incorporate the text offset when positioning the text block
	if (m_textcomponent) 
	{
		INTVECTOR2 pos = m_textcomponent->GetPosition();
		std::string text = m_textcomponent->GetText();
		m_textcomponent->UpdateTextBlock(text.c_str(), pos.x, pos.y, m_textcomponent->GetRenderActive(),
										 m_textcomponent->GetTextColour(), m_textcomponent->GetSize());
	}
}

void UITextBox::SetSize(INTVECTOR2 size)
{
	// Store the new size
	m_size = size;

	// Set the size of all components to match
	if (m_framecomponent) m_framecomponent->SetSize(size.ToFloat());
}

void UITextBox::SetRenderActive(bool render)
{
	// Store the new render state
	m_render = render;

	// Set the render value of each component according to the overall button render state
	if (m_render)
	{
		if (m_framecomponent) m_framecomponent->SetRenderActive(m_render);
		if (m_framefocuscomponent) m_framefocuscomponent->SetRenderActive(false);
		if (m_textcomponent) m_textcomponent->SetRenderActive(true);
	}
	else 
	{
		if (m_framecomponent) m_framecomponent->SetRenderActive(false);
		if (m_framefocuscomponent) m_framefocuscomponent->SetRenderActive(false);
		if (m_textcomponent) m_textcomponent->SetRenderActive(false);
	}
}

void UITextBox::SetTextOffset(INTVECTOR2 offset)
{
	// Make sure this offset is actually different, otherwise quit here for efficiency
	if (offset == m_textoffset) return;

	// Store the new text offset
	m_textoffset.x = offset.x; m_textoffset.y = offset.y;

	// Now reposition the text block according to this new offset
	m_textcomponent->UpdateTextBlock(m_textcomponent->GetText().c_str(), (m_position.x + m_textoffset.x), (m_position.y + m_textoffset.y), 
									 m_textcomponent->GetRenderActive(), m_textcomponent->GetTextColour(), m_textcomponent->GetSize());
}

// Returns the text currently held in this textbox control
const std::string & UITextBox::GetText(void)
{
	if (m_textcomponent)
		return m_textcomponent->GetText();
	else
		return NullString;
}

// Directly sets the text held in this textbox control
void UITextBox::SetText(std::string text)
{
	// Set the underlying text data
	if (m_textcomponent)
	{
		m_textcomponent->SetText(text);
	}

	// Raise an event to inform our UI controller that the textbox contents have changed (or at least been set)
	// TODO: This is a hack in order to get back to the active UI controller and pass on the event.  No guarantee
	// that this control is actually part of that controller (but likely, given that it is being updated)
	if (D::UI && D::UI->GetActiveUIController())
	{
		D::UI->GetActiveUIController()->ProcessTextboxChangedEvent(this);
	}
}

// Directly sets the text held in this textbox control, without raising any associated events or notifying any controllers
void UITextBox::SetTextSilent(const std::string & text)
{
	// Set the underlying text data
	if (m_textcomponent)
	{
		m_textcomponent->SetText(text);
	}
}

// Determines whether the supplied point lies within the control bounds; part of the iUIControl interface
bool UITextBox::WithinControlBounds(INTVECTOR2 point)
{
	return (point.x >= m_position.x && point.y >= m_position.y && 
			point.x <= (m_position.x + m_size.x) && 
			point.y <= (m_position.y + m_size.y) );
}

// Handles changes in control state in response to a mouse hover event over the control
void UITextBox::HandleMouseHoverEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
}

// Handles changes in control state in response to a mouse down event over the control
void UITextBox::HandleMouseDownEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
}

// Handles changes in control state in response to a right mouse down event over the control
void UITextBox::HandleRightMouseDownEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
}

// Handles changes in control state in response to a mouse up event
void UITextBox::HandleMouseUpEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
}

// Handles changes in control state in response to a right mouse up event
void UITextBox::HandleRightMouseUpEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
}

// Event handler for left mouse clicks on the components making up this control
void UITextBox::HandleMouseClickEvent(iUIComponent *component, INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation)
{
}

// Event handler for right mouse clicks on the components making up this control
void UITextBox::HandleMouseRightClickEvent(iUIComponent *component, INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation)
{
}

void UITextBox::RenderControl(void) 
{
	// Render the normal textures for this control when it is not in focus
	m_framecomponent->SetRenderActive(m_render);
	m_framefocuscomponent->SetRenderActive(false);
}

void UITextBox::RenderControlInFocus(void)
{
	// Render the focus textures for this control when it is the active control in focus
	m_framecomponent->SetRenderActive(false);
	m_framefocuscomponent->SetRenderActive(m_render);
}

void UITextBox::ProcessKeyboardInput(GameInputDevice *keyboard)
{
	// See whether we have an alphanumeric key being pressed that is eligble for text entry
	GameInputDevice::InputKey key = keyboard->ReadTextInputKey();
	if (key.keycode != 0)
	{
		// Make sure we are under the textbox width limit, otherwise do not add this character
		// TODO: Textbox should scroll content within available area, not just prevent new characters
		// if (m_textcomponent->GetTextWidth() < (float)(m_size.x - m_textoffset.x - UITextBox::TEXTBOX_RIGHT_MARGIN))
		{
			// Add to the textbox string
			std::string newtext = m_textcomponent->GetText();
			newtext.push_back(key.keychar);
			SetText( newtext );
		}

		// The textbox should consume this key so that it is not also registered by in-game objects
		keyboard->ConsumeKey(key.keycode);
	}
	else if (keyboard->GetKeys()[DIK_BACKSPACE])
	{
		// Also process backspace for deletion of chars in the string
		std::string newtext = m_textcomponent->GetText();
		if (newtext.size() > 0) 
		{
			// Reduce the text by one character
			newtext = newtext.substr(0, newtext.size()-1);
			SetText(newtext);

			// Lock the backspace key to avoid repeated processing each frame, until the next key-up
			keyboard->LockKey(DIK_BACKSPACE);

			// The textbox should consume this key so that it is not also registered by in-game objects
			keyboard->ConsumeKey(DIK_BACKSPACE);
		}
	}
}



UITextBox::~UITextBox(void)
{
}
