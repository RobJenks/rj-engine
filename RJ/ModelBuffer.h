#pragma once

#ifndef __ModelBufferH__
#define __ModelBufferH__

#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "DX11_Core.h"
#include "Texture.h"
#include "Material.h"

// This class has no special alignment requirements
class ModelBuffer
{
public:

	// Default constructor
	ModelBuffer(void);

	// Initialise the buffers based on the supplied model data
	Result							Initialise(ID3D11Device *device, const void **ppVertexdata, unsigned int vertexsize, unsigned int vertexcount,
												const void **ppIndexdata, unsigned int indexsize, unsigned int indexcount);

	// Sets the texture for this object
	Result							SetTexture(const char *filename);
	Result							SetTexture(Texture *texture);

	// Vertex buffer
	ID3D11Buffer *					VertexBuffer;

	// Index buffer
	ID3D11Buffer *					IndexBuffer;

	// Methods to return buffer data
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

	// Returns the number of materials in this model
	CMPINLINE unsigned int			GetMaterialCount(void) const		{ return m_material_count; }

	// Returns a pointer to the material data for this model
	CMPINLINE Material *			GetMaterialData(void) const			{ return m_materials; }

	// Set the number of materials in this model and allocate space accordingly
	void							SetMaterialCount(unsigned int count);

	// Set details for the specified material
	void							SetMaterial(unsigned int index, const Material & material);

	// Releases buffer resources (VB, IB) and initialises back to initial state.  Not required in normal use since this will be
	// handled automatically when the object is deallocated
	void							ReleaseModelBufferResources(void);

	// Releases all resources and initialises back to initial state.  Includes model buffer resources (as per ReleaseModelBufferResources)
	// as well as e.g. texture resources.  Not required in normal use since this will be handled automatically when the object is deallocated
	void							ReleaseAllResources(void);


	// Default destructor
	~ModelBuffer(void);


protected:

	unsigned int			m_vertexcount;
	unsigned int			m_indexcount;

	unsigned int			m_vertexsize;
	unsigned int			m_indexsize;

	Texture *				m_texture;

	Material *				m_materials;
	unsigned int			m_material_count;
};




#endif








