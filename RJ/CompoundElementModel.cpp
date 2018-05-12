#include "CompoundElementModel.h"
#include "FastMath.h"
#include "Model.h"

// Default constructor
CompoundElementModel::CompoundElementModel(void)
	:
	CompoundElementModel(NULL_UINTVECTOR3)
{
}

// Constructor for a given element size
CompoundElementModel::CompoundElementModel(const UINTVECTOR3 & size)
	:
	m_minbounds(NULL_VECTOR), 
	m_maxbounds(NULL_VECTOR), 
	m_compound_model_size(NULL_VECTOR), 
	m_compound_model_centre(NULL_VECTOR)
{
	// Store the element size of this compound model
	SetSize(size);

	// Perform an initial update of the layout vector (for this size, and based on a null model vector)
	ResumeUpdates();
}

// Copy constructor
CompoundElementModel::CompoundElementModel(const CompoundElementModel & other)
{
	// Set to the same size as our target
	SetSize(other.GetSize());

	// Copy all primary model data
	m_models = other.m_models;

	// Resume updates, which will force a calculation of all derived layout data
	ResumeUpdates();
}

// Copy assignment
CompoundElementModel & CompoundElementModel::operator=(const CompoundElementModel & other)
{
	// Set to the same size as our target
	SetSize(other.GetSize());

	// Copy all primary model data
	m_models = other.m_models;

	// Resume updates, which will force a calculation of all derived layout data
	ResumeUpdates();
	return *this;
}

// Move constructor
CompoundElementModel::CompoundElementModel(CompoundElementModel && other)
{
	// Set to the same size as our target
	SetSize(other.GetSize());

	// Copy all primary model data
	m_models = std::move(other.m_models);

	// Resume updates, which will force a calculation of all derived layout data
	ResumeUpdates();
}

// Move assignment
CompoundElementModel & CompoundElementModel::operator=(CompoundElementModel && other)
{
	// Set to the same size as our target
	SetSize(other.GetSize());

	// Copy all primary model data
	m_models = std::move(other.m_models);

	// Resume updates, which will force a calculation of all derived layout data
	ResumeUpdates();
	return *this;
}

// Constructor for a new model instance
CompoundElementModel::CompoundModelInstance::CompoundModelInstance(ModelInstance model, CompoundTileModelType type, const UINTVECTOR3 & location, Rotation90Degree rotation)
	:
	Model(model), 
	Type(type), 
	Location(location), 
	Rotation(rotation)
{
	// Construct the local world (scale * rotation * translation) matrix
	LocalWorld = XMMatrixMultiply(XMMatrixMultiply(
		model.GetWorldMatrix(),																// Scale matrix
		GetRotationMatrixInstance(Rotation)),												// Local rotation
		XMMatrixTranslationFromVector(Game::ElementLocationToPhysicalPosition(location)));	// Local offset translation within the environment

	// LocationIndex cannot be derived here since it requires the overall compound model size; it is assigned when the instance is added to a collection
}

// Add a new model to the collection.  Will also recalculate all model data unless updates are suspended
void CompoundElementModel::AddModel(ModelInstance model, CompoundTileModelType type, const UINTVECTOR3 & location, Rotation90Degree rotation)
{
	// Create a new entry and calculate its location index
	CompoundModelInstance instance(model, type, location, rotation);
	UINT index = ELEMENT_INDEX_EX(location.x, location.y, location.z, m_size);
	instance.LocationIndex = index;

	// Perform a sorted-insert into the primary model collection
	InsertIntoSortedVector<CompoundModelInstance>(m_models, instance, [](const CompoundModelInstance & lhs, const CompoundModelInstance & rhs) 
	{ 
		return (lhs.LocationIndex < rhs.LocationIndex); 
	});

	// Recalculate all derived model data
	RecalculateModelData();
}

// Recalculate all derived data (e.g. layout) based on primary model collection
void CompoundElementModel::RecalculateModelData(void)
{
	if (m_update_suspended) return;

	BuildLayoutMap();
	CalculateModelBounds();
}

void CompoundElementModel::BuildLayoutMap(void)
{
	// Allocate the layout data if required
	if (m_layout.size() != (m_elementcount + 1))
	{
		m_layout.clear();
		m_layout.insert(m_layout.begin(), (m_elementcount + 1), 0U);
	}

	// Process all all SORTED models in the primary collection in turn
	int lastindex = -1;
	CompoundModelIndex model_count = m_models.size();
	for (CompoundModelIndex model = 0; model < model_count; ++model)
	{
		// Get the location index of this model
		int index = static_cast<int>(m_models[model].LocationIndex);
		if (index != lastindex)
		{
			// This is where the models for 'index' begin.  Backfill all unfilled entries
			// up to (and including) this one with the new index.  This works because the model range is 
			// defined as (layout[loc] <= X < layout[loc+1]), so these prior backfilled entries will have
			// begin == end and therefore no models
			for (int el = (lastindex + 1); el <= index; ++el)
			{
				m_layout[el] = model;
			}

			// This is now the new 'last' index to be filled
			lastindex = index;
		}
	}

	// Fill all remaining layout entries with the model count, including the extra end layout element.  This means the last valid entry will 
	// have (layout[loc] <= X < count) which is correct, and all after it will have (count <= X < count) which is zero models
	for (CompoundModelIndex i = (lastindex + 1); i < (m_elementcount + 1); ++i)
	{
		m_layout[i] = model_count;
	}
}

void CompoundElementModel::CalculateModelBounds(void)
{
	XMVECTOR minbounds = LARGE_VECTOR_P;
	XMVECTOR maxbounds = LARGE_VECTOR_N;

	for (auto & model : m_models)
	{
		auto modeldata = model.Model.GetModel();		if (!modeldata) continue;

		// Test whether this model pushes out the overall compound model bounds
		XMVECTOR pos = Game::ElementLocationToPhysicalPosition(model.Location);
		minbounds = XMVectorMin(minbounds, XMVectorAdd(pos, XMLoadFloat3(&modeldata->GetMinBounds())));
		maxbounds = XMVectorMax(maxbounds, XMVectorAdd(pos, XMLoadFloat3(&modeldata->GetMaxBounds())));
	}

	// Make sure that at least one model has bounds, otherwise we remain with the default settings
	if (XMVector3Less(maxbounds, minbounds))
	{
		m_minbounds = m_maxbounds = m_compound_model_centre = m_compound_model_size = NULL_VECTOR;
	}
	else
	{
		// Store these results and calculate other derived data
		m_minbounds = minbounds; 
		m_maxbounds = maxbounds;
		m_compound_model_size = XMVectorSubtract(m_maxbounds, m_minbounds);
		m_compound_model_centre = XMVectorMultiply(XMVectorAdd(m_minbounds, m_maxbounds), HALF_VECTOR);
	}
}

// Suspend updates, to allow multiple models to be added or removed before recalculation.  Model
// layout data should not be used while updates are suspended since it will not be consistent
void CompoundElementModel::SuspendUpdates(void)
{
	m_update_suspended = true;
}

// Resume updates.  Will trigger a recalculation of derived model data
void CompoundElementModel::ResumeUpdates(void)
{
	m_update_suspended = false;
	RecalculateModelData();
}

// Reset the compound model and remove all data
void CompoundElementModel::Reset(void)
{
	// Simply remove all models from the primary collection and force a recalculation
	m_models.clear();
	ResumeUpdates();
}

// Return the range of models corresponding to this element location
CompoundElementModel::ModelIndexRange CompoundElementModel::GetModelIndices(unsigned int x, unsigned int y, unsigned int z)
{
	// Validate parameters
	if (x >= m_size.x || y >= m_size.y || z >= m_size.z) return ModelIndexRange();

	// Return the range of applicable indices, if any
	unsigned int index = ELEMENT_INDEX_EX(x, y, z, m_size);
	return ModelIndexRange(m_layout[index], m_layout[index + 1]);
}

// Return the range of models corresponding to this element location.  Element location is not validated
// and invalid parameters passed into the function will have undefined behaviour
CompoundElementModel::ModelIndexRange CompoundElementModel::GetModelIndicesUnchecked(unsigned int x, unsigned int y, unsigned int z)
{
	// Debug assert the parameters; will not be present in release mode
	assert(!(x >= m_size.x || y >= m_size.y || z >= m_size.z));

	// Return the range of applicable indices, if any
	unsigned int index = ELEMENT_INDEX_EX(x, y, z, m_size);
	return ModelIndexRange(m_layout[index], m_layout[index + 1]);
}

// Indicates whether any model is present at the given location
bool CompoundElementModel::HasModelAtLocation(const UINTVECTOR3 & location)
{
	return GetModelIndices(location).HasModels();
}

// Return all models for the given location
CompoundElementModel::ModelData CompoundElementModel::GetModelsAtLocation(const UINTVECTOR3 & location)
{
	auto indices = GetModelIndices(location);

	CompoundElementModel::ModelData results;
	results.insert(results.begin(), m_models.begin() + indices.begin, m_models.begin() + indices.end);

	return results;
}

// Store a new compound model size
void CompoundElementModel::SetSize(const UINTVECTOR3 & size)
{
	m_size = size;
	m_elementcount = (size.x * size.y * size.z);
}

// String serialisation method for model instances
std::string CompoundElementModel::CompoundModelInstance::str(void) const
{
	return concat("[Model:")(Model.GetModel() != NULL ? Model.GetModel()->GetCode() : "NULL")(", Type:")(static_cast<int>(Type))(", Location: ")
		(Location.ToString())(", LocIndex: ")(LocationIndex)(", Rotation: ")(static_cast<int>(Rotation))("]").str();
}

// Destructor
CompoundElementModel::~CompoundElementModel(void)
{
}

