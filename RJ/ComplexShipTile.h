#pragma once

#ifndef __ComplexShipTileH__
#define __ComplexShipTileH__

#include <new>
#include <vector>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "AlignedAllocator.h"
#include "Utility.h"
#include "FastMath.h"
#include "GameDataExtern.h"
#include "iTakesDamage.h"
#include "RepairableObject.h"
#include "Model.h"
#include "ComplexShipElement.h"
#include "TileConnections.h"
#include "Damage.h"
#include "FadeEffect.h"
#include "HighlightEffect.h"
#include "Power.h"
#include "GameConsoleCommand.h"
#include "ViewPortal.h"
class TiXmlElement;
class ComplexShip;
class ComplexShipSection;
class Hardpoint;
class BoundingObject;
class Resource;
class ProductionCost;

#define DEBUG_LOGINSTANCECREATION

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ComplexShipTile : public ALIGN16<ComplexShipTile>, public iTakesDamage, public RepairableObject
{
public:

	// Static record of the highest ID value in existence, for assigning to new tiles upon registration
	static Game::ID_TYPE					InstanceCreationCount;

	// Method to generate a new unique ID, called for each new tile being instantiated
	static Game::ID_TYPE					GenerateNewUniqueID(void)	{ return (++InstanceCreationCount); }

	// Struct holding information on the individual models making up a tile
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct TileModel : public ALIGN16<TileModel>
	{ 
		enum TileModelType 
		{
			Unknown = 0,
			WallStraight,
			WallCorner,
			WallConnection
		};

		AXMMATRIX rotmatrix; AXMVECTOR offset; Model * model; INTVECTOR3 elementpos; Rotation90Degree rotation; TileModelType type;
		TileModel(void) { model = NULL; offset = NULL_VECTOR; elementpos = NULL_INTVECTOR3; rotation = Rotation90Degree::Rotate0; type = TileModelType::Unknown; rotmatrix = ID_MATRIX; }
		TileModel(Model *_model, INTVECTOR3 _elementpos, Rotation90Degree _rotation, TileModelType _type) 
		{ 
			// Set the supplied parameters
			model = _model; 
			elementpos = _elementpos; 
			rotation = _rotation;
			type = _type;

			// Calculate derived parameters from this data
			offset = Game::ElementLocationToPhysicalPosition(elementpos);

			// Derive a rotation matrix for this part of the model
			if (rotation == Rotation90Degree::Rotate0 || !model)
			{
				rotmatrix = ID_MATRIX;
			}
			else
			{
				// Calculate temporary translation matrices to translate the model to its centre, so we can then rotate, and then translate back
				// Most tile models will be zero-centred, so this offset does nothing, however this also accomodates cases with a non-zero centre
				XMVECTOR centrepoint = XMLoadFloat3(&model->GetModelCentre());
				XMMATRIX off = XMMatrixTranslationFromVector(XMVectorNegate(centrepoint));
				XMMATRIX invoff = XMMatrixTranslationFromVector(centrepoint);
				
				// Derive and store the rotation matrix for this model
				rotmatrix = XMMatrixMultiply(XMMatrixMultiply(off, GetRotationMatrix(rotation)), invoff);
			}
		}
	};

	// Struct used to construct a linked list of model data
	struct ModelLinkedList		
	{ 
		Model *model; Rotation90Degree rotation; TileModel::TileModelType type; ModelLinkedList *next; 

		ModelLinkedList(void) { model = NULL; rotation = Rotation90Degree::Rotate0; type = TileModel::TileModelType::Unknown; next = NULL; }
		ModelLinkedList(Model *_model, Rotation90Degree _rotation, TileModel::TileModelType _type) 
					   { model = _model; rotation = _rotation; type = _type; next = NULL; }

		void Add(Model *_model, Rotation90Degree _rotation, TileModel::TileModelType _type)
		{
			// If the model entry in this node is empty then simply add it here
			if (!model) { model = _model; rotation = _rotation; type = _type; return; }

			// Otherwise, traverse the linked list until we reach the next null item, or until we terminate due to a potential infinite loop
			int its = 0;
			ModelLinkedList *node = this->next;
			while (node != NULL && ++its < 1000) node = node->next;

			// Add a new node at this position, assuming we did reach a null item (and didn't get stuck in an infinite loop)
			if (!node) node = new ModelLinkedList(_model, _rotation, _type);
		}

		bool __HasItem(Model *_model, Rotation90Degree _rotation, TileModel::TileModelType _type, bool testmodel, bool testrot, bool testtype)
		{
			// Traverse the linked list until we find the item, or until we terminate due to a potential infinite loop
			int its = 0;
			ModelLinkedList *node = this;
			while (node != NULL && ++its < 1000) 
			{
				// Search based only on the specified criteria
				if ( (!testmodel || node->model == _model) && 
					 (!testrot || node->rotation == _rotation) && 
					 (!testtype || node->type == _type) )				return true;

				// Move to the next node
				node = node->next;
			}

			// We did not find the item so return false
			return false;
		}

		// Overriden methods to search based on specific criteria
		bool HasItem(Model *_model) { return __HasItem(_model, Rotation90Degree::Rotate0, TileModel::TileModelType::Unknown, true, false, false); }
		bool HasItem(Rotation90Degree _rot) { return __HasItem(NULL, _rot, TileModel::TileModelType::Unknown, false, true, false); }
		bool HasItem(TileModel::TileModelType _type) { return __HasItem(NULL, Rotation90Degree::Rotate0, _type, false, false, true); }
		bool HasItem(Model *_model, Rotation90Degree _rot) { return __HasItem(_model, _rot, TileModel::TileModelType::Unknown, true, true, false); }
		bool HasItem(Rotation90Degree _rot, TileModel::TileModelType _type) { return __HasItem(NULL, _rot, _type, false, true, true); }
		bool HasItem(Model *_model, Rotation90Degree _rot, TileModel::TileModelType _type) { return __HasItem(_model, _rot, _type, true, true, true); }

		void RemoveItem(Model *_model, Rotation90Degree _rotation, TileModel::TileModelType _type)
		{
			// Traverse the linked list until we find the item, or until we terminate due to a potential infinite loop
			int its = 0;
			ModelLinkedList *node = this, *last = NULL;
			while (node != NULL && ++its < 1000) 
			{
				if (node->model == _model && node->rotation == _rotation && node->type == _type) 
				{
					if (last) 
					{
						last->next = node->next;	// Link the previous element to the next, if we have one
						delete node; return;		// Delete the current node and return
					}
					else
					{
						if (node->next)
						{												// If we are first, and have a next node, then 
							node->model = node->next->model;			// copy the model back to this node and
							node->rotation = node->next->rotation;
							node->type = node->next->type;
							ModelLinkedList *tmp = node->next->next;	// get a link to this next node's next item (or NULL)
							delete node->next;							// delete the next node, since we just skipped over it
							node->next = tmp;							// set the link from current node to the next->next node, so we skip over it
							return;										// return after successfully removing the item
						}
						else
						{													// Otherwise, we are the only node (no previous or next node)
							node->model = NULL;								// so simply set the current node model to NULL,
							node->rotation = Rotation90Degree::Rotate0;		// reset the 
							node->type = TileModel::TileModelType::Unknown;	// other parameters,
							return;											// and return since we removed the model
						}
					}
				}

				// Record the last node, and move on to the next one
				last = node;
				node = node->next;
			}

			// We did not find and remove the item
			return;
		}

		void RemoveAllItems(void)
		{
			int its = 0;

			// First, keep removing the next node in the sequence until we have removed all but the first node
			while (next != NULL && ++its < 1000)
				this->RemoveItem(next->model, next->rotation, next->type);

			// Then remove the first (this) node
			model = NULL; next = NULL;
			delete this;
		}
	};


	// Struct holding all compound model data
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct TileCompoundModelSet : public ALIGN16<TileCompoundModelSet>
	{
		// Struct to allow 16-byte alignment of TileModel instances in STL vector
		typedef __declspec(align(16))TileModel ATileModel;	
		typedef __declspec(align(16)) struct ATileModel_P_T : public ALIGN16<ATileModel_P_T> 
		{ 
			ATileModel value;
			ATileModel_P_T(ATileModel & model) { value = model; }
		} ATileModel_P;
		
		// Type definition for aligned collection of tile model instances 
		typedef __declspec(align(16)) std::vector<ATileModel_P, AlignedAllocator<ATileModel_P, 16U>> TileModelCollection;

		TileModelCollection		Models;					// Linear collection of all models, for rendering efficiency
		ModelLinkedList	****	ModelLayout;			// Spatial layout of models, for efficient indexing into the collection.  ModelLinkedList*[x][y][z]	
		INTVECTOR3				Size;					// Size of the compound model, in elements
		AXMVECTOR				MinBounds, MaxBounds;	// Minimum and maximum bounds of the overall compound model, in world space
		AXMVECTOR				CompoundModelSize;		// Actual size of the compound model in world space (max-min bounds)
		AXMVECTOR				CompoundModelCentre;	// Centre point of the compound model in world space (max+min bounds / 2)
		
		// Default constructor
		TileCompoundModelSet(void) { ModelLayout = NULL; Size = NULL_INTVECTOR3; MinBounds = MaxBounds = CompoundModelSize = CompoundModelCentre = NULL_VECTOR; }

		// Allocate space based on the size parameter
		bool Allocate(void)
		{
			if (Size.x < 1 || Size.y < 1 || Size.z < 1) return false;

			// Allocate the x dimension first
			ModelLayout = new (std::nothrow) ModelLinkedList***[Size.x];
			if (!ModelLayout) return false;

			// Now allocate the y dimension within each x dimension
			for (int x=0; x<Size.x; x++)
			{
				ModelLayout[x] = new (std::nothrow) ModelLinkedList**[Size.y];
				if (!ModelLayout[x]) return false;

				// Finally allocate the z dimension within each y dimension
				for (int y=0; y<Size.y; y++)
				{
					// Allocate space
					ModelLayout[x][y] = new (std::nothrow) ModelLinkedList*[Size.z];
					if (!ModelLayout[x][y]) return false;

					// Also initialise all elements to NULL
					for (int z=0; z<Size.z; z++)
						ModelLayout[x][y][z] = NULL;
				}
			}
			
			// Return success
			return true;
		}	

		// Overriden method; apply a default model type if not specified.  Also apply default override parameter as below.
		void AddModel(int x, int y, int z, Model *model, Rotation90Degree rot)
		{
			AddModel(x, y, z, model, rot, TileModel::TileModelType::Unknown, false);
		}

		// Overridden method; do not override existing models in this position by default when adding to the spatial collection
		void AddModel(int x, int y, int z, Model *model, Rotation90Degree rot, TileModel::TileModelType type)
		{
			AddModel(x, y, z, model, rot, type, false);
		}
			
		// Add a model to the location specified.  Overwrite flag determines whether we replace or append
		void AddModel(int x, int y, int z, Model *model, Rotation90Degree rot, TileModel::TileModelType type, bool overwrite)
		{
			// If we currently have no entry in the spatial collection for this location, then simply add here
			if (ModelLayout[x][y][z] == NULL)
				ModelLayout[x][y][z] = new ModelLinkedList(model, rot, type);
			else
			{
				// If we want to overwrite then remove the existing model first
				if (overwrite) RemoveModels(x, y, z);
				
				// Add the new model to the spatial collection
				ModelLayout[x][y][z]->Add(model, rot, type);
			}

			// Add to the linear collection
			INTVECTOR3 location = INTVECTOR3(x, y, z);
			Models.push_back(TileModel(model, location, rot, type));

			// Test whether the compound model bounds have changed based on the addition of this one item
			RecalculateBounds();
		}

		// Gets the model at the specified location
		ModelLinkedList *GetModelAtLocation(int x, int y, int z)
		{
			if (x < Size.x && y < Size.y && z < Size.z)			return ModelLayout[x][y][z];
			else												return NULL;
		}	
		
		// Returns a flag determining whether storage has been allocated
		bool AllocationPerformed(void) { return (Size.x > 0 || Size.y > 0 || Size.z > 0); }

		// Resets storage to the point before allocation
		void ResetModelSet(void)
		{
			// If we have allocated space then shut down now
			if (AllocationPerformed()) Shutdown();

			// Recalculate compound model bounds
			RecalculateBounds();
		}

		// Copies the tile model set data from another instance
		void CopyFrom(const TileCompoundModelSet *src)
		{
			// Reset this object to remove any existing model data
			if (!src) return;
			ResetModelSet();

			// Copy the size parameter and then allocate sufficient space
			Size = src->Size;
			Allocate();

			// Add each item in turn
			const TileModel *m;
			int modelcount = (int)src->Models.size();
			for (int i=0; i<modelcount; ++i)
			{
				m = &(src->Models[i].value);
				AddModel(m->elementpos.x, m->elementpos.y, m->elementpos.z, m->model, m->rotation, m->type, false);
			}

			// Recalculate compound model bounds
			RecalculateBounds();
		}

		// Remove all the models at the specified index
		void RemoveModels(int x, int y, int z)
		{
			int n = (int)Models.size();
			for (int i = 0; i < n; ++i)
			{
				if (Models[i].value.elementpos.x == x && Models[i].value.elementpos.y == y && Models[i].value.elementpos.z == z)
				{
					RemoveFromVectorAtIndex<TileModel>(Models, i);	// Remove from the vector at this index
					--i;											// Decrement the loop counter so we resume at the same location, which now contains the next element
				}
			}

			// Also remove from the model layout collection
			ModelLayout[x][y][z]->RemoveAllItems();

			// Recalculate compound model bounds
			RecalculateBounds();
		}

		void Shutdown(void)
		{
			// Deallocate storage in the spatial layout
			if (ModelLayout)
				for (int x = 0; x < Size.x; x++) 
					for (int y = 0; y < Size.y; y++)
						for (int z = 0; z < Size.z; z++)
							if (ModelLayout[x][y][z] != NULL)
								ModelLayout[x][y][z]->RemoveAllItems();

			// Clear the linear model storage
			Models.clear();

			// Reset the size parameter; also indicates that the storage is not allocated
			Size = NULL_INTVECTOR3;

			// Recalculate compound model bounds
			RecalculateBounds();
		}

		void RecalculateBounds(void)
		{
			// Initialise the min and max bounds before starting
			MinBounds = LARGE_VECTOR_P;
			MaxBounds = LARGE_VECTOR_N;
			bool updated = false;

			// Iterate over each model in turn
			Model *model;
			int n = (int)Models.size();
			for (int i = 0; i < n; ++i)
			{
				// Check this model is valid
				model = Models[i].value.model;
				if (!model) continue;

				// Check whether the min or max bounds for this model would push out the overall bounds
				// Swap y and z coordinates since we are moving from element to world space
				// D3DXVECTOR3 pos = D3DXVECTOR3(((float)elsize.x * Game::C_CS_ELEMENT_MIDPOINT) + Game::ElementLocationToPhysicalPosition(location.x), _z_, _y_);
				XMVECTOR position = XMVectorAdd(Game::ElementLocationToPhysicalPosition(Models[i].value.elementpos), Game::C_CS_ELEMENT_MIDPOINT_V);

				// Update the model bounds if required
				MinBounds = XMVectorMin(MinBounds, XMVectorAdd(position, XMLoadFloat3(&model->GetModelMinBounds())));	// MinBounds are usually -ve.  Add rather than subtract
				MaxBounds = XMVectorMax(MaxBounds, XMVectorAdd(position, XMLoadFloat3(&model->GetModelMaxBounds())));
				updated = true;
			}
			
			// Safety check; if no updates could be made, assign a default min/max bounds
			if (!updated)
			{
				MinBounds = NULL_VECTOR; MaxBounds = XMVectorAdd(MinBounds, XMVectorReplicate(0.0f));
			}

			// Recalculate the model size and centre point based on these values
			CompoundModelSize = XMVectorSubtract(MaxBounds, MinBounds);
			CompoundModelCentre = XMVectorMultiply(XMVectorAdd(MinBounds, MaxBounds), HALF_VECTOR);
		}

		// Indicates whether this compound model was built using any pre-geometry-loading models.  If this is 
		// the case it will be post-processed after geometry is loaded in the load sequence
		CMPINLINE bool RequiresPostProcessing(void) const
		{
			auto it_end = Models.end();
			for (auto it = Models.begin(); it != it_end; ++it)
				if ((*it).value.model && (*it).value.model->IsGeometryLoaded() == false) return true;

			return false;
		}
	};
		
	// Abstract method to return the type of tile this is.  Must be implemented by all subclasses
	virtual D::TileClass				GetClass(void) const			= 0;

	// Abstract method to make a copy of the tile and return it.  Must be implemented by all subclasses
	virtual ComplexShipTile *			Copy(void) const				= 0;

	// Method to return the unique ID of this tile 
	CMPINLINE Game::ID_TYPE				GetID(void) const				{ return m_id; }

	// Retrieves or sets the simulation state for this tile
	CMPINLINE iObject::ObjectSimulationState		GetSimulationState(void) const								{ return m_simulationstate; }
	CMPINLINE void									SetSimulationState(iObject::ObjectSimulationState state)	{ m_simulationstate = state; }
	
	// Flag indicating whether this tile requires simulation time; while it does, the SimulateTile() method will be called every <interval>	
	CMPINLINE void						DeactivateSimulation(void)								{ m_requiressimulation = false; }
	CMPINLINE void						ActivateSimulation(void) 				
	{ 
		m_requiressimulation = true; 
		m_lastsimulation = Game::ClockMs;
	}
	CMPINLINE void						ActivateSimulation(unsigned int interval_ms)
	{
		m_requiressimulation = true;
		m_simulationinterval = interval_ms;
		m_lastsimulation = Game::ClockMs;
	}
	CMPINLINE void						SetTileSimulationRequired(bool simulation_required)		{ if (simulation_required) ActivateSimulation(); else DeactivateSimulation(); }

	// Simulation interval can be adjusted as required.  Time of last simulation is recorded every time the tile is simulated
	CMPINLINE bool						SimulationIsActive(void) const			{ return m_requiressimulation; }
	CMPINLINE unsigned int				LastSimulationTime(void) const			{ return m_lastsimulation; }
	CMPINLINE unsigned int				GetSimulationInterval(void) const		{ return m_simulationinterval; }
	CMPINLINE void						SetSimulationInterval(unsigned int ms)	{ m_simulationinterval = ms; }

	// Returns a value indicating whether this tile requires simulation; based both on the flag and also the time since last simulation
	CMPINLINE bool						RequiresSimulation(void) const			
	{ 
		return (m_requiressimulation && ((Game::ClockMs - m_lastsimulation) > m_simulationinterval));
	}
	
	// Main tile simulation method.  Passthrough to the subclass implementations, and simply maintains the update interval once it returns
	CMPINLINE void						SimulateTile(void)
	{
		// Perform tile simulation immediately
		PerformTileSimulation(Game::ClockMs - m_lastsimulation);

		// Update the time of last simulation
		m_lastsimulation = Game::ClockMs;
	}

	// Virtual method for tile subclasses to perform simulation.  Differs depending on the current simulation state of the tile
	virtual void						PerformTileSimulation(unsigned int delta_ms)			= 0;

	// Flag indicating whether the tile has been rendered this frame
	CMPINLINE bool						IsRendered(void) const { return m_rendered.IsSet(); }
	CMPINLINE void						MarkAsRendered(void) { m_rendered.Set(); }

	// Applies the effects of this tile on the underlying elements
	void								ApplyTile(void);

	// Subclass-implemented virtual method, called by base class method
	virtual void						ApplyTileSpecific(void) = 0;		

	// Abstract method to remove the contents of the tile to its parent objects.  Called upon removal.
//	void								UnapplyTile(void);					// Base class method
//	virtual void						UnapplyTileSpecific(void) = 0;		// Subclass-implemented virtual method, called by base class method

	// Methods to retrieve and set the tile location in element space
	CMPINLINE INTVECTOR3				GetElementLocation(void) const { return m_elementlocation; }
	CMPINLINE XMVECTOR					GetElementPosition(void) const { return m_elementposition; }
	CMPINLINE void						SetElementLocation(INTVECTOR3 loc) 
	{ 
		m_elementlocation = loc; 
		m_elementposition = Game::ElementLocationToPhysicalPosition(m_elementlocation);
		RecalculateTileData();
	}

	// Transforms a global element location to a tile-local location.  Assumes that the element does lie within this tile, 
	// or result is undefined 
	CMPINLINE INTVECTOR3				GetLocalElementLocation(const INTVECTOR3 & global_element_location) const
	{
		return (global_element_location - m_elementlocation);
	}

	// Returns the element index of a local element location in this tile, or -1 if the location is not valid
	CMPINLINE int						GetLocalElementIndex(const INTVECTOR3 & local_location) const
	{
		int id = ELEMENT_INDEX(local_location.x, local_location.y, local_location.z);
		return ((id >= 0 && id < (int)m_elementcount) ? id : -1);
	}

	// Methods to get and set the tile size in element space
	CMPINLINE INTVECTOR3				GetElementSize(void) const { return m_elementsize; }
	CMPINLINE INTVECTOR3 *				GetElementSizePointer(void) { return &m_elementsize; }
	void								SetElementSize(const INTVECTOR3 & size);
	CMPINLINE unsigned int				GetElementCount(void) const { return m_elementcount; }
	CMPINLINE XMVECTOR					GetWorldSize(void) const { return m_worldsize; }
	
	// Methods to get and set the string code of this tile
	CMPINLINE std::string				GetCode(void) const { return m_code; }
	CMPINLINE void						SetCode(std::string code) { m_code = code; }

	// Methods to get and set the string name of this tile
	CMPINLINE std::string				GetName(void) const { return m_code; }
	CMPINLINE void						SetName(std::string name) { m_name = name; }

	// Methods to set or retrieve the flag determining whether this is a standard tile, or just an instance within some parent entity
	CMPINLINE bool						IsStandardTile(void) const { return m_standardtile; }
	CMPINLINE void						SetStandardTile(bool standard) { m_standardtile = standard; }

	// Methods to access and set the geometry of this ship tile
	CMPINLINE Model *					GetModel(void) const { return m_model; }
	CMPINLINE void						SetModel(Model *m) { m_model = m; }

	// Updates the object before it is rendered.  Called only when the object is processed in the render queue (i.e. not when it is out of view)
	void								PerformRenderUpdate(void);

	// Methods to retrieve and set the definition associated with this tile
	const ComplexShipTileDefinition *	GetTileDefinition(void) const;
	void								SetTileDefinition(const ComplexShipTileDefinition *definition);

	// Methods to get and set the tile class type
	CMPINLINE D::TileClass				GetTileClass(void) const		{ return m_classtype; }
	CMPINLINE void						SetTileClass(D::TileClass cls)	{ m_classtype = cls; }

	// Methods to access compound model data
	CMPINLINE bool						HasCompoundModel(void) const		{ return m_multiplemodels; }
	CMPINLINE void						SetHasCompoundModel(bool compound)	{ m_multiplemodels = compound; }
	CMPINLINE TileCompoundModelSet *	GetCompoundModelSet(void)			{ return &m_models; }
	CMPINLINE const TileCompoundModelSet *	GetCompoundModelSet(void) const	{ return &m_models; }

	// Indicates whether this tile has compound model data requiring post-processing with full model geometry
	bool								RequiresCompoundModelPostProcessing(void) const;

	// Recalculate compound model data, including geometry-dependent calculations that are performed during the post-processing load sequence
	void								RecalculateCompoundModelData(void);

	// Gets or sets the rotation of this tile; contents are already rotated, this is mainly for geometry rendering & the SD
	CMPINLINE Rotation90Degree			GetRotation(void) const { return m_rotation; }
	void								SetRotation(Rotation90Degree rot);

	// Rotates the tile by the specified angle, adjusting all contents and geometry accordingly
	void								Rotate(Rotation90Degree rot);

	// Rotates all terrain objects associated with this tile by the specified angle
	void								RotateAllTerrainObjects(Rotation90Degree rotation);

	// Transform all view portals by the same rotation
	void								RotateAllViewPortals(Rotation90Degree rot_delta);

	// Mass of the tile
	CMPINLINE float						GetMass(void) const									{ return m_mass; }
	CMPINLINE void						SetMass(float m)									{ m_mass = m; }

	// 'Hardness' of the tile, used during collision & penetration tests.  Used to approximate e.g. force per cross-sectional area, 
	// or the density of external armour which can deflect a significant impact
	CMPINLINE float						GetHardness(void) const								{ return m_hardness; }
	CMPINLINE void						SetHardness(float h)								{ m_hardness = h; }

	// Returns the impact resistance of this tile, i.e. the remaining force it can withstand from physical 
	// impacts, with an impact point at the specified element
	float								GetImpactResistance(const ComplexShipElement & at_element) const;

	// Methods to get and recalculate the aggregate health value of this tile
	CMPINLINE float						GetAggregateHealth(void)							{ return m_aggregatehealth; }
	void								ElementHealthChanged(void);	
	void								RecalculateAggregateHealth(void);
	
	// Methods to begin and end construction on this tile, and also to test whether production is in progress right now
	void								StartConstruction(void);
	void								ConstructionComplete(void);
	bool								ConstructionIsInProgress(void);

	// Methods to add and remove progress towards the construction of this tile
	float								AddConstructionProgress(INTVECTOR3 element, const Resource *resource, float amountavailable, float timestep);
	float								RemoveConstructionProgress(INTVECTOR3 element, const Resource *resource, float amount, float timestep);

	// Deallocates any existing per-element construction progress for this tile
	void								DeallocatePerElementConstructionProgress(void);

	// Methods to get and set elements of this tile's constructed state
	CMPINLINE ProductionCost *			GetConstructedState(void)							{ return m_constructedstate; }
	CMPINLINE ProductionCost *			GetConstructedStateConst(void) const				{ return m_constructedstate; }
	ProductionCost *					GetElementConstructedState(INTVECTOR3 elpos);
	ProductionCost *					GetElementConstructedState(int x, int y, int z);
	
	// Sets the production cost & state of this tile; called upon tile creation
	void								InitialiseConstructionState(ProductionCost *state);

	// Methods to retrieve components of the element location, for efficiency
	CMPINLINE int						GetElementLocationX(void) const			{ return m_elementlocation.x; }
	CMPINLINE int						GetElementLocationY(void) const			{ return m_elementlocation.y; }
	CMPINLINE int						GetElementLocationZ(void) const			{ return m_elementlocation.z; }

	// Methods to set components of the element location directly, for efficiency
	CMPINLINE int						SetElementLocationX(int x)	{ m_elementlocation.x = x; RecalculateTileData(); }
	CMPINLINE int						SetElementLocationY(int y)	{ m_elementlocation.y = y; RecalculateTileData(); }
	CMPINLINE int						SetElementLocationZ(int z)	{ m_elementlocation.z = z; RecalculateTileData(); }
	
	// Methods to retrieve/set components of the element size, for efficiency
	CMPINLINE int						GetElementSizeX(void) const		{ return m_elementsize.x; }
	CMPINLINE int						GetElementSizeY(void) const		{ return m_elementsize.y; }
	CMPINLINE int						GetElementSizeZ(void) const		{ return m_elementsize.z; }
	CMPINLINE int						SetElementSizeX(int x)			{ m_elementsize.x = x; RecalculateTileData(); }
	CMPINLINE int						SetElementSizeY(int y)			{ m_elementsize.y = y; RecalculateTileData(); }
	CMPINLINE int						SetElementSizeZ(int z)			{ m_elementsize.z = z; RecalculateTileData(); }

	// Return the (local) centre point of the tile in world coordinates (e.g. for a 1x1x1 element tile, centre = [5.0,5.0,5.0]
	CMPINLINE XMVECTOR					GetCentrePoint(void) const		{ return m_centre_point; }

	// Recalculates the state of the tile following a change to its position/size etc.  Includes recalc of the world matrix and bounding volume
	void								RecalculateTileData(void);

	
	// Recalculates the bounding volume for this tile based on the element size in world space
	void								RecalculateBoundingVolume(void);
	CMPINLINE BoundingObject *			GetBoundingObject(void) const				{ return m_boundingbox; }

	// Get the approximate radius of a bounding sphere that encompasses this tile
	CMPINLINE float						GetBoundingSphereRadius(void) const			{ return m_bounding_radius; }

	// Retrieve or recalculate the position and transform matrix for this tile relative to its parent ship object
	CMPINLINE const XMVECTOR			GetRelativePosition(void) const				{ return m_relativeposition; }
	CMPINLINE const XMMATRIX			GetRelativePositionMatrix(void) const		{ return m_relativepositionmatrix; }
	CMPINLINE const XMMATRIX 			GetWorldMatrix(void) const					{ return m_worldmatrix; }
	void								RecalculateWorldMatrix(void);

	// Return pointer to the parent object that owns this tile
	CMPINLINE iSpaceObjectEnvironment *			GetParent(void) const				{ return m_parent; }

	// Override method; sets the parent pointer manually.  Performs no other recalculation.  Only to be used by internal
	// methods that can perform the calculation directly.  Use iSpaceObjectEnvironment::AddTile() to correctly add to a parent environment
	CMPINLINE void								OverrideParentEnvironmentReference(iSpaceObjectEnvironment *env) { m_parent = env; }

	// The set of connections that are present from this tile
	TileConnections								Connections;

	// The set of connections that are possible from this tile
	TileConnections								PossibleConnections;

	// Portals owned by this tile
	CMPINLINE std::vector<ViewPortal> &				GetPortals(void)				{ return m_portals; }
	CMPINLINE std::vector<ViewPortal>::size_type	GetPortalCount(void) const		{ return m_portalcount; }
	void											RecalculatePortalData(void);
	void											AddPortal(const ViewPortal & portal);
	void											AddPortal(ViewPortal && portal);

	// Events generated when the tile is added/removed from an environment
	void										BeforeAddedToEnvironment(iSpaceObjectEnvironment *environment);
	void										AfterAddedToEnvironment(iSpaceObjectEnvironment *environment);
	void										BeforeRemovedToEnvironment(iSpaceObjectEnvironment *environment);
	void										AfterRemovedFromEnvironment(iSpaceObjectEnvironment *environment);

	// Compiles the tile based on its definition
	Result								CompileTile(void);	

	// Validates the tile against its hard-stop requirements and returns the result
	Result								ValidateHardStopRequirements(void);

	// Compiles and validates the tile based on its definition, class & associated hard-stop criteria
	Result								CompileAndValidateTile(void);

	// Generates the geometry for this tile.  Subset of the "CompileTile()" functionality which can
	// be called separately if required
	Result								GenerateGeometry(void);

	// Static method to look up a tile definition and create a new tile based upon it
	static ComplexShipTile *			Create(std::string code);

	// Static method to create a new instance of the specified class of tile.  Creates the object only, no initialisation
	static ComplexShipTile *			New(D::TileClass cls);

	// Static base class methods to generate and read XML data for the base class portion of any tile
	static TiXmlElement *				GenerateBaseClassXML(ComplexShipTile *tile);
	static void							ReadBaseClassXML(TiXmlElement *node, ComplexShipTile *tile);

	// Static base class method to copy data for the base class portion of any tile
	static void							CopyBaseClassData(ComplexShipTile *source, ComplexShipTile *target);

	// Virtual method to read class-specific XML data for the tile
	virtual void						ReadClassSpecificXMLData(TiXmlElement *node)		= 0;

	// Virtual method for implementation by subclasses, that will generate the XML necessary to represent the tile in full
	virtual TiXmlElement *				GenerateXML(void) = 0;

	// Returns a value indicating whether or not this is a primary tile.  Based on the underlying tile class
	bool								IsPrimaryTile(void);

	// Tiles maintain a vector of unique terrain IDs, corresponding to the terrain objects which are 'owned' by the tile
	CMPINLINE std::vector<Game::ID_TYPE> &		GetTerrainObjectLinks(void)						{ return m_terrain_ids; }
	void										AddTerrainObjectLink(Game::ID_TYPE ID);
	void										ClearTerrainObjectLinks(void);

	// Returns a value indicating whether this tile spans multiple elements
	CMPINLINE bool						SpansMultipleElements(void) const						{ return m_multielement; }

	// Flag which indicates whether the connections from this tile have been 'fixed' (true) or are allowed to update based on surroundings
	CMPINLINE bool						ConnectionsAreFixed(void) const							{ return m_connections_fixed; }
	CMPINLINE void						FixConnections(bool fix_connections)					{ m_connections_fixed = fix_connections; }

	// Static method to determine whether a given tileclass is 'infrastructural'
	static bool							IsInfrastructureTile(D::TileClass tileclass);

	// The current power level being supplied to this tile
	CMPINLINE Power::Type				GetPowerLevel(void) const								{ return m_powerlevel; }
	void								SetPowerLevel(Power::Type power);

	// Returns the % power satisfaction for this tile, in the range [0.0 1.0].  A tile with no power
	// requirement will always return a power satisfaction of 1.0
	CMPINLINE float						GetPowerSatisfaction(void) const 
	{
		if (m_powerlevel >= m_powerrequirement) return 1.0f;			// PowerLevel will always be >= 0, so this will always return if we have no power req
		return ((float)m_powerlevel / (float)m_powerrequirement);		// Meaning that we don't need to guard against power_req == 0 and Div/0 here
	}

	// Power requirement in order for the tile to be functional
	CMPINLINE Power::Type				GetPowerRequirement(void) const							{ return m_powerrequirement; }
	CMPINLINE void						SetPowerRequirement(Power::Type power)					{ m_powerrequirement = power; }

	// Indicates whether the tile is currently powered
	CMPINLINE bool						IsPowered(void) const									{ return (m_powerlevel >= m_powerrequirement); }

	// Initialise and clear connection data for the tile
	void								InitialiseConnectionState();

	// Default property set applied to all elements of the tile; element-specific changes are then made afterwards
	bitstring							DefaultProperties;

	// Effects that can be activated on this object
	FadeEffect							Fade;					// Allows the object to be faded in and out
	HighlightEffect						Highlight;				

	// Reference to the hardpoints owned by this tile
	CMPINLINE void									AddHardpointReference(const std::string & hp_ref) { m_hardpoint_refs.push_back(hp_ref); }
	CMPINLINE void									ClearAllHardpointReferences(void) { m_hardpoint_refs.clear(); }
	CMPINLINE const std::vector<std::string> &		GetHardpointReferences(void) { return m_hardpoint_refs; }

	// Determines the code that should be assigned to a hardpoint owned by this tile
	std::string							DetermineTileHardpointCode(Hardpoint *hardpoint);

	// Return a debug string representation of the tile
	CMPINLINE std::string				DebugString(void)  const		{ return concat("Tile (ID=")(m_id)(", Type=")(m_code)(")").str(); }

	// Event triggered upon destruction of the entity
	void								DestroyObject(void);

	// Destroy all terrain objects owned by this tile
	void								DestroyAllOwnedTerrain(void);

	// Shutdown method - not required for this class
	CMPINLINE void						Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	// Default constructor/copy constructor/destructor
	ComplexShipTile(void);
	ComplexShipTile(const ComplexShipTile &C);
	~ComplexShipTile(void);


protected:

	// Unique ID for this tile
	Game::ID_TYPE				m_id;

	// String code and name of the tile
	std::string					m_code;
	std::string					m_name;

	// Pointer back to the tile definition
	const ComplexShipTileDefinition * m_definition;

	// Tile class (for efficiency; this can also be retrieved from the class object by Tile>TileDefinition>TileClass
	D::TileClass				m_classtype;

	// Location and size in element space
	INTVECTOR3					m_elementlocation;
	AXMVECTOR					m_elementposition;
	INTVECTOR3					m_elementsize;
	unsigned int				m_elementcount;
	AXMVECTOR					m_worldsize;
	bool						m_multielement;

	// Position and transform matrix relative to the parent complex ship object, plus the child world matrix
	AXMVECTOR					m_relativeposition;
	AXMMATRIX					m_relativepositionmatrix;
	AXMMATRIX					m_worldmatrix;

	// Pointers to the various parents of this tile
	iSpaceObjectEnvironment *	m_parent;				// The environment that contains this tile

	// The geometry associated with this ship tile
	bool						m_multiplemodels;
	Model *						m_model;
	TileCompoundModelSet		m_models;

	// Flag indicating whether the tile has been rendered this frame
	FrameFlag					m_rendered;

	// Bounding box encompassing this tile; used for more efficient visibility & collision testing
	BoundingObject *			m_boundingbox;

	// Approximate radius of a bounding sphere that encompasses this tile
	float						m_bounding_radius;

	// Centre point of the tile
	AXMVECTOR					m_centre_point;

	// Flag determining whether this is a standard tile, or just an instance within some parent entity
	bool						m_standardtile;

	// Rotation of this tile; contents are already rotated, this is mainly for geometry rendering & the SD
	Rotation90Degree			m_rotation;

	// Mass of the tile
	float						m_mass;

	// 'Hardness' of the tile, used during collision & penetration tests.  Used to approximate e.g. force per cross-sectional area, 
	// or the density of external armour which can deflect a significant impact
	float						m_hardness;

	// Flag which indicates whether the connections from this tile have been 'fixed' (true) or are allowed to update based on surroundings
	bool						m_connections_fixed;
	
	// Vector of unique terrain IDs, corresponding to the terrain objects within our parent environment that are 'owned' by this
	// tile.  We maintain this link so that terrain objects can be efficiently removed with the tile if required
	std::vector<Game::ID_TYPE>	m_terrain_ids;

	// Vector of hardpoint references, corresponding to the hardpoints in our environment that this tile owns
	std::vector<std::string>	m_hardpoint_refs;

	// Collection of portals from this tile
	std::vector<ViewPortal>				m_portals;
	std::vector<ViewPortal>::size_type	m_portalcount;

	// Power requirement for the tile to be functional
	Power::Type					m_powerrequirement;

	// Current power level being supplied to the tile
	Power::Type					m_powerlevel;

	// Simulation state of this tile.  Light implementation for tiles, since the ship & elements handle most of the logic.  This state 
	// just determines the extent of activity within SimulateTile()
	iObject::ObjectSimulationState	m_simulationstate;

	// Flag that determines whether this tile needs simulation; if set, the SimulateTile() method will be called each cycle 
	bool						m_requiressimulation;

	// We simulate the tile on defined (ms) intervals.  interval==0 means it will be simulated every frame
	unsigned int				m_simulationinterval;
	unsigned int				m_lastsimulation;

	// Damage modifiers, incorporated when damage is applied to underlying tiles
	DamageSet					m_damagemodifiers;

	// Aggregate health of the tile, calculated from underlying elements
	float						m_aggregatehealth;

	// Aggregate and detailed data on the constructed status of the tile
	ProductionCost *			m_constructedstate;
	ProductionCost****			m_elementconstructedstate;					// 3D array of constructed state values
	INTVECTOR3					m_constructedstate_previousallocatedsize;	// The last allocation that was made, for purposes of re/de-allocating

	// Applies the effects of this tile to a specific underlying element
	void						ApplyTileToElement(ComplexShipElement *el);

	// Processes a debug tile command from the console.  Protected since this is always called from a subclass
	void								ProcessDebugTileCommand(GameConsoleCommand & command);

public:
	// Debug variables to log instance creation
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		static long inst_con;
		static long inst_des;
	#endif

};


#endif