#include <filesystem>
#include "Logging.h"
#include "FileUtils.h"
#include "ByteString.h"
#include "ModelData.h"
#include "CoreEngine.h"
#include "RenderAssetsDX11.h"
#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"
#include "MaterialDX11.h"
#include "Model.h"
namespace fs = std::experimental::filesystem;

// Initialise static data
Model::ModelCollection Model::Models;
Model::ModelID Model::GlobalModelIDCount = 0U;

// Default constructor
Model::Model(void)
	:
	m_id(++Model::GlobalModelIDCount)
{
}

// Default destructor
Model::~Model(void)
{

}


// Load a model from disk and prepare it for use
Result Model::Initialise(const std::string & code, const std::string & filename, const std::string & material)
{
	return Initialise(code, fs::path(filename), material);
}


// Load a model from disk and prepare it for use
Result Model::Initialise(const std::string & code, fs::path file, const std::string & material)
{
	// Reset all geometry data beforehand so that we don't ever half-load a model over an existing one
	Reset();

	// Store key parameters
	m_code = code;
	m_filename = fs::absolute(file).string();

	// Parameter check
	if (m_filename.empty() || !fs::exists(file))
	{
		Game::Log << LOG_ERROR << "Model file does not exist: \"" << m_filename << "\"\n";
		return ErrorCodes::CouldNotOpenModelFile;
	}

	// Read file contents
	ByteString binary_data = FileUtils::ReadBinaryFile(file);
	if (binary_data.empty())
	{
		Game::Log << LOG_ERROR << "Model file is empty or invalid: \"" << m_filename << "\"\n";
		return ErrorCodes::CannotLoadMesh;
	}

	// Attempt to deserialize all model geometry data
	Geometry = ModelData::Deserialize(binary_data);
	if (Geometry.get() == NULL)
	{
		Game::Log << LOG_ERROR << "Could not deserialize model data from \"" << m_filename << "\"\n";
		return ErrorCodes::CannotDeserializeModel;
	}

	// Store the material name, which will be resolved upon compilation
	m_materialcode = material;
	
	// Compile the model based upon the loaded geometry data
	Result compile_result = CompileModel();
	if (compile_result != ErrorCodes::NoError)
	{
		return compile_result;
	}

	// Return success
	return ErrorCodes::NoError;
}

// Reset all model geometry data
void Model::Reset(void)
{
	Geometry.reset(NULL);
	Data = ModelBuffer();
}

// Compile a model and generate all rendering buffer data
Result Model::CompileModel(void)
{
	// Make sure geometry data has been loaded
	ModelData *data = Geometry.get();
	if (!data)
	{
		Game::Log << LOG_WARN << "Cannot compile model \"" << m_code << "\"; no geometry data available\n";
		return ErrorCodes::CannotCompileModel;
	}

	// Attempt to resolve the model material, otherwise use a default
	const MaterialDX11 * pMaterial = Game::Engine->GetAssets().GetMaterial(m_materialcode);
	if (!pMaterial)
	{
		Game::Log << LOG_WARN << "Cannot find material \"" << m_materialcode << "\" for model \"" << m_code << "\"; using default\n";
		pMaterial = Game::Engine->GetAssets().GetDefaultMaterial();
	}

	// Refresh the buffer with this new data
	Data = ModelBuffer
	(
		VertexBufferDX11(*data),
		IndexBufferDX11(data->VertexCount),
		pMaterial
	);

	// Store reference back to this model within the buffer; useful mostly for debugging
	Data.SetParentModel(this);

	// Return success
	return ErrorCodes::NoError;
}




// Test whether a model exists in the central collection
bool Model::ModelExists(const std::string & code)
{
	return (Models.find(code) != Models.end());
}

// Retrieve a model from the central collection based on its string code
Model *Model::GetModel(const std::string & code)
{
	ModelCollection::const_iterator it = Models.find(code);
	return (it != Models.end() ? it->second : NULL);
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
	if (!model || model->GetCode() == NullString)
	{
		Game::Log << LOG_ERROR << "Could not register new model with global collection; null model or model code\n";
		return;
	}
	else if (Model::ModelExists(model->GetCode()))
	{
		Game::Log << LOG_ERROR << "Could not register new model with global collection; model already exists with code \"" << model->GetCode() << "\"\n";
		return;
	}

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
			SafeDelete(it->second);
		}
	}

	// Clear the collection, now that it is simply full of null pointers
	Model::Models.clear();
}







