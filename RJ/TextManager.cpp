#include "DX11_Core.h"
#include "FontData.h"
#include "FontShader.h"
#include "SentenceType.h"
#include "CoreEngine.h"

#include "TextManager.h"


Result RJ_XM_CALLCONV TextManager::Initialize(HWND hwnd, int screenWidth, int screenHeight, const FXMMATRIX baseViewMatrix, FontShader *fontshader)
{
	// Store the screen width and height.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Store the base view matrix.
	m_baseViewMatrix = baseViewMatrix;

	// Store a pointer to the font shader that this text manager will use for rendering
	m_fontshader = fontshader;

	// Return success if all initialisation has been completed
	return ErrorCodes::NoError;
}

Result TextManager::InitializeFont(std::string name, const char *fontdata, const char *fonttexture, int &fontID)
{
	Result result;

	// Create the font object.
	FontData *font = new FontData;
	if(!font)
	{
		return ErrorCodes::CannotCreateFontObject;
	}

	// Initialize the font object.
	result = font->Initialize(name, fontdata, fonttexture);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// If all completed successfully then add to the collection and pass the ID of this font back to the calling function
	m_fonts.push_back(font);
	fontID = ((int)m_fonts.size()-1);
	
	// Return success
	return ErrorCodes::NoError;	
}

SentenceType *TextManager::CreateSentence(int fontID, int maxlength)
{
	SentenceType *sentence;
	
	// Initialize the sentence
	Result result = InitializeSentence(&sentence, maxlength, fontID);
	if(result != ErrorCodes::NoError)
	{
		return NULL;
	}

	// Add to the sentence collection
	m_sentences.push_back(sentence);

	// Return a reference to this sentence
	return sentence;
}

void TextManager::Shutdown()
{
	// Release each sentence in turn
	int ns = (int)m_sentences.size();
	for (int i=0; i<ns; i++)
	{
		SentenceType *s = m_sentences.at(i);
		if (s)
		{
			ReleaseSentence(&s);
		}

		m_sentences.at(i) = NULL;
	}
	m_sentences.clear();

	// Release each font object in turn
	int nf = (int)m_fonts.size();
	for (int i=0; i<nf; i++)
	{
		FontData *f = m_fonts.at(i);
		if (f)
		{
			f->Shutdown();
			delete f;
			f = 0;
		}
	}
	m_fonts.clear();
}


Result RJ_XM_CALLCONV TextManager::Render(const FXMMATRIX worldMatrix, const CXMMATRIX orthoMatrix)
{
	Result result, overallresult;
	SentenceType *sentence;

	// We will report the overall result based on each individual result (to render as much as possible)
	overallresult = ErrorCodes::NoError;

	// Render each sentence in turn
	std::vector<SentenceType*>::size_type sentencecount = m_sentences.size();
	for (std::vector<SentenceType*>::size_type i = 0; i<sentencecount; ++i)
	{
		// Get a handle to the sentence, and only render if the render flag is set
		sentence = m_sentences.at(i);
		if (sentence && sentence->render)
		{
			// Run this sentence through the font shader to render to screen
			result = RenderSentence(m_sentences.at(i), worldMatrix, orthoMatrix);
			if (result != ErrorCodes::NoError) overallresult = result;
		}
	}

	// Return the overall result of the text rendering
	return overallresult;
}


Result TextManager::InitializeSentence(SentenceType** sentence, int maxLength, int fontID)
{
	VertexType* vertices;
	INDEXFORMAT* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	// Validate the font ID before starting
	if (fontID < 0 || fontID >= (int)m_fonts.size() || m_fonts.at(fontID) == NULL)
		return ErrorCodes::InvalidFontSpecifiedForTextConstruction;

	// Create a new sentence object.
	*sentence = new SentenceType;
	if(!*sentence)
	{
		return ErrorCodes::CannotCreateTextSentenceObject;
	}

	// Store the font ID that will be used to render this sentence
	(*sentence)->fontID = fontID;

	// Initialize the sentence buffers to null.
	(*sentence)->vertexBuffer = 0;
	(*sentence)->indexBuffer = 0;

	// Set the maximum length of the sentence.
	(*sentence)->maxLength = maxLength;
	(*sentence)->sentencewidth = 0.0f;

	// Set the number of vertices in the vertex array.
	(*sentence)->vertexCount = 6 * maxLength;

	// Set the number of indexes in the index array.
	(*sentence)->indexCount = (*sentence)->vertexCount;

	// Create the vertex array.
	vertices = new VertexType[(*sentence)->vertexCount];
	if(!vertices)
	{
		return ErrorCodes::CannotCreateTextVertexArray;
	}

	// Create the index array.
	indices = new INDEXFORMAT[(*sentence)->indexCount];
	if(!indices)
	{
		return ErrorCodes::CannotCreateTextIndexArray;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));

	// Initialize the index array.
	for(i=0; i<(*sentence)->indexCount; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the dynamic vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
	auto *device = Game::Engine->GetRenderDevice()->GetDevice();
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CannotCreateTextVertexBuffer;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(INDEXFORMAT) * (*sentence)->indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
	if(FAILED(result))
	{
		return ErrorCodes::CannotCreateTextIndexBuffer;
	}

	// Release the vertex array as it is no longer needed.
	delete [] vertices;
	vertices = 0;

	// Release the index array as it is no longer needed.
	delete [] indices;
	indices = 0;

	return ErrorCodes::NoError;
}


Result TextManager::UpdateSentence(SentenceType* sentence, const char *text, int positionX, int positionY, bool render, 
								   const XMFLOAT4 & textcolour, float size)
{
	// Make sure that we have been passed a valid sentence pointer
	if (!sentence) return ErrorCodes::CannotPerformUpdateOnNullSentencePointer;

	// Set the sentence position
	SetSentencePosition(sentence, positionX, positionY);

	// Set the render flag
	SetSentenceRendering(sentence, render);

	// Set the colour
	SetSentenceColour(sentence, textcolour);

	// Finally set the text itself, which invokes regeneration of the vertex buffers
	return SetSentenceText(sentence, text, size);
}

Result TextManager::SetSentenceText(SentenceType *sentence, const char *text, float size)
{
	int numLetters;
	VertexType* vertices;
	float drawX, drawY;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;

	// Make sure that we have been passed a valid sentence pointer
	if (!sentence) return ErrorCodes::CannotPerformUpdateOnNullSentencePointer;

	// Get the number of letters in the sentence.
	numLetters = (int)strlen(text);

	// Check for possible buffer overflow.
	if(numLetters > sentence->maxLength)
	{
		return ErrorCodes::TextSentenceExceedsMaxAllocatedSize;
	}

	// Create the vertex array.
	vertices = new VertexType[sentence->vertexCount];
	if(!vertices)
	{
		return ErrorCodes::CannotCreateTextVertexArray;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	// Calculate the X and Y pixel position on the screen to start drawing to.
	drawX = (float)(((m_screenWidth / 2) * -1) + sentence->x);
	drawY = (float)((m_screenHeight / 2) - sentence->y);

	// Obtain a reference to the font object that will handle construction of the text vertex array
	FontData *font = m_fonts.at(sentence->fontID);
	if (!font) return ErrorCodes::FontDataNoLongerExistsForTextConstruction;

	// Use the font class to build the vertex array from the sentence text and sentence draw location.
	font->BuildVertexArray((void*)vertices, text, drawX, drawY, size, &(sentence->sentencewidth), &(sentence->sentenceheight));

	// Lock the vertex buffer so it can be written to.
	auto * context = Game::Engine->GetRenderDevice()->GetDeviceContext();
	result = context->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return ErrorCodes::CouldNotObtainVertexBufferLock;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

	// Unlock the vertex buffer.
	context->Unmap(sentence->vertexBuffer, 0);

	// Release the vertex array as it is no longer needed.
	delete [] vertices;
	vertices = 0;

	return ErrorCodes::NoError;
}


void TextManager::ReleaseSentence(SentenceType** sentence)
{
	if(*sentence)
	{
		// Release the sentence vertex buffer.
		if((*sentence)->vertexBuffer)
		{
			(*sentence)->vertexBuffer->Release();
			(*sentence)->vertexBuffer = 0;
		}

		// Release the sentence index buffer.
		if((*sentence)->indexBuffer)
		{
			(*sentence)->indexBuffer->Release();
			(*sentence)->indexBuffer = 0;
		}

		// Release the sentence.
		delete *sentence;
		*sentence = 0;
	}
}


Result RJ_XM_CALLCONV TextManager::RenderSentence(SentenceType* sentence, const FXMMATRIX worldMatrix, const CXMMATRIX orthoMatrix)
{
	unsigned int stride, offset;

	// Set vertex buffer stride and offset.
	auto * context = Game::Engine->GetRenderDevice()->GetDeviceContext();
    stride = sizeof(VertexType); 
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	context->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	context->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Get a handle to the font object that will be used for rendering of the text
	FontData *font = m_fonts.at(sentence->fontID);
	if (!font) return ErrorCodes::FontDataNoLongerExistsForTextRendering;

	// Render the text using the font shader.
	return m_fontshader->Render( context, sentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix,
								 font->GetTexture(), sentence->colour);
}


TextManager::TextManager(void)
{
	// Set all key pointers to NULL before initialising
	m_fonts.clear();
	m_sentences.clear();
	m_fontshader = 0;
}


TextManager::TextManager(const TextManager& other)
{
}


TextManager::~TextManager()
{
}