#pragma once

#ifndef __FontDataH__
#define __FontDataH__


#include "DX11_Core.h"

#include <fstream>
#include <string>
#include "ErrorCodes.h"
#include "Rendering.h"
#include "CompilerSettings.h"
class TextureDX11;


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

	Result Initialize(const std::string & name, const char *fontFilename, const std::string & texture);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	CMPINLINE std::string GetName(void) { return m_name; }
	CMPINLINE void SetName(std::string name) { m_name = name; }

	void BuildVertexArray(	void *vertices, const char *sentence, float drawX, float drawY, float size, 
							float *pOutSentenceWidth, float *pOutSentenceHeight );

private:

	Result LoadFontData(const char*);
	void ReleaseFontData();

private:

	std::string		m_name;
	FontType*		m_Font;
	TextureDX11 *	m_Texture;
	float			m_spacing;
};

#endif