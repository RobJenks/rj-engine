#include <fstream>
#include <string>
#include "GameDataExtern.h"
#include "FastMath.h"
#include "Utility.h"
#include "HashFunctions.h"
#include "CoreEngine.h"
#include "Texture.h"
#include "Data\Shaders\vertex_definitions.h.hlsl"

#include "Model.h"

// The static central collection of model data
Model::ModelCollection		Model::Models;

// Constructor
Model::Model()
{
	// Assign a unique ID to this model
	static int _Model_internalIDGenerator = 1;
	m_id = _Model_internalIDGenerator++;

	// Geometry is not initially loaded
	m_geometryloaded = false;

	// Initialise buffers and geometry data
	m_vertexCount = 0;
	m_indexCount = 0;
	m_model = 0;
	m_minbounds = m_maxbounds = m_modelsize = m_modelcentre = m_effectivesize = NULL_FLOAT3;
	m_actualsize = m_actualeffectivesize = m_scalingfactor = NULL_FLOAT3;
	m_elementsize = NULL_INTVECTOR3;
	m_origin_centred = true;

	m_iscompound = false; 
	m_compoundmodelcount = 0;
}




// Default destructor
Model::~Model() 
{ 

}

Result Model::Initialise(const char *modelFilename, const char *textureFilename)
{
	Result result;

	// Parameter check; we need a model filename at minimum in order to load anything
	if (!modelFilename || strcmp(modelFilename, "") == 0) return ErrorCodes::CannotLoadGeometryForNullModel;

	// Load in the model data,
	result = LoadModel(modelFilename);
	if(result != ErrorCodes::NoError) return result;

	// Initialise the vertex and index buffers.
	result = InitialiseBuffers();
	if(result != ErrorCodes::NoError) return result;

	// Load the texture for this model (optional)
	if (textureFilename)
	{
		result = m_buffer.SetTexture(textureFilename);
		if (result != ErrorCodes::NoError) return result;
	}

	// Success, if we have run each of the above stages successfully
	return ErrorCodes::NoError;
}

// Accepts a target effective size, then calculates a scaling factor and resizes the model data accordingly
void Model::SetActualModelSize(const XMFLOAT3 & actualeffectivesize)
{
	// We can accept two types of input here; either a fully-specified target size, or one dimension of the size which 
	// is then used to calculate the desired scale and derive the other two dimensions (if we have data with which to do that)
	int numzero =	(actualeffectivesize.x < Game::C_EPSILON ? 1 : 0) + (actualeffectivesize.y < Game::C_EPSILON ? 1 : 0) + 
					(actualeffectivesize.z < Game::C_EPSILON ? 1 : 0);
	bool haveeffsize = (m_effectivesize.x > Game::C_EPSILON && m_effectivesize.y > Game::C_EPSILON && m_effectivesize.z > Game::C_EPSILON);
	float derivedscale;

	// If we have one nonzero value and the requisite other data we can calculate the other two dimensions
	if (numzero == 2) 
	{
		// This is a valid input parameter, so store it
		m_actualeffectivesize = actualeffectivesize;

		// We can also derive the other two components if we have an effective size, or model data to fall back on
		if (haveeffsize || m_geometryloaded)
		{
			// We can derive the effective size from vertex data, if necessary
			if (!haveeffsize) m_effectivesize = m_modelsize;

			// Test each dimension in turn; do we have the x dimension?
			if (actualeffectivesize.x > Game::C_EPSILON) { 
				derivedscale = actualeffectivesize.x / m_effectivesize.x;
				m_actualeffectivesize = XMFLOAT3(actualeffectivesize.x, m_effectivesize.y * derivedscale, m_effectivesize.z * derivedscale);
			}
			else if (actualeffectivesize.y > Game::C_EPSILON) {
				derivedscale = actualeffectivesize.y / m_effectivesize.y;
				m_actualeffectivesize = XMFLOAT3(m_effectivesize.x * derivedscale, actualeffectivesize.y, m_effectivesize.z * derivedscale);
			}
			else if (actualeffectivesize.z > Game::C_EPSILON) {
				derivedscale = actualeffectivesize.z / m_effectivesize.z;
				m_actualeffectivesize = XMFLOAT3(m_effectivesize.x * derivedscale, m_effectivesize.y * derivedscale, actualeffectivesize.z);
			}
			else return;
		}
		else return;
	}
	else if (numzero == 0)
	{
		// Or, if we have all dimensions specified, we can simply go ahead with this vector
		m_actualeffectivesize = actualeffectivesize;
	}
	else
	{
		// Otherwise, we do not have a valid parameter.  Last chance is to derive from effective size (or model data)
		if (haveeffsize)							m_actualeffectivesize = m_effectivesize;
		else if (!haveeffsize && m_geometryloaded)	m_actualeffectivesize = m_effectivesize = m_modelsize;
		else										return;
	}

	// We need to know the effective size of this model to perform any scaling; if we don't have it, we can use the vertex data as a backup (if we have that)
	if (m_effectivesize.x <= Game::C_EPSILON || m_effectivesize.y <= Game::C_EPSILON || m_effectivesize.z <= Game::C_EPSILON) 
	{
		if (m_geometryloaded)			m_effectivesize = m_modelsize;
		else							return;
	}

	// Calculate a new scaling factor based upon the target actual effective size, and the model effective size
	XMFLOAT3 newscale = XMFLOAT3(	m_actualeffectivesize.x / m_effectivesize.x, m_actualeffectivesize.y / m_effectivesize.y,
									m_actualeffectivesize.z / m_effectivesize.z);

	// If the model is already scaled then we need to determine the delta scaling factor to apply at this point
	XMFLOAT3 deltascale;
	if (m_scalingfactor.x >= Game::C_EPSILON && m_scalingfactor.y >= Game::C_EPSILON && m_scalingfactor.z >= Game::C_EPSILON)
		deltascale = XMFLOAT3(newscale.x / m_scalingfactor.x, newscale.y / m_scalingfactor.y, newscale.z / m_scalingfactor.z);
	else
		deltascale = newscale;
	
	// If the delta scaling factor is ~1.0, we do not need to perform any scaling and can quit the method now
	if (abs(deltascale.x - 1.0f) < Game::C_EPSILON && abs(deltascale.y - 1.0f) < Game::C_EPSILON && abs(deltascale.z - 1.0f) < Game::C_EPSILON)
		return;

	// If the model geometry has been loaded, scale it by this delta scaling value now.  If not, we will check again when the geometry is loaded
	if (m_geometryloaded) ScaleModelGeometry(deltascale);

	// Store the NEW scaling factor that has been employed, i.e. the scaling from original model to current size, and the new actual effective size
	m_scalingfactor = newscale;

}

// Accepts a scaling factor, calculates an actual effective size and resizes the model data accordingly
void Model::SetModelScalingFactor(const XMFLOAT3 & scalingfactor)
{
	// Make sure we are provided an acceptable scaling parameter before continuing
	if (scalingfactor.x <= Game::C_EPSILON || scalingfactor.y <= Game::C_EPSILON || scalingfactor.z <= Game::C_EPSILON) return;

	// We need a current effective size to calculate the target size; we can determine this from model data if necessary (and if we have it)
	if (m_effectivesize.x <= Game::C_EPSILON || m_effectivesize.y <= Game::C_EPSILON || m_effectivesize.z <= Game::C_EPSILON) 
	{
		if (m_geometryloaded)			m_effectivesize = m_modelsize;
		else							return;
	}

	// Calculate a new target actual size based upon the target actual effective size, and the model effective size
	XMFLOAT3 actualeffsize = XMFLOAT3(	m_effectivesize.x * scalingfactor.x, m_effectivesize.y * scalingfactor.y,
										m_effectivesize.z * scalingfactor.z);

	// If the model is already scaled then we need to determine the delta scaling factor to apply at this point
	XMFLOAT3 deltascale = XMFLOAT3(scalingfactor.x / m_scalingfactor.x, scalingfactor.y / m_scalingfactor.y, scalingfactor.z / m_scalingfactor.z);

	// If the model geometry has been loaded, scale it by this delta scaling value now.  If not, we will check again when the geometry is loaded
	if (m_geometryloaded) ScaleModelGeometry(deltascale);

	// Store the NEW scaling factor that has been employed, i.e. the scaling from original model to current size, and the new actual effective size
	m_scalingfactor = scalingfactor;
	m_actualeffectivesize = actualeffsize;

}

// Scales the model geometry by a specified factor in each dimension
void Model::ScaleModelGeometry(const XMFLOAT3 & scale)
{
	// Make sure a valid scaling factor has been provided
	if (scale.x <= Game::C_EPSILON || scale.y <= Game::C_EPSILON || scale.z <= Game::C_EPSILON) return;

	// Process each vertex in turn
	for (unsigned int i = 0; i<m_vertexCount; i++)
	{
		// Scale the vertex by this factor
		m_model[i].x *= scale.x;
		m_model[i].y *= scale.y;
		m_model[i].z *= scale.z;
	}

	// The bounds of this model will also now change based on the scaling factor; scale these accordingly, 
	// then recalculate the other model dimensions
	m_minbounds.x *= scale.x; m_minbounds.y *= scale.y; m_minbounds.z *= scale.z;
	m_maxbounds.x *= scale.x; m_maxbounds.y *= scale.y; m_maxbounds.z *= scale.z;
	RecalculateDimensions();
	
	// We have updated the model geometry, so discard and recreate the vertex buffers 
	// now (not the most efficient method, but this is not a regular occurence)
	m_buffer.ReleaseModelBufferResources();
	InitialiseBuffers();
}

void Model::RecalculateDimensions(void)
{
	// Recalculate the model centre and total model size using the min & max bounds
	m_modelcentre = XMFLOAT3(	(m_maxbounds.x + m_minbounds.x) * 0.5f,
								(m_maxbounds.y + m_minbounds.y) * 0.5f,
								(m_maxbounds.z + m_minbounds.z) * 0.5f );
	m_modelsize = XMFLOAT3(	m_maxbounds.x - m_minbounds.x, m_maxbounds.y - m_minbounds.y,
							m_maxbounds.z - m_minbounds.z);

	// Make sure no model has zero size in any dimension, to avoid divide-by-zero errors down the line
	if (m_modelsize.x < Game::C_EPSILON) { m_modelsize.x = 0.01f; m_maxbounds.x = m_minbounds.x + 0.01f; }
	if (m_modelsize.y < Game::C_EPSILON) { m_modelsize.y = 0.01f; m_maxbounds.y = m_minbounds.y + 0.01f; }
	if (m_modelsize.z < Game::C_EPSILON) { m_modelsize.z = 0.01f; m_maxbounds.z = m_minbounds.z + 0.01f; }
}

// Adds a new component to the compound model
void Model::AddCompoundModelComponent(Model *model, const XMFLOAT3 & offset)
{
	// Add to the vector if this is a valid model
	if (!model) return;
	m_compoundmodels.push_back(Model::CompoundModelComponent(model, offset));

	// Update the component count any time a change is made to the collection
	m_compoundmodelcount = m_compoundmodels.size();
}

// Removes a specified component of the compound model
void Model::RemoveCompoundModelComponent(Model *model, const XMFLOAT3 & offset)
{
	// Loop through the vector looking for this combination
	for (CompoundModelComponentCollection::size_type i = 0; i < m_compoundmodelcount; ++i)
	{
		if (m_compoundmodels[i].model == model && Float3NearEqual(m_compoundmodels[i].offset, offset))
		{
			RemoveFromVectorAtIndex<Model::CompoundModelComponent>(m_compoundmodels, i);
			m_compoundmodelcount = m_compoundmodels.size();
			return;
		}
	}
}

// Removes a specified component of the compound model
void Model::RemoveCompoundModelComponent(const XMFLOAT3 & offset)
{
	// Loop through the vector looking for this combination
	for (CompoundModelComponentCollection::size_type i = 0; i < m_compoundmodelcount; ++i)
	{
		if (Float3NearEqual(m_compoundmodels[i].offset, offset))
		{
			RemoveFromVectorAtIndex<Model::CompoundModelComponent>(m_compoundmodels, i);
			m_compoundmodelcount = m_compoundmodels.size();
			return;
		}
	}
}

// Clears all compound data for this model (the model does remain as a compound model, however).  All component models are left untouched
void Model::ClearCompoundModelData(void)
{
	// Clear the vector of component models
	m_compoundmodels.clear();
	m_compoundmodelcount = m_compoundmodels.size();

	// Recalculate the compound model data now that we no long have any components
	RecalculateCompoundModelData();
}

// Recalculates all compound model data based on the 
void Model::RecalculateCompoundModelData(void)
{
	// Make sure we have the correct number of compound model components stored (should be redundant, but do this for safety)
	m_compoundmodelcount = m_compoundmodels.size();
	if (m_compoundmodelcount == 0U)
	{
		// Assign default values if the model has no components
		m_minbounds = m_maxbounds = NULL_FLOAT3;
	}
	else
	{
		// We will recalculate the bounds of the model
		XMFLOAT3 minbounds = XMFLOAT3(999999.0f, 999999.0f, 999999.0f);
		XMFLOAT3 maxbounds = XMFLOAT3(-999999.0f, -999999.0f, -999999.0f);
		XMFLOAT3 modelmin, modelmax;

		// Iterate through the component of this compound model to determine the min and max bounds of the overall model
		Model::CompoundModelComponentCollection::const_iterator it_end = m_compoundmodels.end();
		for (Model::CompoundModelComponentCollection::const_iterator it = m_compoundmodels.begin(); it != it_end; ++it)
		{
			// Retrieve the bounds of this component, allowing for the per-component offset
			modelmin = Float3Add(it->model->GetModelMinBounds(), it->offset);
			modelmax = Float3Add(it->model->GetModelMaxBounds(), it->offset);

			// If this component expands the overall model bounds then record that here
			if (modelmin.x < minbounds.x) minbounds.x = modelmin.x;
			if (modelmin.y < minbounds.y) minbounds.y = modelmin.y;
			if (modelmin.z < minbounds.z) minbounds.z = modelmin.z;
			if (modelmax.x > maxbounds.x) maxbounds.x = modelmax.x;
			if (modelmax.y > maxbounds.y) maxbounds.y = modelmax.y;
			if (modelmax.z > maxbounds.z) maxbounds.z = modelmax.z;
		}

		// Store the overall model bounds
		m_minbounds = minbounds; m_maxbounds = maxbounds;
	}

	// Recalculate remaining model statistics based on this data
	RecalculateDimensions();
}

void Model::Shutdown()
{
	// Release the model data.
	ReleaseModel();
}


void Model::Render(void)
{
	// Take difference action depending on whether this is a simple or compound model
	if (m_iscompound)
	{
		// Iterate through and render each component in turn
		Model::CompoundModelComponentCollection::const_iterator it_end = m_compoundmodels.end();
		for (Model::CompoundModelComponentCollection::const_iterator it = m_compoundmodels.begin(); it != it_end; ++it)
		{
			it->model->Render();
		}
	}
	else
	{
		// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
		RenderBuffers();
	}
}


Result Model::InitialiseBuffers(void)
{
	// Create the vertex array.
	Vertex_Inst_TexNormMatLit *vertices = new Vertex_Inst_TexNormMatLit[m_vertexCount];
	if (!vertices) return ErrorCodes::CouldNotAllocateModelVertexArray;

	// Create the index array.
	INDEXFORMAT *indices = new INDEXFORMAT[m_indexCount];
	if (!indices) 
	{
		if (vertices) SafeDeleteArray(vertices);
		return ErrorCodes::CouldNotAllocateModelIndexArray;
	}

	// Load the vertex array and index array with data.
	for (unsigned int i = 0; i < m_vertexCount; ++i)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].tex = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);
		vertices[i].material = m_model[i].material;

		indices[i] = i;
	}

	// Initialise the model buffers based on this raw data
	Result result = m_buffer.Initialise(Game::Engine->GetDevice(), (const void**)&vertices, sizeof(Vertex_Inst_TexNormMatLit), m_vertexCount,
																   (const void**)&indices, sizeof(INDEXFORMAT), m_indexCount);
	if (result != ErrorCodes::NoError) return result;

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	SafeDeleteArray(vertices);
	SafeDeleteArray(indices);

	// Return success
	return ErrorCodes::NoError;
}

void Model::RenderBuffers(void)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = m_buffer.GetVertexSize();
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	Game::Engine->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_buffer.VertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	Game::Engine->GetDeviceContext()->IASetIndexBuffer(m_buffer.IndexBuffer, /*DXGI_FORMAT_R32_UINT*/ DXGI_FORMAT_R16_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	Game::Engine->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


Result Model::LoadModel(const char *filename)
{
	std::ifstream fin;
	char input;

	// Default values if the model cannot be loaded for some reason
	m_minbounds = NULL_FLOAT3;
	m_maxbounds = NULL_FLOAT3;

	// Open the model file.
	fin.open(filename);
	
	// If it could not open the file then exit.
	if(fin.fail())
	{
		return ErrorCodes::CouldNotOpenModelFile;
	}

	// Read up to the value of vertex count.
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> m_vertexCount;

	// Set the number of indices to be the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the model using the vertex count that was read in.
	m_model = new ModelType[m_vertexCount];
	if(!m_model)
	{
		return ErrorCodes::CouldNotAllocateModelDataStorage;
	}

	// Read up to the beginning of the data.
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Initialise min and max bounds before we start loading the model vertex data
	m_minbounds = XMFLOAT3(99999.0f, 99999.0f, 99999.0f);
	m_maxbounds = XMFLOAT3(-99999.0f, -99999.0f, -99999.0f);

	// Read in the vertex data.
	for (unsigned int i = 0; i < m_vertexCount; ++i)
	{
		// Read in coordinates, texture coords and normals
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
		m_model[i].material = 0U;

		// Also keep track of the min & max model bounds as we load it
		if (m_model[i].x < m_minbounds.x) m_minbounds.x = m_model[i].x;
		if (m_model[i].x > m_maxbounds.x) m_maxbounds.x = m_model[i].x;
		if (m_model[i].y < m_minbounds.y) m_minbounds.y = m_model[i].y;
		if (m_model[i].y > m_maxbounds.y) m_maxbounds.y = m_model[i].y;
		if (m_model[i].z < m_minbounds.z) m_minbounds.z = m_model[i].z;
		if (m_model[i].z > m_maxbounds.z) m_maxbounds.z = m_model[i].z;
	}

	// TODO: also load materials as part of this process
	int nummat = 1;
	Material m; 
	m.Data.AmbientColor = XMFLOAT4(100.0f, 100.0f, 100.0f, 100.0f);
	m.Data.DiffuseColor = XMFLOAT4(50.0f, 50.0f, 50.0f, 50.0f);
	m.Data.SpecularColor = XMFLOAT4(1.0f, 0.8f, 0.8f, 0.75f);
	m.Data.Reflectance = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f);

	m_buffer.SetMaterialCount(nummat);
	m_buffer.SetMaterial(0U, m);

	// Recalculate other model dimensions based on these min and max bounds
	RecalculateDimensions();

	// If the model is not centred about (0,0,0), and if we have not overriden the default centering behaviour
	// for this model, then adjust the vertex data now. This also recalculates he dependent fields by 
	// calling RecalculateDimensions() internally
	if (IsZeroFloat3(m_modelcentre))
	{
		m_origin_centred = true;
	}
	else
	{
		if (m_origin_centred) CentreModelAboutOrigin();
	}

	// Close the model file.
	fin.close();

	return ErrorCodes::NoError;
}

// Adjusts all vertex data to ensure that the model origin lies at (0,0,0), and recalculates
// all dependent data based on the new vertices
void Model::CentreModelAboutOrigin(void)
{
	// Determine the offset to be applied uniformly to every vertex
	XMFLOAT3 offset = Float3MultiplyScalar(Float3Add(m_minbounds, m_maxbounds), -0.5f);
	if (IsZeroFloat3(offset)) return;

	// Loop through all vertices and apply the offset
	// Read in the vertex data.
	Model::ModelType *v = &(m_model[0]);
	for (unsigned int i = 0; i < m_vertexCount; ++i, ++v)
	{
		v->x += offset.x;
		v->y += offset.y;
		v->z += offset.z;
	}

	// Adjust the min/max vertex bounds by this known offset
	m_minbounds = Float3Add(m_minbounds, offset);
	m_maxbounds = Float3Add(m_maxbounds, offset);
	
	// Recalculate all derived fields that are dependent on this vertex data
	RecalculateDimensions();
}


void Model::ReleaseModel()
{
	if(m_model)
	{
		delete [] m_model;
		m_model = 0;
	}

	return;
}

// Test whether a model exists in the central collection
bool Model::ModelExists(const std::string & code)
{
	return (code != NullString && Model::Models.count(code) > 0);
}

// Retrieve a model from the central collection based on its string code
Model *Model::GetModel(const std::string & code)
{
	if (code != NullString && Model::Models.count(code) > 0)	return Model::Models[code];
	else														return NULL;	
}

// Retrieve a model from the central collection based on its filename; requires linear search (on hash values) so less efficient than searching by code
Model *Model::GetModelFromFilename(const std::string & filename)
{
	// Hash the input filename (assuming it is valid) for more efficient comparisons
	if (filename == NullString) return NULL;
	HashVal hash = HashString(filename);

	// Iterate through the collection to look for a model with this filename
	ModelCollection::const_iterator it_end = Model::Models.end();
	for (ModelCollection::const_iterator it = Model::Models.begin(); it != it_end; ++it)
	{
		if (it->second && hash == HashString(it->second->GetFilename())) 
			return it->second;
	}

	// We could not find a model with this filename
	return NULL;
}

// Add a new model to the central collection, indexed by its unique string code
void Model::AddModel(Model *model)
{
	// Make sure the model is valid, and that we do not already have a model with its unique code
	if (!model || model->GetCode() == NullString || Model::Models.count(model->GetCode()) > 0) return;

	// Add to the central collection, indexed by its string code
	Model::Models[model->GetCode()] = model;
}

void Model::TerminateAllModelData(void)
{
	// All standard models are contained within the model collection, so we can iterate over it and dispose
	// of objects one by one via their standard destructor
	ModelCollection::iterator it_end = Model::Models.end();
	for (ModelCollection::iterator it = Model::Models.begin(); it != it_end; ++it)
	{
		if (it->second)
		{
			it->second->Shutdown();
			delete it->second; 
		}
	}

	// Clear the collection, now that it is simply full of null pointers
	Model::Models.clear();
}

// Determines the class of model based upon its string description
Model::ModelClass Model::DetermineModelClass(const std::string s)
{
	std::string type = StrLower(s);
	
	if			(type == "ship")					return ModelClass::Ship;
	else if		(type == "complexshipsection")		return ModelClass::ComplexShipSection;
	else if		(type == "complexshiptile")			return ModelClass::Tile;
	else if		(type == "terrain")					return ModelClass::Terrain;

	else											return ModelClass::Unknown;
}