#ifndef __VolLineShaderH__
#define __VolLineShaderH__

#include "iShader.h"
#include "DX11_Core.h"
#include "ErrorCodes.h"

class Texture;
class ModelBuffer;
class VolumetricLineRenderData;


// This class has no special alignment requirements
class VolLineShader : public iShader
{
public:

	// Static one-vertex base model used as input to the geometry shader, which will then break out into a volumetric line
	static ModelBuffer *										BaseModel;

	// Static collection of line models, indexed by render texture
	static std::unordered_map<Texture*, ModelBuffer*>			LineModels;

	// Constant buffers for each shader in the pipeline
	__declspec(align(16))
	struct VSBufferType
	{
		XMFLOAT4X4 viewmatrix;
	};

	__declspec(align(16))
	struct GSBufferType
	{
		XMFLOAT4X4	projectionmatrix;
	};

	__declspec(align(16))
	struct PSBufferType
	{
		float clipdistance_far;		// Distance to the far clip plane
		float clipdistance_front;	// Distance to the near clip plane
		XMFLOAT2 viewport_size;		// Size of the viewport
	};

public:

	// Constructor
	VolLineShader(void);

	// Initialise the shader
	Result							Initialise(ID3D11Device *device, XMFLOAT2 viewport_size, float clip_near, float clip_far);

	// Methods to initialise each shader in the pipeline in turn
	Result							InitialiseVertexShader(ID3D11Device *device, std::string filename);
	Result							InitialiseGeometryShader(ID3D11Device *device, std::string filename);
	Result							InitialisePixelShader(ID3D11Device *device, std::string filename);
	Result							InitialisePixelShaderTextured(ID3D11Device *device, std::string filename);

	// Renders the shader.
	Result XM_CALLCONV				Render(ID3D11DeviceContext *deviceContext, unsigned int vertexCount, unsigned int indexCount, unsigned int instanceCount,
											const FXMMATRIX viewMatrix, const CXMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

	// Adjust the radius of lines currently being drawn
	CMPINLINE void					SetLineRadius(float radius) { m_radius = radius; }

	// Shut down and deallocate all resources
	void							Shutdown();

	// Initialise the static data used in volumetric line rendering
	static Result					InitialiseStaticData(ID3D11Device *device);

	// Returns a model appropriate for rendering volumetric lines with the specified texture, or for pure
	// non-textured volumetric lines if render_texture == NULL
	static ModelBuffer *			LineModel(Texture *render_texture);

	// Creates a new line model appropriate for rendering volumetric lines with the specified texture, or for pure
	// non-textured volumetric lines if render_texture == NULL
	static ModelBuffer *			CreateLineModel(Texture *render_texture);

protected:
	
	ID3D11VertexShader		* m_vertexShader;
	ID3D11GeometryShader	* m_geometryShader;
	ID3D11PixelShader		* m_pixelShader;
	ID3D11PixelShader		* m_pixelShader_tex;
	ID3D11InputLayout		* m_inputlayout;
	ID3D11SamplerState		* m_sampleState;
	ID3D11Buffer			* m_cbuffer_vs;
	ID3D11Buffer			* m_cbuffer_gs;
	ID3D11Buffer			* m_cbuffer_ps;

	XMFLOAT2				m_viewport_size;
	float					m_clip_near;
	float					m_clip_far;
	float					m_radius;


	// Static texture resource for linear depth rendering across all line models
	static Texture *											LinearDepthTextureObject;
	static ID3D11ShaderResourceView *							LinearDepthTexture;

	// Staticx array of texture resource views for rendering efficiency when passing multiple textures to the renderer
	static ID3D11ShaderResourceView **							PSShaderResources;

};


#endif