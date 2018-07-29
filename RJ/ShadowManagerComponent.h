#pragma once

class DeferredRenderProcess;


class ShadowManagerComponent
{
public:

	// Constructor
	ShadowManagerComponent(void);
	ShadowManagerComponent(DeferredRenderProcess *renderprocess);

	// Initialisation
	void										InitialiseShaders(void);
	void										PerformPostConfigInitialisation(void);

	// Destructor
	~ShadowManagerComponent(void);

private:

	DeferredRenderProcess *						m_renderprocess;



};