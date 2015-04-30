#pragma once

#ifndef __D3DMainH__
#define __D3DMainH__

// Include all DX11-related headers
#include "DX11_Core.h" // #include "FullDX11.h"
#include "ErrorCodes.h"
#include "CompilerSettings.h"
class DXLocaliser;

// Constant values for buffer background colour; each frame reset to this base colour before rendering begins
const float					m_bufferbg[4] = {0.0f, 0.0f, 0.0f, 1.0f};

////////////////////////////////////////////////////////////////////////////////
// Class name: D3DMain
////////////////////////////////////////////////////////////////////////////////
class D3DMain
{
public:
	// Enumeration of possible alpha blending values
	enum AlphaBlendState { AlphaBlendDisabled = 0, AlphaBlendEnabledNormal, AlphaBlendEnabledAdditive };

	D3DMain(const DXLocaliser *locale);		// Constructor; must provide a localiser component
	D3DMain(const D3DMain &copy);			// Copy constructor; currently empty
	~D3DMain();

	Result							Initialise(int, int, bool, HWND, bool, float, float);
	void							Shutdown();
	
	void							BeginScene();
	void							EndScene();

	CMPINLINE ID3D11Device*			GetDevice()					{ return m_device; }
	CMPINLINE ID3D11DeviceContext*	GetDeviceContext()	{ return m_deviceContext; }

	void							GetProjectionMatrix(D3DXMATRIX&);
	void							GetWorldMatrix(D3DXMATRIX&);
	void							GetOrthoMatrix(D3DXMATRIX&);

	void							GetVideoCardInfo(char*, int&);

	// Note: Must reinitialise all D3D components if we change the driver type; is used at creation of device/swap chain
	CMPINLINE void					SetDeviceDriverType(D3D_DRIVER_TYPE d3d_dt) { m_devicetype = d3d_dt; }
	CMPINLINE D3D_DRIVER_TYPE		GetDeviceDriverType(void) { return m_devicetype; }

	void							SetFillMode(D3D11_FILL_MODE fillmode);
	CMPINLINE D3D11_FILL_MODE		GetFillMode(void) { return m_fillmode; }

	void							EnableZBuffer(void);
	void							DisableZBufferWriting(void);
	void							DisableZBuffer(void);

	void							EnableRasteriserCulling(void);
	void							DisableRasteriserCulling(void);

	CMPINLINE float					GetDisplayFOV(void) { return m_FOV; }
	CMPINLINE float					GetDisplayAspectRatio(void) { return m_aspectratio; }
	
	// Returns the current alpha blending state
	CMPINLINE AlphaBlendState		GetAlphaBlendState(void) const { return m_alphablendstate; }

	// Sets alpha blending state; passes to the relevant method depending on the mode required
	CMPINLINE void					SetAlphaBlendState(AlphaBlendState state)
	{
		switch (state)
		{
			case AlphaBlendState::AlphaBlendEnabledNormal: 
				SetAlphaBlendModeEnabled(); return;
			case AlphaBlendState::AlphaBlendEnabledAdditive:
				SetAlphaBlendModeAdditive(); return;
			default:
				SetAlphaBlendModeDisabled(); return;
		}
	}

	// Sets alpha blending to enabled and normal
	CMPINLINE void					SetAlphaBlendModeEnabled(void) 
	{ 
		m_alphablendstate = AlphaBlendState::AlphaBlendEnabledNormal;
		m_deviceContext->OMSetBlendState(m_alphaEnableBlendingState, m_alphablendfactor, 0xffffffff); 
	}
	
	// Enables alpha blending and applies additive filters (for e.g. effect rendering)
	CMPINLINE void					SetAlphaBlendModeAdditive(void) 
	{
		m_alphablendstate = AlphaBlendState::AlphaBlendEnabledAdditive;
		m_deviceContext->OMSetBlendState(m_alphaEnableAdditiveBlendingState, m_alphablendfactor, 0xffffffff); 
	}

	// Disables alpha blending
	CMPINLINE void					SetAlphaBlendModeDisabled(void) 
	{
		m_alphablendstate = AlphaBlendState::AlphaBlendDisabled;
		m_deviceContext->OMSetBlendState(m_alphaDisableBlendingState, m_alphablendfactor, 0xffffffff); 
	}


private:
	const DXLocaliser			*m_locale;
	D3D_DRIVER_TYPE				m_devicetype;
	bool						m_vsync_enabled;
	int							m_videoCardMemory;
	char						m_videoCardDescription[128];
	IDXGISwapChain				*m_swapChain;
	ID3D11Device				*m_device;
	ID3D11DeviceContext			*m_deviceContext;
	ID3D11RenderTargetView		*m_renderTargetView;
	ID3D11Texture2D				*m_depthStencilBuffer;
	ID3D11DepthStencilState		*m_depthStencilState;					// For normal rendering
	ID3D11DepthStencilState		*m_depthDisabledStencilState;			// No depth testing at all, for UI/2D rendering
	ID3D11DepthStencilState		*m_depthStencilStateNoDepthWrite;		// No depth write, for some alpha blending operations
	ID3D11DepthStencilView		*m_depthStencilView;
	ID3D11RasterizerState		*m_rasterState;							// Normal rasterising state
	ID3D11RasterizerState		*m_rasterStateNoCulling;				// Performs no culling; used for drawing e.g. inside of the skybox
	D3DXMATRIX					m_projectionMatrix;
	D3DXMATRIX					m_worldMatrix;
	D3DXMATRIX					m_orthoMatrix;
	D3D11_FILL_MODE				m_fillmode;
	ID3D11BlendState			*m_alphaEnableBlendingState;
	ID3D11BlendState			*m_alphaDisableBlendingState;
	ID3D11BlendState			*m_alphaEnableAdditiveBlendingState;
	float						m_alphablendfactor[4];
	float						m_FOV;
	float						m_aspectratio;
	AlphaBlendState				m_alphablendstate;
};




#endif