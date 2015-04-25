#pragma once

#ifndef __TextManagerH__
#define __TextManagerH__

#include <vector>
#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "CompilerSettings.h"
class FontData;
class FontShader;
class DXLocaliser;
using namespace std;

class TextManager
{
private:
	typedef UINT16 INDEXFORMAT;		

	struct VertexType
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};

public:
	struct SentenceType
	{
		bool render;
		int x, y;
		D3DXVECTOR4 colour;
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		int vertexCount, indexCount, maxLength;
		int fontID;
		float sentencewidth, sentenceheight;
	};

	TextManager(const DXLocaliser *locale);
	TextManager(const TextManager&);
	~TextManager();

	// Initialises the text manager
	Result Initialize(ID3D11Device*, ID3D11DeviceContext*, HWND, int, int, D3DXMATRIX, FontShader*);

	// Initialises a new font data object.  ID of the new font is returned through fontID, if successfull (return value == 0)
	Result InitializeFont(string name, const char *fontdata, const char *fonttexture, int &fontID);

	// Creates a new sentence object, allocating memory and building a vertex buffer
	SentenceType *CreateSentence(int fontID, int maxlength);

	// Updates a sentence to set the text, position, colour and render flag as necessary
	Result UpdateSentence(SentenceType* sentence, char* text, int xpos, int ypos, bool render, 
							D3DXVECTOR4 textcolour, float size);

	// Renders all text sentences (dependent on render flag) using the font shader instance
	Result Render(D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix);

	// Shuts down the text manager instance and releases all resources (including fonts and text sentences)
	void Shutdown(void);

	// Sets properties of a sentence that do not require full regeneration of the vertex buffers: colour & render flag
	CMPINLINE void EnableSentenceRendering(SentenceType *sentence) { sentence->render = true; }
	CMPINLINE void DisableSentenceRendering(SentenceType *sentence) { sentence->render = false; }
	CMPINLINE void SetSentenceRendering(SentenceType *sentence, bool render) { sentence->render = render; }
	CMPINLINE void SetSentencePosition(SentenceType *sentence, int x, int y) { sentence->x = x; sentence->y = y; }
	CMPINLINE void SetSentenceColour(SentenceType *sentence, D3DXVECTOR4 colour) { sentence->colour = colour; }

	// Set the sentence text, which invokes regeneration of the vertex buffers
	Result SetSentenceText(SentenceType *sentence, char *text, float size);



private:
	Result InitializeSentence(SentenceType** sentence, int maxLength, int fontID);
	void ReleaseSentence(SentenceType**);
	Result RenderSentence(SentenceType*, D3DXMATRIX, D3DXMATRIX);

private:
	vector<FontData*> m_fonts;
	vector<SentenceType*> m_sentences;

	ID3D11Device *m_device;
	ID3D11DeviceContext *m_devicecontext;

	int m_screenWidth, m_screenHeight;
	float m_sentencewidth;
	
	D3DXMATRIX m_baseViewMatrix;
	FontShader *m_fontshader;


	const DXLocaliser *m_locale;
};

#endif