#pragma once

#include <memory>
#include <filesystem>
#include "../Definitions/ModelData.h"
#include "ModelBuffer.h"
namespace fs = std::experimental::filesystem;

class Model
{
public:

	// Raw geometry data; TODO: could deallocate this after loading if compiled data doesn't need to be 
	// regenerated, and if we don't need to reference the raw data again (?)
	std::unique_ptr<ModelData>			Geometry;

	// Compiled model data and buffers
	ModelBuffer							Data;


public:

	Model(void);
	~Model(void);

	// Load a model from disk and prepare it for use
	Result								Initialise(const std::string & filename);
	Result								Initialise(fs::path file);

	// Reset all model geometry data
	void								Reset(void);

	// Compile a model and generate all rendering buffer data
	Result								CompileModel(void);


private:

	std::string							m_filename;


};