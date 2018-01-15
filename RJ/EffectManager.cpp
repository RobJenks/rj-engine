#include "CoreEngine.h"
#include "Model.h"
#include "GameDataExtern.h"
#include "Utility.h"
#include "FireEffect.h"
#include "FireShader.h"

#include "EffectManager.h"


// Initialisation function for the effect manager
Result EffectManager::Initialise(Rendering::RenderDeviceType *device)
{
	Result result;

	// Load and initialise all the models used for rendering effects
	result = InitialiseEffectModelData(device);
	if (result != ErrorCodes::NoError) return result;

	// Nothing to do right now, simply return success
	return ErrorCodes::NoError;
}

// Loads and initialises all the models used for rendering effects.  Returns fatal error if any cannot be loaded
Result EffectManager::InitialiseEffectModelData(Rendering::RenderDeviceType *device)
{
	Result result;

	// Create the models that will be used to hold each set of model data
	m_model_unitsquare = new Model();
	m_model_unitcone = new Model();

	// 2D square model used for billboard rendering of effects
	m_model_unitsquare = Model::GetModel("unit_square");
	if (!m_model_unitsquare) return ErrorCodes::CouldNotInitialiseEffectManager;

	// Unit cone model used for rendering tapering effects, e.g. engine thrust 
	m_model_unitcone = Model::GetModel("unit_cone");
	if (!m_model_unitcone) return ErrorCodes::CouldNotInitialiseEffectManager;

	// Return success if all models have been loaded successfully
	return ErrorCodes::NoError;
}


// Forms a link to all shaders used by this effect manager 
Result EffectManager::LinkEffectShaders(FireShader *fireshader)
{
	// Validate that all references are valid
	if (!fireshader) return ErrorCodes::CannotLinkAllRequiredShadersToEffectManager;

	// Store references to all required shader components
	m_fireshader = fireshader;

	// Return success
	return ErrorCodes::NoError;
}

// Shuts down the effect manager
void EffectManager::Shutdown(void)
{
	// Shut down and deallocate each effect in turn
	int n = (int)m_fireeffects.size();
	for (int i=0; i<n; i++)
	{
		FireEffect *e = m_fireeffects.at(i);
		SafeDelete(e);
	}
}

// Begins the rendering of effects for a frame.  Updates the cumulative frame timer that is used for rendering 
// effects at a consistent speed regardless of FPS etc.
void EffectManager::BeginEffectRendering(void)
{
	// Update the cumulative timer with the latest additional time delta
	m_effecttimer += Game::TimeFactor;
	if (m_effecttimer > 1000.0f) m_effecttimer -= 1000.0f;
}


// Fire effect - Add prototype
void EffectManager::AddFireEffectType(FireEffect *e)
{
	// Add to the fire effect collection
	m_fireeffects.push_back(e);
}

// Fire effect - Render
Result XM_CALLCONV EffectManager::RenderFireEffect(FireEffect *e, Rendering::RenderDeviceContextType* deviceContext,
										FXMMATRIX world, CXMMATRIX view, CXMMATRIX projection)
{
	// Determine the model object to use in rendering this effect
	Model *model;
	switch (e->GetEffectModel())
	{
		case EffectBase::EffectModelType::UnitCone:		// Unit cone
			model = m_model_unitcone; break;
		default:										// Unit square (default)
			model = m_model_unitsquare; break;
	}
	
	/*** TODO: This will no longer work under the deferred rendering engine.  However we will likely be replacing it anyway ***/

	// Render the effect model to vertex buffers now
	//model->Render();

	// Now render the fireshader onto these buffered vertices
	/*return m_fireshader->Render(deviceContext, model->GetIndexCount(), world, view, projection, 
								e->GetFireTexture(), e->GetNoiseTexture(), e->GetAlphaTexture(),
								m_effecttimer, e->GetScrollSpeeds(), e->GetScaling(),
								e->GetDistortionParameters1(), e->GetDistortionParameters2(), e->GetDistortionParameters3(), 
								e->GetDistortionScale(), e->GetDistortionBias());*/

	return ErrorCodes::NoError;
}





EffectManager::EffectManager(void)
{
	// Reset the effect timer
	m_effecttimer = 0.0f;
}

EffectManager::~EffectManager(void)
{
}
