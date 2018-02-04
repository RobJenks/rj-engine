#include "ModelData.h"

#ifdef RJ_MODULE_RJ
#	include "../RJ/Logging.h"
#endif

const std::string ModelData::GEOMETRY_FILE_IDENTIFIER = { 'R', 'j', 'G', 'e', 'o' };


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

bool ModelData::AllocateVertexData(unsigned int vertex_count)
{
	DeallocateVertexData();
	
	if (vertex_count > ModelData::VERTEX_COUNT_LIMIT)
	{
#		ifdef LOGGING_AVAILABLE
			Game::Log << LOG_ERROR << "Cannot allocate vertex data for model data; specified vertex count of " << vertex_count << " is out of acceptable bounds\n";
#		endif
		return false;
	}

	VertexData = new TVertex[vertex_count];
	memset(VertexData, 0, sizeof(TVertex) * vertex_count);
	return true;
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
	b.WriteString(ModelData::GEOMETRY_FILE_IDENTIFIER);
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
	// Start reading from buffer start and make sure file identifier is present
	data.ResetRead();
	if (!data.ReadAndVerifyIdentifier(ModelData::GEOMETRY_FILE_IDENTIFIER))
	{
#		ifdef LOGGING_AVAILABLE
			Game::Log << LOG_ERROR << "Model geometry data file is invalid, cannot process further\n";
#		endif
		return NULL;
	}

	// File appears valid so process the header data
	ModelData *m = new ModelData();
	data.ReadObject(m->ModelMaterialIndex);
	data.ReadObject(m->MinBounds);
	data.ReadObject(m->MaxBounds);
	data.ReadObject(m->ModelSize);
	data.ReadObject(m->CentrePoint);
	data.ReadObject(m->VertexCount);

	// Attempt to allocate vertex data
	if (m->AllocateVertexData(m->VertexCount) == false)
	{
		delete(m);
		return NULL;
	}
	
	// Vertex data
	for (unsigned int i = 0U; i < m->VertexCount; ++i)
	{
		data.ReadObject(m->VertexData[i]);
	}

	return std::unique_ptr<ModelData>(m);
}


