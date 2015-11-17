#include "DXLocaliser.h"
#include "Model.h"
#include "GameDataExtern.h"
#include "Utility.h"
#include "FireEffect.h"
#include "FireShader.h"

#include "EffectManager.h"


// Initialisation function for the effect manager
Result EffectManager::Initialise(ID3D11Device *device)
{
	Result result;

	// Load and initialise all the models used for rendering effects
	result = InitialiseEffectModelData(device);
	if (result != ErrorCodes::NoError) return result;

	// Nothing to do right now, simply return success
	return ErrorCodes::NoError;
}

// Loads and initialises all the models used for rendering effects.  Returns fatal error if any cannot be loaded
Result EffectManager::InitialiseEffectModelData(ID3D11Device *device)
{
	Result result;

	// Create the models that will be used to hold each set of model data
	m_model_unitsquare = new Model();
	m_model_unitcone = new Model();

	// 2D square model used for billboard rendering of effects
	result = m_model_unitsquare->Initialise(BuildStrFilename(D::DATA, "Models\\Misc\\unit_square.rjm").c_str(), NULL);
	if (result != ErrorCodes::NoError) return result;

	// Unit cone model used for rendering tapering effects, e.g. engine thrust 
	result = m_model_unitcone->Initialise(BuildStrFilename(D::DATA, "Models\\Misc\\unit_cone.rjm").c_str(), NULL);
	if (result != ErrorCodes::NoError) return result;

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
		e->Shutdown();
		delete e;
		e = NULL;
	}

	// Deallocate the model objects that are used for effect rendering
	if (m_model_unitsquare)
	{
		m_model_unitsquare->Shutdown();
		delete m_model_unitsquare;
		m_model_unitsquare = NULL;
	}
	if (m_model_unitcone) 
	{
		m_model_unitcone->Shutdown();
		delete m_model_unitcone;
		m_model_unitcone = NULL;
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
Result EffectManager::RenderFireEffect(	FireEffect *e, ID3D11DeviceContext* deviceContext, 
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
	
	// Render the effect model to vertex buffers now
	model->Render();

	// Now render the fireshader onto these buffered vertices
	return m_fireshader->Render(deviceContext, model->GetIndexCount(), world, view, projection, 
								e->GetFireTexture(), e->GetNoiseTexture(), e->GetAlphaTexture(),
								m_effecttimer, e->GetScrollSpeeds(), e->GetScaling(),
								e->GetDistortionParameters1(), e->GetDistortionParameters2(), e->GetDistortionParameters3(), 
								e->GetDistortionScale(), e->GetDistortionBias());

}





EffectManager::EffectManager(const DXLocaliser *locale)
{
	// Store a reference to the current DX locale
	m_locale = locale;

	// Reset the effect timer
	m_effecttimer = 0.0f;
}

EffectManager::~EffectManager(void)
{
}
