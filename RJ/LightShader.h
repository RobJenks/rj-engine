////////////////////////////////////////////////////////////////////////////////
// Filename: LightShader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef __LightShaderH__
#define __LightShaderH__


//////////////
// INCLUDES //
//////////////
#include "DX11_Core.h"
#include "Rendering.h"
#include "iShader.h"

// This class has no special alignment requirements
class LightShader : iShader
{
private:
	struct VSBufferType
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

public:

	// Default constructor
	LightShader(void);
	
	// Initialise the shader object
	Result							Initialise(Rendering::RenderDeviceType *, HWND);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(Rendering::RenderDeviceType  *device, std::string filename);
	Result							InitialisePixelShader(Rendering::RenderDeviceType  *device, std::string filename);
	
	// Renders the shader.  Conforms to the iShader interface spec
	Result XM_CALLCONV				Render(	Rendering::RenderDeviceContextType  *deviceContext, UINT vertexCount, UINT indexCount, UINT instanceCount,
											const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

	// Shut down the shader and deallocate all associated resources
	void							Shutdown();

	// Default desctructor
	~LightShader();

protected:
	
	ID3D11VertexShader		* m_vertexShader;
	ID3D11PixelShader		* m_pixelShader;
	ID3D11InputLayout		* m_inputlayout;
	ID3D11SamplerState		* m_sampleState;
	ID3D11Buffer			* m_cbuffer_vs;
	ID3D11Buffer			* m_cbuffer_ps;
};

#endif