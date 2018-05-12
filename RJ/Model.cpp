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
	m_id(++Model::GlobalModelIDCount), 
	m_component_count(0U)
{
	// Derived fields will all be set to defaults when calculated for null model data
	RecalculateDerivedData();
}

// Default destructor
Model::~Model(void)
{

}


// Load a model from disk and prepare it for use
Result Model::Initialise(const std::string & code, std::vector<ModelLoadingData> components)
{
	// Reset all geometry data beforehand so that we don't ever half-load a model over an existing one
	Reset();

	// Store key parameters
	m_code = code;

	// Validation
	if (components.empty())
	{
		Game::Log << LOG_ERROR << "Cannot load model \"" << code << "\"; no valid components have been defined\n";
		return ErrorCodes::ModelHasNoValidComponents;
	}

	// Process each component in turn
	size_t n = components.size();
	for (size_t i = 0U; i < n; ++i)
	{
		const auto & item = components[i];

		// Parameter check
		fs::path file(item.GetFilename());
		if (!fs::exists(file))
		{
			Game::Log << LOG_ERROR << "Cannot load model \"" << code << "[" << i << "]\"; model file does not exist: \"" << item.GetFilename() << "\"\n";
			return ErrorCodes::CouldNotOpenModelFile;
		}

		// Read file contents
		ByteString binary_data = FileUtils::ReadBinaryFile(file);
		if (binary_data.empty())
		{
			Game::Log << LOG_ERROR << "Cannot load model \"" << code << "[" << i << "]\"; model file is empty or invalid: \"" << item.GetFilename() << "\"\n";
			return ErrorCodes::CannotLoadMesh;
		}

		// Attempt to deserialize all model geometry data
		auto geometry = ModelData::Deserialize(binary_data);
		if (geometry.get() == NULL)
		{
			Game::Log << LOG_ERROR << "Could not deserialize model \"" << code << "[" << i << "]\" data from \"" << item.GetFilename() << "\"\n";
			return ErrorCodes::CannotDeserializeModel;
		}

		// Store the new component entry
		Components.push_back(Model::Component(std::move(geometry), NULL, item.GetFilename(), item.GetMaterial()));
	}

	// Compile the model based upon the loaded geometry data
	Result compile_result = CompileModel();
	if (compile_result != ErrorCodes::NoError)
	{
		return compile_result;
	}

	// Perform a recalculation of derived data based on this newly-compiled geometry
	RecalculateDerivedData();

	// Return success
	return ErrorCodes::NoError;
}

// Reset all model geometry data
void Model::Reset(void)
{
	// Release all data
	Components.clear();

	// Perform a recalculation which will reset all derived data back to defaults
	RecalculateDerivedData();
}

// Calculate fields that are based upon the model data, for example the overall geometry bounds
void Model::RecalculateDerivedData(void)
{
	// Store the number of valid model components
	m_component_count = Components.size();

	// Determine minimum and maximum vertex bounds
	XMFLOAT3 min_bounds = XMFLOAT3(+1e6, +1e6, +1e6);
	XMFLOAT3 max_bounds = XMFLOAT3(-1e6, -1e6, -1e6);
	for (const auto & item : Components)
	{
		const auto geo = item.Geometry.get();
		if (!geo) continue;

		min_bounds = Float3Min(min_bounds, geo->MinBounds);
		max_bounds = Float3Max(max_bounds, geo->MaxBounds);
	}

	// If we have no valid non-null components, revert to defaults
	if (!Float3GreaterThan(max_bounds, min_bounds))
	{
		min_bounds = XMFLOAT3(0.0f, 0.0f, 0.0f);
		max_bounds = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	// Calculate & store the derived data
	m_minbounds = min_bounds;
	m_maxbounds = max_bounds;
	m_modelsize = XMFLOAT3(max_bounds.x - min_bounds.x, max_bounds.y - min_bounds.y, max_bounds.z - min_bounds.z);
	m_centrepoint = XMFLOAT3(min_bounds.x + (m_modelsize.x * 0.5f), min_bounds.y + (m_modelsize.y * 0.5f), min_bounds.z + (m_modelsize.z * 0.5f));
}

// Compile a model and generate all rendering buffer data
Result Model::CompileModel(void)
{
	// Compile each component in turn
	size_t n = Components.size();
	for (size_t i = 0U; i < n; ++i)
	{
		auto & component = Components[i];

		// Make sure geometry data has been loaded
		ModelData *data = component.Geometry.get();
		if (!data)
		{
			Game::Log << LOG_WARN << "Cannot compile model \"" << m_code << "[" << i << "]\"; no geometry data available\n";
			return ErrorCodes::CannotCompileModel;
		}

		// Attempt to resolve the model material, otherwise use a default
		const MaterialDX11 * pMaterial = Game::Engine->GetAssets().GetMaterial(component.MaterialCode);
		if (!pMaterial)
		{
			Game::Log << LOG_WARN << "Cannot find material \"" << component.MaterialCode << "\" for model \"" << m_code << "[" << i << "]\"; using default\n";
			pMaterial = Game::Engine->GetAssets().GetDefaultMaterial();
		}

		// Refresh the buffer with this new data
		component.Data = std::make_unique<ModelBuffer>
		(
			VertexBufferDX11(*data),
			IndexBufferDX11(*data),
			pMaterial
		);

		// Store reference back to this model within the buffer; useful mostly for debugging
		component.Data->SetParentModel(this);
	}

	// Store total component count for render-time
	m_component_count = n;

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


// Add a new model to the central collection, indexed by its unique string code
void Model::AddModel(Model *model)
{
	// Make sure the model is valid, and that we do not already have a model with its unique code
	if (!model || model->GetCode() == NullString)
	{
		Game::Log << LOG_ERROR << "Could not register new model with global collection; null model or model code\n";
		return;
	}
	if (Model::ModelExists(model->GetCode()))
	{
		Game::Log << LOG_ERROR << "Could not register new model with global collection; model already exists with code \"" << model->GetCode() << "\"\n";
		return;
	}

	// Add to the central collection, indexed by its string code
	Model::Models[model->GetCode()] = model;
}

// Runtime reloading of model geometry data
void Model::ReloadModel(const std::string & code)
{
	ReloadModel(Model::GetModel(code));
}

// Runtime reloading of model geometry data
void Model::ReloadModel(Model *model)
{
	if (!model) return;
	Game::Log << LOG_DEBUG << "Reloading model geometry for \"" << model->GetCode() << "\"\n";

	// We can simply re-execute the initialisation and compilation steps; all source data is already available 
	// Build a collection of source references and then re-run initialisation
	std::vector<ModelLoadingData> source;
	for (const auto & item : model->Components)
	{
		source.push_back(ModelLoadingData(item.Filename, item.MaterialCode));
	}

	Result result = model->Initialise(model->GetCode(), source);
	if (result != ErrorCodes::NoError)
	{
		Game::Log << LOG_ERROR << "Failed to re-initialise model \"" << model->GetCode() << "\"; error code " << (int)result << "\n";
	}

	result = model->CompileModel();
	if (result != ErrorCodes::NoError)
	{
		Game::Log << LOG_ERROR << "Failed to re-compile model \"" << model->GetCode() << "\"; error code " << (int)result << "\n";
	}
}

// Runtime reloading of model geometry data
void Model::ReloadAllModels(void)
{
	Game::Log << LOG_DEBUG << "Reloading all model geometry\n";

	for (auto & entry : Model::Models)
	{
		if (entry.second) ReloadModel(entry.second);
	}

	Game::Log << LOG_DEBUG << "All model geometry reloaded\n";
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







