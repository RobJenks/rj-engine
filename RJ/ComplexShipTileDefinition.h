#pragma once

#ifndef __ComplexShipTileDefinitionH__
#define __ComplexShipTileDefinitionH__

#include <vector>
#include <unordered_map>
#include "ErrorCodes.h"
#include "GameDataExtern.h"
#include "Model.h"
#include "ComplexShipElement.h"
#include "ProductionCost.h"
#include "TileConnections.h"
#include "ElementStateDefinition.h"
#include "Power.h"
#include "Hardpoints.h"
#include "ViewPortal.h"
class TiXmlElement;
class ComplexShipTile;
class ComplexShipTileClass;
class BoundingObject;


// This class does not have any special alignment requirements
class ComplexShipTileDefinition
{

protected:
	// Structs holding data on the probability-weighted models to be applied to a compound tile
	struct ProbabilityWeightedModel 
	{ 
		Model *model; float probability;
		ProbabilityWeightedModel(void) { model = NULL; probability = 0.0f; }
		ProbabilityWeightedModel(Model *_model, float _probability) { model = _model; probability = _probability; }
	};

	// Struct maintaining a collection of probability-weighted models
	struct ProbabilityWeightedModelCollection
	{
		std::vector<ProbabilityWeightedModel> models;
		float totalprob;

		ProbabilityWeightedModelCollection(void) { totalprob = 0.0f; }
		void AddItem(Model *model, float prob) 
		{
			models.push_back(ProbabilityWeightedModel(model, prob));
			totalprob += prob;
		}
	};

	// Struct used to store adjacency info for tile corner elements
	// The struct holds { xpos = [0|1], ypos = [0|1], rotation of a corner tile at this pos, rotation of one edge tile, rotation of the second edge tile }
	struct CornerAdjacencyData
	{
		int x, y; Rotation90Degree CornerRotation, Edge1Rotation, Edge2Rotation;
		CornerAdjacencyData(void) { x = 0; y = 0; CornerRotation = Edge1Rotation = Edge2Rotation = Rotation90Degree::Rotate0; }
		CornerAdjacencyData(int _x, int _y, Rotation90Degree _CornerRotation, Rotation90Degree _Edge1Rotation, Rotation90Degree _Edge2Rotation)
							{ x = _x; y = _y; CornerRotation = _CornerRotation; Edge1Rotation = _Edge1Rotation; Edge2Rotation = _Edge2Rotation; }
	};
	static const CornerAdjacencyData CornerData[4];

public:

	// Methods to add and retrieve probability-weighted models for contructing tiles
	typedef std::unordered_map<std::string, ProbabilityWeightedModelCollection> TileModelSet;
	void AddModelToSet(std::string category, Model *model, float probability);
	Model *GetModelFromSet(const ProbabilityWeightedModelCollection *models) const;
	
	// Static method to create definition objects of the desired subclass type
	static ComplexShipTileDefinition *	Create(D::TileClass cls);

	// Methods to get and set the class of this tile definition
	bool								SetClass(const std::string & cls);
	bool								SetClass(ComplexShipTileClass *cls);
	CMPINLINE D::TileClass				GetClass(void) const { return m_classtype; }
	CMPINLINE ComplexShipTileClass *	GetClassObject(void) const { return m_class; }

	// Methods to get and set key properties
	CMPINLINE std::string	GetCode(void) const										{ return m_code; }
	CMPINLINE void			SetCode(const std::string & code)						{ m_code = code; }
	CMPINLINE std::string	GetName(void) const										{ return m_name; }
	CMPINLINE void			SetName(const std::string & name)						{ m_name = name; }
	CMPINLINE INTVECTOR3	GetElementSize(void) const								{ return m_elementsize; }
	void					SetElementSize(const INTVECTOR3 & size);
	
	// Virtual method implemented by subclasses, if required, to apply subclass-specific properties to a tile
	virtual void			ApplyClassSpecificDefinition(ComplexShipTile *tile) const = 0;

	// Virtual method to read class-specific XML data for the tile definition
	virtual void			ReadClassSpecificXMLData(TiXmlElement *node) = 0;

	// Get and set the numeric tile level, that indicates how advanced a tile this is in the tech tree
	CMPINLINE int			GetTileLevel(void) const								{ return m_level; }
	CMPINLINE void			SetTileLevel(int level)									{ m_level = level; }

	// Mass of the tile, excluding any contents
	CMPINLINE float			GetMass(void) const										{ return m_mass; }
	CMPINLINE void			SetMass(float mass)										{ m_mass = mass; }

	// Hardness multiplier for the tile, roughly approximating 'density' in impact calculations
	CMPINLINE float			GetHardness(void) const									{ return m_hardness; }
	CMPINLINE void			SetHardness(float hardness)								{ m_hardness = hardness; }

	// Vector of all terrain objects which are contained within this tile definition
	std::vector<StaticTerrain*>							TerrainObjects;
	CMPINLINE void										AddTerrainObject(StaticTerrain *t)		{ TerrainObjects.push_back(t); }
	CMPINLINE void										RemoveTerrainObject(StaticTerrain *t)	{ RemoveFromVector<StaticTerrain*>(TerrainObjects, t); }
	CMPINLINE std::vector<StaticTerrain*>::size_type	GetTerrainObjectCount(void) const		{ return TerrainObjects.size(); }

	// Hardpoints which are instantiated along with instances of this tile
	CMPINLINE const Hardpoints::HardpointCollection &	GetHardpoints(void) const				{ return m_hardpoints; }
	void												AddHardpoint(Hardpoint *hardpoint);

	// Power requirement in order for tiles of this type to be functional
	CMPINLINE Power::Type								GetPowerRequirement(void) const			{ return m_powerrequirement; }
	CMPINLINE void										SetPowerRequirement(Power::Type power)	{ m_powerrequirement = power; }

	// Production cost of this tile, specified per-element
	CMPINLINE ProductionCost *	GetProductionCost(void)						{ return m_productioncost; }
	CMPINLINE void				SetProductionCost(ProductionCost *prodcost)	
	{
		// Deallocate any existing data
		if (m_productioncost) SafeDelete(m_productioncost); 

		// Store a pointer to the new object
		m_productioncost = prodcost; 
	}
	
	// Determines whether the tile type has a fixed size in all dimensions, or if it can vary for each instance
	CMPINLINE bool			HasFixedSize(void) const
	{
		return (m_elementsize.x > 0 && m_elementsize.y > 0 && m_elementsize.z > 0);
	}

	// Model properties
	CMPINLINE bool				HasCompoundModel(void) const					{ return m_multiplemodels; }
	CMPINLINE void				SetHasCompoundModel(bool compound)				{ m_multiplemodels = compound; }
	CMPINLINE Model *			GetModel(void)									{ return m_model; }
	CMPINLINE void				SetModel(Model *model)							{ m_model = model; }
	CMPINLINE TileModelSet *	GetModelSet(void)								{ return &m_models; }
	CMPINLINE BoundingObject *	GetBoundingVolume(void)							{ return m_boundingbox; }
	CMPINLINE void				SetBoundingVolume(BoundingObject *obj)			{ m_boundingbox = obj; }
	
	// Data on the dynamic tileset (if any) this definition belongs to
	CMPINLINE bool				BelongsToDynamicTileSet(void) const				{ return m_member_of_dynamic_tileset; }
	void						AddToDynamicTileSet(const std::string & tileset);
	void						RemoveLinkToDynamicTileSet(void);
	std::string					GetDynamicTileSet(void) const;

	// Inline method to return a model from a set based on category name; simply calls overloaded function after looking up that category
	CMPINLINE Model *ComplexShipTileDefinition::GetModelFromSet(std::string category) const
	{
		if (category == NullString || m_models.count(category) == 0)	return NULL; 
		else															return GetModelFromSet(&(m_models.at(category)));
	}

	// Default property set applied to all elements of the tile; element-specific changes are then made when compiling the tile
	ElementStateDefinition			DefaultElementState;

	// Creates a new tile based on this definition
	ComplexShipTile *				CreateTile(void) const;

	// Generates the geometry for this tile.  Typically called during tile compilation
	Result							GenerateGeometry(ComplexShipTile *tile) const;

	// Attempts to compile *and validate* a tile based on the parameters that have been set
	Result							CompileAndValidateTile(ComplexShipTile *tile) const;
	
	// Defines the connectivity for tiles created from this definition
	TileConnections					Connectivity;
	
	// Portals owned by this tile
	CMPINLINE const std::vector<ViewPortal> &		GetPortals(void) const							{ return m_portals; }
	void											AddPortal(const ViewPortal & portal);

	// Indicates whether the tile definition contains class-specific data
	CMPINLINE bool					TileDefinitionHasClassSpecificData(void) const					{ return m_haveclassspecificdata; }
	CMPINLINE void					RegisterClassSpecificDataForTileDefinition(void)				{ m_haveclassspecificdata = true; }

	// Shutdown method to deallocate all resources
	void							Shutdown(void);

	// Constructor / destructor
	ComplexShipTileDefinition(void);
	~ComplexShipTileDefinition(void);

	// Adds a new model to the tile model set
	CMPINLINE bool AddItemToCompoundModelCollection(std::string type, std::string code, float prob)
	{
		// Parameter check
		if (type == NullString || code == NullString || prob < Game::C_EPSILON) return false;

		// Attempt to retrieve this model from the central collection and add it to the compound model
		Model *model = Model::GetModel(code);
		if (model)									{ m_models[type].AddItem(model, prob); return true; }
		else										{ return false; }
	}

protected:
	// String code and name of the tile
	std::string					m_code;
	std::string					m_name;

	// The class of this tile
	D::TileClass				m_classtype;
	ComplexShipTileClass *		m_class;

	// Flag that indicates whether we have any class-specific data in the subclass definition
	bool						m_haveclassspecificdata;

	// The numeric level of this tile, indicating how advanced it is in the tech tree
	int							m_level;
	
	// Physical properties of the tile
	float						m_mass;
	float						m_hardness;

	// The geometry associated with this ship tile, if it uses only a single model
	Model *						m_model;

	// Collection of models used for this tile definition, if it is made up of a compound model
	bool						m_multiplemodels;
	TileModelSet				m_models;

	// Name of the dynamic tile set this definition belongs to, if applicable
	bool						m_member_of_dynamic_tileset;
	std::string					m_dynamic_tileset;

	// Collection of any hardpoints that are instantiated along with tiles
	Hardpoints::HardpointCollection		m_hardpoints;

	// Collection of portals from this tile
	std::vector<ViewPortal>		m_portals;

	// Size of the model.  If set to -1/-1/-1 then the tile is stretchable to user-defined size; if not, it maintains the size specified
	INTVECTOR3					m_elementsize;

	// Power requirement for tiles of this type to be functional
	Power::Type					m_powerrequirement;

	// Production cost of this tile, specified per elemnet
	ProductionCost *			m_productioncost;

	// Bounding box encompassing this tile; used for more efficient visibility & collision testing
	BoundingObject *			m_boundingbox;

	// Returns the model set for the specified category of element, or NULL if no model set exists
	CMPINLINE const ProbabilityWeightedModelCollection *	GetModelSet(const std::string &category) const
	{ 
		if (m_models.count(category) > 0)	return &(m_models.at(category)); 
		else								return NULL; 
	}

	// Builds a tile based on this definition, and the data already loaded into the tile object
	Result							CompileTile(ComplexShipTile *tile) const;

	// Validates a tile based on its hard stop requirements
	Result							ValidateTileHardStop(ComplexShipTile *tile) const;

};



#endif