#pragma once

#ifndef __EffectManagerH__
#define __EffectManagerH__


#include <vector>

class Model;
class EffectBase;
class FireEffect;
class FireShader;


// This class has no special alignment requirements
class EffectManager
{
public:
	EffectManager(void);
	~EffectManager(void);

	// Initialisation and preparation functions
	Result Initialise(ID3D11Device *device);
	Result InitialiseEffectModelData(ID3D11Device *device);
	Result LinkEffectShaders(FireShader *fireshader);
	void BeginEffectRendering(void);

	// Shutdown functions
	void Shutdown(void);


	/* Fire effects */

	// Standard methods: Add, Get and Render a fire effect type
	void					AddFireEffectType(FireEffect *e);
	CMPINLINE FireEffect*	GetFireEffectType(int effectIndex) { return m_fireeffects.at(effectIndex); }
	Result XM_CALLCONV 		RenderFireEffect(FireEffect *e, ID3D11DeviceContext* deviceContext,
											 FXMMATRIX world, CXMMATRIX view, CXMMATRIX projection);
	CMPINLINE Result XM_CALLCONV 	RenderFireEffect(int effectindex, ID3D11DeviceContext* deviceContext,
											 FXMMATRIX world, CXMMATRIX view, CXMMATRIX projection)
											{ return RenderFireEffect(m_fireeffects.at(effectindex), deviceContext, world, view, projection); }



private:
	float						m_effecttimer;					// Current effect timer (secs), wraps at 1000.0

	// Effect collections
	std::vector<FireEffect*>	m_fireeffects;

	// References to all required shader objects
	FireShader					*m_fireshader;

	// Model objects used for rendering to the world
	Model						*m_model_unitsquare;
	Model						*m_model_unitcone;
};


#endif