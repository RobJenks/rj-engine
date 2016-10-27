#include "CoreEngine.h"
#include "ParticleEngine.h"
#include "iSpaceObject.h"

#include "SpaceEmitter.h"


// Recalculates the current position of this object
void SpaceEmitter::SimulateObject(void)
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
	iSpaceObject::InitialiseCopiedObject(source);
}

// Shuts down the emitter and releases all resources
void SpaceEmitter::Shutdown(void)
{
	// Shut down the associated particle emitter within the game engine
	if (m_emitter && m_emittercode != NullString) 
		Game::Engine->GetParticleEngine()->ShutdownParticleEmitter(m_emittercode);

	// Pass control back to the base class
	iSpaceObject::Shutdown();
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


// Default destructor
SpaceEmitter::~SpaceEmitter(void)
{
}

// Custom debug string function
std::string	SpaceEmitter::DebugString(void) const
{
	return iObject::DebugString(concat
		("Type=")(m_emitter ? m_emitter->GetTypeCode() : "(Error)")
		(", Active=")(m_emitter ? (m_emitter->IsEmitting() ? "True" : "False") : "(Error)").str());
}



// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void SpaceEmitter::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(GetEmitter)

	// Mutator methods
	REGISTER_DEBUG_FN(SimulateObject)
	REGISTER_DEBUG_FN(RefreshPositionImmediate)
	REGISTER_DEBUG_FN(DestroyObject)
	

	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		iSpaceObject::ProcessDebugCommand(command);

}
