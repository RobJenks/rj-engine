#include "XML\\tinyxml.h"
#include "ErrorCodes.h"
#include "FastMath.h"
#include "GameDataExtern.h"
#include "ElementConnection.h"
#include "ComplexShipTile.h"
#include "ComplexShipTileClass.h"
#include "ComplexShipElement.h"
#include "BoundingObject.h"
#include "ProductionCost.h"
#include "Hardpoint.h"
#include "Hardpoints.h"
#include "CSCorridorTileDefinition.h"
#include "CSQuartersTileDefinition.h"
#include "CSLifeSupportTileDefinition.h"
#include "CSPowerGeneratorTileDefinition.h"
#include "CSEngineRoomTileDefinition.h"
#include "CSArmourTileDefinition.h"

#include "ComplexShipTileDefinition.h"


// Data struct used to store adjacency info for tile corner elements
// The struct holds { xpos = [0|1], ypos = [0|1], rotation of a corner tile at this pos, rotation of one edge tile, rotation of the second edge tile }
const ComplexShipTileDefinition::CornerAdjacencyData ComplexShipTileDefinition::CornerData[] = 
{ 
	/* Bottom-left */	ComplexShipTileDefinition::CornerAdjacencyData(0, 1, Rotation90Degree::Rotate0, Rotation90Degree::Rotate0, Rotation90Degree::Rotate90), 
	/* Bottom-right */	ComplexShipTileDefinition::CornerAdjacencyData(1, 1, Rotation90Degree::Rotate90, Rotation90Degree::Rotate90, Rotation90Degree::Rotate180), 
	/* Top-right */		ComplexShipTileDefinition::CornerAdjacencyData(1, 0, Rotation90Degree::Rotate180, Rotation90Degree::Rotate180, Rotation90Degree::Rotate270), 
	/* Top-left */		ComplexShipTileDefinition::CornerAdjacencyData(0, 0, Rotation90Degree::Rotate270, Rotation90Degree::Rotate270, Rotation90Degree::Rotate0), 
};

ComplexShipTileDefinition::ComplexShipTileDefinition(void)
	: m_code(NullString), m_name(NullString), m_classtype(D::TileClass::Unknown), m_class(NULL), m_level(0), m_powerrequirement(0),
	  m_haveclassspecificdata(false), m_model(NULL), m_multiplemodels(false), m_elementsize(NULL_INTVECTOR3), m_boundingbox(NULL), 
	  m_mass(1.0f), m_hardness(1.0f), m_productioncost(NULL), m_member_of_dynamic_tileset(false), m_dynamic_tileset(NullString), 
	  DefaultElementState(ElementStateFilters::TILE_PROPERTIES)
{
}

// Static method to create definition objects of the desired subclass type
ComplexShipTileDefinition *	ComplexShipTileDefinition::Create(D::TileClass cls)
{
	// Initialise the definition based on the desired subclass type
	ComplexShipTileDefinition *def;
	switch (cls)
	{
		case D::TileClass::Corridor:		def = new CSCorridorTileDefinition();		break;
		case D::TileClass::Quarters:		def = new CSQuartersTileDefinition();		break;
		case D::TileClass::LifeSupport:		def = new CSLifeSupportTileDefinition();	break;
		case D::TileClass::PowerGenerator:	def = new CSPowerGeneratorTileDefinition();	break;
		case D::TileClass::EngineRoom:		def = new CSEngineRoomTileDefinition();		break; 
		case D::TileClass::Armour:			def = new CSArmourTileDefinition();			break;

		// Return error if the class is unknown
		default:							return NULL;
	}

	// Return the newly-created definition object
	return def;
}

ComplexShipTile * ComplexShipTileDefinition::CreateTile(void) const
{
	// Create a new tile depending on our class
	ComplexShipTile *tile = ComplexShipTile::New(m_classtype);
	if (tile == NULL) return NULL;
	
	// Set all base tile data
	tile->SetCode(m_code);
	tile->SetName(m_name);
	tile->SetTileDefinition(this);
	tile->SetTileClass(m_classtype);
	tile->SetPowerRequirement(m_powerrequirement);
	tile->SetMass(m_mass);
	tile->SetHardness(m_hardness);

	// Set a default tile size; either the class-specified default, or 1x1x1 if it is a resizable tile
	if (m_elementsize.x > 0 && m_elementsize.y > 0 && m_elementsize.z > 0)
		tile->SetElementSize(m_elementsize);
	else
		tile->SetElementSize(INTVECTOR3(1, 1, 1));

	// Copy all connectivity data into the new tile
	tile->PossibleConnections = TileConnections(Connectivity);

	// Copy all portal definitions into the tile
	for (const auto & portal : m_portals) tile->AddPortal(ViewPortal(portal));
	
	// Finally, perform any class-specific initialisation (if required) via the subclass virtual method
	if (TileDefinitionHasClassSpecificData()) this->ApplyClassSpecificDefinition(tile);
	
	// Return the newly-created tile
	return tile;
}

// Attempts to compile and validate a tile based on the parameters that have been set
Result ComplexShipTileDefinition::CompileAndValidateTile(ComplexShipTile *tile) const
{
	// Parameter check
	Result res;
	if (!tile) return ErrorCodes::CouldNotCompileNullTile;

	// Attempt to build the tile based on the parameters that were loaded
	res = CompileTile(tile);
	if (res != ErrorCodes::NoError) return res;

	// Validate based on hard-stop requirements of the tile class
	res = ValidateTileHardStop(tile);
	if (res != ErrorCodes::NoError) return res;

	// If this tile has a compound model, validate whether it requires post-processing with full geometry data
	if (tile->RequiresCompoundModelPostProcessing())
	{
		return ErrorCodes::TileCompiledWithoutGeometryDataLoaded;
	}
	else
	{
		return ErrorCodes::NoError;
	}
}

// Builds a tile based on this definition, and the data loaded into the tile object by the GenerateTile method
Result ComplexShipTileDefinition::CompileTile(ComplexShipTile *tile) const
{
	// Make sure the supplied parameter is valid
	if (!tile) return ErrorCodes::CannotBuildTileWithInvalidPointer;

	// Retrieve the size of this tile and make sure it is valid
	const INTVECTOR3 & size = tile->GetElementSize();
	if (size.x <= 0 || size.y <= 0 || size.z <= 0) return ErrorCodes::CannotCompileTileWithInvalidSize;

	// We can initialise the construction state of this tile based on the production cost maintained in the tile definition
	if (m_productioncost)
	{
		// Create a clone of our per-element production cost, scaled up to accomodate the size of the new tile
		tile->InitialiseConstructionState(m_productioncost->CreateClone((float)(size.x * size.y * size.z)));
	}

	// Generate the model geometry.  This MUST be performed after all other model compilation is complete
	Result result = GenerateGeometry(tile);
	if (result != ErrorCodes::NoError) return result;

	// Perform a recalculation of tile data, to ensure that e.g. the world matrix is correctly recalculated
	tile->RecalculateTileData();

	// Return success
	return ErrorCodes::NoError;
}

// Generates the geometry for this tile.  Typically called during tile compilation
Result ComplexShipTileDefinition::GenerateGeometry(ComplexShipTile *tile) const 
{
	// Parameter check
	if (!tile) return ErrorCodes::CannotGenerateGeometryForNullTile;

	// Take different action depending on whether this tile has a simple or compound model
	if (!m_multiplemodels)
	{
		// If this is a single tile, simply copy the model assigned to the tile definition
		tile->SetSingleModel(m_model);
		return ErrorCodes::NoError;
	}

	// Otherwise we need to construct a compound tile model based on the models specified in the tile definition
	tile->SetMultipleModels();

	// Get a reference to the tile model collection and tile size for convenience
	ComplexShipTile::TileCompoundModelSet *models = tile->GetCompoundModelSet();
	const INTVECTOR3 & size = tile->GetElementSize();
	assert(models->Size == size);

	// Get a pointer to each of the key model types for efficiency before looping.
	const ComplexShipTileDefinition::ProbabilityWeightedModelCollection *medge = GetModelSet("wall_straight");
	const ComplexShipTileDefinition::ProbabilityWeightedModelCollection *mcorner = GetModelSet("wall_corner");
	const ComplexShipTileDefinition::ProbabilityWeightedModelCollection *minterior = GetModelSet("interior");
	const ComplexShipTileDefinition::ProbabilityWeightedModelCollection *mconn = GetModelSet("connection");

	// First, set any tiles with a connection
	if (mconn)
	{
		// TODO: We currently only look at walkable connections
		TileConnections::TileConnectionType type = TileConnections::TileConnectionType::Walkable;

		// Iterate over each element in turn
		INTVECTOR3 location;
		for (int x = 0; x < size.x; ++x)
		{
			for (int y = 0; y < size.y; ++y)
			{
				for (int z = 0; z < size.z; ++z)
				{
					// Check whether there are any connections from this element; if not, skip it immediately
					location = INTVECTOR3(x, y, z);
					if (!tile->Connections.ConnectionExistsInAnyDirection(type, location)) continue;

					// There is at least one connection; process each possible direction in turn
					bitstring data = tile->Connections.GetConnectionState(type, location);
					if (CheckBit_Any(data, DirectionBS::Up_BS))
						models->AddModel(x, y, z, GetModelFromSet(mconn), Rotation90Degree::Rotate90, ComplexShipTile::TileModel::TileModelType::WallConnection, false);

					if (CheckBit_Any(data, DirectionBS::Right_BS))
						models->AddModel(x, y, z, GetModelFromSet(mconn), Rotation90Degree::Rotate180, ComplexShipTile::TileModel::TileModelType::WallConnection, false);

					if (CheckBit_Any(data, DirectionBS::Down_BS))
						models->AddModel(x, y, z, GetModelFromSet(mconn), Rotation90Degree::Rotate270, ComplexShipTile::TileModel::TileModelType::WallConnection, false);

					if (CheckBit_Any(data, DirectionBS::Left_BS))
						models->AddModel(x, y, z, GetModelFromSet(mconn), Rotation90Degree::Rotate0, ComplexShipTile::TileModel::TileModelType::WallConnection, false);
				}
			}
		}
	}

	// Now consider each corner in turn; may require composition of edge/connection pieces depending on the location of connection tiles
	ComplexShipTile::ModelLinkedList *mlink = NULL;
	bool have1 = false, have2 = false;
	for (int z = 0; z<size.z; z++)
	{
		// Iterate over the set of corner adjacency data for each corner
		for (int i = 0; i < 4; i++)
		{
			// Check whether we have either edge already placed at the corner, as a connection, using the x/y and edge rotation values
			have1 = have2 = false;
			mlink = models->ModelLayout[CornerData[i].x * (size.x - 1)][CornerData[i].y * (size.y - 1)][z];
			if (mlink)
			{
				// Check for connection or wall tiles at this corner
				have1 = (mlink->HasItem(CornerData[i].Edge1Rotation, ComplexShipTile::TileModel::TileModelType::WallConnection)) ||
					(mlink->HasItem(CornerData[i].Edge1Rotation, ComplexShipTile::TileModel::TileModelType::WallStraight));
				have2 = (mlink->HasItem(CornerData[i].Edge2Rotation, ComplexShipTile::TileModel::TileModelType::WallConnection)) ||
					(mlink->HasItem(CornerData[i].Edge2Rotation, ComplexShipTile::TileModel::TileModelType::WallStraight));
			}

			// If we have neither edge covered then we can add a normal corner tile at this position
			if (!have1 && !have2)
			{
				models->AddModel(CornerData[i].x * (size.x - 1), CornerData[i].y * (size.y - 1), z, GetModelFromSet(mcorner),
					CornerData[i].CornerRotation, ComplexShipTile::TileModel::TileModelType::WallCorner, false);
				continue;
			}

			// Otherwise, we have at least one edge covered.  We therefore fill in in the missing edges 
			if (!have1) models->AddModel(CornerData[i].x * (size.x - 1), CornerData[i].y * (size.y - 1), z, GetModelFromSet(medge),
				CornerData[i].Edge1Rotation, ComplexShipTile::TileModel::TileModelType::WallStraight, false);
			if (!have2) models->AddModel(CornerData[i].x * (size.x - 1), CornerData[i].y * (size.y - 1), z, GetModelFromSet(medge),
				CornerData[i].Edge2Rotation, ComplexShipTile::TileModel::TileModelType::WallStraight, false);
		}
	}

	// Set any remaining edge tiles
	if (medge)
	{
		for (int z = 0; z < size.z; z++)
		{
			// Traverse the y dimension and fill in the left (x=0) and right (x=n-1) columns
			for (int y = 1; y<size.y - 1; y++)
			{
				if (!(models->ModelLayout[0][y][z]))		models->AddModel(0, y, z, GetModelFromSet(medge), Rotation90Degree::Rotate0,
					ComplexShipTile::TileModel::TileModelType::WallStraight, false);
				if (!(models->ModelLayout[size.x - 1][y][z])) models->AddModel(size.x - 1, y, z, GetModelFromSet(medge), Rotation90Degree::Rotate180,
					ComplexShipTile::TileModel::TileModelType::WallStraight, false);
			}

			// Traverse the x dimension and fill in the top (y=0) and bottom (y=n-1) rows
			for (int x = 1; x<size.x - 1; x++)
			{
				if (!(models->ModelLayout[x][0][z]))		models->AddModel(x, 0, z, GetModelFromSet(medge), Rotation90Degree::Rotate270,
					ComplexShipTile::TileModel::TileModelType::WallStraight, false);
				if (!(models->ModelLayout[x][size.y - 1][z])) models->AddModel(x, size.y - 1, z, GetModelFromSet(medge), Rotation90Degree::Rotate90,
					ComplexShipTile::TileModel::TileModelType::WallStraight, false);
			}
		}
	}

	// Finally loop across the tile and set all other elements as interior elements		
	if (minterior)
	{
		for (int z = 0; z<size.z; z++)
		{
			for (int x = 1; x<size.x - 1; x++) {
				for (int y = 1; y<size.y - 1; y++)
				{
					if (!(models->ModelLayout[x][y][z])) models->AddModel(x, y, z, GetModelFromSet(minterior), Rotation90Degree::Rotate0);
				}
			}
		}
	}

	// Return success
	return ErrorCodes::NoError;
}

// Validates a tile based on hard stop requirements of the class
Result ComplexShipTileDefinition::ValidateTileHardStop(ComplexShipTile *tile) const
{
	// Validate the tile against any hard-stop tile class requirements
	if (m_class && !m_class->ValidateHardStopRequirements(tile))
	{
		return ErrorCodes::TileFailedHardStopRequirements;
	}

	// We have created and validated the tile, so return success
	return ErrorCodes::NoError;
}

// Sets the element size for this definition, allocating other data accordingly
void ComplexShipTileDefinition::SetElementSize(const INTVECTOR3 & size)					
{
	// Store the new element size
	m_elementsize = size;

	// Initialise the connection data to an equivalent size
	Connectivity.Initialise(m_elementsize);
}

// Hardpoints which are instantiated along with instances of this tile
void ComplexShipTileDefinition::AddHardpoint(Hardpoint *hardpoint)
{
	// Validate the hardpoint and make sure we don't already have one with the same code
	if (!hardpoint) return;
	Hardpoints::HardpointCollection::const_iterator it = std::find_if(m_hardpoints.begin(), m_hardpoints.end(),
		[&hardpoint](const Hardpoint *hp) { return (hp->Code == hardpoint->Code); });
	if (it != m_hardpoints.end()) return;

	m_hardpoints.push_back(hardpoint);
}

// Add a new portal to this definition.  Portal will only be accepted if it lies fully within
// the bounds of this tile
void ComplexShipTileDefinition::AddPortal(const ViewPortal & portal)
{
	// Make sure the portal bounds lie within the area of this tile, otherwise reject it
	XMVECTOR extent = XMVectorMultiply(Game::ElementLocationToPhysicalPosition(m_elementsize), HALF_VECTOR_P);
	for (int i = 0; i < 4; ++i)
	{
		// Reject the portal if any vertex lies outside the tile bounds
		if (XMVector3Greater(portal.Vertices[i], extent) || XMVector3Less(portal.Vertices[i], XMVectorNegate(extent))) return;
	}

	// Add the portal to our collection
	m_portals.push_back(portal);
}

// Adds a link to a dynamic tileset
void ComplexShipTileDefinition::AddToDynamicTileSet(const std::string & tileset)
{
	// Store the tileset code and set the flag
	m_member_of_dynamic_tileset = true;
	m_dynamic_tileset = tileset;
}

// Removes this definition from its current dynamic tileset
void ComplexShipTileDefinition::RemoveLinkToDynamicTileSet(void)
{
	// Clear the tileset code and also reset the flag
	m_member_of_dynamic_tileset = false;
	m_dynamic_tileset = NullString;
}

// Returns the dynamic tileset that this definition belongs to, if applicable
std::string ComplexShipTileDefinition::GetDynamicTileSet(void) const
{
	return m_dynamic_tileset;
}


// Adds a model to the probability-weighted model collection for this tile type
void ComplexShipTileDefinition::AddModelToSet(std::string category, Model *model, float probability)
{
	// Parameter check
	if (category == NullString || !model || probability < Game::C_EPSILON) return;

	// Add a new item to the collection
	m_models[category].AddItem(model, probability);
}

// Retrieves a model for the specified category, taking into account the probability assigned to each model type
Model *ComplexShipTileDefinition::GetModelFromSet(const ProbabilityWeightedModelCollection *modelset) const
{
	// Make sure we have at least one model for this category
	if (!modelset) return NULL;

	// If we have only one model then return it immediately now
	if (modelset->models.size() == 1) return modelset->models.at(0).model;

	// Otherwise choose a random number between 0 and the maximum probability value
	float r = frand_h(modelset->totalprob); float totalr = 0.0f;
	std::vector<ComplexShipTileDefinition::ProbabilityWeightedModel>::const_iterator it_end = modelset->models.end();
	for (std::vector<ComplexShipTileDefinition::ProbabilityWeightedModel>::const_iterator it = modelset->models.begin(); it != it_end; ++it)
	{
		// If this model is >= the selected probability then we will return this one
		totalr += it->probability;
		if (totalr >= r) return it->model;
	}

	// Return NULL by default, although we should not expect to reach this point
	return NULL;
}

// Sets the class of this tile definition, based on the string code of the class
bool ComplexShipTileDefinition::SetClass(const std::string & cls)
{
	// Attempt to retrieve a tile class with this code
	if (cls == NullString) return false;
	std::string s = cls; StrLowerC(s);
	ComplexShipTileClass *tc = D::ComplexShipTileClasses.Get(cls);

	// Pass control to the overloaded function with this class object
	return SetClass(tc);
}

// Sets the class of this tile definition, based on the provided class object
bool ComplexShipTileDefinition::SetClass(ComplexShipTileClass *cls)
{
	// Make sure the class object is valid
	if (!cls) return false;

	// Store the class type, and a pointer to the class itself, within this tile definition
	m_class = cls;
	m_classtype = cls->GetClassType();

	// Return true for success
	return true;
}

// Shutdown method to deallocate all resources
void ComplexShipTileDefinition::Shutdown(void)
{
	// Deallocate objects maintained within this tile definition
	delete m_boundingbox;		m_boundingbox = NULL;
	delete m_productioncost;	m_productioncost = NULL;

	// Deallocate all terrain objects stored within the definition
	std::vector<StaticTerrain*>::size_type n = TerrainObjects.size();
	for (std::vector<StaticTerrain*>::size_type i = 0; i < n; ++i)
	{
		if (TerrainObjects[i]) SafeDelete(TerrainObjects[i]);
	}
	TerrainObjects.clear();
}








ComplexShipTileDefinition::~ComplexShipTileDefinition(void)
{
}
