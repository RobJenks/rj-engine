#pragma once

#ifndef __Render2DManagerH__
#define __Render2DManagerH__

#include <unordered_map>
#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "Render2DGroup.h"
class DXLocaliser;
class TextureShader;
class GameInputDevice;
using namespace std;
using namespace std::tr1;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Render2DManager : public ALIGN16<Render2DManager>
{
public:
	typedef unordered_map<string, Render2DGroup*> RenderGroupCollection; 

	Render2DManager(const DXLocaliser *locale);
	~Render2DManager(void);

	CMPINLINE RenderGroupCollection *RenderGroups(void) { return &m_rendergroups; }
	Render2DGroup *CreateRenderGroup(string code);

	CMPINLINE Render2DGroup *GetRenderGroup(string code) 
	{ 
		if (m_rendergroups.count(code) > 0)					return m_rendergroups[code];
		else												return NULL;
	}

	void ActivateGroup(string code);
	void DeactivateGroup(string code);
	void DeactivateAllGroups(void);

	void ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse);

	Result Initialise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, 
					  int screenWidth, int screenHeight, const FXMMATRIX baseviewmatrix);

	void Render(void);
	void Shutdown(void);

private:
	RenderGroupCollection	m_rendergroups;

	const DXLocaliser*		m_locale;
	ID3D11Device*			m_device;
	ID3D11DeviceContext*	m_devicecontext;
	HWND					m_hwnd;
	int						m_screenwidth, m_screenheight;

	AXMMATRIX				m_baseworldmatrix;
	AXMMATRIX				m_baseviewmatrix;
};


#endif