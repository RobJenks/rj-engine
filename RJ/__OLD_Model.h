#pragma once

#include "DX11_Core.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "ErrorCodes.h"
#include "Utility.h"
#include "CompilerSettings.h"
#include "../Definitions/ModelData.h"
#include "ModelBuffer.h"
#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"
#include "CollisionSpatialDataF.h"


// This class has no special alignment requirements
class __OLD_Model
{
	public:
		// Model data is stored in static unordered_map collections & indexed by unique string code
		typedef std::unordered_map<std::string, Model*> ModelCollection;

		// Enumeration of the different supported model classes
		enum ModelClass { Unknown = 0, Ship, ComplexShipSection, Tile, Terrain };


	public:
		
		Model(void);
		~Model(void);

		// Primary store of all raw model data
		ModelData					Geometry;

		// Primary store of all compiled model data & buffers
		ModelBuffer					Data;

		// Public model manipulation methods
		Result						Initialise(const std::string & filename);
		void						Shutdown();
		void						Render(void);

		// Overloaded method to allow initialisation via string filenames
		CMPINLINE Result			Initialise(const std::string & filename, const std::string & texturefilename)
		{
			if (filename != NullString && texturefilename != NullString)
				return Initialise(filename.c_str(), texturefilename.c_str());
			else return ErrorCodes::CouldNotInitialiseModelWithInvalidStringParams;
		}

		// Passthrough methods to model buffer for convenience
		CMPINLINE ModelBuffer &				GetModelBuffer(void)				{ return Data; }
		CMPINLINE const ID3D11Buffer *		GetCompiledVertexBuffer(void) const	{ return Data.VertexBuffer.GetCompiledBuffer(); }
		CMPINLINE const ID3D11Buffer *		GetCompiledIndexBuffer(void) const	{ return Data.IndexBuffer.GetCompiledBuffer(); }
		CMPINLINE auto						GetVertexCount(void) const			{ return Data.VertexBuffer.GetVertexCount(); }
		CMPINLINE auto						GetIndexCount(void) const			{ return Data.IndexBuffer.GetIndexCount(); }
		CMPINLINE auto						GetVertexMemorySize(void) const		{ return Data.VertexBuffer.GetVertexSize(); }
		CMPINLINE auto						GetIndexMemorySize(void) const		{ return Data.IndexBuffer.GetIndexSize(); }
		

		// Public accessor/modifer methods for key variables
		CMPINLINE int				GetID(void) { return m_id; }
		CMPINLINE void				SetID(int id) { m_id = id; }
		CMPINLINE std::string		GetCode(void) { return Data.GetCode(); }
		CMPINLINE void				SetCode(std::string code) { Data.SetCode(code); }
		CMPINLINE std::string		GetFilename(void) { return m_filename; }
		CMPINLINE void				SetFilename(std::string filename) { m_filename = filename; }
		CMPINLINE ModelClass		GetModelClass(void) { return m_modelclass; }
		CMPINLINE void				SetModelClass(ModelClass modelclass) { m_modelclass = modelclass; }
		CMPINLINE bool				IsGeometryLoaded(void) const { return m_geometryloaded; }
		CMPINLINE void				SetGeometryLoaded(bool loaded) { m_geometryloaded = loaded; }
		CMPINLINE bool				IsStandardModel(void) { return m_standardmodel; }
		CMPINLINE void				SetStandardModel(bool standard) { m_standardmodel = standard; }
		CMPINLINE XMFLOAT3			GetModelMinBounds(void)	{ return m_minbounds; }
		CMPINLINE XMFLOAT3			GetModelMaxBounds(void) { return m_maxbounds; }
		void						RecalculateDimensions(void);
		CMPINLINE XMFLOAT3			GetModelSize(void)		{ return m_modelsize; }
		CMPINLINE XMFLOAT3			GetModelCentre(void)	{ return m_modelcentre; }
		CMPINLINE XMFLOAT3			GetEffectiveModelSize(void) { return m_effectivesize; }
		CMPINLINE void				SetEffectiveModelSize(const XMFLOAT3 & size) { m_effectivesize = size; }
		CMPINLINE INTVECTOR3		GetElementSize(void)				{ return m_elementsize; }
		CMPINLINE void				SetElementSize(INTVECTOR3 elsize)	{ m_elementsize = elsize; }

		// Determines whether the model will be centred about the origin upon loading (default), or not adjusted on load
		CMPINLINE bool				CentredAboutOrigin(void) const		{ return m_origin_centred; }
		CMPINLINE void				SetCentredAboutOrigin(bool centred)	{ m_origin_centred = centred; }

		// Methods which accept either a target effective size, or a scaling factor, and resize the model data accordingly
		CMPINLINE XMFLOAT3			GetActualModelSize(void)	{ return m_actualeffectivesize; }
		CMPINLINE XMFLOAT3			GetModelScalingFactor(void)	{ return m_scalingfactor; }
		void						SetActualModelSize(const XMFLOAT3 & actualeffectivesize);
		void						SetModelScalingFactor(const XMFLOAT3 & scalingfactor);
		
		// Scales the model geometry by a specified factor in each dimension
		void						ScaleModelGeometry(const XMFLOAT3 & scale);

		// Methods to handle compound model operations
		CMPINLINE bool				IsCompoundModel(void)					{ return m_iscompound; }
		CMPINLINE void				SetIsCompoundModel(bool compound)		{ m_iscompound = true; RecalculateCompoundModelData(); }
		void						AddCompoundModelComponent(Model *model, const XMFLOAT3 & offset);
		void						RemoveCompoundModelComponent(Model *model, const XMFLOAT3 & offset);
		void						RemoveCompoundModelComponent(const XMFLOAT3 & offset);
		void						ClearCompoundModelData(void);
		void						RecalculateCompoundModelData(void);

		// List of collision objects attached to this model
		CMPINLINE std::vector<CollisionSpatialDataF> & CollisionData(void)	{ return m_collision; }
		CMPINLINE void				SetCollisionData(const std::vector<CollisionSpatialDataF> & collision) { m_collision = collision; }
		CMPINLINE void				SetCollisionData(std::vector<CollisionSpatialDataF> && collision) { m_collision = std::move(collision); }

		// Central model storage methods
		static bool					ModelExists(const std::string & code);
		static Model *				GetModel(const std::string & code);
		static Model *				GetModelFromFilename(const std::string & filename);
		static void					AddModel(Model *model);

		// Deallocates all centrally-maintained model data
		static void TerminateAllModelData(void);

		// Identifies the class of model based on its string description
		static ModelClass DetermineModelClass(const std::string s);

	public:

		// Static collection of all models in the game, indexed by string code
		static ModelCollection Models;

	private:

		// Private methods for individual model class operations
		Result		InitialiseBuffers(void);
		
		Result		LoadModel(const char*);
		void		ReleaseModel();

		void		CentreModelAboutOrigin(void);

	private:

		// Private variables for buffer / model storage
		int						m_id;
		std::string				m_filename;
		ModelClass				m_modelclass;
		bool					m_geometryloaded;
		bool					m_standardmodel;
		bool					m_origin_centred;
		XMFLOAT3				m_minbounds, m_maxbounds, m_modelsize, m_modelcentre, m_effectivesize;
		XMFLOAT3				m_actualsize, m_actualeffectivesize, m_scalingfactor;

		// Optional ability to specify size in game elements; if set, mesh will be scaled by load post-processing functions
		INTVECTOR3				m_elementsize;

		// List of collision objects attached to this model
		std::vector<CollisionSpatialDataF> m_collision;

		// Fields allowing generation of compound models
		struct					CompoundModelComponent 
		{ 
			Model *model; XMFLOAT3 offset; 
			CompoundModelComponent(Model *_model, XMFLOAT3 _offset) { model = _model; offset = _offset; }
		};
		typedef											std::vector<CompoundModelComponent> CompoundModelComponentCollection;
		bool											m_iscompound;
		CompoundModelComponentCollection				m_compoundmodels;
		CompoundModelComponentCollection::size_type		m_compoundmodelcount;
	
};
