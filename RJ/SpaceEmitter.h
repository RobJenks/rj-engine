#pragma once

#ifndef __SpaceEmitterH__
#define __SpaceEmitterH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "ParticleEmitter.h"

#include "iSpaceObject.h"

class SpaceEmitter : public iSpaceObject
{
public:
	
	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void								InitialiseCopiedObject(SpaceEmitter *source);

	// Gets a reference to the particle emitter encapsulated by this space object
	ParticleEmitter*			GetEmitter(void) { return m_emitter; }
	void						SetEmitter(ParticleEmitter *emitter);

	// Recalculates the current position of this object
	void						SimulateObject(void);

	// Virtual inheritance from iObject.  No action since this object does not collide with anything
	CMPINLINE void				CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact) { }

	// Virtual inheritance from iObject.  There are no intra-frame activities we can take to refresh our data, since  
	// emitters are only updated by some parent object and cannot change themselves outside of that
	CMPINLINE void				RefreshPositionImmediate(void) { }
	
	// Shutdown method to deallocate & delete all object resources
	void						Shutdown(void);


	SpaceEmitter(void);
	~SpaceEmitter(void);


private:
	ParticleEmitter*				m_emitter;					// The particle emitter encapsulated by this space object
	std::string						m_emittercode;				// String code of the emitter assigned to this object


};



#endif