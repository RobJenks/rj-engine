#pragma once
#ifndef __ModelH__
#define __ModelH__

#include "DX11_Core.h" // #include "FullDX11.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "ErrorCodes.h"
#include "Utility.h"
#include "CompilerSettings.h"
#include "Texture.h"

using namespace std;
using namespace std::tr1;

class Model
{
	public:
		// Model data is stored in static unordered_map collections & indexed by unique string code
		typedef unordered_map<std::string, Model*> ModelCollection;

		// Enumeration of the different supported model classes
		enum ModelClass { Unknown = 0, Ship, ComplexShipSection, Tile, Terrain };

	private:
		struct VertexType
		{
			D3DXVECTOR3 position;
			D3DXVECTOR2 texture;
			D3DXVECTOR3 normal;
		};

		struct ModelType
		{
			float x, y, z;
			float tu, tv;
			float nx, ny, nz;
		};

	public:
		// The data format used to hold index buffer data.  Note that DX11 (feature level 11.0) appears to support
		// UINT32 sized indices, but feature level 9.1 only appears to support UINT16.  Using the latter for now
		// to maintain compatibility; likely too major a change to handle via the localiser
		typedef UINT16 INDEXFORMAT;		

		Model(void);
		~Model(void);

		// Public model manipulation methods
		Result						Initialise(const char *filename, const char *texturefilename);
		void						Shutdown();
		void						Render(void);

		// Overloaded method to allow initialisation via string filenames
		CMPINLINE Result			Initialise(const std::string & filename, const std::string & texturefilename)
		{
			if (filename != NullString && texturefilename != NullString)
				return Initialise(filename.c_str(), texturefilename.c_str());
			else return ErrorCodes::CouldNotInitialiseModelWithInvalidStringParams;
		}

		CMPINLINE int						GetIndexCount() const	{ return m_indexCount; }
		CMPINLINE ID3D11ShaderResourceView*	GetTexture() const		{ return m_texture->GetTexture(); }

		// Public accessor/modifer methods for key variables
		CMPINLINE int				GetID(void) { return m_id; }
		CMPINLINE void				SetID(int id) { m_id = id; }
		CMPINLINE string			GetCode(void) { return m_code; }
		CMPINLINE void				SetCode(string code) { m_code = code; }
		CMPINLINE string			GetFilename(void) { return m_filename; }
		CMPINLINE void				SetFilename(string filename) { m_filename = filename; }
		CMPINLINE ModelClass		GetModelClass(void) { return m_modelclass; }
		CMPINLINE void				SetModelClass(ModelClass modelclass) { m_modelclass = modelclass; }
		CMPINLINE string			GetTextureFilename(void) { return m_texturefilename; }
		CMPINLINE void				SetTextureFilename(string filename) { m_texturefilename = filename; }
		CMPINLINE bool				IsGeometryLoaded(void) { return m_geometryloaded; }
		CMPINLINE void				SetGeometryLoaded(bool loaded) { m_geometryloaded = loaded; }
		CMPINLINE bool				IsStandardModel(void) { return m_standardmodel; }
		CMPINLINE void				SetStandardModel(bool standard) { m_standardmodel = standard; }
		CMPINLINE D3DXVECTOR3		GetModelMinBounds(void)	{ return m_minbounds; }
		CMPINLINE D3DXVECTOR3		GetModelMaxBounds(void) { return m_maxbounds; }
		void						RecalculateDimensions(void);
		CMPINLINE D3DXVECTOR3		GetModelSize(void)		{ return m_modelsize; }
		CMPINLINE D3DXVECTOR3		GetModelCentre(void)	{ return m_modelcentre; }
		CMPINLINE D3DXVECTOR3		GetEffectiveModelSize(void) { return m_effectivesize; }
		CMPINLINE void				SetEffectiveModelSize(D3DXVECTOR3 size) { m_effectivesize = size; }
		CMPINLINE INTVECTOR3		GetElementSize(void)				{ return m_elementsize; }
		CMPINLINE void				SetElementSize(INTVECTOR3 elsize)	{ m_elementsize = elsize; }

		// Methods which accept either a target effective size, or a scaling factor, and resize the model data accordingly
		CMPINLINE D3DXVECTOR3		GetActualModelSize(void)	{ return m_actualeffectivesize; }
		CMPINLINE D3DXVECTOR3		GetModelScalingFactor(void)	{ return m_scalingfactor; }
		void						SetActualModelSize(D3DXVECTOR3 actualeffectivesize);
		void						SetModelScalingFactor(D3DXVECTOR3 scalingfactor);
		
		// Scales the model geometry by a specified factor in each dimension
		void						ScaleModelGeometry(D3DXVECTOR3 scale);

		// Methods to handle compound model operations
		CMPINLINE bool				IsCompoundModel(void)					{ return m_iscompound; }
		CMPINLINE void				SetIsCompoundModel(bool compound)		{ m_iscompound = true; RecalculateCompoundModelData(); }
		void						AddCompoundModelComponent(Model *model, D3DXVECTOR3 offset);
		void						RemoveCompoundModelComponent(Model *model, D3DXVECTOR3 offset);
		void						RemoveCompoundModelComponent(D3DXVECTOR3 offset);
		void						ClearCompoundModelData(void);
		void						RecalculateCompoundModelData(void);

		// Methods providing public access to buffer data, for centralised instanced rendering by the core engine
		ID3D11Buffer *				GetVertexBuffer(void) const				{ return m_vertexBuffer; }
		ID3D11Buffer *				GetIndexBuffer(void) const				{ return m_indexBuffer; }
		static unsigned int			GetVertexMemorySize(void)				{ return (unsigned int)sizeof(VertexType); }

		// Central model storage methods
		static Model *GetModel(const std::string & code);
		static Model *GetModelFromFilename(const std::string & filename);
		static void AddModel(Model *model);

		// Deallocates all centrally-maintained model data
		static void TerminateAllModelData(void);

		// Identifies the class of model based on its string description
		static ModelClass DetermineModelClass(const string s);

	public:

		// Static collection of all models in the game, indexed by string code
		static ModelCollection Models;

	private:

		// Private methods for individual model class operations
		Result		InitialiseBuffers(void);
		void		ShutdownBuffers();
		void		RenderBuffers(void);

		Result		LoadTexture(const char*);
		void		ReleaseTexture();

		Result		LoadModel(const char*);
		void		ReleaseModel();

		void		CentreModelAboutOrigin(void);

	private:
		// Private variables for buffer / model storage
		ID3D11Buffer			*m_vertexBuffer, *m_indexBuffer;
		int						m_vertexCount, m_indexCount;
		Texture*				m_texture;
		ModelType*				m_model;

		// Private variables for other, supporting model information
		int						m_id;
		string					m_code;
		string					m_filename;
		ModelClass				m_modelclass;
		string					m_texturefilename;
		bool					m_geometryloaded;
		bool					m_standardmodel;
		D3DXVECTOR3				m_minbounds, m_maxbounds, m_modelsize, m_modelcentre, m_effectivesize;
		D3DXVECTOR3				m_actualsize, m_actualeffectivesize, m_scalingfactor;

		// Optional ability to specify size in game elements; if set, mesh will be scaled by load post-processing functions
		INTVECTOR3				m_elementsize;

		// Fields allowing generation of compound models
		struct					CompoundModelComponent 
		{ 
			Model *model; D3DXVECTOR3 offset; 
			CompoundModelComponent(Model *_model, D3DXVECTOR3 _offset) { model = _model; offset = _offset; }
		};
		typedef								vector<CompoundModelComponent> CompoundModelComponentCollection;
		bool								m_iscompound;
		CompoundModelComponentCollection	m_compoundmodels;
		int									m_compoundmodelcount;
	
};


#endif