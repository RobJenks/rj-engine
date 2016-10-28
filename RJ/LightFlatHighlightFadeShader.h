////////////////////////////////////////////////////////////////////////////////
// Filename: LightFlatHighlightFadeShader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef __LightFlatHighlightFadeShaderH__
#define __LightFlatHighlightFadeShaderH__


//////////////
// INCLUDES //
//////////////
#include "DX11_Core.h"
#include "iShader.h"


// This class has no special alignment requirements
class LightFlatHighlightFadeShader : iShader
{
private:
	struct VSBufferType
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};


public:

	// Default constructor
	LightFlatHighlightFadeShader(void);

	// Initialise the shader object
	Result							Initialise(ID3D11Device*, HWND);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(ID3D11Device *device, std::string filename);
	Result							InitialisePixelShader(ID3D11Device *device, std::string filename);

	// Renders the shader.  Conforms to the iShader interface spec
	Result XM_CALLCONV				Render(ID3D11DeviceContext *deviceContext, UINT vertexCount, UINT indexCount, UINT instanceCount,
		const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

	// Shut down the shader and deallocate all associated resources
	void							Shutdown();

	// Default desctructor
	~LightFlatHighlightFadeShader();


protected:

	ID3D11VertexShader		* m_vertexShader;
	ID3D11PixelShader		* m_pixelShader;
	ID3D11InputLayout		* m_inputlayout;
	ID3D11SamplerState		* m_sampleState;
	ID3D11Buffer			* m_cbuffer_vs;
	ID3D11Buffer			* m_cbuffer_ps;
};

#endif