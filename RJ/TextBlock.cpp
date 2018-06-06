#include "ErrorCodes.h"
#include "FastMath.h"
#include "CoreEngine.h"
#include "TextRenderer.h"
#include "TextBlock.h"


// Constructor
TextBlock::TextBlock(void)
	:
	TextBlock(NullString)
{
}

// Constructor
TextBlock::TextBlock(const std::string & code)
	:
	m_text(NullString),
	m_font(0U),
	m_fontsize(12.0f),
	m_basecolour(ONE_FLOAT4),
	m_outlinecolour(ONE_FLOAT4),
	m_outlinefactor(0.5f),
	m_parentrender(true)
{
	SetCode(code);
	SetPosition(NULL_FLOAT2);
	SetRenderActive(true);
}

// Render the text block 
void TextBlock::Render(void)
{
	if (!m_text.empty())
	{
		// TODO: Have base component store the position in XMVECTOR form as well, to save many SSE loads per frame
		Game::Engine->GetTextRenderer()->RenderString(m_text, m_font, DecalRenderingMode::ScreenSpace, XMLoadFloat2(&m_position), 
			m_fontsize, m_basecolour, m_outlinecolour, TextAnchorPoint::TopLeft, m_outlinefactor);
	}
}

// Calculate the rendered size of the text inside this text block
XMFLOAT2 TextBlock::CalculateTextDimensions(void) const
{
	return Game::Engine->GetTextRenderer()->CalculateTextDimensions(m_text, m_font, m_fontsize);
}



void TextBlock::ParentRenderStateChanged(bool parentstate)
{
	// Store the new parent render state
	m_parentrender = parentstate;

	// Re-evaluate the render state of this component based on the new parent state by calling the component render state method
	// TODO: This will not currently work.  Make the accounting for parent visibility part of the iUIComponent base object
	SetRenderActive(m_render);
}
	

// Shutdown method to deallocate all resources assigned to this text block
void TextBlock::Shutdown(void)
{
}


TextBlock::~TextBlock(void)
{
}
