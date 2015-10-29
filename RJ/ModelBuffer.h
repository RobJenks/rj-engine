#pragma once

#ifndef __ModelBufferH__
#define __ModelBufferH__

#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "DX11_Core.h"
#include "Texture.h"

class ModelBuffer
{
public:

	// Default constructor
	ModelBuffer(void);

	// Initialise the buffers based on the supplied model data
	Result							Initialise(	ID3D11Device *device, const void *vertexdata, unsigned int vertexsize, unsigned int vertexcount,
												const void *indexdata, unsigned int indexsize, unsigned int indexcount);

	// Sets the texture for this object
	Result							SetTexture(const char *filename);

	// Vertex buffer
	ID3D11Buffer *					VertexBuffer;

	// Index buffer
	ID3D11Buffer *					IndexBuffer;

	// Methods to return buffer data
//	CMPINLINE ID3D11Buffer *		GetVertexBuffer(void) const			{ return m_vertexbuffer; }
//	CMPINLINE ID3D11Buffer *		GetIndexBuffer(void) const			{ return m_indexbuffer; }
	CMPINLINE Texture *				GetTexture(void)					{ return m_texture; }
	CMPINLINE unsigned int			GetVertexCount(void) const			{ return m_vertexcount; }
	CMPINLINE unsigned int			GetIndexCount(void) const			{ return m_indexcount; }
	CMPINLINE unsigned int			GetVertexSize(void) const			{ return m_vertexsize; }
	CMPINLINE unsigned int			GetIndexSize(void) const			{ return m_indexsize; }

	// Return the underlying shader texture resource for this object, if applicable
	CMPINLINE ID3D11ShaderResourceView *		GetTextureResource(void) 				
	{ 
		return (m_texture ? m_texture->GetTexture() : NULL);
	}

	// Releases all resources and initialises back to initial state.  Not required in normal use since this will be
	// handled automatically when the object is deallocated
	void							ReleaseResources(void);

	// Default destructor
	~ModelBuffer(void);


protected:

	unsigned int			m_vertexcount;
	unsigned int			m_indexcount;

	unsigned int			m_vertexsize;
	unsigned int			m_indexsize;

	Texture *				m_texture;
};




#endif








