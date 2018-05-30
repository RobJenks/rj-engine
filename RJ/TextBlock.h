#pragma once

#ifndef __TextBlockH__
#define __TextBlockH__

#include <string>
#include "CompilerSettings.h"
#include "Utility.h"
#include "Font.h"
#include "iUIComponentRenderable.h"



// Class has no special alignment requirements
class TextBlock : public iUIComponentRenderable
{
public:

	// Constructor
	TextBlock(void);
	TextBlock(const std::string & code);

	// Set properties of the text block
	CMPINLINE void SetText(const std::string & text) { m_text = text; }
	CMPINLINE void SetFont(Font::ID font) { m_font = font; }
	CMPINLINE void SetFontSize(float size) { m_fontsize = size; }
	CMPINLINE void SetColour(const XMFLOAT4 & colour) { m_basecolour = m_outlinecolour = colour; }
	CMPINLINE void SetColour(const XMFLOAT4 & basecolour, const XMFLOAT4 & outlinecolour) { m_basecolour = basecolour; m_outlinecolour = outlinecolour; }
	CMPINLINE void SetOutlineFactor(float factor) { m_outlinefactor = factor; }

	// Return text block properties
	CMPINLINE std::string GetText(void) const { return m_text; }
	CMPINLINE Font::ID GetFont(void) const { return m_font; }
	CMPINLINE float GetFontSize(void) const { return m_fontsize; }
	CMPINLINE XMFLOAT4 GetBaseColour(void) const { return m_basecolour; }
	CMPINLINE XMFLOAT4 GetOutlineColour(void) const { return m_outlinecolour; }
	CMPINLINE float GetOutlineFactor(void) const { return m_outlinefactor; }


	// Methods to change the render state of the textblock
	//void SetRenderActive(bool render);
	void ParentRenderStateChanged(bool parentstate);
	bool GetRenderActive(void) const { return m_render; }
	bool IsActuallyVisible(void) { return (m_render && m_parentrender); }

	// Calculate the rendered size of the text inside this text block
	XMFLOAT2			CalculateTextDimensions(void) const;

	// Render the text block 
	void				Render(void);

	
	// Shutdown method to deallocate all resources assigned to this text block
	void				Shutdown(void);

	// Destructor
	~TextBlock(void);


private:
	
	std::string							m_text;
	Font::ID							m_font;
	float								m_fontsize;
	XMFLOAT4							m_basecolour;
	XMFLOAT4							m_outlinecolour;
	float								m_outlinefactor;

	bool								m_parentrender;

};




#endif