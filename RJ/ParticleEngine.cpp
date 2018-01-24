#include <cstdlib>
#include <string>
#include "DX11_Core.h"

#include "ErrorCodes.h"

#include "CameraClass.h"
#include "ParticleEmitter.h"
#include "ParticleShader.h"
#include "ParticleEngine.h"
#include "FastMath.h"
#include <vector>

Result ParticleEngine::Initialise(void)
{
	// Nothing to do at this point; simply return success
	return ErrorCodes::NoError;
}

Result ParticleEngine::LinkParticleShader(ParticleShader *pshader)
{
	// Make sure we have valid references
	if (!pshader) return ErrorCodes::CannotLinkAllRequiredShadersToParticleEngine;

	// Store reference to the particle shader that we will use for rendering
	m_pshader = pshader;

	// Return success
	return ErrorCodes::NoError;
}

Result ParticleEngine::AddEmitterPrototype(std::string key, ParticleEmitter *prototype)
{
	// Check parameters
	if (!prototype || key == NullString) return ErrorCodes::ReceivedNullParticleEmitterKey;

	// Make sure this emitter doesn't already exist
	if (m_prototypes.count(key) > 0) return ErrorCodes::ParticleEmitterKeyAlreadyAssigned;

	// Add to the collection and return success
	m_prototypes[key] = prototype;
	return ErrorCodes::NoError;
}

Result ParticleEngine::RemoveEmitterPrototype(std::string key)
{
	// Parameter check
	if (key == NullString) return ErrorCodes::ReceivedNullParticleEmitterKey;

	// Make sure this key exists
	if (m_prototypes.count(key) == 0) return ErrorCodes::CannotRemoveParticleEmitterThatDoesNotExist;

	// Get a handle to the prototype emitter
	ParticleEmitter *emit = m_prototypes[key];
	if (!emit) return ErrorCodes::CannotRemoveParticleEmitterThatDoesNotExist;

	// Call the emitter shutdown method and delete the prototype
	emit->Shutdown();
	delete emit;

	// Remove the emitter prototype key and return success
	m_prototypes[key] = NULL;
	return ErrorCodes::NoError;
}

ParticleEmitter *ParticleEngine::CreateNewParticleEmitter(std::string key, std::string prototype)
{
	// Parameter check
	if (key == NullString || prototype == NullString) return NULL;

	// Make sure this prototype exists, and that the target key doesn't exist
	if (m_prototypes.count(prototype) == 0) return NULL;
	if (m_emitters.count(key) > 0) return NULL;

	// Make sure the source prototype is valid
	ParticleEmitter *src = m_prototypes[prototype];
	if (!src) return NULL;

	// Create a new emitter based on this source prototype
	ParticleEmitter *e = m_prototypes[prototype]->CreateClone();

	// Set the unique string key of this emitter.  This must be unique & match the key used to index it into the collection
	e->SetCode(key);

	// Add emitter to the collection and return a reference
	m_emitters[key] = e;
	return e;
}

// Shuts down a specific emitter and releases all resources
void ParticleEngine::ShutdownParticleEmitter(std::string key)
{
	// Make sure this is a valid object
	if (m_emitters.count(key) == 0) return;

	// Get a handle to the emitter in question
	ParticleEmitter *emit = m_emitters[key];
	if (!emit) return;

	// Call the emitter shutdown method and then delete the object
	emit->Shutdown();
	delete emit; 
	
	// Finally set the emitter key to NULL to signify that the object no longer exists
	m_emitters[key] = NULL;
}

void RJ_XM_CALLCONV ParticleEngine::Render(const FXMMATRIX view, const CXMMATRIX proj, D3DMain *D3D, CameraClass *camera)
{
	/* TODO: Disabled since this does not work within the deferred rendering engine; replace with compliant implementation */

	// Enable alpha blending and disable writing to the depth buffer (for correct alpha rendering) before rendering any particles
	// TODO: Currently use additive alpha.  Make dependent on the prototypes being rendered (disabled vs enabled vs additive vs ...)
/*	D3D->SetAlphaBlendModeEnabled();
	D3D->DisableZBufferWriting();

	// Retrieve the camera basis vectors for use in billboarding particles
	XMFLOAT3 vright = camera->GetViewRightBasisVectorF();
	XMFLOAT3 vup = camera->GetViewUpBasisVectorF();

	// Operate on each particle emitter in turn
	ParticleEmitterCollection::const_iterator it_end = m_emitters.end();
	for (ParticleEmitterCollection::const_iterator it = m_emitters.begin(); it != it_end; ++it) 
	{
		// Get a reference to this particle emitter, and make sure it is active
		ParticleEmitter *e = it->second;
		if (!e || !e->Exists()) continue;

		// TODO: Perform distance/view/frustrum test to see if we should actually render it
		if (false) continue;

		// Prepare all particles, calculate vertex data and promote all to the buffer ready for shader rendering
		e->Render(D3D->GetDeviceContext(), vright, vup);

		// Render particles using the particle shader
		m_pshader->Render( D3D->GetDeviceContext(), e->GetVertexLimit(), ID_MATRIX,
						   view, proj, e->GetParticleTexture() );
	}

	// Disable alpha blending after rendering all particle emitters
	// TODO: Move to somewhere more global once we have further rendering that requires alpha effects.  Avoid multiple on/off switches
	D3D->SetAlphaBlendModeDisabled();
	D3D->EnableZBuffer();
	*/
}

void ParticleEngine::Shutdown(void)
{
	// Loop through each particle emitter in turn and dispose of it
	ParticleEmitterCollection::const_iterator it_end = m_emitters.end();
	for (ParticleEmitterCollection::const_iterator it = m_emitters.begin(); it != it_end; ++it) 
	{
		// Get a reference to this particle emitter
		ParticleEmitter *e = it->second;
		if (!e) continue;

		// Call the emitter shutdown method
		e->Shutdown();

		// Delete and deallocate the object
		delete e;
		e = NULL;
	}

	// Clear out the emitter collection
	m_emitters.clear();
}


ParticleEngine::ParticleEngine(void)
{
	// Set all key pointers to NULL
	m_pshader = NULL;
}

ParticleEngine::~ParticleEngine(void)
{
}



