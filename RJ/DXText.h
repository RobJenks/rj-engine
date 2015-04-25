#ifndef DXTEXT_H
#define DXTEXT_H

#include "DX11_Core.h"
#include <string>

using namespace std;

//-----------------------------------------------------------------------------
// Classes contained in this header file
//-----------------------------------------------------------------------------
class TextManager;
class Font;
class TextRenderer;

namespace DXText {

	enum HALIGN { Left_HA = DT_LEFT, Centre_HA = DT_CENTER, Right_HA = DT_RIGHT  };
	enum VALIGN { Top_VA = DT_TOP, Centre_VA = DT_VCENTER, Bottom_VA = DT_BOTTOM };

	//-----------------------------------------------------------------------------
	// Name: class Font
	// Desc: Specifies the font, color, and size of the text to be drawn
	//-----------------------------------------------------------------------------
	class Font
	{
		public:
			Font();
			~Font() {};
			void SetSize(int);
			int GetSize();
			void SetColorRGB(int, int, int);
			D3DCOLOR GetColor();
			void Italic(bool);
			bool GetItalic();
			void SetHAlign(HALIGN);
			HALIGN GetHAlign();
			void SetVAlign(VALIGN);
			VALIGN GetVAlign();
			void SetFont(string);
			string GetFont();

		private:
			int fontSize;
			bool italic;
			HALIGN halign;
			VALIGN valign;
			string font;
			D3DCOLOR color;
	};




	//-----------------------------------------------------------------------------
	// Name: class TextManager
	// Desc: Sets up the area of the screen to draw on, and draws the text
	//-----------------------------------------------------------------------------
	class TextManager
	{
		public:
			TextManager();
			void SetTextBoxBoundaries(int, int, int, int);
			RECT GetTextBoxBoundaries();
			int GetTextBoxXPos();
			int GetTextBoxYPos();
			int GetTextBoxWidth();
			int GetTextBoxHeight();
			void DrawText(Font, LPDIRECT3DDEVICE11, LPCTSTR);
			void Release();
			~TextManager();

		private:
			LPD3DXFONT font;
			D3DXFONT_DESC fontDesc;
			RECT textRect;
			int xPos;
			int yPos;
			int rectWidth;
			int rectHeight;
	};


	//-----------------------------------------------------------------------------
	// Name: class Instance
	// Desc: A persistent instance used to render text into the application buffer
	//-----------------------------------------------------------------------------
	class TextRenderer
	{
		public:
			TextRenderer();
			~TextRenderer();
			void Release();

			DXText::Font *DrawFont;
			DXText::TextManager *Renderer;
	};


	
	// Persistent instance declaration, and initialisation function to generate the instance
	extern DXText::TextRenderer *Instance;
	void InitialiseTextRendering();
	void TerminateTextRendering();

}
#endif
