#include "DX11_Core.h" // #include "FullDX11.h"
#include "ErrorCodes.h"
#include "FastMath.h"
#include "Texture.h"

#include "ParticleEmitter.h"

// The constant set of particle texture coordinate orientations.  Ordered clockwise from bottom-left vertex,
// i.e. 0=BL, 1=TL, 2=TR, 3=BR
const D3DXVECTOR2 ParticleEmitter::TexCoord[4][4] = { 
	{ D3DXVECTOR2(0.0f, 1.0f), D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f) }, 
	{ D3DXVECTOR2(1.0f, 1.0f), D3DXVECTOR2(0.0f, 1.0f), D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f) }, 
	{ D3DXVECTOR2(1.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f), D3DXVECTOR2(0.0f, 1.0f), D3DXVECTOR2(0.0f, 0.0f) }, 
	{ D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f), D3DXVECTOR2(0.0f, 1.0f) } 
};

// Initialises the particle emitter.  Properties should have been set first.
Result ParticleEmitter::Initialise(ID3D11Device *device)
{
	Result result;

	// Initialise the particle data collection based on supplied parameters
	result = InitialiseParticles();
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Now initialise the buffers used for rendering
	result = InitialiseBuffers(device);
	if (result != ErrorCodes::NoError)
	{
		return result;
	}

	// Return success if initialisation is complete
	return ErrorCodes::NoError;
}

Result ParticleEmitter::InitialiseParticles(void)
{
	int tex = 0;

	// Attempt to initialise storage for the particle and vertex data
	m_particles = new (nothrow) Particle		[m_particlelimit];
	m_vertices	= new (nothrow) ParticleVertex	[m_vertexlimit];

	// If either allocation failed then attempt to recover the memory and return; no change will have been made
	if (!m_particles)		{ if (m_vertices) delete[] m_vertices; return ErrorCodes::CouldNotAllocateParticleDataMemory; }
	if (!m_vertices)		{ if (m_particles) delete[] m_particles; return ErrorCodes::CouldNotAllocateParticleVertexMemory; }

	// Zero all memory by default; as one effect, this sets all to Active=false and Alpha=0 by default which is desired behaviour
	memset((void*)m_particles, 0, sizeof(Particle) * m_particlelimit);
	memset((void*)m_vertices, 0, sizeof(ParticleVertex) * m_vertexlimit);

	// Initialise the vertex texture coordinates for each particle, so we only have to perform this step once.  Each particle will
	// take a random set of coordinates to avoid all particles having the same orientation
	ParticleVertex *v = &m_vertices[0];
	for (int i=0; i<m_particlelimit; i++)
	{
		// Identify the random set of texture coordinates that will be used for this particle
		tex = (rand() % 4);

		// Bottom left [0]
		v->texture = ParticleEmitter::TexCoord[tex][0];
		v++;

		// Top left [1]
		v->texture = ParticleEmitter::TexCoord[tex][1];
		v++;

		// Bottom right [2]
		v->texture = ParticleEmitter::TexCoord[tex][3];
		v++;

		// Bottom right [3]
		v->texture = ParticleEmitter::TexCoord[tex][3];
		v++;

		// Top left [4]
		v->texture = ParticleEmitter::TexCoord[tex][1];
		v++;

		// Top right [5]
		v->texture = ParticleEmitter::TexCoord[tex][2];
		v++;
	}

	// Return success once all particles are set up
	return ErrorCodes::NoError;
}

Result ParticleEmitter::InitialiseBuffers(ID3D11Device *device)
{
	INDEXFORMAT *indices;
	int i;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
		
	// Create the index array.
	indices = new INDEXFORMAT[m_vertexlimit];
	if(!indices) return ErrorCodes::CouldNotAllocateParticleIndexBuffer;

	// Initialize the index array, simply with sequential data (since we are duplicating all common vertices as per DX best practice)
	for(i=0; i<m_vertexlimit; i++)	indices[i] = i;

	// Set up the description of the dynamic vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(ParticleVertex) * m_vertexlimit;
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
    indexBufferDesc.ByteWidth = sizeof(INDEXFORMAT) * m_vertexlimit;
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

void ParticleEmitter::UpdateParticles(const D3DXVECTOR3 *rightbasisvector, const D3DXVECTOR3 *upbasisvector)
{
	// Pointer to the first inactive particle, plus basis vectors to be used later
	int inactiveid = -1;
	D3DXVECTOR3 vright, vup;
	D3DXVECTOR4 newcolour;

	// For each particle in this emitter
	Particle *p = m_particles;
	ParticleVertex *v = m_vertices;

	// Note we only loop to [maxparticlescreate+1], which improves efficiency while the entire buffer has not yet been used.  We
	// still have to go to +1 to make sure that the next potential new index is included.  This counter has been constrained to
	// only ever reach limit-1, so it is safe to do this
	for (int i=0; i<=m_maxparticleidcreated; i++, p++, v+=6)
	{
		// Reduce life of this particle and see if that kills it
		if (p->Active && ((p->Life -= Game::TimeFactor) <= 0.0f)) {
			p->Active = false;
			--m_numactiveparticles;
		}

		// Now process particle depending on whether it is alive or not
		if (!p->Active)
		{
			// If particle is not active then simply set alpha=0 to make it invisible, wherever it is
			v[0].colour.w = v[1].colour.w = v[2].colour.w = v[3].colour.w = v[4].colour.w = v[5].colour.w = 0.0f;

			// Also set the inactive pointer if it is not already set
			if (inactiveid == -1) inactiveid = i;
		}
		else	// (Particle IS active)
		{
			// Update colour, if required
			if (m_updateflag_colour) {
				newcolour = D3DXVECTOR4(	v->colour.x + ((frand_lh(m_updatecolour[0].x, m_updatecolour[1].x) * Game::TimeFactor)), 
											v->colour.y + ((frand_lh(m_updatecolour[0].y, m_updatecolour[1].y) * Game::TimeFactor)), 
											v->colour.z + ((frand_lh(m_updatecolour[0].z, m_updatecolour[1].z) * Game::TimeFactor)), 
											v->colour.w + ((frand_lh(m_updatecolour[0].w, m_updatecolour[1].w) * Game::TimeFactor)) );
				if (newcolour.x < 0.0f) newcolour.x = 0.0f; if (newcolour.x > 1.0f) newcolour.x = 1.0f;
				if (newcolour.y < 0.0f) newcolour.y = 0.0f; if (newcolour.y > 1.0f) newcolour.y = 1.0f;
				if (newcolour.z < 0.0f) newcolour.z = 0.0f; if (newcolour.z > 1.0f) newcolour.z = 1.0f;
				if (newcolour.w < 0.0f) newcolour.w = 0.0f; if (newcolour.w > 1.0f) newcolour.w = 1.0f;

				v[0].colour = v[1].colour = v[2].colour = v[3].colour = v[4].colour = v[5].colour = newcolour;			
					 
			}

			// Update size, if required
			if (m_updateflag_size) p->Size += (frand_lh(m_updatesize[0], m_updatesize[1]) * Game::TimeFactor);

			// Update velocity, if required
			if (m_updateflag_velocity) {
				p->Velocity = D3DXVECTOR3(	p->Velocity.x + ((frand_lh(m_updatevelocity[0].x, m_updatevelocity[1].x)) * Game::TimeFactor),
											p->Velocity.y + ((frand_lh(m_updatevelocity[0].y, m_updatevelocity[1].y)) * Game::TimeFactor),
											p->Velocity.z + ((frand_lh(m_updatevelocity[0].z, m_updatevelocity[1].z)) * Game::TimeFactor) );
			}

			// Move the particle (vertex 0 position) by its velocity
			v[0].position += (p->Velocity * Game::TimeFactor);						// Vertex 0 is the bottom-left

			// Now set the other (1-5) vertex coordinates using basis vectors and the particle size
			vright = (*rightbasisvector) * p->Size;
			vup = (*upbasisvector) * p->Size;

			v[1].position = v[4].position = (v[0].position + vup);			// Vertices 1 & 4 are top-left
			v[2].position = v[3].position = (v[0].position + vright);		// Vertices 2 & 3 are bottom-right
			v[5].position = (v[3].position + vup);							// Vertex 5 is the top-right; add an additional up vector to bottom right
		}
	}

	// Finally, see if we need to add a new particle following all the updates (and potentially deletions).  Add at the inactive ID
	if ((m_numactiveparticles < m_particlelimit) && m_emitting && ((m_timetonextemission -= Game::TimeFactor) < 0.0f))
	{
		// Determine whether we have an available slot; either an inactive slot we located above, or the next free slot beyond our currently
		// used range (if that range is less than the overall particle limit)
		int id = (inactiveid != -1 ? inactiveid : m_maxparticleidcreated + 1);
		if (id < m_particlelimit)
		{
			// Add a new particle
			AddParticle(id);

			// Reset the counter for the next emission to a value in the specified range
			m_timetonextemission = frand_lh(m_emissionfreq[0], m_emissionfreq[1]);
		}
	}
}

void ParticleEmitter::AddParticle(int id)
{
	// Make sure we are adding to a valid index
	if (id < 0 || id >= m_particlelimit) return;
	ParticleVertex *v = &m_vertices[id * 6];

	// Set initial properties
	m_particles[id].Active = true;
	m_particles[id].Life = frand_lh(m_initiallifetime[0], m_initiallifetime[1]);
	m_particles[id].Size = frand_lh(m_initialsize[0], m_initialsize[1]);

	// Initial velocity is based on world orientation
	m_particles[id].Velocity = D3DXVECTOR3( frand_lh(m_initialvelocity[0].x, m_initialvelocity[1].x), 
											frand_lh(m_initialvelocity[0].y, m_initialvelocity[1].y), 
											frand_lh(m_initialvelocity[0].z, m_initialvelocity[1].z) );


	// Transform the velocity vector by our world orientation matrix
	D3DXVec3TransformCoord(&m_particles[id].Velocity, &m_particles[id].Velocity, &m_orientmatrix);

	// Set initial position.  Note we initially set all to v[0] position; will be updated properly next frame by the update method
	v[0].position = D3DXVECTOR3(	frand_lh(m_initialposition[0].x, m_initialposition[1].x),
									frand_lh(m_initialposition[0].y, m_initialposition[1].y),
									frand_lh(m_initialposition[0].z, m_initialposition[1].z) );

	// Transform initial position by the world matrix, then set all other vertices to match
	D3DXVec3TransformCoord(&v[0].position, &v[0].position, &m_worldmatrix);
	v[1].position = v[2].position = v[3].position = v[4].position = v[5].position = v[0].position;

	// Set initial colour
	v[0].colour = v[1].colour = v[2].colour = v[3].colour = v[4].colour = v[5].colour = 
		D3DXVECTOR4(	frand_lh(m_initialcolour[0].x, m_initialcolour[1].x), 
						frand_lh(m_initialcolour[0].y, m_initialcolour[1].y), 
						frand_lh(m_initialcolour[0].z, m_initialcolour[1].z), 
						frand_lh(m_initialcolour[0].w, m_initialcolour[1].w) ); 
	
	// Increment active count.  Also, if this is the 'furthest' particle we have ever created then update that counter
	++m_numactiveparticles;
	if (id > m_maxparticleidcreated) m_maxparticleidcreated = id;

	// Fix: set maxparticlescreated back to ubound-1 if necessary, so we can always loop to that limit + 1 in the update function
	//if (m_maxparticleidcreated >= m_particlelimit) m_maxparticleidcreated = m_particlelimit-1;
}

void ParticleEmitter::Render(ID3D11DeviceContext *devicecontext, const D3DXVECTOR3 *vright, const D3DXVECTOR3 *vup)
{
	// Update the particle data
	UpdateParticles(vright, vup);

	// Render all particle data to the buffers
	RenderBuffers(devicecontext);
}

void ParticleEmitter::RenderBuffers(ID3D11DeviceContext *devicecontext)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedresource;
	ParticleVertex *pdata = NULL;
	unsigned int stride;
	unsigned int offset;

	// Lock the particle vertex buffer
	result = devicecontext->Map(m_vertexbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedresource);
	if (FAILED(result)) return;

	// Get a pointer to the destination location in the vertex buffer, and copy vertex data to this location
	pdata = (ParticleVertex*)mappedresource.pData;
	memcpy(pdata, (void*)m_vertices, sizeof(ParticleVertex) * m_vertexlimit);

	// Unlock the particle vertex buffer
	devicecontext->Unmap(m_vertexbuffer, NULL);	

	// Set the stride & offset data to define how vertex data is laid out in contiguous memory
	stride = sizeof(ParticleVertex);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	devicecontext->IASetVertexBuffers(0, 1, &m_vertexbuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
    devicecontext->IASetIndexBuffer(m_indexbuffer, /*DXGI_FORMAT_R32_UINT*/ DXGI_FORMAT_R16_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer.
    devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}



Result ParticleEmitter::LoadTexture(ID3D11Device* device, const char *filename)
{
	// If texture already exists then release and deallocate it first
	if (m_texture)
	{
		m_texture->Shutdown();
		delete m_texture;
		m_texture = NULL;
	}

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

void ParticleEmitter::Shutdown(void)
{
	// Dispose and deallocate the texture object
	if (m_texture)
	{
		m_texture->Shutdown();
		delete m_texture;
		m_texture = NULL;
	}

	// Now deallocate the particle data array
	if (m_particles)
	{
		delete[] m_particles;
		m_particles = NULL;
	}

	// Also deallocate the vertex data array
	if (m_vertices)
	{
		delete[] m_vertices;
		m_vertices = NULL;
	}

	// Delete and deallocate the vertex buffer
	if (m_vertexbuffer)
	{
		m_vertexbuffer->Release();
		m_vertexbuffer = NULL;
	}

	// Do the same for the index buffer
	if (m_indexbuffer)
	{
		m_indexbuffer->Release();
		m_indexbuffer = NULL;
	}
}

ParticleEmitter *ParticleEmitter::CreateClone(void)
{
	// Create a new instance of particle emitter using the default copy constructor, which will set many fields for us
	ParticleEmitter *e = new ParticleEmitter(*this);

	// Set the pos/orient specifically to ensure the world matrix is recalculated
	e->SetPositionAndOrientation(this->GetPosition(), this->GetOrientation());
	
	// Extract resource information from the source emitter texture, for use in creating the new one
	ID3D11Device *tdev;
	ID3D11Resource *tres;
	D3D11_SHADER_RESOURCE_VIEW_DESC tdesc;
	this->GetParticleTexture()->GetDevice(&tdev);
	this->GetParticleTexture()->GetResource(&tres);
	this->GetParticleTexture()->GetDesc(&tdesc);
	
	// Set a new texture object, using the same texture resources as the old object
	Texture *tex = new Texture();
	tex->Initialise(tres, &tdesc);
	e->SetParticleTexture(tex);
	
	// Run initialisation functions on the emitter
	e->Initialise(tdev);

	// Return a reference to the new emitter
	return e;
}


ParticleEmitter::ParticleEmitter(void)
{
	// Initialise values to NULL or their defaults
	m_particles = NULL;
	m_vertices = NULL;
	m_vertexbuffer = NULL;
	m_indexbuffer = NULL;

	m_position = NULL_VECTOR;
	m_orientation = ID_QUATERNION;
	m_numactiveparticles = 0;
	m_maxparticleidcreated = -1;
	m_particlelimit = 0;
	m_timetonextemission = 0.0f;
	m_texture = NULL;
	m_emitting = false;
	m_exists = false;

	m_emissionfreq[0] = 0.0f; m_emissionfreq[1] = 0.0f;
	m_initialposition[0] = NULL_VECTOR; m_initialposition[1] = NULL_VECTOR;
	m_initiallifetime[0] = 0.0f; m_initiallifetime[1] = 0.0f;
	m_initialcolour[0] = NULL_VECTOR4; m_initialcolour[1] = NULL_VECTOR4;
	m_initialsize[0] = 0.0f, m_initialsize[1] = 0.0f;
	m_initialvelocity[0] = NULL_VECTOR; m_initialvelocity[1] = NULL_VECTOR;

	m_updatecolour[0] = NULL_VECTOR4; m_updatecolour[1] = NULL_VECTOR4;
	m_updatesize[0] = 0.0f; m_updatesize[1] = 0.0f;
	m_updatevelocity[0] = NULL_VECTOR; m_updatevelocity[1] = NULL_VECTOR;

	// Flags that determine whether each property is updated per frame
	m_updateflag_colour = false;
	m_updateflag_size = false;
	m_updateflag_velocity = false;

}

ParticleEmitter::~ParticleEmitter(void)
{
}
