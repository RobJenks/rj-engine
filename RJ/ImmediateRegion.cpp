#include "DX11_Core.h"

#include <vector>
#include <sstream>
#include <cmath>
#include "FastMath.h"
#include "DustParticle.h"
#include "ErrorCodes.h"
#include "Texture.h"
#include "Utility.h"
#include "GameVarsExtern.h"
#include <tchar.h>

using namespace std;

#include "ImmediateRegion.h"

// Initialisation method
Result ImmediateRegion::Initialise(	ID3D11Device *device, const char *particletexture,
									const FXMVECTOR centre,
									const FXMVECTOR minbounds, const FXMVECTOR maxbounds,
									const GXMVECTOR updatethreshold)
{
	Result result;

	// Validation of the sizing parameters passed in for initialisation; (max-min) should be greater than the update threshold
	if (!XMVector3Greater(XMVectorSubtract(maxbounds, minbounds), updatethreshold))
		return ErrorCodes::RegionTooSmallForSpecifiedUpdateThreshold;
		
	// Initialise position; we also set the previous centre to be equal, then perform the initial update so no boundary update required at start
	this->m_centre = centre;
	this->m_prevcentre = centre;

	// Set region size and update thresholds
	SetRegionBounds(minbounds, maxbounds);
	this->m_threshold = updatethreshold;

	// Calculate the max particle creation bounds, which are a constant % smaller than the overall max bounds
	// This limits the creation of particles right near the max bounds which are immediately removed and replaced again
	this->m_creationmaxbounds = XMVectorScale(m_maxbounds, CREATION_DISTANCE_FACTOR);

	// Set parameters to their defaults
	m_cyclessinceupdate = 0;
	SetDustColour(DEFAULT_DUST_COLOUR);
	SetDustSize(DEFAULT_DUST_SIZE);

	// Load the texture resource that will be mapped to each particle
	result = LoadTexture(device, particletexture);
	if (result != ErrorCodes::NoError) return result;

	// Initialise dust density to the default value; this also allocates particle/vertex memory as required
	SetDustDensity(DEFAULT_DUST_DENSITY);

	// Initialise the vertex and index buffers.  Must be performed after the dust density is set, since that method
	// also allocates & builds the memory storage that will be referenced when creating buffers
	result = InitialiseBuffers(device);
	if (result != ErrorCodes::NoError) return result;

	// Perform a one-off update of the region to populate with a full set of starting particles
	UpdateRegion();

	// Return success if we have set everything up
	return ErrorCodes::NoError;
}

// Method to move the region to a new centre point; performs the logic to determine whether any updates are necessary
void ImmediateRegion::MoveRegion(const FXMVECTOR centre)
{
	// Store the new region centre
	this->m_centre = centre;

	// Determine whether we need to perform a boundary update, and execute one if we do
	if (DetermineThresholdUpdateRequired(m_boundary))
	{
		// Update the new boundary regions
		UpdateRegionBoundaries(m_boundary);

		// Register the change in position to this new location
		m_prevcentre = m_centre;
	}
}

void ImmediateRegion::UpdateRegionBoundaries(const FXMVECTOR boundary)
{
	// Calculate the new region bounds, now the move has been performed
	XMVECTOR bmax = XMVectorAdd(m_centre, m_maxbounds);
	XMVECTOR bmin = XMVectorSubtract(m_centre, m_maxbounds);
	
	// Traverse the collection and repurpose any particles now out of bounds
	DustParticle *p = &m_dust[0];
	for (int i=0; i<m_dustactive; i++, p++)
	{
		// Test whether this particle has left the region bounds
		if (!(XMVector3GreaterOrEqual(p->Location, bmin) && XMVector3Less(p->Location, bmax)))
			{
				// Refresh the particle and reposition back to a point within the region, UNLESS we want to REDUCE the particle count
				if (m_dustactive <= m_dustcount)
					RefreshParticle(i);
				else
					DeleteParticle(i);
			}
	}

	// Check whether we want to INCREASE the particle count
	if (m_dustactive < m_dustcount)
		AddParticle();			
}

void ImmediateRegion::SetDustSize(float size)
{
	// Store the new dust size, which will take effect in all new particles
	m_dustsize = size;
	m_dustsize_v = XMVectorSetW(XMVectorReplicate(m_dustsize), 0.0f);

	// Precalculate the relevant adjustment vectors for each of the three other vertices (bottom left remains at particle position)
	m_adj_tl = XMVectorSetY(NULL_VECTOR, -size);							// Top-left:		(0, -size, 0)
	m_adj_br = XMVectorSetX(NULL_VECTOR, size);								// Bottom-right:	(size, 0, 0)
	m_adj_tr = XMVectorAdd(m_adj_tl, m_adj_br);								// Top-right:		(size, -size, 0)	[= TL + BR]
}	

void ImmediateRegion::AddParticle(void)
{
	// Refresh this particle (at the current 'active' index) to an initial state and random location within the region
	RefreshParticle(m_dustactive);	

	// Increase the number of active particles by one (this one)
	m_dustactive++;
}

void ImmediateRegion::DeleteParticle(int index)
{
	// Decrement the number of active particles; index of the last active particle is now == m_dustactive 
	// (unlike normal, where this would be the first free element).  We can then return if this was the last particle
	if (--m_dustactive == 0) return;

	// We will delete this particle via the swap+pop method; first execute for the particle data
	m_dust[index] = m_dust[m_dustactive];

	// Now peform the same swap for the vertex data
	memcpy(&m_vertices[index * 6], &m_vertices[m_dustactive * 6], sizeof(ParticleVertexData) * 6);
}

void ImmediateRegion::RefreshParticle(int index)
{
	// Get a reference to this particle
	DustParticle & p = m_dust[index];

	// Set properties to the current desired starting values.  NOTE alpha begins at zero, and will be blended in to the 
	// m_dustcolour.w value over a defined period of time
	p.Colour = NULL_VECTOR;
	p.Size = m_dustsize;
	
	// Reposition the particle to a random location within the region
	// p.Location = m_centre.x + frand_lh((-m_creationmaxbounds.x), m_creationmaxbounds.x) for each component
	XMVECTOR randpos = XMVectorSet(frand_lh(-1.0f, 1.0f), frand_lh(-1.0f, 1.0f), frand_lh(-1.0f, 1.0f), 0.0f);
	p.Location = XMVectorMultiplyAdd(randpos, m_creationmaxbounds, m_centre);
	
	// Note that this is the only place where we DO take the minbounds into account, to allow us to avoid 
	// creating particles right on top of the player.  Move out of the min region if we would be creating there
	// Multiply by "InBounds" == 1 to ensure we only change required components.  Add either -minbounds or +minbounds
	// depending on whether the location is -ve or +ve, using "VectorSelect"
	p.Location = XMVectorAdd(p.Location, XMVectorMultiply(XMVectorInBounds(p.Location, m_minbounds),			
		XMVectorSelect(XMVectorNegate(m_minbounds), m_minbounds, XMVectorGreater(p.Location, NULL_VECTOR))));	

	// Update the vertex data for this particle; start from a zeroed block of memory
	ParticleVertexData *v = &m_vertices[index*6];
	memset((void*)v, 0, sizeof(ParticleVertexData) * 6);
	{
		// Bottom left
		XMStoreFloat3(&(v->position), p.Location);			// Set location to the particle location.  Other vertices will be postitioned arond this one
		v->texture = XMFLOAT2(0.0f, 1.0f);
		//v->colour = D3DXVECTOR4(p->Colour);				// No need to set the colour; will start at 0 and fade in

		// Top left
		//v++;												// No need to increment once here as we do not touch this vertex.  Inc by 2 for the next item
		//v->texture = D3DXVECTOR2(0.0f, 0.0f);				// No need to set texture coordinates as will be (0,0)
		//v->colour = D3DXVECTOR4(p->Colour);				// No need to set the colour; will start at 0 and fade in

		// Bottom right
		v+=2;												// Increment by 2 as we did not need to update the previous vertex
		v->texture = XMFLOAT2(1.0f, 1.0f);
		//v->colour = D3DXVECTOR4(p->Colour);				// No need to set the colour; will start at 0 and fade in

		// Bottom right
		v++; *v = *(v-1);									// Simply copy this vertex as it is a duplicate between the two triangles

		// Top left
		//v++; *v = *(v-3);									// Simply copy this vertex as it is a duplicate between the two triangles
															// No need to copy this as it should be all zeroes
															// No need to increment by 1 as we do nothing here.  Inc by 2 in the next item

		// Top right
		v+=2;												// Increment by two as we made no change to the previous vertex
		v->texture = XMFLOAT2(1.0f, 0.0f);
		//v->colour = D3DXVECTOR4(p->Colour);				// No need to set the colour; will start at 0 and fade in
	}
}

// Performs a full update of the immediate region; removes anything that currently exists and starts again
void ImmediateRegion::UpdateRegion(void)
{
	// Clear out the current region
	ClearRegionOfParticles();

	// Call the method to generate new dust particles in a region, as many times as required for the full set
	for (int i=0; i<m_dustcount; i++)
		AddParticle();
}

// Deletes and deallocates all particles currently in the region, with immediate effect
void ImmediateRegion::ClearRegionOfParticles(void)
{
	// We just need to set the current active particle count to zero; desired & capacity for particles remain the same
	m_dustactive = 0;
}

void ImmediateRegion::SetDustDensity(float dustdensity)
{
	// Store in local variables until we have confirmation that the allocation was successful
	int dustcount;

	// Calculate the total dust particle count as a function of region size.  Multiply by 8 as is (2x2x2) which accounts
	// for the fact that the maxbounds extend in +ve and -ve direction.  e.g. 3^3 = 27, 6^6 = 216, 216/27 = 8 = 2^3
	// NOTE we ignore the minbounds as this should always be zero for the immediate area
	dustcount = (int)ceil( m_maxboundsf.x * m_maxboundsf.y * m_maxboundsf.z * 8.0f * dustdensity );

	// Ensure the parameters remain in a valid range
	dustcount = max(0, dustcount);

	// If we are reducing the size then take no action; particles will be removed slowly as we perform updates and the capacity can remain the same
	// If we are increasing the size then we need to allocate new memory and copy into it
	if (dustcount > m_dustcapacity)
	{
		// Attempt to allocate new memory.  Always go for twice as much as required now, to avoid some future repeats
		DustParticle *dust				= new DustParticle			[dustcount * 2];
		ParticleVertexData *vertices	= new ParticleVertexData	[dustcount * 2 * 6];

		// If either allocation failed then attempt to recover the memory and return; no change will have been made
		if (!dust)		{ if (vertices) SafeDeleteArray(vertices); return; }
		if (!vertices)	{ if (dust)		SafeDeleteArray(dust); return; }

		// Copy memory into the new allocations
		memcpy(dust, m_dust, sizeof(DustParticle) * m_dustcount);
		memcpy(vertices, m_vertices, sizeof(ParticleVertexData) * m_dustcount * 6);

		// Now free the existing memory 
		SafeDelete(m_dust);
		SafeDelete(m_vertices);

		// Set pointers to reference the newly-allocated memory
		m_dust = dust;
		m_vertices = vertices;

		// Finally update the capacity based on this newly-extended memory allocation
		m_dustcapacity = dustcount * 2;
	}

	// Store the new field values for dust density, and the derived dust count
	m_dustcount = dustcount;
	m_dustdensity = dustdensity;
}

void XM_CALLCONV ImmediateRegion::Render(ID3D11DeviceContext *devicecontext, const FXMMATRIX view)
{
	// Render all dust particles to the vertex buffer
	RenderDustParticles(devicecontext, view);

	// When all buffers are populated, render the buffers to the DX output stream
	RenderBuffers(devicecontext);
}

void XM_CALLCONV ImmediateRegion::RenderDustParticles(ID3D11DeviceContext *devicecontext, const FXMMATRIX view)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedresource;
	ParticleVertexData *pdata = NULL;

	// Prepare the vertex data by completing the final per-frame data; use this rotation matrix to derive the vertex positions
	PrepareVertexBuffers(view);

	// Lock the particle vertex buffer
	result = devicecontext->Map(m_vertexbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedresource);
	if (FAILED(result)) return;

	// Get a pointer to the destination location in the vertex buffer, and copy vertex data to this location
	pdata = (ParticleVertexData*)mappedresource.pData;
	memcpy(pdata, (void*)m_vertices, sizeof(ParticleVertexData) * m_dustactive * 6);

	// Unlock the particle vertex buffer
	devicecontext->Unmap(m_vertexbuffer, NULL);	
}

void ImmediateRegion::RenderBuffers(ID3D11DeviceContext *devicecontext)
{
	unsigned int stride;
	unsigned int offset;

	// Set the stride & offset data to define how vertex data is laid out in contiguous memory
	stride = sizeof(ParticleVertexData);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	devicecontext->IASetVertexBuffers(0, 1, &m_vertexbuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    devicecontext->IASetIndexBuffer(m_indexbuffer, /*DXGI_FORMAT_R32_UINT*/ DXGI_FORMAT_R16_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer.
    devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void XM_CALLCONV ImmediateRegion::PrepareVertexBuffers(const FXMMATRIX view)
{
	// Transpose the input view matrix since we want to retrieve columns of data (and have direct vector access to rows)
	XMMATRIX viewT = XMMatrixTranspose(view);

	// Precalculate the three adjustments we need (for each other corner vertex) based on the current view matrix 
	// up & right vectors.  These two vectors can then be applied to all vertices of all particles
	XMVECTOR vright = viewT.r[0];		// Need [_11,_21,_31], is transposed into [_11,_12,_13] / r[0] |  vright = D3DXVECTOR3(view->_11, view->_21, view->_31);
	XMVECTOR vup = viewT.r[1];			// Need [_12,_22,_32], is transposed into [_21,_22,_23] / r[1] |  vup =    D3DXVECTOR3(view->_12, view->_22, view->_32);
	
	// Normalise both vectors and multiply up to desired particle size
	vright = XMVectorMultiply(XMVector3NormalizeEst(vright), m_dustsize_v);
	vup = XMVectorMultiply(XMVector3NormalizeEst(vup), m_dustsize_v);
	
	// Convert to local float representations since we want to use in manipulating the vertex struct
	XMFLOAT3 vright_f, vup_f;
	XMStoreFloat3(&vright_f, vright);
	XMStoreFloat3(&vup_f, vup);

	// Derive an alpha blend-in value for this update cycle, based on the blend-in rate and time elapsed since last update
	// Also define an alpha target vector; we can test each particle is < this since all but the alpha component will automatically be true
	XMVECTOR blendfactor = XMVectorScale(m_dustcolour, Game::TimeFactor * Game::C_DUST_PARTICLE_BLEND_RATE);
	XMVECTOR alphatarget = XMVectorSetW(LARGE_VECTOR_P, m_dustalpha);
	
	// Now apply to each set of vertices in turn
	ParticleVertexData *v = &m_vertices[0];
	DustParticle *p = &m_dust[0];
	for (int i=0; i<m_dustactive; i++, p++, v+=6)
	{
		// Bottom-left vertex (with index 0) is already set with the particle position

		// Top left vertex (with indices 1 and 4) 
		v[1].position = v[4].position = Float3Add(v->position, vup_f);

		// Bottom right vertex (with indices 2 and 3)
		v[2].position = v[3].position = Float3Add(v->position, vright_f);

		// Top right vertex (with index 5) - add an extra up vector to the existing bottom-right vertex
		v[5].position = Float3Add(v[3].position, vup_f);

		// Apply a fade-in effect to any particles that are recently created, until they reach the target colour/alpha value
		if (XMVector4Less(p->Colour, alphatarget))
		{
			// Update particle data.  Note: this means the value may go > target value, but adds an element of variety
			p->Colour = XMVectorAdd(p->Colour, blendfactor);		

			// Also update colour of each vertex
			XMStoreFloat4(&(v[0].colour), p->Colour);
			v[1].colour = v[2].colour = v[3].colour = v[4].colour = v[5].colour		= v[0].colour;
		}
	}
}


Result ImmediateRegion::InitialiseBuffers(ID3D11Device* device)
{
	INDEXFORMAT *indices;
	int i;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// We will allocate buffers for the maximum particle capacity, and x6 since there will be 6 vertices per particle
	int maxvertices = m_dustcapacity * 6;
		
	// Create the index array.
	indices = new INDEXFORMAT[maxvertices];
	if(!indices) return ErrorCodes::CouldNotAllocateParticleIndexBuffer;

	// Initialise vertex data to zeros at first
	memset(m_vertices, 0, (sizeof(ParticleVertexData) * maxvertices));

	// Initialize the index array, simply with sequential data (since we are duplicating all common vertices as per DX best practice)
	for(i=0; i<maxvertices; i++)	indices[i] = i;

	// Set up the description of the dynamic vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(ParticleVertexData) * maxvertices;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexbuffer);
	if(FAILED(result))	return ErrorCodes::CouldNotCreateParticleVertexBuffer;

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(INDEXFORMAT) * maxvertices;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexbuffer);
	if(FAILED(result))	return ErrorCodes::CouldNotCreateParticleIndexBuffer;

	// Release the index array since it is no longer needed.
	delete [] indices; indices = NULL;

	// If we have reached this point then all buffers are initialised, so return success
	return ErrorCodes::NoError;
}
// ********** TODO: Need to reinitialise buffers if capacity changes *************************

void ImmediateRegion::ReleaseBuffers(void)
{
	// Release the index buffer.
	if(m_indexbuffer)
	{
		m_indexbuffer->Release();
		m_indexbuffer = NULL;
	}

	// Release the vertex buffer.
	if(m_vertexbuffer)
	{
		m_vertexbuffer->Release();
		m_vertexbuffer = NULL;
	}
}

void ImmediateRegion::Terminate(void)
{
	// Clear all particles from the region
	ClearRegionOfParticles();

	// Release the buffers and texture resource, and deallocate all memory
	ReleaseBuffers();
	ReleaseTexture();
	ReleaseRegionObjects();
}

Result ImmediateRegion::LoadTexture(ID3D11Device* device, const char *filename)
{
	// Create the texture object
	m_texture = new Texture();
	if(!m_texture) return ErrorCodes::CouldNotCreateTextureObject;

	// Initialize the texture object
	Result result = m_texture->Initialise(filename);
	if(result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success if we loaded the texture successfully
	return ErrorCodes::NoError;
}

void ImmediateRegion::ReleaseTexture(void)
{
	// Release the texture object.
	if(m_texture)
	{
		m_texture->Shutdown();
		delete m_texture;
		m_texture = NULL;
	}
}

void ImmediateRegion::ReleaseRegionObjects(void)
{
	// Delete all data from the particle data collections
	if (m_dust)		delete[] m_dust;
	if (m_vertices) delete[] m_vertices;

	// Set counters back to zero
	m_dustcapacity = 0;
	m_dustcount = 0;
	m_dustactive = 0;
}


void ImmediateRegion::DebugPrintParticleCoordinates(void)
{
	std::ostringstream s;

	int limit = 1; // = m_dustactive;			// The number of particles to be streamed to output

	// Stream the coordinates of each particle in turn to the string stream
	DustParticle *p = &m_dust[0];
	for (int i=0; i<limit; i++, p++)
	{
		if (p)
			s << "P" << i << ", Pos=" << Vector3ToString(p->Location) << ", Col=" << Vector3ToString(p->Colour) << "\n";
		else
			s << "P" << i << "(NULL), (NULL)\n";
	}

	// Now stream the contents of each vertex object
	ParticleVertexData *v = &m_vertices[0];
	for (int i=0; i<limit*6; i++, v++)
		s << "V" << i << ", Pos=" << Vector3ToString(v->position) << ", Col=" << Vector4ToString(v->colour) << ", Tex=" << Vector2ToString(v->texture) << "\n";

	OutputDebugString(s.str().c_str());
}


// Constructor
ImmediateRegion::ImmediateRegion(void) :	UPDATE_EPSILON(3.0f), 
											DEFAULT_DUST_DENSITY(0.000001f),
											DEFAULT_DUST_COLOUR(XMVectorSet(0.235f, 0.569f, 0.569f, 0.4f)), // (60,145,145)
											DEFAULT_DUST_SIZE(4.0f),
											CREATION_DISTANCE_FACTOR(0.8f)
{
	// Set indices and pointers to NULL
	m_dust = NULL;
	m_vertices = NULL;
	m_vertexbuffer = NULL;
	m_indexbuffer = NULL;
	m_texture = NULL;

	m_dustactive = 0;
	m_dustcount = 0;
	m_dustcapacity = 0;
}

// Destructor
ImmediateRegion::~ImmediateRegion(void)
{
}
