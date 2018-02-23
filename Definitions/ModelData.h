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

	static const std::string 			GEOMETRY_FILE_IDENTIFIER;
	static const unsigned int			VERTEX_COUNT_LIMIT = 10000000;		// 10e6
	static const unsigned int			INDEX_COUNT_LIMIT = VERTEX_COUNT_LIMIT;

	// Header data
	unsigned int						ModelMaterialIndex;		// Note: not the same as MaterialId.  This is the index of materials loaded within this single model
	XMFLOAT3							MinBounds;
	XMFLOAT3							MaxBounds;
	XMFLOAT3							ModelSize;
	XMFLOAT3							CentrePoint;
	unsigned int						VertexCount;			// Determines total size of the VertexData array
	unsigned int						IndexCount;				// Determines total size of the IndexData array

	// Model data; uses the non-instanced definition since this class is used for [de]serialization from disk and instance data is not required
	TVertex *							VertexData;				// Array of per-vertex data only; no instance data included

	// Index data; structured to map directly into the trianglelist primitive topology (i.e. face -> { v1, v2, v3})
	INDEX_BUFFER_TYPE *					IndexData;				// Will always be IndexCount elements of type INDEX_BUFFER_TYPE


public:

	ModelData(void);
	~ModelData(void);

	ByteString							Serialize(void) const;
	static std::unique_ptr<ModelData>	Deserialize(ByteString & data);

	bool								AllocateVertexData(unsigned int vertex_count);
	void								DeallocateVertexData(void);

	bool								AllocateIndexData(unsigned int vertex_count);
	void								DeallocateIndexData(void);

	bool								DetrmineIfTextureCoordsPresent(void) const;
	bool								DetemineIfNormalDataPresent(void) const;
	bool								DetermineIfTangentSpaceDataPresent(void) const;

private:


};