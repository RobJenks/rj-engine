#pragma once

#include <vector>
#include <tuple>
#include "IntVector.h"
#include "FastMath.h"
#include "CompoundTileModelType.h"
#include "ModelInstance.h"

class CompoundElementModel
{
public:

	// Holds data on a particular model in the collection
	struct CompoundModelInstance
	{
		ModelInstance									Model;
		CompoundTileModelType							Type; 
		UINTVECTOR3										Location;					// Element location; may not actually be required
		size_t											LocationIndex;				// Element index of the model; dependent on overall model size
		Rotation90Degree								Rotation;
		AXMMATRIX										LocalWorld;					// Local world (scale * rotation * translation)

		CompoundModelInstance(void) : Type(CompoundTileModelType::Unknown), Location(NULL_UINTVECTOR3), LocationIndex(0U), LocalWorld(ID_MATRIX), Rotation(Rotation90Degree::Rotate0) { }
		CompoundModelInstance(ModelInstance model, CompoundTileModelType type, const UINTVECTOR3 & location, Rotation90Degree rotation);
		
		std::string										str(void) const;

	};

	class ModelData : public std::vector<CompoundModelInstance>
	{
	public:

		ModelData::iterator			FindItem(CompoundTileModelType type) { return std::find_if(begin(), end(), [type](auto & item) { return (item.Type == type); }); }
		ModelData::iterator			FindItem(CompoundTileModelType type, Rotation90Degree rotation) 
		{ return std::find_if(begin(), end(), [type, rotation](ModelData::value_type & item) { return (item.Type == type && item.Rotation == rotation); }); }

		bool						HasItem(CompoundTileModelType type) { return (FindItem(type) != end()); }
		bool						HasItem(CompoundTileModelType type, Rotation90Degree rotation) { return (FindItem(type, rotation) != end()); }

	};


	// Type definitions for all key types in this class
	typedef CompoundElementModel::ModelData				CompoundModelData;			// Linear vector of all models; |vec| == N for N models
	typedef CompoundModelData::size_type				CompoundModelIndex;			// Standard index type into the model collection
	typedef std::vector<CompoundModelIndex>				CompoundModelLayoutMap;		// Linear vector of indices per element; |vec| == (sx*sy*sz + 1) for size s{x|y|z}


	// Represents a range of elements in the model collection
	struct ModelIndexRange
	{ 
		CompoundModelIndex begin; CompoundModelIndex end;

		CMPINLINE ModelIndexRange(void) : begin(0U), end(0U) { }
		CMPINLINE ModelIndexRange(CompoundModelIndex _begin, CompoundModelIndex _end) : begin(_begin), end(_end) { }

		CMPINLINE bool HasModels(void) const					{ return (begin != end); }
		CMPINLINE CompoundModelIndex GetModelCount(void) const	{ return (end - begin); }
	};

	// Constructors
	CompoundElementModel(void);
	CompoundElementModel(const UINTVECTOR3 & size);

	// Copy constructor/assignment
	CompoundElementModel(const CompoundElementModel & other);
	CompoundElementModel & operator=(const CompoundElementModel & other);

	// Move constructor/assignment
	CompoundElementModel(CompoundElementModel && other);
	CompoundElementModel & operator=(CompoundElementModel && other);

	// Return a reference to the full model collection
	CMPINLINE CompoundModelData &						GetModels(void) { return m_models; }
	CMPINLINE const CompoundModelData &					GetModels(void) const { return m_models; }

	// Add a new model to the collection
	void												AddModel(ModelInstance model, CompoundTileModelType type, const UINTVECTOR3 & location, Rotation90Degree rotation);

	// Number of model components making up this compound model
	CMPINLINE CompoundModelIndex						GetModelCount(void) const { return m_models.size(); }

	// Element size of the overall model
	CMPINLINE UINTVECTOR3								GetSize(void) const { return m_size; }

	// Layout information for the compound model
	CMPINLINE XMVECTOR									GetMinBounds(void) const { return m_minbounds; }
	CMPINLINE XMVECTOR									GetMaxBounds(void) const { return m_maxbounds; }
	CMPINLINE XMVECTOR									GetCompoundModelCentre(void) const { return m_compound_model_centre; }
	CMPINLINE XMVECTOR									GetCompoundModelSize(void) const { return m_compound_model_size; }

	// Recalculate model layout data based on the full set of model data in our collection
	void												RecalculateModelData(void);

	// Suspend or resume updates, to allow multiple models to be added or removed before recalculation.  Model
	// layout data should not be used while updates are suspended since it will not be consistent
	void												SuspendUpdates(void);
	void												ResumeUpdates(void);

	// Reset the compound model and remove all data
	void												Reset(void);

	// Get indices for the range of models corresponding to a particular element
	ModelIndexRange										GetModelIndices(unsigned int x, unsigned int y, unsigned int z);
	ModelIndexRange										GetModelIndicesUnchecked(unsigned int x, unsigned int y, unsigned int z);
	CMPINLINE ModelIndexRange							GetModelIndices(const UINTVECTOR3 & location)			{ return GetModelIndices(location.x, location.y, location.z); }
	CMPINLINE ModelIndexRange							GetModelIndicesUnchecked(const UINTVECTOR3 & location)	{ return GetModelIndicesUnchecked(location.x, location.y, location.z); }

	// Indicates whether any model is present at the given location
	bool												HasModelAtLocation(const UINTVECTOR3 & location);

	// Return all models for the given location
	CompoundElementModel::ModelData						GetModelsAtLocation(const UINTVECTOR3 & location);


	// Destructor
	~CompoundElementModel(void);


private:

	// Class data
	CompoundModelData					m_models;
	CompoundModelLayoutMap				m_layout;

	UINTVECTOR3							m_size;
	CompoundModelIndex					m_elementcount;

	bool								m_update_suspended;

	AXMVECTOR							m_minbounds, m_maxbounds;	// Minimum and maximum bounds of the overall compound model, in world space
	AXMVECTOR							m_compound_model_size;		// Actual size of the compound model in world space (max-min bounds)
	AXMVECTOR							m_compound_model_centre;	// Centre point of the compound model in world space (max+min bounds / 2)


	// Store the element size of this collection; does not affect model data
	void								SetSize(const UINTVECTOR3 & size);

	// Internal components of the recalculation process
	void								BuildLayoutMap(void);
	void								CalculateModelBounds(void);


};
