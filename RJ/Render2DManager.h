#pragma once

#ifndef __Render2DManagerH__
#define __Render2DManagerH__

#include <unordered_map>
#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "CompilerSettings.h"
#include "Render2DGroup.h"

class TextureShader;
class GameInputDevice;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class Render2DManager : public ALIGN16<Render2DManager>
{
public:
	typedef std::unordered_map<std::string, Render2DGroup*> RenderGroupCollection;

	Render2DManager(void);
	~Render2DManager(void);

	CMPINLINE RenderGroupCollection *RenderGroups(void) { return &m_rendergroups; }
	Render2DGroup *CreateRenderGroup(std::string code);

	CMPINLINE Render2DGroup *GetRenderGroup(std::string code)
	{ 
		if (m_rendergroups.count(code) > 0)					return m_rendergroups[code];
		else												return NULL;
	}

	void ActivateGroup(std::string code);
	void DeactivateGroup(std::string code);
	void DeactivateAllGroups(void);

	void ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse);

	Result XM_CALLCONV Initialise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd,
					  int screenWidth, int screenHeight, const FXMMATRIX baseviewmatrix);

	void Render(void);
	void Shutdown(void);

private:
	RenderGroupCollection	m_rendergroups;

	ID3D11Device*			m_device;
	ID3D11DeviceContext*	m_devicecontext;
	HWND					m_hwnd;
	int						m_screenwidth, m_screenheight;

	AXMMATRIX				m_baseworldmatrix;
	AXMMATRIX				m_baseviewmatrix;
};


#endif