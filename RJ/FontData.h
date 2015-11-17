#pragma once

#ifndef __FontDataH__
#define __FontDataH__


#include "DX11_Core.h"

#include <fstream>
#include <string>
#include "Texture.h"
#include "CompilerSettings.h"
using namespace std;


// This class has no special alignment requirements
class FontData
{
private:
	struct FontType
	{
		float left, right;
		int size;
	};

	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:
	FontData();
	FontData(const FontData&);
	~FontData();

	Result Initialize(ID3D11Device* device, string name, const char *fontFilename, const char *textureFilename);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	CMPINLINE string GetName(void) { return m_name; }
	CMPINLINE void SetName(string name) { m_name = name; }

	void BuildVertexArray(	void *vertices, const char *sentence, float drawX, float drawY, float size, 
							float *pOutSentenceWidth, float *pOutSentenceHeight );

private:
	Result LoadFontData(const char*);
	void ReleaseFontData();
	Result FontData::LoadTexture(ID3D11Device* device, const char *filename);
	void ReleaseTexture();

private:
	string		m_name;
	FontType*	m_Font;
	Texture*	m_Texture;
	float		m_spacing;
};

#endif