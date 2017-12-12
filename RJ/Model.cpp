#include <filesystem>
#include "Logging.h"
#include "FileUtils.h"
#include "../Definitions/ByteString.h"
#include "../Definitions/ModelData.h"
#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"
#include "MaterialDX11.h"
#include "Model.h"
namespace fs = std::experimental::filesystem;


Model::Model(void)
{


}


Model::~Model(void)
{

}


// Load a model from disk and prepare it for use
Result Model::Initialise(const std::string & filename)
{
	return Initialise(fs::path(filename));
}


// Load a model from disk and prepare it for use
Result Model::Initialise(fs::path file)
{
	// Reset all geometry data beforehand so that we don't ever half-load a model over an existing one
	Reset();

	// Parameter check
	m_filename = fs::absolute(file).string();
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
		Game::Log << LOG_WARN << "Cannot compile model; no geometry data available\n";
		return ErrorCodes::CannotCompileModel;
	}

	// Refresh the buffer with this new data
	Data = ModelBuffer
	(
		VertexBufferDX11(*data),
		IndexBufferDX11(data->VertexCount),
		NULL									// TODO: Assign material here
	);

	// Return success
	return ErrorCodes::NoError;
}










