//-----------------------------------------------------------------------------
// File: DXText.cpp
//
// Desc: Wrapper classes for the Direct X font object
//
//-----------------------------------------------------------------------------

#include "DXText.h"

namespace DXText {

	// Persistent instance of the renderer that we will use for all general-purpose text rendering
	DXText::TextRenderer *Instance = NULL;		

	//-----------------------------------------------------------------------------
	// Name: InitialiseTextRendering()
	// Desc: Initialises the persistent text rendering component that we will use for all general case rendering
	//-----------------------------------------------------------------------------
	void InitialiseTextRendering()
	{
		DXText::Instance = new DXText::TextRenderer();
	}

	//-----------------------------------------------------------------------------
	// Name: TerminateTextRendering()
	// Desc: Disposes of all text rendering components
	//-----------------------------------------------------------------------------
	void TerminateTextRendering()
	{
		DXText::Instance->Release();
		delete DXText::Instance;
		DXText::Instance = NULL;
	}
	

	//-----------------------------------------------------------------------------
	// Name: TextRenderer::TextRenderer()
	// Desc: TextRenderer constructor.  Initialises a new instance of font/textmanager objects
	//-----------------------------------------------------------------------------
	TextRenderer::TextRenderer()
	{
		this->DrawFont = new DXText::Font();
		this->Renderer = new DXText::TextManager();
	}

	
	//-----------------------------------------------------------------------------
	// Name: TextRenderer::TextRenderer()
	// Desc: Releases all resources associated with the instance.  
	//       Releases the instances of font/textmanager objects
	//-----------------------------------------------------------------------------
	void TextRenderer::Release()
	{
		delete this->DrawFont;
		this->DrawFont = NULL;
		this->Renderer->Release();
		delete this->Renderer;
		this->Renderer = NULL;
	}

	//-----------------------------------------------------------------------------
	// Name: TextRenderer::~TextRenderer()
	// Desc: Destructor.  Set pointers to null
	//-----------------------------------------------------------------------------
	TextRenderer::~TextRenderer() 
	{ 
		this->DrawFont = NULL;
		this->Renderer = NULL;
	}

	//-----------------------------------------------------------------------------
	// Name: Font::Font()
	// Desc: Font constructor.  Sets a few default values
	//-----------------------------------------------------------------------------
	Font::Font()
	{
		fontSize = 12;
		italic = false;
		halign = DXText::HALIGN::Centre_HA;
		valign = DXText::VALIGN::Top_VA;
		font = "Arial";
		color = D3DCOLOR_XRGB(0, 0, 0);
	}

	//-----------------------------------------------------------------------------
	// Name: Font::SetSize()
	// Desc: Sets the size of the font
	//-----------------------------------------------------------------------------
	void Font::SetSize(int size)
	{
		fontSize = size;
	}

	//-----------------------------------------------------------------------------
	// Name: Font::GetSize()
	// Desc: Returns the size of the font
	//-----------------------------------------------------------------------------
	int Font::GetSize()	
	{
		return fontSize;
	}

	//-----------------------------------------------------------------------------
	// Name: Font::SetColorRGB()
	// Desc: Sets the color of the font using RGB values
	//-----------------------------------------------------------------------------
	void Font::SetColorRGB(int red, int green, int blue)
	{
		color = D3DCOLOR_XRGB(red, green, blue);
	}

	//-----------------------------------------------------------------------------
	// Name: Font::GetColorRGB()
	// Desc: Returns the color of the font
	//-----------------------------------------------------------------------------
	D3DCOLOR Font::GetColor()
	{
		return color;
	}

	//-----------------------------------------------------------------------------
	// Name: Font::Italic()
	// Desc: Sets italics to true or false
	//-----------------------------------------------------------------------------
	void Font::Italic(bool boolItalic)
	{
		italic = boolItalic;
	}

	//-----------------------------------------------------------------------------
	// Name: Font::Italic()
	// Desc: Returns true or false
	//-----------------------------------------------------------------------------
	bool Font::GetItalic()
	{
		return italic;
	}

	// Set horizontal alignment
	void Font::SetHAlign(HALIGN Alignment)
	{
		halign = Alignment;
	}

	// Get horizontal alignment
	HALIGN Font::GetHAlign()
	{
		return halign;
	}

	// Set vertical alignment
	void Font::SetVAlign(VALIGN Alignment)
	{
		valign = Alignment;
	}

	// Get vertical alignment
	VALIGN Font::GetVAlign()
	{
		return valign;
	}


	//-----------------------------------------------------------------------------
	// Name: Font::SetFont()
	// Desc: Sets the font using the system fonts
	//-----------------------------------------------------------------------------
	void Font::SetFont(string textFont)
	{
		font = textFont;
	}


	//-----------------------------------------------------------------------------
	// Name: Font::GetFont()
	// Desc: Returns the font
	//-----------------------------------------------------------------------------
	string Font::GetFont()
	{
		return font;
	}



	//-----------------------------------------------------------------------------
	// Name: TextManager::TextManager()
	// Desc: TextManager constructor
	//-----------------------------------------------------------------------------
	TextManager::TextManager()
	{
		font = NULL;
	}


	//-----------------------------------------------------------------------------
	// Name: TextManager::SetTextBoxBoundaries()
	// Desc: Sets where the text will be drawn
	//-----------------------------------------------------------------------------
	void TextManager::SetTextBoxBoundaries(int x, int y, int width, int height)
	{
		textRect.top = x;
		textRect.left = y;
		textRect.right = textRect.left + width;
		textRect.bottom = textRect.top + height;

		xPos = x;
		yPos = y;
		rectWidth = width;
		rectHeight = height;
	}

	//-----------------------------------------------------------------------------
	// Name: TextManager::GetTextBoxBoundaries()
	// Desc: Returns the position in a RECT struct
	//-----------------------------------------------------------------------------
	RECT TextManager::GetTextBoxBoundaries()
	{
		return textRect;
	}

	//-----------------------------------------------------------------------------
	// Name: TextManager::GetTextBoxXPos()
	// Desc: Returns the X position of the box
	//-----------------------------------------------------------------------------
	int TextManager::GetTextBoxXPos()
	{
		return xPos;
	}

	//-----------------------------------------------------------------------------
	// Name: TextManager::GetTextBoxYPos()
	// Desc: Returns the Y position of the box
	//-----------------------------------------------------------------------------
	int TextManager::GetTextBoxYPos()
	{
		return yPos;
	}

	//-----------------------------------------------------------------------------
	// Name: TextManager::GetTextBoxWidth()
	// Desc: Returns the width of the box
	//-----------------------------------------------------------------------------
	int TextManager::GetTextBoxWidth()
	{
		return rectWidth;
	}

	//-----------------------------------------------------------------------------
	// Name: TextManager::GetTextBoxHeight()
	// Desc: Returns the height of the box
	//-----------------------------------------------------------------------------
	int TextManager::GetTextBoxHeight()
	{
		return rectHeight;
	}

	//-----------------------------------------------------------------------------
	// Name: TextManager::DrawText()
	// Desc: Draws the text to the screen using the font object
	//-----------------------------------------------------------------------------
	void TextManager::DrawText(Font fontObj, LPDIRECT3DDEVICE11 dxDevice, LPCTSTR text)
	{
		string fontName = "Arial";

		fontDesc.Height = fontObj.GetSize();
		fontDesc.Width = 0;
		fontDesc.Weight = 400;
		fontDesc.MipLevels = 0;
		fontDesc.Italic = fontObj.GetItalic();
		fontDesc.CharSet = DEFAULT_CHARSET;
		fontDesc.OutputPrecision = OUT_TT_PRECIS;
		fontDesc.Quality = CLIP_DEFAULT_PRECIS;
		fontDesc.PitchAndFamily = DEFAULT_PITCH;
		strcpy(fontDesc.FaceName, fontObj.GetFont().c_str());

		D3DXCreateFontIndirect(dxDevice, &fontDesc, &font);

		font->DrawText(NULL,
			text,
			-1,
			&textRect,
			fontObj.GetHAlign() | fontObj.GetVAlign() | DT_WORDBREAK,
			fontObj.GetColor());

		font->Release(); font = NULL;
	}

	//-----------------------------------------------------------------------------
	// Name: TextManager::Release()
	// Desc: Destroy the font object
	//-----------------------------------------------------------------------------
	void TextManager::Release()
	{
		if (font != NULL)
		{
			font->Release();
			font = NULL;
		}
	}

	//-----------------------------------------------------------------------------
	// Name: TextManager::~TextManager()
	// Desc: Deconstructor
	//-----------------------------------------------------------------------------
	TextManager::~TextManager()
	{
		Release();
	}
}