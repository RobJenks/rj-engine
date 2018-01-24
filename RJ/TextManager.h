#pragma once

#ifndef __TextManagerH__
#define __TextManagerH__

#include <vector>
#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "SentenceType.h"
class FontData;
class FontShader;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class TextManager : public ALIGN16<TextManager>
{
private:
	typedef UINT16 INDEXFORMAT;		

	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:

	TextManager(void);
	TextManager(const TextManager&);
	~TextManager();

	// Initialises the text manager
	Result RJ_XM_CALLCONV Initialize(HWND, int, int, const FXMMATRIX, FontShader*);

	// Initialises a new font data object.  ID of the new font is returned through fontID, if successfull (return value == 0)
	Result InitializeFont(std::string name, const char *fontdata, const char *fonttexture, int &fontID);

	// Creates a new sentence object, allocating memory and building a vertex buffer
	SentenceType *CreateSentence(int fontID, int maxlength);

	// Updates a sentence to set the text, position, colour and render flag as necessary
	Result UpdateSentence(SentenceType* sentence, const char *text, int xpos, int ypos, bool render, 
							const XMFLOAT4 & textcolour, float size);

	// Renders all text sentences (dependent on render flag) using the font shader instance
	Result RJ_XM_CALLCONV Render(const FXMMATRIX worldMatrix, const CXMMATRIX orthoMatrix);

	// Shuts down the text manager instance and releases all resources (including fonts and text sentences)
	void Shutdown(void);

	// Sets properties of a sentence that do not require full regeneration of the vertex buffers: colour & render flag
	CMPINLINE void EnableSentenceRendering(SentenceType *sentence) { sentence->render = true; }
	CMPINLINE void DisableSentenceRendering(SentenceType *sentence) { sentence->render = false; }
	CMPINLINE void SetSentenceRendering(SentenceType *sentence, bool render) { sentence->render = render; }
	CMPINLINE void SetSentencePosition(SentenceType *sentence, int x, int y) { sentence->x = x; sentence->y = y; }
	CMPINLINE void SetSentenceColour(SentenceType *sentence, XMFLOAT4 colour) { sentence->colour = colour; }

	// Set the sentence text, which invokes regeneration of the vertex buffers
	Result SetSentenceText(SentenceType *sentence, const char *text, float size);



private:
	Result InitializeSentence(SentenceType** sentence, int maxLength, int fontID);
	void ReleaseSentence(SentenceType**);
	Result RJ_XM_CALLCONV RenderSentence(SentenceType*, const FXMMATRIX, const CXMMATRIX);

private:
	std::vector<FontData*> m_fonts;
	std::vector<SentenceType*> m_sentences;

	int m_screenWidth, m_screenHeight;
	float m_sentencewidth;
	
	AXMMATRIX m_baseViewMatrix;
	FontShader *m_fontshader;

};

#endif