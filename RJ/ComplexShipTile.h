#pragma once

#ifndef __ComplexShipTileH__
#define __ComplexShipTileH__

#include <vector>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "Utility.h"
#include "FastMath.h"
#include "GameDataExtern.h"
#include "Model.h"
#include "ComplexShipElement.h"
#include "ElementConnection.h"
#include "Damage.h"
#include "FadeEffect.h"
#include "HighlightEffect.h"
class TiXmlElement;
class ComplexShip;
class ComplexShipSection;
class BoundingObject;
class Resource;
class ProductionCost;
using namespace std;

#define DEBUG_LOGINSTANCECREATION


class ComplexShipTile 
{
public:

	// Static record of the highest ID value in existence, for assigning to new tiles upon registration
	static Game::ID_TYPE					InstanceCreationCount;

	// Method to generate a new unique ID, called for each new tile being instantiated
	static Game::ID_TYPE					GenerateNewUniqueID(void)	{ return (++InstanceCreationCount); }

	// Struct holding information on the individual models making up a tile
	struct TileModel 
	{ 
		enum TileModelType 
		{
			Unknown = 0,
			WallStraight,
			WallCorner,
			WallConnection
		};

		Model * model; D3DXVECTOR3 offset; INTVECTOR3 elementpos; Rotation90Degree rotation; TileModelType type; D3DXMATRIX rotmatrix;
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
				rotmatrix = ID_MATRIX;
			else
			{
				// Calculate temporary translation matrices to translate the model to its centre, so we can then rotate, and then translate back
				D3DXMATRIX off, invoff;
				D3DXVECTOR3 halfsize = Game::ElementLocationToPhysicalPosition(model->GetElementSize()) * 0.5f;
				D3DXMatrixTranslation(&off, -halfsize.x, -halfsize.y, -halfsize.z);
				D3DXMatrixTranslation(&invoff, halfsize.x, halfsize.y, halfsize.z);

				// Derive and store the rotation matrix for this model
				D3DXMatrixMultiply(&rotmatrix, &off, GetRotationMatrix(rotation));
				D3DXMatrixMultiply(&rotmatrix, &rotmatrix, &invoff);
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
	struct TileCompoundModelSet
	{
		typedef					vector<TileModel>	TileModelCollection;

		TileModelCollection		Models;					// Linear collection of all models, for rendering efficiency
		ModelLinkedList	****	ModelLayout;			// Spatial layout of models, for efficient indexing into the collection.  ModelLinkedList*[x][y][z]	
		INTVECTOR3				Size;					// Size of the compound model, in elements
		D3DXVECTOR3				MinBounds, MaxBounds;	// Minimum and maximum bounds of the overall compound model, in world space
		D3DXVECTOR3				CompoundModelSize;		// Actual size of the compound model in world space (max-min bounds)
		D3DXVECTOR3				CompoundModelCentre;	// Centre point of the compound model in world space (max+min bounds / 2)
		
		// Default constructor
		TileCompoundModelSet(void) { ModelLayout = NULL; Size = NULL_INTVECTOR3; MinBounds = MaxBounds = CompoundModelSize = CompoundModelCentre = NULL_VECTOR; }

		// Allocate space based on the size parameter
		bool Allocate(void)
		{
			if (Size.x < 1 || Size.y < 1 || Size.z < 1) return false;

			// Allocate the x dimension first
			ModelLayout = new (nothrow) ModelLinkedList***[Size.x];
			if (!ModelLayout) return false;

			// Now allocate the y dimension within each x dimension
			for (int x=0; x<Size.x; x++)
			{
				ModelLayout[x] = new (nothrow) ModelLinkedList**[Size.y];
				if (!ModelLayout[x]) return false;

				// Finally allocate the z dimension within each y dimension
				for (int y=0; y<Size.y; y++)
				{
					// Allocate space
					ModelLayout[x][y] = new (nothrow) ModelLinkedList*[Size.z];
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
			RecalculateBounds(model, location);
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
				m = &(src->Models[i]);
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
				if (Models[i].elementpos.x == x && Models[i].elementpos.y == y && Models[i].elementpos.z == z)
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

		void RecalculateBounds(Model *model, const INTVECTOR3 & elementlocation)
		{
			// Parameter check
			if (!model) return;

			// Check whether the min or max bounds for this model would push out the overall bounds
			// Swap y and z coordinates since we are moving from element to world space
			const INTVECTOR3 & elsize = model->GetElementSize();
			D3DXVECTOR3 pos = D3DXVECTOR3(	((float)elsize.x * Game::C_CS_ELEMENT_MIDPOINT) + Game::ElementLocationToPhysicalPosition(elementlocation.x),
											((float)elsize.z * Game::C_CS_ELEMENT_MIDPOINT) + Game::ElementLocationToPhysicalPosition(elementlocation.z),
											((float)elsize.y * Game::C_CS_ELEMENT_MIDPOINT) + Game::ElementLocationToPhysicalPosition(elementlocation.y));

			// Update the model bounds if required
			VectorMin(MinBounds, pos - model->GetModelMinBounds());
			VectorMax(MaxBounds, pos + model->GetModelMaxBounds());

			// Recalculate the model size and centre point based on these values
			CompoundModelSize = (MaxBounds - MinBounds);
			CompoundModelCentre = ((MinBounds + MaxBounds) * 0.5f);
		}

		void RecalculateBounds(void)
		{
			// Initialise the min and max bounds before starting
			MinBounds = D3DXVECTOR3(9999999.0f, 9999999.0f, 9999999.0f); 
			MaxBounds = D3DXVECTOR3(-9999999.0f, -9999999.0f, -9999999.0f);
			bool updated = false;

			// Iterate over each model in turn
			Model *model;
			int n = (int)Models.size();
			for (int i = 0; i < n; ++i)
			{
				// Check this model is valid
				model = Models[i].model;
				if (!model) continue;

				// Check whether the min or max bounds for this model would push out the overall bounds
				// Swap y and z coordinates since we are moving from element to world space
				const INTVECTOR3 & elsize = model->GetElementSize();
				const INTVECTOR3 & location = Models[i].elementpos;
				D3DXVECTOR3 pos = D3DXVECTOR3(	((float)elsize.x * Game::C_CS_ELEMENT_MIDPOINT) + Game::ElementLocationToPhysicalPosition(location.x),
												((float)elsize.z * Game::C_CS_ELEMENT_MIDPOINT) + Game::ElementLocationToPhysicalPosition(location.z),
												((float)elsize.y * Game::C_CS_ELEMENT_MIDPOINT) + Game::ElementLocationToPhysicalPosition(location.y));

				// Update the model bounds if required
				VectorMin(MinBounds, pos - model->GetModelMinBounds());
				VectorMax(MaxBounds, pos + model->GetModelMaxBounds());
				updated = true;
			}
			
			// Safety check; if no updates could be made, assign a default min/max bounds
			if (!updated)
			{
				MinBounds = NULL_VECTOR; MaxBounds = MinBounds + D3DXVECTOR3(0.01f, 0.01f, 0.01f);
			}

			// Recalculate the model size and centre point based on these values
			CompoundModelSize = (MaxBounds - MinBounds);
			CompoundModelCentre = ((MinBounds + MaxBounds) * 0.5f);
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
	void											UpdateSimulationStateFromParentElements(void);

	// Flag indicating whether this tile requires simulation time; while it does, the SimulateTile() method will be called every <interval>	
	CMPINLINE void						DeactivateSimulation(void)				{ m_requiressimulation = false; }
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

	// Method to link this tile to a parent ship/sections/elements.  Used primarily on ship creation
	void								LinkToParent(ComplexShip *ship);

	// Events that are generated pre- and post-link to parent objects.  Exposed for use by subclasses as required
	virtual void						BeforeLinkToParent(ComplexShip *ship) = 0;
	virtual void						AfterLinkToParent(ComplexShip *ship) = 0;

	// Unlink this tile from its parent objects, applying any removal logic as part of the same process
	void								UnlinkFromParent(void);

	// Events that are generated pre- and post-unlink from parent objects.  Exposed for use by subclasses as required
	virtual void						BeforeUnlinkFromParent(void) = 0;
	virtual void						AfterUnlinkFromParent(ComplexShip *oldship) = 0;

	// Abstract method to apply the contents of the tile to its parent objects.  Called upon linking, plus on repair of the ship
	void								ApplyTile(void);					// Base class method; calls the element-specific method one element at a time
	void								ApplyTile(ComplexShipElement *el);	// Base class method
	
	// Subclass-implemented virtual method, called by base class method
	virtual void						ApplyTileSpecific(ComplexShipElement *el) = 0;		

	// Abstract method to remove the contents of the tile to its parent objects.  Called upon removal.
//	void								UnapplyTile(void);					// Base class method
//	virtual void						UnapplyTileSpecific(void) = 0;		// Subclass-implemented virtual method, called by base class method

	// Methods to retrieve and set the tile location in element space
	CMPINLINE INTVECTOR3				GetElementLocation(void) const { return m_elementlocation; }
	CMPINLINE void						SetElementLocation(INTVECTOR3 loc) 
	{ 
		m_elementlocation = loc; 
		RecalculateTileData();
	}

	// Methods to get and set the tile size in element space
	CMPINLINE INTVECTOR3				GetElementSize(void) const { return m_elementsize; }
	CMPINLINE INTVECTOR3 *				GetElementSizePointer(void) { return &m_elementsize; }
	void								SetElementSize(const INTVECTOR3 & size);
	
	// Methods to get and set the string code of this tile
	CMPINLINE string					GetCode(void) const { return m_code; }
	CMPINLINE void						SetCode(string code) { m_code = code; }

	// Methods to get and set the string name of this tile
	CMPINLINE string					GetName(void) const { return m_code; }
	CMPINLINE void						SetName(string name) { m_name = name; }

	// Methods to set or retrieve the flag determining whether this is a standard tile, or just an instance within some parent entity
	CMPINLINE bool						IsStandardTile(void) const { return m_standardtile; }
	CMPINLINE void						SetStandardTile(bool standard) { m_standardtile = standard; }

	// Methods to access and set the geometry of this ship tile
	CMPINLINE Model *					GetModel(void) const { return m_model; }
	CMPINLINE void						SetModel(Model *m) { m_model = m; }

	// Updates the object before it is rendered.  Called only when the object is processed in the render queue (i.e. not when it is out of view)
	void								PerformRenderUpdate(void);

	// Methods to retrieve and set the definition associated with this tile
	ComplexShipTileDefinition *			GetTileDefinition(void) const;
	void								SetTileDefinition(ComplexShipTileDefinition *definition);

	// Methods to get and set the tile class type
	CMPINLINE D::TileClass				GetTileClass(void) const		{ return m_classtype; }
	CMPINLINE void						SetTileClass(D::TileClass cls)	{ m_classtype = cls; }

	// Methods to access compound model data
	CMPINLINE bool						HasCompoundModel(void) const		{ return m_multiplemodels; }
	CMPINLINE void						SetHasCompoundModel(bool compound)	{ m_multiplemodels = compound; }
	CMPINLINE TileCompoundModelSet *	GetCompoundModelSet(void)			{ return &m_models; }
	CMPINLINE const TileCompoundModelSet *	GetCompoundModelSet(void) const	{ return &m_models; }

	// Gets or sets the rotation of this tile; contents are already rotated, this is mainly for geometry rendering & the SD
	CMPINLINE Rotation90Degree			GetRotation(void) const { return m_rotation; }
	CMPINLINE void						SetRotation(Rotation90Degree rot) 
	{ 
		m_rotation = rot; 
		RecalculateTileData();
	}

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

	// Recalculates the state of the tile following a change to its position/size etc.  Includes recalc of the world matrix and bounding volume
	void								RecalculateTileData(void);

	
	// Recalculates the bounding volume for this tile based on the element size in world space
	void								RecalculateBoundingVolume(void);
	CMPINLINE BoundingObject *			GetBoundingObject(void) const				{ return m_boundingbox; }

	// Retrieve or recalculate the position and transform matrix for this tile relative to its parent ship object
	CMPINLINE D3DXVECTOR3				GetRelativePosition(void)					{ return m_relativeposition; }
	CMPINLINE const D3DXMATRIX *		GetWorldMatrix(void)						{ return &m_worldmatrix; }
	void								RecalculateWorldMatrix(void);

	// Return pointers to the parent objects associated with this tile
	CMPINLINE ComplexShip *					GetParentShip(void) const						{ return m_parentship; }
	CMPINLINE ComplexShipSection *			GetParentShipSection(void) const				{ return m_parentsection; }
	CMPINLINE ComplexShipElement *			GetParentShipElement(void) const				{ return m_parentelement; }

	// Methods to query and set connection points for elements within the tile
	CMPINLINE ElementConnectionSet *			GetConnections(void)										{ return &m_connections; }					
	CMPINLINE ElementConnection					GetConnection(ElementConnectionSet::size_type index) const	{ return m_connections[index]; }
	int											GetConnection(INTVECTOR3 loc) const;
	int											GetConnection(INTVECTOR3 loc, Direction dir) const;
	CMPINLINE ElementConnectionSet::size_type	GetConnectionCount(void) const						{ return m_connections.size(); }
	CMPINLINE bool								HasConnection(INTVECTOR3 loc, Direction dir) const	{ return (GetConnection(loc, dir) > -1); }
	void										AddConnection(INTVECTOR3 loc, Direction dir);
	void										RemoveConnection(ElementConnectionSet::size_type index);
	void										RemoveConnection(INTVECTOR3 loc, Direction dir);
	CMPINLINE ElementConnectionSet::iterator	GetConnectionIteratorStart(void)			{ return m_connections.begin(); }
	CMPINLINE ElementConnectionSet::iterator	GetConnectionIteratorEnd(void)				{ return m_connections.end(); }
	void										SetConnections(const ElementConnectionSet &source);

	// Compiles the tile based on its definition
	Result								CompileTile(void);	

	// Validates the tile against its hard-stop requirements and returns the result
	Result								ValidateHardStopRequirements(void);

	// Compiles and validates the tile based on its definition, class & associated hard-stop criteria
	Result								CompileAndValidateTile(void);

	// Static method to create a new tile.  Looks up the definition specified and generates a new 

	// Static method to look up a tile definition and create a new tile based upon it
	static ComplexShipTile *			Create(string code);

	// Static method to create a new instance of the specified class of tile.  Creates the object only, no initialisation
	static ComplexShipTile *			New(D::TileClass cls);

	// Static base class methods to generate and read XML data for the base class portion of any tile
	static TiXmlElement *				GenerateBaseClassXML(ComplexShipTile *tile);
	static void							ReadBaseClassXML(TiXmlElement *node, ComplexShipTile *tile);

	// Virtual method to read class-specific XML data for the tile
	virtual void						ReadClassSpecificXMLData(TiXmlElement *node)		= 0;

	// Virtual method for implementation by subclasses, that will generate the XML necessary to represent the tile in full
	virtual TiXmlElement *				GenerateXML(void) = 0;

	// Returns a value indicating whether or not this is a primary tile.  Based on the underlying tile class
	bool								IsPrimaryTile(void);

	// Tiles maintain a vector of unique terrain IDs, corresponding to the terrain objects which are 'owned' by the tile
	CMPINLINE std::vector<Game::ID_TYPE> &		GetTerrainObjectLinks(void)						{ return m_terrain_ids; }
	CMPINLINE void								AddTerrainObjectLink(Game::ID_TYPE ID)			{ m_terrain_ids.push_back(ID); }
	void										ClearTerrainObjectLinks(void);

	// Returns a value indicating whether this tile spans multiple elements
	CMPINLINE bool						SpansMultipleElements(void) const				{ return m_multielement; }

	// Static method to determine whether a given tileclass is 'infrastructural'
	static bool							IsInfrastructureTile(D::TileClass tileclass);

	// Default property set applied to all elements of the tile; element-specific changes are then made afterwards
	vector<ComplexShipElement::PropertyValue>		DefaultProperties;

	// Effects that can be activated on this object
	FadeEffect							Fade;					// Allows the object to be faded in and out
	HighlightEffect						Highlight;				

	// Default constructor/copy constructor/destructor
	ComplexShipTile(void);
	ComplexShipTile(const ComplexShipTile &C);
	~ComplexShipTile(void);


protected:

	// Unique ID for this tile
	Game::ID_TYPE				m_id;

	// String code and name of the tile
	string						m_code;
	string						m_name;

	// Pointer back to the tile definition
	ComplexShipTileDefinition * m_definition;

	// Tile class (for efficiency; this can also be retrieved from the class object by Tile>TileDefinition>TileClass
	D::TileClass				m_classtype;

	// Location and size in element space
	INTVECTOR3					m_elementlocation;
	INTVECTOR3					m_elementsize;
	bool						m_multielement;

	// Position and transform matrix relative to the parent complex ship object, plus the child world matrix
	D3DXVECTOR3					m_relativeposition;
	D3DXMATRIX					m_worldmatrix;

	// Pointers to the various parents of this tile
	ComplexShip *				m_parentship;			// The ship that contains this tile
	ComplexShipSection *		m_parentsection;		// The section of the ship that contains this tile
	ComplexShipElement *		m_parentelement;		// The ship element at the top-top-left of this tile

	// The geometry associated with this ship tile
	bool						m_multiplemodels;
	Model *						m_model;
	TileCompoundModelSet		m_models;

	// Bounding box encompassing this tile; used for more efficient visibility & collision testing
	BoundingObject *			m_boundingbox;

	// Flag determining whether this is a standard tile, or just an instance within some parent entity
	bool						m_standardtile;

	// Rotation of this tile; contents are already rotated, this is mainly for geometry rendering & the SD
	Rotation90Degree			m_rotation;
	
	// Vector of unique terrain IDs, corresponding to the terrain objects within our parent environment that are 'owned' by this
	// tile.  We maintain this link so that terrain objects can be efficiently removed with the tile if required
	std::vector<Game::ID_TYPE>	m_terrain_ids;

	// The set of connections that are possible from this tile
	ElementConnectionSet		m_connections;

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

public:
	// Debug variables to log instance creation
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		static long inst_con;
		static long inst_des;
	#endif

};


#endif