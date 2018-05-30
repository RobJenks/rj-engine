#include <string>
#include "Utility.h"
#include "FastMath.h"
#include "Image2D.h"
#include "TextManager.h"
#include "TextBlock.h"
#include "UIButton.h"

UIButton::UIButton(std::string code,
					Image2D *upcomponent, 
					Image2D *downcomponent, 
					TextBlock *textcomponent, 
					INTVECTOR2 pos, INTVECTOR2 size, bool render)
{
	// Store references to these objects; 
	m_code = code;
	m_state = iUIControl::ControlState::Default;
	m_upcomponent = upcomponent;
	m_downcomponent = downcomponent;
	m_textcomponent = textcomponent;

	// Also store references to these pointers in the component collection
	m_components.clear();
	m_components.push_back(m_upcomponent);
	m_components.push_back(m_downcomponent);
	m_components.push_back(m_textcomponent);

	// Now set properties of these components as per the default button state
	SetPosition(pos);
	SetSize(size);
	SetRenderActive(render);
	SetCanAcceptFocus(true);

	// If we want the button text to be centred, work out the text position now so any offset can be incorporated during button setup
	// Note: we will perform text centring by default, which can then be altered via SetTextOffset() later if required
	// Note: this also calls the RecalculateTextPosition() method, so no longer any need to call it directly here
	CentreTextInControl();
}

// Centres the text in this control, both horizontally and vertically
void UIButton::CentreTextInControl(void)
{
	// We can only perform centring if we have a text and a button frame component to manipulate
	if (m_textcomponent && m_upcomponent)
	{
		// Get the size of the text string and the component
		INTVECTOR2 csize(m_upcomponent->GetSize());
		XMFLOAT2 textsize = m_textcomponent->CalculateTextDimensions();


		// Now determine the relative left & top position for the text based on the relative size of the control itself
		XMFLOAT2 offset(
			(csize.x / 2.0f) - (textsize.x / 2.0f),			// Calculate x offset
			(csize.y / 2.0f) - (textsize.y / 2.0f) + 2.0f   // Calculate y offset (note: includes small offset to account for misalignment)
		);

		// Set the text position using these offsets
		m_textcomponent->SetPosition(Float2Add(m_upcomponent->GetPosition(), offset));

		// Call the method to recalculate the text offset, so that the text moves with this control if the position changes
		CalculateTextOffset();
	}
}

// Recalculates the text offset based on the relative position of button and text controls
void UIButton::CalculateTextOffset(void)
{
	if (m_textcomponent && m_upcomponent)
		m_textoffset = (INTVECTOR2(m_textcomponent->GetPosition()) - INTVECTOR2(m_upcomponent->GetPosition()));
	else
		m_textoffset = INTVECTOR2(0, 0);
}

void UIButton::SetPosition(INTVECTOR2 pos)
{
	// Store the new position
	m_position = pos;
	XMFLOAT2 fpos = pos.ToFloat();

	// Set the position of all components to match
	if (m_upcomponent) m_upcomponent->SetPosition(fpos);
	if (m_downcomponent) m_downcomponent->SetPosition(fpos);

	// Also incorporate the text offset when positioning the text block
	if (m_textcomponent) 
	{
		m_textcomponent->SetPosition(Float2Add(fpos, m_textoffset.ToFloat()));
	}
}

void UIButton::SetSize(INTVECTOR2 size)
{
	// Store the new size
	m_size = size;

	// Set the size of all components to match
	if (m_upcomponent) m_upcomponent->SetSize(size.ToFloat());
	if (m_downcomponent) m_downcomponent->SetSize(size.ToFloat());
}

void UIButton::SetRenderActive(bool render)
{
	// Store the new render state
	m_render = render;

	// Set the render value of each component according to the overall button render state
	if (m_render)
	{
		if (m_upcomponent) m_upcomponent->SetRenderActive(true);
		if (m_downcomponent) m_downcomponent->SetRenderActive(false);
		if (m_textcomponent) m_textcomponent->SetRenderActive(true);
	}
	else 
	{
		if (m_upcomponent) m_upcomponent->SetRenderActive(false);
		if (m_downcomponent) m_downcomponent->SetRenderActive(false);
		if (m_textcomponent) m_textcomponent->SetRenderActive(false);
	}
}

void UIButton::SetTextOffset(INTVECTOR2 offset)
{
	// Make sure this offset is actually different, otherwise quit here for efficiency
	if (offset == m_textoffset) return;

	// Store the new text offset
	m_textoffset.x = offset.x; m_textoffset.y = offset.y;

	// Now reposition the text block according to this new offset
	m_textcomponent->SetPosition((m_position + m_textoffset).ToFloat());
}

// Determines whether the supplied point lies within the control bounds; part of the iUIControl interface
bool UIButton::WithinControlBounds(INTVECTOR2 point)
{
	return (point.x >= m_position.x && point.y >= m_position.y && 
			point.x <= (m_position.x + m_size.x) && 
			point.y <= (m_position.y + m_size.y) );
}

// Handles changes in control state in response to a mouse hover event over the control
void UIButton::HandleMouseHoverEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{

}

// Handles changes in control state in response to a mouse down event over the control
void UIButton::HandleMouseDownEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
	// Only take action if we are not currently in this state
	if (m_state == iUIControl::ControlState::LMBDown) return;

	// Change our current state to reflect the left mouse being down
	m_state = iUIControl::ControlState::LMBDown;

	// Set the button appearance to reflect the LMB being held down
	if (m_upcomponent)		m_upcomponent->SetRenderActive(false);
	if (m_downcomponent)	m_downcomponent->SetRenderActive(m_render);
}

// Handles changes in control state in response to a right mouse down event over the control
void UIButton::HandleRightMouseDownEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
	// Normal buttons do not respond to a RMB-down event
}

// Handles changes in control state in response to a mouse up event
void UIButton::HandleMouseUpEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
	// Only take action if we are not currently in this state
	if (m_state == iUIControl::ControlState::Default) return;

	// Change our current state to reflect the left mouse being released
	m_state = iUIControl::ControlState::Default;

	// Set the button appearance to reflect the LMB being released down
	if (m_upcomponent)		m_upcomponent->SetRenderActive(m_render);
	if (m_downcomponent)	m_downcomponent->SetRenderActive(false);
}

// Handles changes in control state in response to a right mouse up event
void UIButton::HandleRightMouseUpEvent(iUIComponent *component, INTVECTOR2 mouselocation)
{
	// Normal buttons do not respond to a RMB-down event
}


// Event handler for left mouse clicks on the components making up this control
void UIButton::HandleMouseClickEvent(iUIComponent *component, INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation)
{
}

// Event handler for right mouse clicks on the components making up this control
void UIButton::HandleMouseRightClickEvent(iUIComponent *component, INTVECTOR2 mouselocation, INTVECTOR2 mousestartlocation)
{
}

void UIButton::RenderControl(void) 
{
	// There is no additional rendering required for buttons that are out of focus; will not even be called by the UI manager
}

void UIButton::RenderControlInFocus(void)
{
	// There is no additional rendering required for buttons that are in focus;  will not even be called by the UI manager
}

UIButton::~UIButton(void)
{
}
