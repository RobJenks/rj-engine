#ifndef __VolLineShaderH__
#define __VolLineShaderH__

#include "iShader.h"
#include "DX11_Core.h"
#include "ErrorCodes.h"
class DXLocaliser;

class VolLineShader : public iShader
{
protected:

	// Constant buffers for each shader in the pipeline
	struct VSBufferType
	{
		D3DXMATRIX viewmatrix;
	};

	struct GSBufferType
	{
		D3DXMATRIX	projectionmatrix;
		float		radius;
		D3DXVECTOR3	PADDING;		// Add extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
	};

	struct PSBufferType
	{
		float radius;				// Radius of the line
		float clipdistance_far;		// Distance to the far clip plane
		float clipdistance_front;	// Distance to the near clip plane
		D3DXVECTOR2 viewport_size;	// Size of the viewport
		D3DXVECTOR3	PADDING;		// Add extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
	};

public:

	// Constructor
	VolLineShader(const DXLocaliser *locale);

	// Initialise the shader
	Result							Initialise(ID3D11Device *device, D3DXVECTOR2 viewport_size, float clip_near, float clip_far);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(ID3D11Device *device, std::string filename);
	Result							InitialiseGeometryShader(ID3D11Device *device, std::string filename);
	Result							InitialisePixelShader(ID3D11Device *device, std::string filename);

	// Renders the shader.
	Result							Render(	ID3D11DeviceContext *deviceContext, int vertexCount, int indexCount, int instanceCount,
											D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

	// Adjust the radius of lines currently being drawn
	CMPINLINE void					SetLineRadius(float radius) { m_radius = radius; }

	// Shut down and deallocate all resources
	void							Shutdown();


protected:

	const DXLocaliser		* m_locale;
	ID3D11VertexShader		* m_vertexShader;
	ID3D11GeometryShader	* m_geometryShader;
	ID3D11PixelShader		* m_pixelShader;
	ID3D11InputLayout		* m_inputlayout;
	ID3D11SamplerState		* m_sampleState;
	ID3D11Buffer			* m_cbuffer_vs;
	ID3D11Buffer			* m_cbuffer_gs;
	ID3D11Buffer			* m_cbuffer_ps;

	D3DXVECTOR2				m_viewport_size;
	float					m_clip_near;
	float					m_clip_far;
	float					m_radius;
};


#endif