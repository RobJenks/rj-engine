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
#include "CSCorridorTile.h"
#include "CSQuartersTile.h"
#include "CSLifeSupportTile.h"
#include "ComplexShipTile.h"
#include "Damage.h"
#include "ProductionProgress.h"
#include "ProductionCost.h"
#include "FadeEffect.h"

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
	m_model = NULL;
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
	m_worldmatrix = ID_MATRIX;
	m_definition = NULL;
	m_classtype = D::TileClass::Unknown;
	m_aggregatehealth = 1.0f;
	m_constructedstate = new ProductionCost();
	m_elementconstructedstate = NULL;
	m_constructedstate_previousallocatedsize = NULL_INTVECTOR3;

	// By default, tiles will not be simulated
	m_requiressimulation = false;
	m_simulationinterval = 1000U;
	m_lastsimulation = 0U;
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
	m_boundingbox = BoundingObject::Copy(C.GetBoundingObject());
	m_multiplemodels = C.HasCompoundModel();
	m_models.CopyFrom(C.GetCompoundModelSet());
	DefaultProperties = C.DefaultProperties;

	// We will copy construction requirements; however construction progress is always reset to 0% or 100% (in clone method) depending on the parent
	// Per-element construction state can therefore always be initialised to NULL
	if (C.GetConstructedStateConst())
		m_constructedstate = C.GetConstructedStateConst()->CreateClone();
	else
		m_constructedstate = new ProductionCost();
	m_elementconstructedstate = NULL;

	// Set the element size of this tile, which recalculates relevant derived fields
	SetElementSize(C.GetElementSize());

	// We always reset the 'standard' flag to false on copy, since only a small subset of tiles in the global collection
	// should ever be flagged as standard
	m_standardtile = false;

	// Set parent pointer to NULL since the copied element will likely not share the same parent
	m_parent = NULL;

	// Copy the simulation parameters of our copy source, and treat the new object as having just been simulated
	m_requiressimulation = C.SimulationIsActive();
	m_simulationinterval = C.GetSimulationInterval();
	m_lastsimulation = Game::ClockMs;

	// Copy the connection data for this tile 
	//for (int i = C.GetConnectionCount() - 1; i >= 0; i--)
	for (ElementConnectionSet::size_type i = 0; i < C.GetConnectionCount(); ++i)
		m_connections.push_back(C.GetConnection(i));

	// Recalculate tile data, relative position and world matrix based on this new data
	RecalculateTileData();
}


// Recalculates the state of the tile following a change to its position/size etc.  Includes recalc of the world matrix and bounding volume
void ComplexShipTile::RecalculateTileData(void)
{
	// Update the flag that indicates whether this tile spans multiple elements (for use at render-time)
	m_multielement = (m_elementsize.x > 1 || m_elementsize.y > 1 || m_elementsize.z > 1);

	// Recalculate the world matrix for this tile based on its current position
	RecalculateWorldMatrix();

	// Also recalculate the tile bounding box
	RecalculateBoundingVolume();
}

void ComplexShipTile::RecalculateWorldMatrix(void)
{ 
	// Make sure that we have model data, for either single- or compound-model mode.  If not, simply set WM = ID
	if ((!m_multiplemodels && !m_model) || (m_multiplemodels && !m_models.AllocationPerformed()))
	{
		m_relativeposition = NULL_VECTOR;
		m_worldmatrix = ID_MATRIX;
		return;
	}

	// Tcentre = model translation to its centre point, prior to rotation about its centre
	XMVECTOR centrepoint = (m_multiplemodels ? m_models.CompoundModelCentre : XMLoadFloat3(&m_model->GetModelCentre()));
	
	// Telement = translation from model centre to top-left corner (0.5*numelements*elementsize in world coords) + translation to element position
	// Will switch y and z coordinates since we are moving from element to world space
	// m_relativeposition = D3DXVECTOR3((float)(m_elementsize.x * Game::C_CS_ELEMENT_MIDPOINT) + Game::ElementLocationToPhysicalPosition(m_elementlocation.x), ...
	m_relativeposition = XMVectorAdd(XMVectorMultiply(m_worldsize, HALF_VECTOR), m_elementposition);

	// Multiply the matrices together to determine the final tile world matrix
	// World = (CentreTrans * Rotation * ElementPosTranslation)
	m_worldmatrix = XMMatrixMultiply(XMMatrixMultiply(
		XMMatrixTranslationFromVector(XMVectorNegate(centrepoint)),		// Translate to centre point
		GetRotationMatrix(m_rotation)),									// Rotate about centre
		XMMatrixTranslationFromVector(m_relativeposition));				// Translate centre point to centre point of element location
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
		float tmp = size.x; size.x = size.z; size.z = tmp;
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

// Event generated before the tile is added to an environment
void ComplexShipTile::BeforeAddedToEnvironment(iSpaceObjectEnvironment *environment)
{
	// Clear all terrain object links; terrain associated with this tile will be recreated
	// and added to the environment when the tile is added
	ClearTerrainObjectLinks();

	// Store a pointer to the new environment (this may be NULL, if not being assigned anywhere)
	m_parent = environment;
}

// Event generated after the tile is added to an environment
void ComplexShipTile::AfterAddedToEnvironment(iSpaceObjectEnvironment *environment)
{

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

	// Set the element pointer to this tile
	el->SetTile(this);

	// Set element properties
	el->SetProperties(DefaultProperties);

	// See if there are any connection values specified for this element, and apply them if there are
	INTVECTOR3 tgt = (el->GetLocation() - m_elementlocation);
	el->SetConnectionState(0);
	ElementConnectionSet::const_iterator it_end = m_connections.end();
	for (ElementConnectionSet::const_iterator it = m_connections.begin(); it != it_end; ++it)
	{
		if ((*it).Location == tgt) el->SetConnectionStateInDirection(DirectionToBS((*it).Connection), true);
	}
}


// Returns the index of a connection matching the supplied criteria, or -1 if such a connection does not exist
int ComplexShipTile::GetConnection(INTVECTOR3 loc, Direction dir) const
{
	int n = (int)m_connections.size();
	for (int i = 0; i < n; ++i)
	{
		if (m_connections[i].Location == loc && m_connections[i].Connection == dir) return i;
	}
	return -1;
}

// Returns the index of a connection matching the supplied criteria, or -1 if such a connection does not exist
int ComplexShipTile::GetConnection(INTVECTOR3 loc) const
{
	int n = (int)m_connections.size();
	for (int i = 0; i < n; ++i)
	{
		if (m_connections[i].Location == loc) return i;
	}
	return -1;
}

// Adds a new connection at the specified location & in the specified direction
void ComplexShipTile::AddConnection(INTVECTOR3 loc, Direction dir)
{
	int index = GetConnection(loc, dir);
	if (index == -1) m_connections.push_back(ElementConnection(loc, dir));
}

// Removes the connection at the specified index
void ComplexShipTile::RemoveConnection(ElementConnectionSet::size_type index)
{
	// Make sure the supplied index is valid
	ElementConnectionSet::size_type size = m_connections.size();
	if (index < 0 || index >= size) return;

	// If it is, perform a swap & pop to remove the item efficiently
	std::swap(m_connections[index], m_connections[size-1]);
	m_connections.pop_back();
}

// Removes the connection matching the supplied criteria, if such a connection exists
void ComplexShipTile::RemoveConnection(INTVECTOR3 loc, Direction dir)
{
	RemoveConnection(GetConnection(loc, dir));
}

// Copies a set of source connection data into this tile
void ComplexShipTile::SetConnections(const ElementConnectionSet &source)
{
	// Clear our current connection set
	m_connections.clear();

	// Copy all elements from source
	m_connections = source;
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
	if (size.x <= 0 || size.y <= 0 || size.z <= 0) return;

	// Store the new element size
	m_elementsize = size; 

	// Determine the world size of this tile based on the element size
	m_worldsize = Game::ElementLocationToPhysicalPosition(m_elementsize);

	// Determine the approximate radius of a bounding sphere that encompasses this tile
	m_bounding_radius = GetElementBoundingSphereRadius(max(max(m_elementsize.x, m_elementsize.y), m_elementsize.z));

	// Determine the centre point of this tile in world space
	m_centre_point = XMVectorScale(Game::ElementLocationToPhysicalPosition(m_elementsize), 0.5f);

	// Recalculate properties derived from the element size
	RecalculateTileData(); 

	// Reset the tile to be 0% constructed and start the construction process
	m_constructedstate->ResetToZeroPcProgress();
	StartConstruction();
}

// Clears all terrain object links.  If a tile definition exists for this tile, the method also reserves
// space for the expected number of terrain objects that will be linked to the tile upon addition to an environment
void ComplexShipTile::ClearTerrainObjectLinks(void)
{
	m_terrain_ids.clear();
	if (m_definition)
	{
		// Reserve space for the number of terrain objects in our terrain definition, if applicable
		std::vector<StaticTerrain*>::size_type n = m_definition->GetTerrainObjectCount();
		if (n > 0) m_terrain_ids.reserve(n);
	}
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
	// If the health of an underlying element changes we want to recalculate the aggregate health of this tile
	RecalculateAggregateHealth();
}

void ComplexShipTile::RecalculateAggregateHealth(void)
{
	ComplexShipElement *el;

	// Before we start, make sure the ship & element size are valid to avoid any errors later
	if (!m_parent || m_elementsize.x == 0 || m_elementsize.y == 0 || m_elementsize.z == 0) return;

	// We will build up aggregate construction state by first summing the state of every element
	m_aggregatehealth = 0.0f;

	// Loop over each dimension in turn
	for (int x = 0; x < m_elementsize.x; x++) {
		for (int y = 0; y < m_elementsize.y; y++) {
			for (int z = 0; z < m_elementsize.z; z++)
			{
				el = m_parent->GetElement(x, y, z);
				if (el) m_aggregatehealth += el->GetHealth();
			}
		}
	}

	// Divide through by the number of elements to get an average aggregate value
	m_aggregatehealth /= (m_elementsize.x * m_elementsize.y * m_elementsize.z);
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
	if (m_constructedstate) SafeDelete(m_constructedstate);

	// Dispose of all per-element construction state objects, if they exist
	DeallocatePerElementConstructionProgress();

}


// Static method to determine whether a given tileclass is 'infrastructural'
bool ComplexShipTile::IsInfrastructureTile(D::TileClass tileclass)
{
	return ( (tileclass == D::TileClass::Power) || (tileclass == D::TileClass::Coolant) );
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
	IO::Data::LinkIntVector3AttrXMLElement("Location", tile->GetElementLocation(), node);
	IO::Data::LinkIntVector3AttrXMLElement("Size", tile->GetElementSize(), node);
	//if (tile->GetModel()) IO::Data::LinkStringXMLElement("Model", tile->GetModel()->GetCode(), node);
	IO::Data::LinkIntegerXMLElement("Rotation", (int)tile->GetRotation(), node);

	// Add an entry for each connection point on this tile
	ElementConnectionSet::const_iterator it_end = tile->GetConnectionIteratorEnd();
	for (ElementConnectionSet::const_iterator it = tile->GetConnectionIteratorStart(); it != it_end; ++it)
	{
		TiXmlElement *conn = new TiXmlElement("ConnectionPoint");
		conn->SetAttribute("x", (*it).Location.x);
		conn->SetAttribute("y", (*it).Location.y);
		conn->SetAttribute("z", (*it).Location.z);
		conn->SetAttribute("Direction", DirectionToString((*it).Connection).c_str());
		node->LinkEndChild(conn);
	}
	
	// Return the node containing all data on this base class
	return node;
}

// Static base class method to read XML data for the base class portion of any tile
void ComplexShipTile::ReadBaseClassXML(TiXmlElement *node, ComplexShipTile *tile)
{
	string skey, key, val;

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

				if (key == "location") {										
					tile->SetElementLocation( IO::GetInt3CoordinatesFromAttr(child) );
				}
				else if (key == "size") {
					tile->SetElementSize( IO::GetInt3CoordinatesFromAttr(child) );
				}
				else if (key == "rotation") {
					val = child->GetText();
					int rot = atoi(val.c_str());
					tile->SetRotation((Rotation90Degree)rot);
				}
				else if (key == "connectionpoint") {
					INTVECTOR3 loc = INTVECTOR3();
					child->Attribute("x", &loc.x);
					child->Attribute("y", &loc.y);
					child->Attribute("z", &loc.z);
					const char *dir = child->Attribute("Direction");
					if (dir) {
						Direction d = DirectionFromString(dir);
						tile->AddConnection(loc, d);
					}
				}
			}
		}
		// Call the virtual method in this tile subclass if we have subclass-specific data to extract
		else if (skey == "classspecificdata") {
			tile->ReadClassSpecificXMLData(section);
		}
	}
}

// Static method to look up a tile definition and create a new tile based upon it
ComplexShipTile * ComplexShipTile::Create(string code)
{
	// See whether we have a definition matching this code
	if (code == NullString || D::ComplexShipTiles.Exists(code) == false) return NULL;

	// Since we do, generate and return a new tile from the definition
	return D::ComplexShipTiles.Get(code)->CreateTile();
}

// Compiles the tile based on its definition
Result ComplexShipTile::CompileTile(void)
{
	// Assuming we have a tile definition, compile against it and return the result
	if (m_definition) return m_definition->CompileTile(this);
	
	// If no definition exists then simply return success
	return ErrorCodes::NoError;
}

// Validates the tile against its hard-stop requirements and returns the result
Result ComplexShipTile::ValidateHardStopRequirements(void)
{
	// Assuming we have an associated tile definition, pass control back to that component to perform validation
	if (m_definition) return m_definition->ValidateTileHardStop(this);
	
	// If we have no definition to validate against then pass by default
	return ErrorCodes::NoError;
}

// Compiles and validates the tile based on its definition, class & associated hard-stop criteria
Result ComplexShipTile::CompileAndValidateTile(void)
{
	// Assuming we have an associated tile definition, pass control back to that component to perform build and validation
	if (m_definition) return m_definition->CompileAndValidateTile(this);

	// If we have no definition to validate against then pass by default
	return ErrorCodes::NoError;
}

// Methods to retrieve and set the definition associated with this tile
ComplexShipTileDefinition *	ComplexShipTile::GetTileDefinition(void) const		{ return m_definition; }
void ComplexShipTile::SetTileDefinition(ComplexShipTileDefinition *definition)	{ m_definition = definition; }

// Static method to create a new instance of the specified class of tile.  Creates the object only, no initialisation
ComplexShipTile * ComplexShipTile::New(D::TileClass cls)
{
	switch (cls)
	{
		case D::TileClass::Corridor:			return new CSCorridorTile();
		case D::TileClass::Quarters:			return new CSQuartersTile();
		case D::TileClass::LifeSupport:			return new CSLifeSupportTile();

		default:								return NULL;
	}
}

