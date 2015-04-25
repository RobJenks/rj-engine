#include <vector>
#include <string>
#include <random>
#include "ErrorCodes.h"
#include "GameDataExtern.h"
#include "GameVarsExtern.h"
#include "Actor.h"

#include "ActorBase.h"

ActorBase::ActorBase(void)
{	
	// Initialise all paramters to default values.  Attributes are automatically initialised to defaults
	m_code = "";
	m_name = "";
	m_model = NULL;
	m_size = NULL_VECTOR;
	m_mass = 10.0f;
	m_defaultanimation = NULL;
	
	m_viewoffset = D3DXVECTOR3(0.0f, 0.9f, 0.0f);
	m_headbobamount = Game::C_ACTOR_DEFAULT_HEAD_BOB_AMOUNT;
	m_headbobspeed = Game::C_ACTOR_DEFAULT_HEAD_BOB_SPEED;
}

// Method to create a new instance of this actor
Actor *ActorBase::CreateInstance(void)
{
	// Create a new instance to be populated, including the link back to this base class for the future
	Actor *a = new Actor(this);

	// Create a new instance of the skinned model and assign it to this actor
	if (!m_model) { delete a; a = NULL; return NULL; }
	m_model->CreateInstance(a->GetModelReference());
	
	// Generate a random set of attributes based on the base class traits
	for (int i=0; i<(int)ActorAttr::A_COUNT; i++)
		a->Attributes[i].BaseValue = this->Attributes[i].Generate();

	// Apply any other effects, e.g. random events influencing the actor traits, add equipped items etc.

	// Have the actor recalculate its final trait values once all base values and effects have been applied
	a->RecalculateAttributes();

	// Return a reference to the new actor
	return a;
}

// Attempts to look up the model from its code and calls the overloaded method
Result ActorBase::SetModel(const std::string & model)
{
	return SetModel(D::GetSkinnedModel(model));
}

// Sets the model used by this actor type
Result ActorBase::SetModel(SkinnedModel *model)
{
	// Make sure this is a valid skinned model
	if (!model) return ErrorCodes::CannotInitialiseActorWithInvalidModelCode;

	// Store a reference to this model
	m_model = model;

	// Retrieve the actor size from its underlying model data
	m_size = m_model->GetModelSize();

	// Player view offset will be retrieved from the model data
	m_viewoffset = m_model->GetViewOffset();

	// Return success
	return ErrorCodes::NoError;
}

ActorBase::~ActorBase(void)
{
}
