#pragma once

#include <memory>	
#include <DirectXMath.h>
#include "../Definitions/ByteString.h"
#include "VertexDefinitions.hlsl.h"
using namespace DirectX;

class ModelData
{
public:

	typedef Vertex_Standard				TVertex;

	// Header data
	unsigned int						ModelMaterialIndex;		// Note: not the same as MaterialId.  This is the index of materials loaded within this single model
	XMFLOAT3							MinBounds;
	XMFLOAT3							MaxBounds;
	XMFLOAT3							ModelSize;
	XMFLOAT3							CentrePoint;
	unsigned int						VertexCount;			// Determines total size of the VertexData array
	


	// Model data; uses the non-instanced definition since this class is used for [de]serialization from disk and instance data is not required
	TVertex *							VertexData;				// Array of per-vertex data only; no instance data included


public:

	ModelData(void);
	~ModelData(void);

	ByteString							Serialize(void) const;
	static std::unique_ptr<ModelData>	Deserialize(ByteString & data);

	void								AllocateVertexData(unsigned int vertex_count);
	void								DeallocateVertexData(void);


private:


};