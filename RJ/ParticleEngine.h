#pragma once

#ifndef __ParticleEngineH__
#define __ParticleEngineH__

#include "DX11_Core.h"

#include <vector>
#include <unordered_map>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "ParticleEmitter.h"
using namespace std;

class D3DMain;

class CameraClass;
class ParticleShader;

// This class has no special alignment requirements
class ParticleEngine
{
public:
	typedef unordered_map<string, ParticleEmitter*> ParticleEmitterCollection;

	// Initialisation methods
	Result Initialise(void);
	Result LinkParticleShader(ParticleShader *pshader);

	// Methods to add/retrieve/remove a particle emitter prototype
	CMPINLINE ParticleEmitterCollection *GetEmitterPrototypes(void) { return &m_prototypes; }
	CMPINLINE ParticleEmitter *GetEmitterPrototype(string key) { return m_prototypes[key]; }
	Result AddEmitterPrototype(string key, ParticleEmitter *prototype);
	Result RemoveEmitterPrototype(string key);

	// Methods to add/retrieve/remove a particle emitter, based on a prototype object
	CMPINLINE ParticleEmitterCollection *	GetEmitters(void) { return &m_emitters; }
	CMPINLINE ParticleEmitter *				GetEmitter(string key) { return m_emitters[key]; }
	ParticleEmitter *						CreateNewParticleEmitter(string key, string prototype);
	void									ShutdownParticleEmitter(string key);


	// Rendering methods
	void XM_CALLCONV Render(const FXMMATRIX view, const CXMMATRIX proj, D3DMain *D3D, CameraClass *camera);

	// Shutdown methods
	void Shutdown(void);

	ParticleEngine(void);
	~ParticleEngine(void);

private:

	// Collection of particle emitter prototypes, that are used to create new instances of particle emitter
	unordered_map<string, ParticleEmitter*> m_prototypes;

	// Collection of particle emitters maintained by this particle engine
	unordered_map<string, ParticleEmitter*> m_emitters;

	// Particle shader used for rendering
	ParticleShader *m_pshader;
};


#endif