////////////////////////////////////////////////////////////////////////////////
// Filename: LightHighlightShader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef __LightHighlightShaderH__
#define __LightHighlightShaderH__


//////////////
// INCLUDES //
//////////////
#include "DX11_Core.h"
#include "iShader.h"

// This class has no special alignment requirements
class LightHighlightShader : iShader
{
private:
	struct VSBufferType
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct PSBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding; 
	};

public:

	// Default constructor
	LightHighlightShader(void);

	// Initialise the shader object
	Result							Initialise(ID3D11Device*, HWND);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(ID3D11Device *device, std::string filename);
	Result							InitialisePixelShader(ID3D11Device *device, std::string filename);

	// Renders the shader.  Conforms to the iShader interface spec
	Result XM_CALLCONV				Render(	ID3D11DeviceContext *deviceContext, UINT vertexCount, UINT indexCount, UINT instanceCount,
											const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

	// Sets the parameters specific to the light shader, i.e. light type / direction / colour
	Result							SetLightParameters(XMFLOAT3 lightDirection, XMFLOAT4 ambientColor, XMFLOAT4 diffuseColor);

	// Shut down the shader and deallocate all associated resources
	void							Shutdown();

	// Default desctructor
	~LightHighlightShader();


protected:
	
	ID3D11VertexShader		* m_vertexShader;
	ID3D11PixelShader		* m_pixelShader;
	ID3D11InputLayout		* m_inputlayout;
	ID3D11SamplerState		* m_sampleState;
	ID3D11Buffer			* m_cbuffer_vs;
	ID3D11Buffer			* m_cbuffer_ps;

	XMFLOAT3				m_lightdirection;
	XMFLOAT4				m_ambientcolour;
	XMFLOAT4				m_diffusecolour;
};

#endif