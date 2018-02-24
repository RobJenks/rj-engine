#include "ModelData.h"

#ifdef RJ_MODULE_RJ
#	include "../RJ/Logging.h"
#endif

const std::string ModelData::GEOMETRY_FILE_IDENTIFIER = { 'R', 'j', 'G', 'e', 'o' };


ModelData::ModelData(void)
	:
	VertexData(NULL),
	IndexData(NULL), 
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
	DeallocateIndexData();
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

bool ModelData::AllocateIndexData(unsigned int index_count)
{
	DeallocateIndexData();

	if (index_count > ModelData::INDEX_COUNT_LIMIT)
	{
#		ifdef LOGGING_AVAILABLE
			Game::Log << LOG_ERROR << "Cannot allocate index data for model data; specified index count of " << index_count << " is out of acceptable bounds\n";
#		endif
		return false;
	}

	IndexData = new INDEX_BUFFER_TYPE[index_count];
	memset(IndexData, 0, sizeof(INDEX_BUFFER_TYPE) * index_count);
	return true;
}

void ModelData::DeallocateIndexData(void)
{
	if (IndexData)
	{
		delete[] IndexData;
		IndexData = NULL;
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
	b.WriteObject(this->IndexCount);

	// Vertex data
	for (unsigned int i = 0U; i < VertexCount; ++i)
	{
		b.WriteObject(VertexData[i]);
	}

	// Index data
	for (unsigned int i = 0U; i < IndexCount; ++i)
	{
		b.WriteObject(IndexData[i]);
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
	data.ReadObject(m->IndexCount);

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

	// Attempt to allocate index data
	if (m->AllocateIndexData(m->IndexCount) == false)
	{
		delete(m);
		return NULL;
	}

	// Index data
	for (unsigned int i = 0U; i < m->IndexCount; ++i)
	{
		data.ReadObject(m->IndexData[i]);
	}


	return std::unique_ptr<ModelData>(m);
}



bool ModelData::DetrmineIfTextureCoordsPresent(void) const
{
	for (unsigned int i = 0U; i < VertexCount; ++i)
	{
		if (VertexData[i].tex.x != 0.0f || VertexData[i].tex.y != 0.0f) return true;
	}

	return false;
}

bool ModelData::DetemineIfNormalDataPresent(void) const
{
	for (unsigned int i = 0U; i < VertexCount; ++i)
	{
		if (VertexData[i].normal.x != 0.0f || VertexData[i].normal.y != 0.0f || VertexData[i].normal.z != 0.0f) return true;
	}

	return false;
}

bool ModelData::DetermineIfTangentSpaceDataPresent(void) const
{
	for (unsigned int i = 0U; i < VertexCount; ++i)
	{
		if (VertexData[i].binormal.x != 0.0f || VertexData[i].binormal.y != 0.0f || VertexData[i].binormal.z != 0.0f) return true;
		if (VertexData[i].tangent.x != 0.0f  || VertexData[i].tangent.y != 0.0f  || VertexData[i].tangent.z != 0.0f) return true;
	}

	return false;
}






