#pragma once

#include <memory>
#include <filesystem>
#include "ModelData.h"
#include "ModelBuffer.h"
#include "ModelLoadingData.h"
#include "CollisionSpatialDataF.h"
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

	struct Component
	{
		// Raw geometry data; TODO: could deallocate this after loading if compiled data doesn't need to be 
		// regenerated, and if we don't need to reference the raw data again (?)
		std::unique_ptr<ModelData>			Geometry;

		// Compiled model data and buffers
		std::unique_ptr<ModelBuffer>		Data;

		// Metadata to be stored for the component
		std::string							Filename;
		std::string							MaterialCode;

		// Constructor
		Component(void) { }
		Component(std::unique_ptr<ModelData> && geometry, std::unique_ptr<ModelBuffer> && data, const std::string & filename, const std::string & materialcode)
			:
			Geometry(std::move(geometry)), Data(std::move(data)), Filename(filename), MaterialCode(materialcode) { }
	};

	// List of components in the model
	typedef std::vector<Component>			ModelComponents;
	ModelComponents							Components;

public:

	Model(void);
	~Model(void);

	// Load a model from disk and prepare it for use
	Result								Initialise(const std::string & code, std::vector<ModelLoadingData> components);

	// Return basic data on the model
	CMPINLINE ModelID					GetID(void) const { return m_id; }
	CMPINLINE std::string				GetCode(void) const { return m_code; }

	// Query the number of model components; value is calculated during model compilation
	CMPINLINE ModelComponents::size_type	GetComponentCount(void) const { return m_component_count; }
	CMPINLINE bool							HasData(void) const { return (m_component_count != 0U); }

	// Return data derived from the model geometry
	CMPINLINE XMFLOAT3					GetMinBounds(void) const { return m_minbounds; }
	CMPINLINE XMFLOAT3					GetMaxBounds(void) const { return m_maxbounds; }
	CMPINLINE XMFLOAT3					GetModelSize(void) const { return m_modelsize; }
	CMPINLINE XMFLOAT3					GetCentrePoint(void) const { return m_centrepoint; }

	// List of collision objects attached to this model
	CMPINLINE std::vector<CollisionSpatialDataF> & CollisionData(void) { return m_collision; }
	CMPINLINE const std::vector<CollisionSpatialDataF> & CollisionData(void) const { return m_collision; }
	CMPINLINE void				SetCollisionData(const std::vector<CollisionSpatialDataF> & collision) { m_collision = collision; }
	CMPINLINE void				SetCollisionData(std::vector<CollisionSpatialDataF> && collision) { m_collision = std::move(collision); }
	
	// Reset all model geometry data
	void								Reset(void);

	// Calculate fields that are based upon the model data, for example the overall geometry bounds
	void								RecalculateDerivedData(void);

	// Compile a model and generate all rendering buffer data
	Result								CompileModel(void);


private:

	ModelID								m_id;
	std::string							m_code;
	ModelComponents::size_type			m_component_count;

	XMFLOAT3							m_minbounds;				// Aggregated across all components; calculated during model compilation
	XMFLOAT3							m_maxbounds;				// Aggregated across all components; calculated during model compilation
	XMFLOAT3							m_modelsize;				// Aggregated across all components; calculated during model compilation
	XMFLOAT3							m_centrepoint;				// Aggregated across all components; calculated during model compilation


	// List of collision objects attached to this model
	std::vector<CollisionSpatialDataF>	m_collision;


public:

	// Static collection of all models in the game, indexed by string code
	static ModelCollection			Models;

	// Central model storage methods
	static bool						ModelExists(const std::string & code);
	static Model *					GetModel(const std::string & code);
	static void						AddModel(Model *model);

	// Runtime reloading of model geometry data
	static void						ReloadModel(const std::string & code);
	static void						ReloadModel(Model *model);
	static void						ReloadAllModels(void);

	// Deallocates all centrally-maintained model data
	static void						TerminateAllModelData(void);




};