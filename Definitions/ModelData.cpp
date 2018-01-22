#include "ModelData.h"

ModelData::ModelData(void)
	:
	VertexData(NULL),
	VertexCount(0U),
	ModelMaterialIndex(0U), 
	MinBounds(0.0f, 0.0f, 0.0f), 
	MaxBounds(0.0f, 0.0f, 0.0f), 
	ModelSize(0.0f, 0.0f, 0.0f), 
	CentrePoint(0.0f, 0.0f, 0.0f)
{
}

ModelData::~ModelData(void)
{
	DeallocateVertexData();
}

void ModelData::AllocateVertexData(unsigned int vertex_count)
{
	DeallocateVertexData();
	
	if (vertex_count > 0U)
	{
		VertexData = new TVertex[vertex_count];
		memset(VertexData, 0, sizeof(TVertex) * vertex_count);
	}
}

void ModelData::DeallocateVertexData(void)
{
	if (VertexData)
	{
		delete[] VertexData;
		VertexData = NULL;
	}
}

ByteString ModelData::Serialize(void) const
{
	// Header data
	ByteString b;
	b.WriteObject(this->ModelMaterialIndex);
	b.WriteObject(this->MinBounds);
	b.WriteObject(this->MaxBounds);
	b.WriteObject(this->ModelSize);
	b.WriteObject(this->CentrePoint);
	b.WriteObject(this->VertexCount);

	// Vertex data
	for (unsigned int i = 0U; i < VertexCount; ++i)
	{
		b.WriteObject(VertexData[i]);
	}

	return b;
}

std::unique_ptr<ModelData> ModelData::Deserialize(ByteString & data)
{
	// Start reading from buffer start and deserialize into a new model data object
	ModelData *m = new ModelData();
	data.ResetRead();

	// Header data
	data.ReadObject(m->ModelMaterialIndex);
	data.ReadObject(m->MinBounds);
	data.ReadObject(m->MaxBounds);
	data.ReadObject(m->ModelSize);
	data.ReadObject(m->CentrePoint);
	data.ReadObject(m->VertexCount);

	// Vertex data
	m->AllocateVertexData(m->VertexCount);
	for (unsigned int i = 0U; i < m->VertexCount; ++i)
	{
		data.ReadObject(m->VertexData[i]);
	}

	return std::unique_ptr<ModelData>(m);
}




