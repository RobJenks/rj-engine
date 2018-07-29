#include "ShadowManagerComponent.h"

// Constructor
ShadowManagerComponent::ShadowManagerComponent(void)
{
	// Only for default intialisation
}

// Constructor
ShadowManagerComponent::ShadowManagerComponent(DeferredRenderProcess *renderprocess)
	:
	m_renderprocess(renderprocess)
{
}

// Initialisation
void ShadowManagerComponent::InitialiseShaders(void)
{

}

// Initialisation of all components which are config-dependent or should be refreshed on device parameters change (e.g. display size)
void ShadowManagerComponent::PerformPostConfigInitialisation(void)
{

}

// Destructor
ShadowManagerComponent::~ShadowManagerComponent(void)
{

}
