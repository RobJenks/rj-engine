#include <string>
#include "ErrorCodes.h"
#include "FontData.h"
#include "CoreEngine.h"
#include "RenderAssetsDX11.h"

Result FontData::Initialize(const std::string & name, const char *fontFilename, const std::string & texture)
{
	Result result;

	// Store the name of this font
	m_name = name;

	// Initialise font spacing
	// TODO: Also to be read from the font data file or overall xml file?
	m_spacing = 3.0f;

	// Load in the text file containing the font data.
	result = LoadFontData(fontFilename);
	if (result != ErrorCodes::NoError)
	{
		Game::Log << LOG_WARN << "Could not load descriptor data for font \"" << name << "\" (" << fontFilename << ")\n";
		return result;
	}

	// Load the texture that has the font characters on it.
	m_Texture = Game::Engine->GetAssets().GetTexture(texture);
	if (!m_Texture)
	{
		Game::Log << LOG_WARN << "Could not load font texture \"" << texture << "\" for font \"" << name << "\"\n";
		return ErrorCodes::CannotLoadFontDataFile;
	}

	return ErrorCodes::NoError;
}


void FontData::Shutdown()
{
	// Release the font data.
	ReleaseFontData();
}


Result FontData::LoadFontData(const char *filename)
{
	std::ifstream fin;
	int i;
	char temp;

	// Create the font spacing buffer.
	m_Font = new FontType[95];
	if(!m_Font)
	{
		return ErrorCodes::CannotAllocateFontSpacingBuffer;
	}

	// Read in the font size and spacing between chars.
	fin.open(filename);
	if(fin.fail())
	{
		return ErrorCodes::CannotLoadFontDataFile;
	}

	// Read in the 95 used ascii characters for text.
	for(i=0; i<95; i++)
	{
		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}
		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}

		fin >> m_Font[i].left;
		fin >> m_Font[i].right;
		fin >> m_Font[i].size;
	}

	// Close the file.
	fin.close();

	return ErrorCodes::NoError;
}


void FontData::ReleaseFontData()
{
	// Release the font data array.
	if(m_Font)
	{
		delete [] m_Font;
		m_Font = 0;
	}
}


ID3D11ShaderResourceView* FontData::GetTexture()
{
	return m_Texture->GetShaderResourceView();
}


void FontData::BuildVertexArray(void* vertices, const char *sentence, float drawX, float drawY, float size, 
								float *pOutSentenceWidth, float *pOutSentenceHeight)
{
	VertexType* vertexPtr;
	int numLetters, index, i, letter;
	float spacingpixels, letterwidth, letterheight;
	FontType fontitem;

	// Take no action if we have not yet initailised the required font data
	if (!m_Font) return;

	// Store the starting point of this sentence, for use in calculating sentence width later
	float startX = drawX;

	// Cast the input vertices into a VertexType structure.
	vertexPtr = (VertexType*)vertices;

	// Get the number of letters in the sentence, and also precalculate the letter height & spacing
	numLetters = (int)strlen(sentence);
	spacingpixels = m_spacing * size;
	letterheight = 16.0f * size;

	// Initialize the index to the vertex array.
	index = 0;

	// Draw each letter onto a quad.
	for(i=0; i<numLetters; i++)
	{
		letter = ((int)sentence[i]) - 32;
		fontitem = m_Font[letter];
		letterwidth = fontitem.size * size;

		// If the letter is a space then just move over by the spacing amount
		if(letter == 0)
		{
			drawX = drawX + spacingpixels;
		}
		else
		{
			// First triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(fontitem.left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + letterwidth), (drawY - letterheight), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(fontitem.right, 1.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX, (drawY - letterheight), 0.0f);  // Bottom left.
			vertexPtr[index].texture = XMFLOAT2(fontitem.left, 1.0f);
			index++;

			// Second triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(fontitem.left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX + letterwidth, drawY, 0.0f);  // Top right.
			vertexPtr[index].texture = XMFLOAT2(fontitem.right, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + letterwidth), (drawY - letterheight), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(fontitem.right, 1.0f);
			index++;

			// Update the x location for drawing by the size of the letter and (one pixel * size multiplier)
			drawX = drawX + letterwidth + size;
		}
	}

	// Note that 'drawx' now represents the rightmost extent of the text string; return this as a parameter if we have one set
	if (pOutSentenceWidth) (*pOutSentenceWidth) = (drawX - startX);
	if (pOutSentenceHeight) (*pOutSentenceHeight) = letterheight;
}

FontData::FontData()
{
	// Initialise all pointers to NULL
	m_Font = 0;
	m_Texture = 0;
}


FontData::FontData(const FontData& other)
{
}


FontData::~FontData()
{
}
