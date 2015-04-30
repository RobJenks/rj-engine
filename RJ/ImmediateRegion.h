#pragma once
#ifndef __ImmediateRegionH__
#define __ImmediateRegionH__

#include "DX11_Core.h"

#include <vector>
#include "CompilerSettings.h"
#include "DustParticle.h"
#include "ErrorCodes.h"
#include "Texture.h"
class ParticleShader;
class DXLocaliser;
using namespace std;

#include "RegionBase.h"

class ImmediateRegion :	public RegionBase
{
public:
	typedef UINT16 INDEXFORMAT;	

	struct ParticleVertexData
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR4 colour;
	};

	ImmediateRegion(void);
	~ImmediateRegion(void);

	// The threshold below which we do not even check a dimension for changes upon an update.  Specified as a % of the boundary threshold
	const float				UPDATE_EPSILON;

	// Initialisation method
	Result					Initialise( ID3D11Device *device, const char *texturefilename,
										const D3DXVECTOR3 & centre, 
										const D3DXVECTOR3 & minbounds, const D3DXVECTOR3 & maxbounds, 
										const D3DXVECTOR3 & updatethreshold);

	// Entry method, called whenever the central object which this region is attached to moves.  Only performs an update 
	// every few cycles, to prevent needless calculations of very small changes
	CMPINLINE void ImmediateRegion::RegionCentreMoved(D3DXVECTOR3 centre)
	{
		if (++m_cyclessinceupdate == UPDATE_FREQUENCY)
		{
			MoveRegion(centre);
			m_cyclessinceupdate = 0;
		}
	}

	
	// Method to move the region to a new centre point; performs the logic to determine whether any updates are necessary
	void					MoveRegion(D3DXVECTOR3 centre);

	// Method to update the region boundaries
	void					UpdateRegionBoundaries(D3DXVECTOR3 boundary);

	// Method to update the entire region
	void					UpdateRegion(void);

	// Adds a new dust particle to the collection
	void					AddParticle(void);

	// Deletes a particle from the collection
	void					DeleteParticle(int index);

	// Refreshes the particle back to initial state, and repositions to a random location within the region
	void					RefreshParticle(int index);

	// Clears out all particles etc from the immediate region
	void					ClearRegionOfParticles(void);

	// Returns the number of ACTIVE particles; may be <= the desired particle count and the particle capacity
	CMPINLINE int			GetActiveParticleCount(void) { return m_dustactive; }

	// Change dust density.  Alters desired dust particle count as a function of region volume
	CMPINLINE float			GetDustDensity(void) { return m_dustdensity; }
	void					SetDustDensity(float dustdensity);
	
	// Sets the dust size.  Also precalculates the relative vertex positions to save processing time when rendering
	void					SetDustSize(float size);
	CMPINLINE float			GetDustSize(void) { return m_dustsize; }

	// Terminates the region, including all particles within it
	void					Terminate(void);
	void					ReleaseRegionObjects(void);

	// Debug method to output the coordinates of all particles in string format to the debug output window
	void					DebugPrintParticleCoordinates(void);

	// Returns a pointer to the texture resource that will be used for rendering immediate-space particles
	CMPINLINE ID3D11ShaderResourceView* 
							GetParticleTextureResource(void) { return m_texture->GetTexture(); }

	// Prepares the vertex buffers by filling in the render-time data (positon of vertices 1-5)
	void					PrepareVertexBuffers(const D3DXMATRIX *view);

	// Method to render the everything in the region
	void					Render(ID3D11DeviceContext *devicecontext, const D3DXMATRIX *view);

	// Render each component of the region to the vertex buffer
	void					RenderDustParticles(ID3D11DeviceContext *devicecontext, const D3DXMATRIX *view);

	// Methods to initialise and release the vertex/index buffers
	Result					InitialiseBuffers(ID3D11Device* device);
	void					ReleaseBuffers(void);

	// Methods to initialise and release the texture resource
	Result					LoadTexture(ID3D11Device* device, const char *filename);
	void					ReleaseTexture(void);

	// Renders the vertex buffers to the DX output stream, once all data is prepared in the buffers
	void					RenderBuffers(ID3D11DeviceContext *devicecontext);

private:

	// Counter that allows us to only update the region every X cycles, to save processing time
	int									m_cyclessinceupdate;
	static const int					UPDATE_FREQUENCY = 50;

	// Collection of space dust particles that allows us to visualise movement; also equivalent array of particle vertices
	DustParticle						*m_dust;
	ParticleVertexData					*m_vertices;		// Will be 6 times larger than the above.  m_vertices[i*6] = ...

	// Density of dust particles per cubic unit.  Multiplied up by region size to give total number of particles
	const float							DEFAULT_DUST_DENSITY;
	float								m_dustdensity;
	
	// Key counter fields to keep track of the region particles.  
	// Assuming no pending creations/allocations/deletions, active <= count <= capacity
	int									m_dustactive;		// The number of particles that currently exist
	int									m_dustcount;		// The desired number of particles that should exist
	int									m_dustcapacity;		// The maximum size of the particle data array

	// Fields relating to particle creation
	const float							CREATION_DISTANCE_FACTOR;	// % of region size in which we create new particles
	D3DXVECTOR3							m_creationmaxbounds;		// The creation max bounds, equal to maxbounds * factor above

	// The properties to be applied to particles as they are refreshed
	D3DXVECTOR4							m_dustcolour;
	const D3DXVECTOR4					DEFAULT_DUST_COLOUR;
	float								m_dustsize;
	const float							DEFAULT_DUST_SIZE;
	
	// Adjustment vectors for each other three particle vertices to be calculated; saves time at rendering
	D3DXVECTOR3							m_adj_tl;
	D3DXVECTOR3							m_adj_br; 
	D3DXVECTOR3							m_adj_tr;
	
	// Vertex and index buffers for rendering the particles
	ID3D11Buffer						*m_vertexbuffer, *m_indexbuffer;

	// Texture resource that will be mapped to each particle
	Texture								*m_texture;
};




#endif