#pragma once

#ifndef __ParticleEmitterH__
#define __ParticleEmitterH__


#include "DX11_Core.h"

#include <vector>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Texture.h"

class ParticleEmitter
{
public:
	enum Prop { MinValue = 0, MaxValue = 1 };						// Index into particle properties when setting min or max values

	// Definition of a particle object
	struct Particle
	{
		//				Position			// Is not specified here, since included within the vertex definition
		//				Colour				// Is not specified here, since included within the vertex definition
		bool			Active;				// Is the particle currently active?
		float			Life;				// Remaining particle lifetime (secs)
		D3DXVECTOR3		Velocity;			// Velocity of the particle
		float			Size;				// Size of the particle
	};

	// Indexing data format; use UINT-16 for compatibility back to SM2.0
	typedef UINT16 INDEXFORMAT;	

	// Definition of a particle vertex; position & colour are located here and so not also in the Particle def to avoid duplication
	struct ParticleVertex
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR4 colour;
	};

	// Static collection of possible texture coordinate orientations.  Ordered clockwise from bottom-left vertex
	static const D3DXVECTOR2	TexCoord[4][4];

	// Initialisation function
	Result						Initialise(ID3D11Device *device);

	// Methods for adding and per-frame updating of particles
	void						AddParticle(int id);
	void						UpdateParticles(const D3DXVECTOR3 *rightbasisvector, const D3DXVECTOR3 *upbasisvector);

	// Rendering method
	void						Render(ID3D11DeviceContext *devicecontext, const D3DXVECTOR3 *vright, const D3DXVECTOR3 *vup);

	// Shutdown functions
	void						Shutdown(void);

	// Clone function, for creating new instances from a prototype emitter object
	ParticleEmitter				*CreateClone(void);

	// Methods to get & set the unique string key for this emitter
	CMPINLINE string			GetCode(void) { return m_code; }
	CMPINLINE void				SetCode(string code) { m_code = code; }

	// Accessor functions for emitter location properties
	CMPINLINE D3DXVECTOR3		*GetPosition(void) { return &m_position; }
	CMPINLINE D3DXQUATERNION	*GetOrientation(void) { return &m_orientation; }
	CMPINLINE D3DXMATRIX		*GetEmitterWorldMatrix(void) { return &m_worldmatrix; }
	
	// Returns the inverse world matrix for this emitter.  Note: this is calculated when called
	CMPINLINE D3DXMATRIX		*GetEmitterInverseWorldMatrix(void) 
	{ 
		D3DXMatrixInverse(&m_inverseworld, NULL, &m_worldmatrix);
		return &m_inverseworld; 
	}

	// Methods to set pos/orient of the emitter; also recalculate the world matrix upon update
	CMPINLINE void				SetPosition(const D3DXVECTOR3 *pos)				{ m_position = *pos;		RecalculateWorldMatrix(); }
	CMPINLINE void				SetOrientation(const D3DXQUATERNION *orient)	{ m_orientation = *orient;	RecalculateWorldMatrix(); }
	CMPINLINE void				SetPositionAndOrientation(const D3DXVECTOR3 *pos, const D3DXQUATERNION *orient)
									{ m_position = *pos; m_orientation = *orient; RecalculateWorldMatrix(); }

	// Methods to set the position & orientation, specifically WITHOUT also recalculating the world matrix.  Useful where
	// we are recalculating elsewhere, or even calculating first and then decomposing to get pos & orient (e.g. in iSpaceObject)
	CMPINLINE void				SetPositionNoRecalc(const D3DXVECTOR3 pos)				{ m_position = pos; }
	CMPINLINE void				SetOrientationNoRecalc(const D3DXQUATERNION orient)		
	{ 
		// Store the new orientation value
		m_orientation = orient; 

		// Also recalculate the orientation matrix
		D3DXMatrixRotationQuaternion(&m_orientmatrix, &orient);
	}

	// We also allow setting of the world matrix directly, if it has been calculated elsewhere.  Note this could leave it
	// out of sync with the pos/orient unless these are also updated
	CMPINLINE void				SetEmitterWorldMatrix(const D3DXMATRIX world)			{ m_worldmatrix = world; }


	// Set the position, orientation and world matrix directly.  Used for efficiency when a parent object has already done these calculations
	CMPINLINE void SetPositionOrientAndWorldNoRecalc(const D3DXVECTOR3 & pos, const D3DXQUATERNION & orient, const D3DXMATRIX & world)
	{
		// Set the parameters directly
		m_position = pos;
		m_orientation = orient;
		m_worldmatrix = world;

		// Also calculate the orientation matrix, used for orienting particles in space
		D3DXMatrixRotationQuaternion(&m_orientmatrix, &m_orientation);
	}


	// Methods to get and set other emitter properties
	Result						LoadTexture(ID3D11Device* device, const char *filename);
	CMPINLINE ID3D11ShaderResourceView 
								*GetParticleTexture(void) { return m_texture->GetTexture(); }
	CMPINLINE void				SetParticleTexture(Texture *tex) { m_texture = tex; }

	CMPINLINE bool				Exists(void) { return m_exists; }
	CMPINLINE void				SetExists(bool exists) { m_exists = exists; }
	CMPINLINE bool				IsEmitting(void) { return m_emitting; }
	CMPINLINE void				SetEmitting(bool emitting) { m_emitting = emitting; }
	CMPINLINE void				Activate(void) { m_exists = true; m_emitting = true; }
	CMPINLINE string			GetTypeCode(void) { return m_typecode; }
	CMPINLINE void				SetTypeCode(string typecode) { m_typecode = typecode; }
	CMPINLINE void				SetParticleLimit(int limit) { m_particlelimit = limit; m_vertexlimit = m_particlelimit * 6; }
	CMPINLINE int				GetParticleLimit(void) { return m_particlelimit; }
	CMPINLINE int				GetVertexLimit(void) { return m_vertexlimit; }
	CMPINLINE int				GetNumActiveParticles(void) { return m_numactiveparticles; }
	CMPINLINE float				GetParticleEmissionFrequency(Prop prop) { return m_emissionfreq[(int)prop]; }
	CMPINLINE void				SetParticleEmissionFrequency(Prop prop, float freq) { m_emissionfreq[(int)prop] = freq; }

	// Methods to set and get initial particle properties
	CMPINLINE D3DXVECTOR3		GetInitialParticleLocation(Prop prop) { return m_initialposition[(int)prop]; }
	CMPINLINE void				SetInitialParticleLocation(Prop prop, D3DXVECTOR3 pos) { m_initialposition[(int)prop] = pos; }
	CMPINLINE float				GetInitialParticleLifetime(Prop prop) { return m_initiallifetime[(int)prop]; }
	CMPINLINE void				SetInitialParticleLifetime(Prop prop, float life) { m_initiallifetime[(int)prop] = life; }
	CMPINLINE D3DXVECTOR4		GetInitialParticleColour(Prop prop) { return m_initialcolour[(int)prop]; }
	CMPINLINE void				SetInitialParticleColour(Prop prop, D3DXVECTOR4 col) { m_initialcolour[(int)prop] = col; }
	CMPINLINE float				GetInitialParticleSize(Prop prop) { return m_initialsize[(int)prop]; }
	CMPINLINE void				SetInitialParticleSize(Prop prop, float size) { m_initialsize[(int)prop] = size; }
	CMPINLINE D3DXVECTOR3		GetInitialParticleVelocity(Prop prop) { return m_initialvelocity[(int)prop]; }
	CMPINLINE void				SetInitialParticleVelocity(Prop prop, D3DXVECTOR3 vel) { m_initialvelocity[(int)prop] = vel; }

	// Methods to set and get particle update properties
	CMPINLINE D3DXVECTOR4		GetParticleColourUpdate(Prop prop) { return m_updatecolour[(int)prop]; }
	CMPINLINE void				SetParticleColourUpdate(Prop prop, D3DXVECTOR4 col) { m_updatecolour[(int)prop] = col; m_updateflag_colour = true; }
	CMPINLINE float				GetParticleSizeUpdate(Prop prop) { return m_updatesize[(int)prop]; }
	CMPINLINE void				SetParticleSizeUpdate(Prop prop, float size) { m_updatesize[(int)prop] = size; m_updateflag_size = true; }
	CMPINLINE D3DXVECTOR3		GetParticleVelocityUpdate(Prop prop) { return m_updatevelocity[(int)prop]; }
	CMPINLINE void				SetParticleVelocityUpdate(Prop prop, D3DXVECTOR3 vel) { m_updatevelocity[(int)prop] = vel; m_updateflag_velocity = true; }

	ParticleEmitter(void);
	~ParticleEmitter(void);



private:
	// Initialisation functions
	Result						InitialiseParticles(void);
	Result						InitialiseBuffers(ID3D11Device *device);

	// Rendering methods
	void						RenderBuffers(ID3D11DeviceContext *devicecontext);

	// Memory allocated for storage of particle & vertex data, plus buffers required for rendering
	Particle					*m_particles;						// The array of Particle objects
	ParticleVertex				*m_vertices;						// The array of vertices (6n, where n is number of particles)
	ID3D11Buffer				*m_vertexbuffer, *m_indexbuffer;	// Vertex and index buffers for rendering the particle data

	// Emitter properties
	string						m_code;								// String key of the unique emitter object
	string						m_typecode;							// String key of the prototype, or the prototye this instance is based on
	D3DXVECTOR3					m_position;							// Position of this emitter
	D3DXQUATERNION				m_orientation;						// Orientation of this emitter
	D3DXMATRIX					m_worldmatrix;						// World matrix for this emitter calculated from pos+orient whenever changed
	D3DXMATRIX					m_inverseworld;						// Inverse world matrix for the emitter, precalculated for rendering efficiency
	D3DXMATRIX					m_orientmatrix;						// Rotation component only of the world matrix

	bool						m_emitting;							// Determines whether the emitter is currently active (i.e. emitting particles)
	bool						m_exists;							// Determines whether the emitter even exists (i.e. whether to even render)

	int							m_numactiveparticles;				// The number of particles that are currently active
	int							m_maxparticleidcreated;				// The highest index of particle we have ever created (and thus upper bound we have to process)

	int							m_particlelimit;					// Maximum number of particles that can exist at any one time
	int							m_vertexlimit;						// Similar limit for vertices; always equal to 6*particle limit
	Texture						*m_texture;							// Texture applied to each particle (can be NULL for coloured particles)

	float						m_emissionfreq[2];					// Time between each particle emission (secs)
	float						m_timetonextemission;				// Time until the next particle should be emitted (secs)

	// Initial properties; each are [2] for the min/max range in which the value can fall
	float						m_initiallifetime[2];				// Initial lifetime for each particle (secs)
	D3DXVECTOR3					m_initialposition[2];				// Particle starting position, as offset from emitter location
	D3DXVECTOR4					m_initialcolour[2];					// Initial colour for each particle
	float						m_initialsize[2];					// Initial size of each particle
	D3DXVECTOR3					m_initialvelocity[2];				// Initial velocity of each particle

	// Update properties for per-frame updates; each are again [2] to allow specification of min/max
	D3DXVECTOR4					m_updatecolour[2];					// Change to particle colour (/sec)
	float						m_updatesize[2];					// Change to particle size (/sec)
	D3DXVECTOR3					m_updatevelocity[2];				// Change to particle velocity (/sec)

	// Flags that indicate whether each property needs to be updated per frame
	bool						m_updateflag_colour;
	bool						m_updateflag_size;
	bool						m_updateflag_velocity;

	// Method to recalculate the world matrix following a change to emitter position and/or orientation
	CMPINLINE void RecalculateWorldMatrix(void) 
	{ 
		// Set the world matrix to a rotation as specified by the orientation quaternion
		D3DXMatrixRotationQuaternion(&m_orientmatrix, &m_orientation);
		
		// Now set the translation components as per the emitter position
		D3DXMATRIX trans;
		D3DXMatrixTranslation(&trans, m_position.x, m_position.y, m_position.z);
		D3DXMatrixMultiply(&m_worldmatrix, &m_orientmatrix, &trans);

		// Finally also calculate the inverse world matrix for rendering efficiency (not any more; calculated 
		// when accessed since this is never used during normal execution)
		//D3DXMatrixInverse(&m_inverseworld, NULL, &m_worldmatrix);
	}

};


#endif