#pragma once

#ifndef __SpaceEmitterH__
#define __SpaceEmitterH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "ParticleEmitter.h"

#include "iSpaceObject.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class SpaceEmitter : public iSpaceObject
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(SpaceEmitter)

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void						InitialiseCopiedObject(SpaceEmitter *source);

	// Gets a reference to the particle emitter encapsulated by this space object
	ParticleEmitter*			GetEmitter(void) { return m_emitter; }
	void						SetEmitter(ParticleEmitter *emitter);

	// Recalculates the current position of this object
	void						SimulateObject(void);

	// Virtual inheritance from iObject.  No action since this object does not collide with anything
	CMPINLINE void				CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact) { }

	// Virtual inheritance from iObject.  There are no intra-frame activities we can take to refresh our data, since  
	// emitters are only updated by some parent object and cannot change themselves outside of that
	CMPINLINE void				RefreshPositionImmediate(void) { }
	
	// Event triggered upon destruction of the object; inherited from iObject/iTakesDamage and not required here
	CMPINLINE void				DestroyObject(void) { }

	// Shutdown method to deallocate & delete all object resources
	void						Shutdown(void);

	// Custom debug string function
	std::string					DebugString(void) const;
	
	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void						ProcessDebugCommand(GameConsoleCommand & command);


	SpaceEmitter(void);
	~SpaceEmitter(void);


private:
	ParticleEmitter*				m_emitter;					// The particle emitter encapsulated by this space object
	std::string						m_emittercode;				// String code of the emitter assigned to this object


};



#endif