#include "CoreEngine.h"
#include "ParticleEngine.h"
#include "iSpaceObject.h"

#include "SpaceEmitter.h"


// Recalculates the current position of this object
void SpaceEmitter::SimulateObject(bool PermitMovement)
{
	// The only action a space emitter needs to take is ensure that its associated particle emitter is brought along with it
	if (m_simulationstate == iObject::ObjectSimulationState::FullSimulation)
	{
		m_emitter->SetPositionOrientAndWorldNoRecalc(m_position, m_orientation, m_worldmatrix);
	}
}


// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void SpaceEmitter::InitialiseCopiedObject(SpaceEmitter *source)
{
	// Pass control to all base classes
	iSpaceObject::InitialiseCopiedObject((iSpaceObject*)source);
}

// Shuts down the emitter and releases all resources
void SpaceEmitter::Shutdown(void)
{
	// Shut down the associated particle emitter within the game engine
	if (m_emitter && m_emittercode != NullString) 
		Game::Engine->GetParticleEngine()->ShutdownParticleEmitter(m_emittercode);
}

SpaceEmitter::SpaceEmitter(void)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::SpaceEmitterObject);

	// Initialise all pointers and key fields upon creation
	m_emitter = NULL;
}

void SpaceEmitter::SetEmitter(ParticleEmitter *emitter)
{ 
	// Parameter check; if NULL is provided then we want to remove the emitter entirely
	if (!emitter) { m_emitter = NULL; m_emittercode = ""; return; }

	// Store the emitter, and also the unique code of the emitter for later deallocation
	m_emitter = emitter; 
	m_emittercode = emitter->GetCode();
}



SpaceEmitter::~SpaceEmitter(void)
{
}
