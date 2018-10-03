#include "VolumetricLine.h"
#include "ModelBuffer.h"

// Default values for additional parameter vector
const XMFLOAT4 VolumetricLine::DEFAULT_PARAMS = XMFLOAT4(3.0f, 0.0f, 0.0f, 0.0f);

// Static provider for vertex and other model data
std::unique_ptr<Vertex_Standard> VolumetricLine::SPDefaultVertex;
std::unique_ptr<ModelBuffer>	 VolumetricLine::SPDefaultModel;

void VolumetricLine::InitialiseStaticData(void)
{
	auto v = new Vertex_Standard[1]();
	v[0].position = XMFLOAT3(0, 0, 0);
	v[0].normal = XMFLOAT3(0, 0, 1);
	v[0].tex = XMFLOAT2(0, 0);
	v[0].binormal = XMFLOAT3(0, 0, 0);
	v[0].tangent = XMFLOAT3(0, 0, 0);
	
	SPDefaultVertex.reset(v);
	SPDefaultModel = std::make_unique<ModelBuffer>((const void**)&v, static_cast<unsigned int>(sizeof(v[0])), 1U, (const MaterialDX11*)NULL);
}

ModelBuffer * VolumetricLine::DefaultModel(void)
{
	return SPDefaultModel.get();
}