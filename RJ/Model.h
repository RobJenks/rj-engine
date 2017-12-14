#pragma once

#include <memory>
#include <filesystem>
#include "../Definitions/ModelData.h"
#include "ModelBuffer.h"
namespace fs = std::experimental::filesystem;

class Model
{
public:

	// Unique ID assigned to each model
	typedef unsigned int ModelID;
	static ModelID GlobalModelIDCount;

	// Model data is stored in static unordered_map collections & indexed by unique string code
	typedef std::unordered_map<std::string, Model*> ModelCollection;

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

	// Return basic data on the model
	CMPINLINE ModelID					GetID(void) const { return m_id; }
	CMPINLINE std::string				GetCode(void) const { return m_code; }
	CMPINLINE std::string				GetFilename(void) const { return m_filename; }

	// Set model metadata
	CMPINLINE void						SetCode(const std::string & code) { m_code = code; }
	CMPINLINE void						SetFilename(const std::string & filename) { m_filename = filename; }

	// Reset all model geometry data
	void								Reset(void);

	// Compile a model and generate all rendering buffer data
	Result								CompileModel(void);


public:

	// Static collection of all models in the game, indexed by string code
	static ModelCollection			Models;

	// Central model storage methods
	static bool						ModelExists(const std::string & code);
	static Model *					GetModel(const std::string & code);
	static Model *					GetModelFromFilename(const std::string & filename);
	static void						AddModel(Model *model);

	// Deallocates all centrally-maintained model data
	static void						TerminateAllModelData(void);



private:

	ModelID								m_id;
	std::string							m_code;
	std::string							m_filename;


};