#include "DX11_Core.h"

#include "XML\\tinyxml.h"
#include "FastMath.h"
#include "Utility.h"
#include "FileInput.h"
#include "XMLGenerator.h"
#include "GameDataExtern.h"
#include "Model.h"
#include "BoundingObject.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "ComplexShipTileDefinition.h"
#include "ComplexShipTileClass.h"
#include "ComplexShipElement.h"
#include "ElementConnection.h"
#include "Hardpoint.h"
#include "CSCorridorTile.h"
#include "CSQuartersTile.h"
#include "CSLifeSupportTile.h"
#include "CSPowerGeneratorTile.h"
#include "CSEngineRoomTile.h"
#include "CSArmourTile.h"
#include "ComplexShipTile.h"
#include "Damage.h"
#include "ProductionProgress.h"
#include "ProductionCost.h"
#include "FadeEffect.h"
#include "DataInput.h"
#include "DataOutput.h"
#include "iSpaceObjectEnvironment.h"
#include "ShadowSettings.h"

#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
	long ComplexShipTile::inst_con = 0;
	long ComplexShipTile::inst_des = 0;
#endif

// Static record of the current maximum ID for space objects; allows all space objects to be given a uniquely-identifying reference
Game::ID_TYPE ComplexShipTile::InstanceCreationCount = 0;


ComplexShipTile::ComplexShipTile(void)
{
	// Debug purposes: record the number of elements being created
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_con;
	#endif

	// Assign a new unique ID to this tile
	m_id = GenerateNewUniqueID();

	// Set properties to default values on object creation
	m_parent = NULL;
	m_standardtile = false;
	m_multiplemodels = false;
	m_rotation = Rotation90Degree::Rotate0;
	m_elementlocation = NULL_INTVECTOR3;
	m_elementsize = NULL_INTVECTOR3;		
	m_elementposition = NULL_VECTOR;
	m_worldsize = NULL_VECTOR;
	m_centre_point = NULL_VECTOR;
	m_multielement = false;
	m_bounding_radius = 1.0f;
	m_boundingbox = new BoundingObject();
	m_relativeposition = NULL_VECTOR;
	m_relativepositionmatrix = ID_MATRIX;
	m_worldmatrix = ID_MATRIX;
	m_definition = NULL;
	m_classtype = D::TileClass::Unknown;
	m_mass = 1.0f;
	m_hardness = 1.0f;
	m_aggregatehealth = 1.0f;
	m_constructedstate = new ProductionCost();
	m_elementconstructedstate = NULL;
	m_constructedstate_previousallocatedsize = NULL_INTVECTOR3;
	m_connections_fixed = false;
	m_powerlevel = m_powerrequirement = (Power::Type)0;
	m_portalcount = 0U;
	
	// By default, tiles will not be simulated
	m_requiressimulation = false;
	m_simulationinterval = 1000U;
	m_lastsimulation = Game::ClockMs;
}


// Copy constructor
ComplexShipTile::ComplexShipTile(const ComplexShipTile &C)
{
	// Debug purposes: record the number of elements being created
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_con;
	#endif

	// Assign a new unique ID to the new tile
	m_id = GenerateNewUniqueID();

	// Copy all base class properties to this new instance
	m_code = C.GetCode();
	m_name = C.GetName();
	m_definition = C.GetTileDefinition();
	m_classtype = C.GetTileClass();
	m_elementlocation = C.GetElementLocation();
	m_elementposition = C.GetElementPosition();
	m_worldsize = C.GetWorldSize();
	m_model = C.GetModel();
	m_rotation = C.GetRotation(); 
	m_mass = C.GetMass();
	m_hardness = C.GetHardness();
	m_boundingbox = BoundingObject::Copy(C.GetBoundingObject());
	m_multiplemodels = C.HasCompoundModel();
	m_models = C.GetCompoundModelSet();
	DefaultProperties = C.DefaultProperties;
	m_connections_fixed = C.ConnectionsAreFixed();
	m_powerrequirement = C.GetPowerRequirement();
	m_powerlevel = 0;

	// Set parent pointer to NULL since the copied element will likely not share the same parent
	m_parent = NULL;

	// We will copy construction requirements; however construction progress is always reset to 0% or 100% (in clone method) depending on the parent
	// Per-element construction state can therefore always be initialised to NULL
	// TODO: IMPORTANT: There are currently class-slicing problems in the tile class hierarchy.  These need 
	// to be fixed so that the production cost/progress data can be reactivated
	/*if (C.GetConstructedStateConst())
		m_constructedstate = C.GetConstructedStateConst()->CreateClone();
	else
		m_constructedstate = new ProductionCost();*/
	m_elementconstructedstate = NULL;

	// Set the element size of this tile, which recalculates relevant derived fields
	SetElementSize(C.GetElementSize());

	// We always reset the 'standard' flag to false on copy, since only a small subset of tiles in the global collection
	// should ever be flagged as standard
	m_standardtile = false;

	// Copy the simulation parameters of our copy source, and treat the new object as having just been simulated
	m_requiressimulation = C.SimulationIsActive();
	m_simulationinterval = C.GetSimulationInterval();
	m_lastsimulation = Game::ClockMs;

	// Take the simulation state of the source tile
	SetSimulationState(C.GetSimulationState());

	// Copy the connection data for this tile 
	Connections = TileConnections(C.Connections);
	PossibleConnections = TileConnections(C.PossibleConnections);

	// Replicate any portal data for this tile
	m_portals = C.m_portals;
	m_portalcount = C.m_portalcount;

	// Recalculate tile data, relative position and world matrix based on this new data
	RecalculateTileData();
}


// Sets the rotation of this tile, adjusting all contents and geometry accordingly
void ComplexShipTile::SetRotation(Rotation90Degree rot)
{
	// Determine the rotation being performed between old and new rotation states
	Rotation90Degree rot_delta = Rotation90BetweenValues(m_rotation, rot);

	// Store the new rotation value
	m_rotation = rot;

	// No action is required if this will not involve a rotation (e.g. going from 360d to 0d)
	if (rot_delta == Rotation90Degree::Rotate0) return;

	// Transform all terrain objects associated with this tile by the relevant rotation
	RotateAllTerrainObjects(rot_delta);

	// Transform all view portals by the same rotation
	RotateAllViewPortals(rot_delta);

	// Recalculate all derived data for the tile
	RecalculateTileData();
}

// Rotates the tile by the specified angle, adjusting all contents and geometry accordingly
void ComplexShipTile::Rotate(Rotation90Degree rot)
{
	// Determine the new desired tile rotation and call the primary rotation method
	SetRotation(Compose90DegreeRotations(GetRotation(), rot));
}

// Rotates all terrain objects associated with this tile by the specified angle
void ComplexShipTile::RotateAllTerrainObjects(Rotation90Degree rotation)
{
	// Parameter check
	if (!m_parent || rotation == Rotation90Degree::Rotate0) return;

	// Iterate through all terrain objects in our parent environment
	Terrain *t;
	iSpaceObjectEnvironment::TerrainCollection::iterator it_end = m_parent->TerrainObjects.end();
	for (iSpaceObjectEnvironment::TerrainCollection::iterator it = m_parent->TerrainObjects.begin(); it != it_end; ++it)
	{
		// Get a reference to the terrain object
		t = (*it); if (!t) continue;

		// We only care about terrain objects associated with this tile
		if (t->GetParentTileID() != m_id) continue;

		// This is a terrain object which we need to update
		RotateTileTerrainObject(t, rotation);
	}
}

// Rotate a single terrain object owned by this tile by the given rotation, about the tile centre
void ComplexShipTile::RotateTileTerrainObject(Terrain *terrain, Rotation90Degree rotation)
{
	// We can only rotate this terrain if it is valid and is owned by this tile
	if (!terrain || terrain->GetParentTileID() != GetID()) return;

	// Transform the terrain environment-relative position into tile-relative space so we can 
	// transform relative to the tile, then return to environment space
	XMVECTOR tile_centre_pos = XMVectorAdd(m_elementposition, m_centre_point);
	XMVECTOR rel_pos = XMVectorSubtract(terrain->GetPosition(), tile_centre_pos);
	rel_pos = XMVector3TransformCoord(rel_pos, GetRotationMatrix(rotation));
	terrain->SetPosition(XMVectorAdd(rel_pos, tile_centre_pos));

	// Also adjust the terrain object orientation about its local centre
	XMVECTOR rotation_quaternion = GetRotationQuaternion(rotation);
	terrain->ChangeOrientation(rotation_quaternion);

	// Terrain object extents need to be transformed by the same rotation
	terrain->SetExtent(XMVectorAbs(XMVector3Rotate(terrain->GetExtentV(), rotation_quaternion)));
}

// Transform all view portals by the same rotation
void ComplexShipTile::RotateAllViewPortals(Rotation90Degree rot_delta)
{
	// Portal positions are all defined relative to the tile centre point.  We therefore simply
	// need to apply a rotation about the origin to the portal bounds
	XMMATRIX rot_matrix = GetRotationMatrixInstance(rot_delta);
	for (auto & portal : m_portals)
	{
		portal.Transform(rot_matrix);
	}
}

// Recalculates the state of the tile following a change to its position/size etc.  Includes recalc of the world matrix and bounding volume
void ComplexShipTile::RecalculateTileData(void)
{
	// Update any size data (e.g. world size) in case the structure/size of the tile has changed
	ElementSizeChanged();

	// Update the flag that indicates whether this tile spans multiple elements (for use at render-time)
	m_multielement = (m_elementcount > 1);

	// Recalculate the world matrix for this tile based on its current position
	RecalculateWorldMatrix();

	// Update the tile based on its model data if required
	if (HasCompoundModel()) RecalculateCompoundModelData();

	// Also recalculate the tile bounding box
	RecalculateBoundingVolume();

	// Import any collision data from our tile models and store as collision-terrain objects
	UpdateCollisionDataFromModels();

}

void ComplexShipTile::RecalculateWorldMatrix(void)
{ 
	// Make sure that we have model data, for either single- or compound-model mode.  If not, simply set WM = ID
	// Tcentre = model translation to its centre point, prior to rotation about its centre
	XMVECTOR centrepoint;
	if (!m_multiplemodels && m_model.GetModel())
	{
		centrepoint = XMLoadFloat3(&m_model.GetModel()->GetCentrePoint());
	}
	else if (m_multiplemodels)
	{
		centrepoint = m_models.GetCompoundModelCentre();
	}
	else
	{
		m_relativeposition = NULL_VECTOR;
		m_relativepositionmatrix = ID_MATRIX;
		m_worldmatrix = ID_MATRIX;
		return;
	}
	
	// Telement = translation from model centre to top-left corner (0.5*numelements*elementsize in world coords) + translation to element position
	// Will switch y and z coordinates since we are moving from element to world space
	// m_relativeposition = D3DXVECTOR3((float)(m_elementsize.x * Game::C_CS_ELEMENT_MIDPOINT) + Game::ElementLocationToPhysicalPosition(m_elementlocation.x), ...
	m_relativeposition = XMVectorAdd(XMVectorMultiply(m_worldsize, HALF_VECTOR), m_elementposition);
	m_relativepositionmatrix = XMMatrixTranslationFromVector(m_relativeposition);

	// Multiply the matrices together to determine the final tile world matrix
	// World = (Scale * CentreTrans * Rotation * ElementPosTranslation)
	m_worldmatrix = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(
		m_model.GetWorldMatrix(),										// Inherent model world matrix (== scale)
		XMMatrixTranslationFromVector(XMVectorNegate(centrepoint))),	// Translate to centre point
		GetRotationMatrix(m_rotation)),									// Rotate about centre
		m_relativepositionmatrix);										// Translate centre point to centre point of element location
}

// Recalculates the bounding volume for this tile based on the element size in world space
void ComplexShipTile::RecalculateBoundingVolume(void)
{
	// Get the bounding box size in world space based on the element size of the tile.  
	XMFLOAT3 size = Game::ElementLocationToPhysicalPositionF(m_elementsize);

	// Only recalculate the bounding volume if we have a nonzero positive size in each dimension
	if (size.x <= 0.0f || size.y <= 0.0f || size.z <= 0.0f) return;

	// Switch x and z dimensions if the tile is rotated by 90 degrees (anti)clockwise
	if (m_rotation == Rotation90Degree::Rotate90 || m_rotation == Rotation90Degree::Rotate270)
	{
		std::swap(size.x, size.z);
	}

	// Create a new cuboid bounding volume to cover the tile 
	m_boundingbox->CreateCuboidBound(size);
}

// Updates the object before it is rendered.  Called only when the object enters the render queue (i.e. not when it is out of view)
void ComplexShipTile::PerformRenderUpdate(void)
{
	// Update any render effects that may be active on the object
	Fade.Update();
}

// Handle the import of additional collision data from the models that comprise this tile
// Import any collision data from our tile models and store as collision-terrain objects
void ComplexShipTile::UpdateCollisionDataFromModels()
{
	// Remove any collision data that was previously imported from our geometry
	RemoveAllCollisionDataFromModels();

	// We either need to pull data from single or compound model data
	if (!HasCompoundModel())
	{
		// The tile has only a single model
		AddCollisionDataFromModel(GetModel().GetModel());
	}
	else
	{
		// We have a compound model; add from each in turn
		for (const auto & model_element : m_models.GetModels())
		{
			AddCollisionDataFromModel(model_element.Model, model_element.Location, model_element.Rotation);
		}
	}
}

// Handle the import of additional collision data from the models that comprise this tile
// Remove any collision-terrain objects that were added to the tile based on its model data
void ComplexShipTile::RemoveAllCollisionDataFromModels()
{
	if (m_parent)
	{
		m_parent->RemoveCollisionTerrainFromTileGeometry(this);
	}
}

// Handle the import of additional collision data from the models that comprise this tile
// Import collision data from the single specified model, with no location or rotation offset required
void ComplexShipTile::AddCollisionDataFromModel(const ModelInstance & model)
{
	AddCollisionDataFromModel(model, NULL_UINTVECTOR3, Rotation90Degree::Rotate0);
}

// Handle the import of additional collision data from the models that comprise this tile
// Import collision data from the single specified model, with the given element location
// and rotation offsets applied during calculation of the collision volumes
void ComplexShipTile::AddCollisionDataFromModel(const ModelInstance & model, const UINTVECTOR3 & element_offset, Rotation90Degree rotation_offset)
{
	const Model *m = model.GetModel();
	if (!m || !m_parent || m->CollisionData().empty()) return;

	// Offset of the model centre from the tile centre = ((element_pos + 1/2element) - tilecentre - model_centre_point)
	// E.g. 3x3 tile, 1x1 zero-centred model in (2,1), offset = ((25,15)-(15,15)-(0,0)) = (10,0).  Equals ((10,0)+(15,15)) in non-tile-centred space = (25,15) = el[2,1].centre
	XMVECTOR model_centre = XMLoadFloat3(&m->GetCentrePoint());
	XMVECTOR pos_offset = XMVectorSubtract(XMVectorSubtract(XMVectorAdd(Game::ElementLocationToPhysicalPosition(element_offset), Game::C_CS_ELEMENT_MIDPOINT_V), m_centre_point), model_centre);
	XMVECTOR orient_offset = GetRotationQuaternion(rotation_offset);

	// Process every collision volume separately
	for (const auto & collision : m->CollisionData())
	{
		XMVECTOR basepos = XMVector3TransformCoord(XMLoadFloat3(&collision.Position), model.GetWorldMatrix());	// Scale pos by model size
		XMVECTOR position = XMVectorAdd(XMVector3Rotate(basepos, orient_offset), pos_offset);
		XMVECTOR orient = XMQuaternionMultiply(XMLoadFloat4(&collision.Orientation), orient_offset);
		XMVECTOR extent = XMVector3TransformCoord(XMLoadFloat3(&collision.Extent), model.GetWorldMatrix());		// Scale extent by model size

		Terrain *terrain = Terrain::Create();
		terrain->PostponeUpdates();
		{
			terrain->SetPosition(position);
			terrain->SetOrientation(orient);
			terrain->SetExtent(extent);
			terrain->SetParentTileID(GetID());
			terrain->SetSourceType(Terrain::TerrainSourceType::SourcedFromModel);
		}
		terrain->ResumeUpdates();
		
		m_parent->AddTerrainObjectFromTile(terrain, this);
	}
}

// Sets the power level of this tile, triggering updates if necessary
void ComplexShipTile::SetPowerLevel(Power::Type power) 
{ 
	// Store the new power level
	m_powerlevel = power; 

	// Simulate the tile next frame to account for the change in power level, UNLESS this is a
	// power generation tile.  If that is the case, activating simulation would result in a cycle
	// of power emission > simulation required > power emission > simulation required > ...
	if (m_classtype != D::TileClass::PowerGenerator) ActivateSimulation();
}

// Event generated before the tile is added to an environment
void ComplexShipTile::BeforeAddedToEnvironment(iSpaceObjectEnvironment *environment)
{
	// Store a pointer to the new environment (this may be NULL, if not being assigned anywhere)
	m_parent = environment;
}

// Event generated after the tile is added to an environment
void ComplexShipTile::AfterAddedToEnvironment(iSpaceObjectEnvironment *environment)
{
	// Recalculate internal tile data 
	RecalculateTileData();
}

// Event generated before the tile is removed from an environment
void ComplexShipTile::BeforeRemovedToEnvironment(iSpaceObjectEnvironment *environment)
{

}

// Event generated after the tile is removed from an environment
void ComplexShipTile::AfterRemovedFromEnvironment(iSpaceObjectEnvironment *environment)
{
	// Remove our pointer to the environment
	m_parent = NULL;
}

// Applies the effects of this tile on the underlying elements
void ComplexShipTile::ApplyTile(void)
{
	// Parameter check
	if (!m_parent) return;

	// Determine the extent of this tile, and ensure it is valid
	INTVECTOR3 bounds = (m_elementlocation + m_elementsize);
	if (!(bounds > m_elementlocation)) return;

	// This tile will apply to all elements it covers
	for (int x = m_elementlocation.x; x < bounds.x; ++x)
	{
		for (int y = m_elementlocation.y; y < bounds.y; ++y)
		{
			for (int z = m_elementlocation.z; z < bounds.z; ++z)
			{
				// Update the element
				ApplyTileToElement(m_parent->GetElement(x, y, z));
			}
		}
	}

	// Now call the subclass method to apply class-specific properties to the elements
	ApplyTileSpecific();
}

// Applies the effects of this tile to a specific underlying element
void ComplexShipTile::ApplyTileToElement(ComplexShipElement *el)
{
	// Parameter check
	if (!el) return;

	// Get the relative offset within this tile, and make sure we actually cover the element
	INTVECTOR3 tgt = (el->GetLocation() - m_elementlocation);
	if (!(tgt >= NULL_INTVECTOR3 && tgt < m_elementsize)) return;

	// Set the element pointer to this tile
	el->SetTile(this);

	// See if there are any connection values specified for this element, and apply them if there are
	// TODO: Currently only copying the "walkable" connection state.  Element may need to store all types in future
	el->SetConnectionState(Connections.GetConnectionState(TileConnections::TileConnectionType::Walkable, tgt));

	// We only apply these effects to this specific element if the tile & element are both alive.  NOTE: in future
	// we may want to define a bitmask of properties which can/cannot be set on destroyed elements, then
	// can bitwise-& it here to filter only the ones we still want to set
	if (!this->IsDestroyed() && !el->IsDestroyed())
	{
		// Set element properties based on our tile definition
		if (m_definition)
		{
			el->ApplyElementState(m_definition->DefaultElementState.GetElementState(tgt, m_rotation));
		}
	}
}

// Returns a value indicating whether or not this is a primary tile.  Based on the underlying tile class
bool ComplexShipTile::IsPrimaryTile(void)
{
	if (!m_definition) return false;
	ComplexShipTileClass *cls = m_definition->GetClassObject();
	if (!cls) return false;

	return cls->IsPrimaryTile();
}

// Sets the size of this tile in elements, and recalculates any derived properties based on this
void ComplexShipTile::SetElementSize(const INTVECTOR3 & size)
{ 
	// Make sure this is a valid size
	if (size.x < 1 || size.y < 1 || size.z < 1) return;

	// Store the new element size and count of elements
	m_elementsize = size; 
	
	// Update derived size data (e.g. world size) based on this element size
	ElementSizeChanged();

	// Reallocate connection data to be appropriate for this new size
	InitialiseConnectionState();

	// Recalculate properties derived from the element size
	RecalculateTileData(); 

	// Reset the tile to be 0% constructed and start the construction process
	// TODO: IMPORTANT: Disabling for now since there are problems with class slicing.  Resolves these issues with 
	// the tile class hierarchy
	/*m_constructedstate->ResetToZeroPcProgress();		// TODO: FIX
	StartConstruction();*/								// TODO: FIX
}

// Method which recalculates derived size data following a change to the element size (e.g. world size)
void ComplexShipTile::ElementSizeChanged(void)
{
	// Element size is guaranteed to be >= 1 in all dimensions.  
	// Update count of elements within this tile
	m_elementcount = (unsigned int)(m_elementsize.x * m_elementsize.y * m_elementsize.z);

	// Determine the world size of this tile based on the element size; set the model geometry accordingly
	m_worldsize = Game::ElementLocationToPhysicalPosition(m_elementsize);
	m_model.SetSize(m_worldsize);

	// Determine the approximate radius of a bounding sphere that encompasses this tile
	m_bounding_radius = GetElementBoundingSphereRadius(max(max(m_elementsize.x, m_elementsize.y), m_elementsize.z));

	// Determine the centre point of this tile in world space
	m_centre_point = XMVectorScale(Game::ElementLocationToPhysicalPosition(m_elementsize), 0.5f);
}

// Copies the basic properties of a tile from the given source
void ComplexShipTile::CopyBasicProperties(const ComplexShipTile & source)
{
	SetElementLocation(source.GetElementLocation());
	SetElementSize(source.GetElementSize());
	SetRotation(source.GetRotation());
}

// Reallocate connection data to be appropriate for this new size
void ComplexShipTile::InitialiseConnectionState()
{
	// Read the potential connection data from our tile definition, if possible, and only if this is
	// a fixed-size definition that will therefore have per-element connection data already specified
	if (m_definition && m_definition->HasFixedSize())
	{
		// Copy all data from the definition.  This will deallocate any data that already exists
		PossibleConnections = TileConnections(m_definition->Connectivity);
	}
	else
	{
		// Otherwise simply initialise to the correct size, and replicate all data from element 0 as
		// per stanard behaviour for variable-sized tiles.  Initialisation will deallocate any 
		// connection data that has already been stored
		PossibleConnections.Initialise(m_elementsize);
		for (int c = 0; c < (int)TileConnections::TileConnectionType::_COUNT; ++c)							// Copy all def data to [0]
			PossibleConnections.SetConnectionState((TileConnections::TileConnectionType)c, 0,
				m_definition->Connectivity.GetConnectionState((TileConnections::TileConnectionType)c, 0));
		PossibleConnections.ReplicateConnectionState(0);													// Then replicate [0] to [*]
		
	}

	// Initialise the actual connection data to match the tile size
	Connections.Initialise(m_elementsize);
}

// Returns the impact resistance of this tile, i.e. the remaining force it can withstand from physical 
// impacts, with an impact point at the specified element
float ComplexShipTile::GetImpactResistance(const ComplexShipElement & at_element) const
{
	// Calculated as ((mass * hardness) / elementsize) * element_health_percentage
	if (m_elementcount <= 1)
		return ((m_mass * m_hardness) * at_element.GetHealth());
	else
		return (((m_mass * m_hardness) / (float)m_elementcount) * at_element.GetHealth());
}

// Add a new portal to this tile
void ComplexShipTile::AddPortal(const ViewPortal & portal)
{
	m_portals.push_back(portal);
	RecalculatePortalData();
}

// Add a new portal to this tile
void ComplexShipTile::AddPortal(ViewPortal && portal)
{
	m_portals.push_back(std::move(portal));
	RecalculatePortalData();
}

// Recalculate all portal-related data following a change to the collection
void ComplexShipTile::RecalculatePortalData(void)
{
	m_portalcount = m_portals.size();
}

// Deallocates any existing per-element construction progress for this tile
void ComplexShipTile::DeallocatePerElementConstructionProgress(void)
{
	// Check that we have actually allocated space for this data
	if (m_elementconstructedstate && m_constructedstate_previousallocatedsize.x > 0 && 
		m_constructedstate_previousallocatedsize.y > 0 && m_constructedstate_previousallocatedsize.z > 0)
	{
		// Loop through the dimensions of this array and deallocate it one dimension at a time
		for (int x = 0; x < m_constructedstate_previousallocatedsize.x; x++)
		{
			if (m_elementconstructedstate[x]) {
			for (int y = 0; y < m_constructedstate_previousallocatedsize.y; y++)
			{
				if (m_elementconstructedstate[x][y]) {
				for (int z = 0; z < m_constructedstate_previousallocatedsize.z; z++)
				{
					delete m_elementconstructedstate[x][y][z];
					m_elementconstructedstate[x][y][z] = NULL;
				}}
				delete[] m_elementconstructedstate[x][y];
				m_elementconstructedstate[x][y] = NULL;
			}}
			delete[] m_elementconstructedstate[x];
			m_elementconstructedstate[x] = NULL;
		}

		delete m_elementconstructedstate;
		m_elementconstructedstate = NULL;
	}
}

// Begins construction of this tile, allocating space for per-element production progress to be maintained
void ComplexShipTile::StartConstruction(void)
{
	// Attempt to deallocate any existing per-element construction progress 
	DeallocatePerElementConstructionProgress();

	// Make sure the top-level ProductionCost object exists.  If it does, reset it to be either 0% or 100%
	if (!m_constructedstate) return;
	if (m_constructedstate->IsComplete())		m_constructedstate->ResetTo100PcProgress();
	else										m_constructedstate->ResetToZeroPcProgress();

	// Determine the total number of elements in the tile
	float elementscale = 1.0f / (float)(m_elementsize.x * m_elementsize.y * m_elementsize.z);

	// Now we can allocate based on the new size, if this size if valid
	if (m_elementsize.x > 0 && m_elementsize.y > 0 && m_elementsize.z > 0)
	{
		m_elementconstructedstate = new ProductionCost***[m_elementsize.x];
		for (int x = 0; x < m_elementsize.x; x++)
		{
			m_elementconstructedstate[x] = new ProductionCost**[m_elementsize.y];
			for (int y = 0; y < m_elementsize.y; y++)
			{
				m_elementconstructedstate[x][y] = new ProductionCost*[m_elementsize.z];
				for (int z = 0; z < m_elementsize.z; z++)
				{
					// Clone from the top-level construction state object; scale based upon the number of elements so the 
					// resource/time requirements sum correctly at the top level
					m_elementconstructedstate[x][y][z] = m_constructedstate->CreateClone(elementscale);
				}
			}		
		}
	}

	// Store the dimensions of the new space that was allocated
	m_constructedstate_previousallocatedsize = m_elementsize;
}

// Sets the construction of this tile to be 100% complete, and deallocates the per-element construction progress that is maintained during construction
void ComplexShipTile::ConstructionComplete(void)
{
	// Update the overall construction progress object
	m_constructedstate->ResetTo100PcProgress();

	// Deallocate the per-element construction progress that we have maintained during construction
	DeallocatePerElementConstructionProgress();
}

ProductionCost * ComplexShipTile::GetElementConstructedState(int x, int y, int z)
{
	// Make sure space has been allocated
	if (!m_elementconstructedstate) return NULL;
		
	// Perform bounds checking
	if (x >= 0 && x < m_elementsize.x && y >= 0 && y < m_elementsize.y && z >= 0 && z < m_elementsize.z)
		return m_elementconstructedstate[x][y][z];
	else
		return NULL;
}
ProductionCost * ComplexShipTile::GetElementConstructedState(INTVECTOR3 elpos)
{
	// Make sure space has been allocated
	if (!m_elementconstructedstate) return NULL;
		
	// Perform bounds checking
	if (elpos.x >= 0 && elpos.x < m_elementsize.x && elpos.y >= 0 && elpos.y < m_elementsize.y && elpos.z >= 0 && elpos.z < m_elementsize.z)
		return m_elementconstructedstate[elpos.x][elpos.y][elpos.z];
	else
		return NULL;
}

// Adds progress towards the construction of this tile.  Returns the amount that was added.  Timestep is the time since we last
// added resources to the tile element
float ComplexShipTile::AddConstructionProgress(INTVECTOR3 element, const Resource *resource, float amountavailable, float timestep)
{
	// Validate element position is within bounds
	if (element.x < 0 || element.x >= m_elementsize.x || element.y < 0 || 
		element.y >= m_elementsize.y || element.z < 0 || element.z >= m_elementsize.z) return 0.0f;

	// Validate other parameters.  If we are already complete, or have no resource to add, then return now
	if (m_constructedstate->IsComplete() || !resource || amountavailable <= 0) return 0.0f;

	// Attempt to retrieve the element-specific item at this location.  If we cannot do this (e.g. on first starting construction) then reset progress
	// to 0% and force start constuction. If this doesn't work, force complete the construction and return now
	ProductionCost *el = GetElementConstructedState(element);
	if (!el) 
	{ 
		StartConstruction(); 
		el = GetElementConstructedState(element);
		if (!el) { ConstructionComplete(); return 0.0f; }
	}

	// Call the AddProgress method of this element-specific object, and store the amount that was actually added
	float amt = el->AddProgress(resource, amountavailable, timestep);

	// Now add this actual amount to the overall tile construction progress
	m_constructedstate->AddProgress(resource, amt, timestep);

	// Check whether this has now completed the tile construction
	if (m_constructedstate->IsComplete())
	{
		ConstructionComplete();
	}

	// Return the amount that was added to overall construction progress
	return amt;
}

// Removes progress towards the construction of this tile.  Returns the amount that was removed.  Timestep is the time since we
// last removed resources from the tile element
float ComplexShipTile::RemoveConstructionProgress(INTVECTOR3 element, const Resource *resource, float amount, float timestep)
{
	// Validate element position is within bounds
	if (element.x < 0 || element.x >= m_elementsize.x || element.y < 0 || 
		element.y >= m_elementsize.y || element.z < 0 || element.z >= m_elementsize.z) return 0.0f;

	// Validate other parameters.  Make sure we are removing a valid amount of a valid resource
	if (!resource || amount <= 0.0f) return 0.0f;

	// If we are currently complete, then start construction of the tile to generate all the per-element progress objects again
	if (m_constructedstate->IsComplete())
	{
		StartConstruction();
	}

	// Attempt to retrieve the element-specific item at this location.  If we cannot do this (e.g. on first starting construction) then reset progress
	// to 0% and force start constuction. If this doesn't work, force complete the construction and return now
	ProductionCost *el = GetElementConstructedState(element);
	if (!el) 
	{ 
		StartConstruction(); 
		el = GetElementConstructedState(element);
		if (!el) { ConstructionComplete(); return 0.0f; }
	}

	// Call the RemoveProgress method of this element-specific object, and store the amount that was actually removed
	float amt = el->RemoveProgress(resource, amount, timestep);

	// Now remove this actual amount from the overall tile construction progress
	m_constructedstate->RemoveProgress(resource, amt, timestep);

	// Return the amount that was removed from overall construction progress
	return amt;
}

// Sets the production cost & state of this tile; called upon tile creation
void ComplexShipTile::InitialiseConstructionState(ProductionCost *state)
{
	// TODO: IMPORTANT: There are currently class-slicing problems in the tile class hierarchy.  These need 
	// to be fixed so that the production cost/progress data can be reactivated
	return;

	// If we already have a production state object then remove it here first
	if (m_constructedstate) { delete m_constructedstate; m_constructedstate = NULL; }

	// Store the new construction state object
	m_constructedstate = state;

	// Deallocate any per-element construction progress (which is now invalidated) and reset all overall progress to 0%
	DeallocatePerElementConstructionProgress();
	if (m_constructedstate) m_constructedstate->ResetToZeroPcProgress();
}

void ComplexShipTile::ElementHealthChanged(void)
{
	float health = 0.0f;

	// If the health of an underlying element changes we want to recalculate the aggregate health of this tile
	// If any elements have been destroyed, this tile will also be destroyed
	if (!m_parent || m_elementsize.x == 0 || m_elementsize.y == 0 || m_elementsize.z == 0) return;

	// Iterate over all elements covered by this tile
	ComplexShipElement *el;
	for (int x = 0; x < m_elementsize.x; x++) {
		for (int y = 0; y < m_elementsize.y; y++) {
			for (int z = 0; z < m_elementsize.z; z++) 
			{
				el = m_parent->GetElement(m_elementlocation.x + x, m_elementlocation.y + y, m_elementlocation.z + z);
				if (el && !el->IsDestroyed())
				{
					health += el->GetHealth();
				}
				else
				{
					// At least one element under this tile has been destroyed, so (repairably) destroy the tile
					SetHealth(0.0f);
				}
			}
		}
	}

	// Divide through by the number of elements to get an average aggregate value
	SetHealth(health / (m_elementsize.x * m_elementsize.y * m_elementsize.z));

}

// Event triggered upon destruction of the entity
void ComplexShipTile::DestroyObject(void)
{
	// If we are already destroyed then take no further action
	if (IsDestroyed()) return;

	// Apply any destruction effect

	// Destroy all terrain objects owned by this tile
	DestroyAllOwnedTerrain();

	// Now (repairably) destroy the object
	MarkObjectAsDestroyed();
}

// Destroy all terrain objects owned by this tile
void ComplexShipTile::DestroyAllOwnedTerrain(void)
{
	if (!m_parent) return;

	// Tell our parent environment to destroy all items of terrain that we own	
	m_parent->DestroyTerrainFromTile(GetID());
}

// Default destructor
ComplexShipTile::~ComplexShipTile(void)
{
	// Debug purposes: record the number of elements being destructed
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_des;
	#endif

	// Dispose of the bounding volume data for this tile
	if (m_boundingbox) SafeDelete(m_boundingbox);

	// Dispose of overall tile construction data
	// TODO: IMPORTANT: There are currently class-slicing problems in the tile class hierarchy.  These need 
	// to be fixed so that the production cost/progress data can be reactivated
	//if (m_constructedstate) SafeDelete(m_constructedstate);

	// Dispose of all per-element construction state objects, if they exist
	DeallocatePerElementConstructionProgress();

}


// Static base class method to generate XML data for the base class portion of any tile
TiXmlElement * ComplexShipTile::GenerateBaseClassXML(ComplexShipTile *tile)
{	
	// Create a new top-level node to hold the base class data
	if (!tile) return NULL;
	TiXmlElement *node = new TiXmlElement(D::NODE_ComplexShipTileBaseData);

	// Add nodes for each key attribute of the base class
	//IO::Data::LinkStringXMLElement("Code", tile->GetCode(), node);
	//IO::Data::LinkStringXMLElement("Name", tile->GetName(), node);
	IO::Data::LinkIntVector3AttrXMLElement(HashedStrings::H_Location.Text, tile->GetElementLocation(), node);
	IO::Data::LinkIntVector3AttrXMLElement(HashedStrings::H_Size.Text, tile->GetElementSize(), node);
	IO::Data::LinkIntegerXMLElement(HashedStrings::H_Rotation.Text, (int)tile->GetRotation(), node);

	// Add entries for each connection point, and possible connection, from this tile
	IO::Data::SaveTileConnectionState(node, HashedStrings::H_CanConnect.Text, &(tile->PossibleConnections));
	IO::Data::SaveTileConnectionState(node, HashedStrings::H_Connection.Text, &(tile->Connections));

	// Return the node containing all data on this base class
	return node;
}

// Static base class method to read XML data for the base class portion of any tile
void ComplexShipTile::ReadBaseClassXML(TiXmlElement *node, ComplexShipTile *tile)
{
	std::string skey, key, val;
	HashVal hash;

	// Parameter check
	if (!node || !tile) return;

	// Parse the top level of the tile node; should be split into base- and class-specific-data
	TiXmlElement *section = node->FirstChildElement();
	for (section; section; section=section->NextSiblingElement())
	{
		// All key comparisons are case-insensitive
		skey = section->Value(); StrLowerC(skey);

		// Load all the base tile data
		if (skey == "basetiledata")
		{
			// Parse the contents of this node to populate the tile details
			TiXmlElement *child = section->FirstChildElement();
			for (child; child; child=child->NextSiblingElement())
			{
				// All key comparisons are case-insensitive
				key = child->Value(); StrLowerC(key);
				hash = HashString(key);

				if (hash == HashedStrings::H_Location) {
					tile->SetElementLocation( IO::GetInt3CoordinatesFromAttr(child) );
				}
				else if (hash == HashedStrings::H_Size) {
					tile->SetElementSize( IO::GetInt3CoordinatesFromAttr(child) );
				}
				else if (hash == HashedStrings::H_Rotation) {
					val = child->GetText();
					int rot = atoi(val.c_str());
					tile->SetRotation((Rotation90Degree)rot);
				}
				else if (hash == HashedStrings::H_CanConnect) {
					IO::Data::LoadAndApplyTileConnectionState(child, &(tile->PossibleConnections));
				}
				else if (hash == HashedStrings::H_Connection) {
					IO::Data::LoadAndApplyTileConnectionState(child, &(tile->Connections));
				}
				else if (hash == HashedStrings::H_PowerRequirement) {
					tile->SetPowerRequirement(IO::GetIntValue(child));
				}
			}
		}
		// Call the virtual method in this tile subclass if we have subclass-specific data to extract
		else if (skey == "classspecificdata") {
			tile->ReadClassSpecificXMLData(section);
		}
	}
}


// Static base class method to copy data for the base class portion of any tile
void ComplexShipTile::CopyBaseClassData(ComplexShipTile *source, ComplexShipTile *target)
{
	// Parameter check
	if (!source || !target) return;

	// Copy all base parameters
	target->SetElementSize(source->GetElementSize());
	target->SetElementLocation(source->GetElementLocation());
	target->SetRotation(source->GetRotation());
	target->Connections = TileConnections(source->Connections);
	target->PossibleConnections = TileConnections(source->PossibleConnections);
}

// Set the tile to use a single (specified) tile model, and perform any dependent initialisation
void ComplexShipTile::SetSingleModel(Model *model)
{
	// Shut down any compound-model data if it has previously been allocated, and set this tile to single-model mode
	if (HasCompoundModel())
	{
		m_models.Reset();
		SetHasCompoundModel(false);
	}

	// Store a reference to the model
	SetModel(model);
}

// Set the tile to use a multiple & variable-sized tile geometry.  Will deallocate any existing geometry and allocate 
// sufficient space for geometry covering the tile element size
void ComplexShipTile::SetMultipleModels(void)
{
	// Remove any reference to single-model data and set this tile to compound model mode.  Don't bother testing whether we are
	// already in this state; the branch is more costly than setting the data every time
	SetModel(NULL);
	SetHasCompoundModel(true);

	// Reset and deallocate any existing compound model data
	m_models = CompoundElementModel(GetElementSize().Convert<UINT>());

}

// Set the single model data for this tile.  Should not be used directly by callers; model should be instantiated via either
// SetSingleModel() or SetMultipleModels()
void ComplexShipTile::SetModel(Model *m) 
{
	m_model.SetModel(m);

	DetermineInstanceRenderingFlags();
}

// Update instance rendering flags based on the current tile configuration
void ComplexShipTile::DetermineInstanceRenderingFlags(void)
{
	// Shadow casting state: if we either have a non-null single model, or are just defined to have
	// multiple models (for simplicity).  Tiles will always be large enough to make shadows necessary, unless
	// they have a null model
	InstanceFlags.SetFlagState(InstanceFlags::INSTANCE_FLAG_SHADOW_CASTER, HasCompoundModel() || GetModel().GetModel() != NULL);
}

// Static method to look up a tile definition and create a new tile based upon it
ComplexShipTile * ComplexShipTile::Create(std::string code)
{
	// See whether we have a definition matching this code
	if (code == NullString || D::ComplexShipTiles.Exists(code) == false) return NULL;

	// Since we do, generate and return a new tile from the definition
	return D::ComplexShipTiles.Get(code)->CreateTile();
}

// Compiles and validates the tile based on its definition, class & associated hard-stop criteria
Result ComplexShipTile::CompileAndValidateTile(void)
{
	// Assuming we have an associated tile definition, pass control back to that component to perform build and validation
	if (m_definition) return m_definition->CompileAndValidateTile(this);

	// If we have no definition to validate against then pass by default
	return ErrorCodes::NoError;
}

// Generates the geometry for this tile.  Subset of the "CompileTile()" functionality which can
// be called separately if required
Result ComplexShipTile::GenerateGeometry(void)
{
	// Assuming we have an associated tile definition, pass control back to that component for geometry generation
	if (m_definition) return m_definition->GenerateGeometry(this);

	// If we have no definition to validate against then simply return success
	return ErrorCodes::NoError;
}

// Recalculate compound model data, including geometry-dependent calculations that are performed 
// during the post-processing load sequence
void ComplexShipTile::RecalculateCompoundModelData(void)
{
	// Make sure this tile does have compound model data
	if (m_multiplemodels)
	{
		m_models.RecalculateModelData();
	}
}

// Determines the code that should be assigned to a hardpoint owned by this tile
std::string ComplexShipTile::DetermineTileHardpointCode(Hardpoint *hardpoint) const
{
	if (!hardpoint) return NullString;
	return DetermineTileHardpointCode(hardpoint->Code);
}

// Determines the code that should be assigned to a hardpoint owned by this tile
std::string ComplexShipTile::DetermineTileHardpointCode(const std::string & hardpoint_code) const
{
	return concat(m_code)("_")(m_id)("_")(hardpoint_code).str();
}

// Methods to retrieve and set the definition associated with this tile
const ComplexShipTileDefinition *	ComplexShipTile::GetTileDefinition(void) const				{ return m_definition; }
void ComplexShipTile::SetTileDefinition(const ComplexShipTileDefinition *definition)			{ m_definition = definition; }


// Static method to create a new instance of the specified class of tile.  Creates the object only, no initialisation
ComplexShipTile * ComplexShipTile::New(D::TileClass cls)
{
	switch (cls)
	{
		case D::TileClass::Corridor:			return new CSCorridorTile();
		case D::TileClass::Quarters:			return new CSQuartersTile();
		case D::TileClass::LifeSupport:			return new CSLifeSupportTile();
		case D::TileClass::PowerGenerator:		return new CSPowerGeneratorTile();
		case D::TileClass::EngineRoom:			return new CSEngineRoomTile();
		case D::TileClass::Armour:				return new CSArmourTile();

		default:								return NULL;
	}
}

// Processes a debug tile command from the console
void ComplexShipTile::ProcessDebugTileCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros for convenience
	INIT_DEBUG_TILE_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, Parameter(2) is the already-matched tile ID, and Parameter(3) is the function name
	// We therefore pass Parameter(4) onwards as arguments

	// Accessor methods
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetClass)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetTileClassIndex)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetID)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetSimulationState)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(SimulationIsActive)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(LastSimulationTime)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetSimulationInterval)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(RequiresSimulation)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetElementLocation)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetElementPosition)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetLocalElementLocation, INTVECTOR3(command.ParameterAsInt(4), command.ParameterAsInt(5), command.ParameterAsInt(6)))
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetLocalElementIndex, INTVECTOR3(command.ParameterAsInt(4), command.ParameterAsInt(5), command.ParameterAsInt(6)))
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetElementSize)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetElementCount)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetWorldSize)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetCode)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetName)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(IsStandardTile)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(HasCompoundModel)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetRotation)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetMass)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetHardness)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetAggregateHealth)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetBoundingSphereRadius)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetRelativePosition)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetWorldMatrix)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(IsPrimaryTile)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(SpansMultipleElements)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(ConnectionsAreFixed)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetPowerLevel)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(GetPowerRequirement)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(IsPowered)
	REGISTER_DEBUG_TILE_ACCESSOR_FN(DebugString)
	
	// Mutator methods
	REGISTER_DEBUG_TILE_FN(SetSimulationState, (iObject::ObjectSimulationState)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(DeactivateSimulation)
	REGISTER_DEBUG_TILE_FN(ActivateSimulation)
	REGISTER_DEBUG_TILE_FN(SetTileSimulationRequired, command.ParameterAsBool(4))
	REGISTER_DEBUG_TILE_FN(SetSimulationInterval, (unsigned int)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(SimulateTile)
	REGISTER_DEBUG_TILE_FN(ApplyTile)
	REGISTER_DEBUG_TILE_FN(SetElementLocation, INTVECTOR3(command.ParameterAsInt(4), command.ParameterAsInt(5), command.ParameterAsInt(6)))
	REGISTER_DEBUG_TILE_FN(SetElementSize, INTVECTOR3(command.ParameterAsInt(4), command.ParameterAsInt(5), command.ParameterAsInt(6)))
	REGISTER_DEBUG_TILE_FN(SetCode, command.Parameter(4))
	REGISTER_DEBUG_TILE_FN(SetName, command.Parameter(4))
	REGISTER_DEBUG_TILE_FN(SetStandardTile, command.ParameterAsBool(4))
	REGISTER_DEBUG_TILE_FN(PerformRenderUpdate)
	REGISTER_DEBUG_TILE_FN(SetTileClass, (D::TileClass)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(SetRotation, (Rotation90Degree)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(Rotate, (Rotation90Degree)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(RotateAllTerrainObjects, (Rotation90Degree)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(SetMass, command.ParameterAsFloat(4))
	REGISTER_DEBUG_TILE_FN(SetHardness, command.ParameterAsFloat(4))
	REGISTER_DEBUG_TILE_FN(ElementHealthChanged)
	REGISTER_DEBUG_TILE_FN(StartConstruction)
	REGISTER_DEBUG_TILE_FN(ConstructionComplete)
	REGISTER_DEBUG_TILE_FN(RecalculateTileData)
	REGISTER_DEBUG_TILE_FN(RecalculateBoundingVolume)
	REGISTER_DEBUG_TILE_FN(RecalculateCompoundModelData)
	REGISTER_DEBUG_TILE_FN(RecalculateWorldMatrix)
	REGISTER_DEBUG_TILE_FN(CompileAndValidateTile)
	REGISTER_DEBUG_TILE_FN(GenerateGeometry)
	REGISTER_DEBUG_TILE_FN(FixConnections, command.ParameterAsBool(4))
	REGISTER_DEBUG_TILE_FN(SetPowerLevel, (Power::Type)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(SetPowerRequirement, (Power::Type)command.ParameterAsInt(4))
	REGISTER_DEBUG_TILE_FN(DestroyObject)
	REGISTER_DEBUG_TILE_FN(DestroyAllOwnedTerrain)
}



