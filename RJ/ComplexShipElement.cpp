#include "FastMath.h"
#include "Utility.h"
#include "ErrorCodes.h"
#include "ComplexShipTile.h"
#include "iSpaceObjectEnvironment.h"
#include "Damage.h"

#include "ComplexShipElement.h"

#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
	long ComplexShipElement::inst_con = 0;
	long ComplexShipElement::inst_des = 0;
#endif

// Static variables
std::vector<bool> ComplexShipElement::DefaultPropertyValues;

ComplexShipElement::ComplexShipElement(void)
{
	// Debug purposes: record the number of elements being created
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_con;
	#endif

	// Initialise key values & pointers to default values
	m_parent = NULL;
	m_properties = ComplexShipElement::DefaultPropertyValues;
	m_simulationstate = iObject::ObjectSimulationState::NoSimulation;
	m_health = 1.0f;
	m_hasobjects = m_hasterrain = false;

	// No links to neighbouring elements to begin with
	m_left = m_up = m_right = m_down = m_zup = m_zdown = NULL;
	
	// No walkable connections by default either
	m_cleft = m_cup = m_cright = m_cdown = m_cupleft = m_cupright = m_cdownright = m_cdownleft = m_czup = m_czdown = false;

	// No nav node positions or connections are specified by default
	m_navnodepositions = NULL; m_navnodeconnections = NULL;
	m_numnavnodepositions = 0; m_numnavnodeconnections = NULL;
}

// Method to handle the addition of a ship tile to this object
void ComplexShipElement::ShipTileAdded(ComplexShipTile *tile)
{
	// Simply perform a full update of the element based on all our attached tiles (which will include the newly-added tile)
	UpdateElementBasedOnTiles();
}

// Method to handle the removal of a ship tile from this object
void ComplexShipElement::ShipTileRemoved(ComplexShipTile *tile)
{
	// Simply perform a full update of the element based on all our attached tiles (which will now not included the removed tile)
	UpdateElementBasedOnTiles();
}

// Updates this element based on all attached tiles
void ComplexShipElement::UpdateElementBasedOnTiles(void)
{
	/*** Step 1: Reset any properties of the element that are determined by the attached tiles ***/

	// Reset all properties of this element to the default
	m_properties = vector<bool>(ComplexShipElement::PROPERTY::PROPERTY_COUNT, false);
	m_damagemodifiers.clear();

	// (Reset any other properties/fields of the element that will be determined by the attached tiles)

	/*** Step 2: Apply each tile in turn to build up the final element state ***/

	// Iterate over each tile in turn
	iContainsComplexShipTiles::ComplexShipTileCollection::const_iterator it_end = m_tiles[0].end();
	for (iContainsComplexShipTiles::ComplexShipTileCollection::const_iterator it = m_tiles[0].begin(); it != it_end; ++it)
	{
		// Apply the tile to this element
		(*it)->ApplyTile(this);
	}
}

// Adds a damage modifier to this element
void ComplexShipElement::AddDamageModifier(Damage modifier)
{
	// If we already have a modifier for this damage type then locate it and apply the new modifier.  All
	// modifiers are multipliers so this is commutative and can be pre-calculated
	DamageSet::size_type n = m_damagemodifiers.size();
	for (DamageSet::size_type i = 0; i < n; ++i)
	{
		if (m_damagemodifiers[i].Type == modifier.Type)
		{
			m_damagemodifiers[i].Amount *= modifier.Amount;
			return;
		}
	}

	// If we don't currently have a modifier for this type then add one now
	m_damagemodifiers.push_back(modifier);
}

// Returns the damage modifier for this damage type, or 1.0 if no explicit modifier exists
float ComplexShipElement::GetDamageModifier(DamageType type)
{
	DamageSet::const_iterator dam_end = m_damagemodifiers.end();
	for (DamageSet::const_iterator dam = m_damagemodifiers.begin(); dam != dam_end; ++dam)
		if (dam->Type == type) 
			return dam->Amount;

	return 1.0f;
}

ComplexShipElement::~ComplexShipElement(void)
{
	// Debug purposes: record the number of elements being deallocated
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_des;
	#endif

	// Clear the set of attach point data
	m_attachpoints.clear();

	// Clear the properties vector
	m_properties.clear();
	vector<bool>().swap(m_properties);

	// Deallocate the memory assigned for nav point position & connection data
	free(m_navnodepositions); m_navnodepositions = NULL; m_numnavnodepositions = 0;
	free(m_navnodeconnections); m_navnodeconnections = NULL; m_numnavnodeconnections = 0;
}

// Performs a full deallocation (if necessary), allocation and initialisation of an element space
Result ComplexShipElement::CreateElementSpace(iSpaceObjectEnvironment *parent, ComplexShipElement ****elements, INTVECTOR3 elementsize)
{
	Result result;

	// If we already have element storage allocated then dispose of it first.  NOTE that this
	// relies on the elementsize values being consistent since the time this storage was allocated.
	// If they are not then this could result in memory leak or attempts to deallocate past allocated space
	if (elements && (*elements)) 
	{
		result = ComplexShipElement::DeallocateElementStorage(elements, elementsize);
		if (result != ErrorCodes::NoError) return result;
	}

	// Allocate memory space for the three-dimensional matrix of elements
	result = ComplexShipElement::AllocateElementStorage(elements, elementsize);
	if (result != ErrorCodes::NoError) return result;

	// Finally, initialise the elements in this new allocation so that they are ready for use
	result = ComplexShipElement::InitialiseElementStorage((*elements), elementsize, parent, NULL);
	if (result != ErrorCodes::NoError) return result;

	// Return success
	return ErrorCodes::NoError;
}

// Copies the element space of an object and generates a new one for the target that is identical
Result ComplexShipElement::CopyElementSpace(iSpaceObjectEnvironment *src, iSpaceObjectEnvironment *target)
{
	Result result;

	// Make sure we have a valid copy source and target
	if (!target || !src || !src->GetElements() || src->GetElementSizeX() < 1 || src->GetElementSizeY() < 1 || src->GetElementSizeZ() < 1) 
		return ErrorCodes::CannotCopyElementSpaceWithNullSourceOrTarget;

	// If we already have element storage allocated then dispose of it first.  NOTE that this
	// relies on the elementsize values being consistent since the time this storage was allocated.
	// If they are not then this could result in memory leak or attempts to deallocate past allocated space
	if (target->GetElements())
	{
		result = ComplexShipElement::DeallocateElementStorage(target->GetElementsPointer(), src->GetElementSize());
		if (result != ErrorCodes::NoError) return result;
	}

	// Allocate memory space for the three-dimensional matrix of elements, to match the dimensions of the copy source
	result = ComplexShipElement::AllocateElementStorage(target->GetElementsPointer(), src->GetElementSize());
	if (result != ErrorCodes::NoError) return result;

	// Finally, initialise the elements in this new allocation using the copy source to update elements
	result = ComplexShipElement::InitialiseElementStorage(target->GetElements(), src->GetElementSize(), target, src);
	if (result != ErrorCodes::NoError) return result;

	// Update the element space size in the target object
	target->SetElementSize(src->GetElementSize());

	// Return success
	return ErrorCodes::NoError;
}

// Allocates a three-dimensional space of element storage based on the parameters provided
Result ComplexShipElement::AllocateElementStorage(ComplexShipElement ****elements, INTVECTOR3 elementsize)
{
	// Now allocate space for elements in each dimension; first, x
	(*elements) = new (nothrow) ComplexShipElement**[elementsize.x];
	if (!(*elements)) return ErrorCodes::CouldNotAllocateStorageForCShipElementSpace;

	// Allocate space for the y and z dimensions within each x element
	for (int x=0; x<elementsize.x; x++)
	{
		// First allocate a y dimension within each x item
		(*elements)[x] = new (nothrow) ComplexShipElement*[elementsize.y];
		if (!(*elements)[x]) return ErrorCodes::CouldNotAllocateStorageForCShipElementSpace;

		// Now allocate a z dimension within each of those xy dimensions
		for (int y=0; y<elementsize.y; y++)
		{
			(*elements)[x][y] = new (nothrow) ComplexShipElement[elementsize.z];
			if (!(*elements)[x][y]) return ErrorCodes::CouldNotAllocateStorageForCShipElementSpace;
		}
	}	

	// Return success
	return ErrorCodes::NoError;
}


// Initialises a three-dimensional space of elements, including locations details and links to neighbouring elements.  Will
// also initialise with data from an existing element space if required (used when resizing an element space)
Result ComplexShipElement::InitialiseElementStorage(ComplexShipElement ***elements, INTVECTOR3 elementsize, 
													iSpaceObjectEnvironment *parent,
													iSpaceObjectEnvironment *copysource)
{
	ComplexShipElement *el, *src;
	INTVECTOR3 srcsize;

	// Sanity check to make sure we have been passed valid parameters
	if (!elements || elementsize.IsZeroVector3()) return ErrorCodes::CannotInitialiseElementSpaceWithInvalidParams;

	// Determine whether we are also copying source element data during initialisation
	bool havesource = (copysource != NULL);
	if (havesource) srcsize = copysource->GetElementSize();

	// For efficiency, generate a temporary vector of the desired upper bounds for linking neighbours
	INTVECTOR3 effbounds = INTVECTOR3(elementsize.x - 1, elementsize.y - 1, elementsize.z - 1);

	// Loop over each dimension of the element array
	for (int x = 0; x < elementsize.x; x++) {
		for (int y = 0; y < elementsize.y; y++) {
			for (int z = 0; z < elementsize.z; z++) 
			{
				// Get a reference to the element in this position
				el = &( elements[x][y][z] );
				if (!el) return ErrorCodes::EncounteredNullCSElementDuringInitialisation;

				// Set the location property of this element
				el->SetLocation(x, y, z);

				// Set a reference back to the parent of this element
				el->SetParent(parent);

				// Also link to adjacent elements, unless this element is on that edge of the element space
				// Note that default for all links is NULL so we can simply not set the edge entries
				if (x > 0)					el->LinkLeft(&(elements	[x-1][y][z]));
				if (x < effbounds.x)		el->LinkRight(&(elements[x+1][y][z]));
				if (y > 0)					el->LinkUp(&(elements	[x][y-1][z]));
				if (y < effbounds.y)		el->LinkDown(&(elements	[x][y+1][z]));
				if (z > 0)					el->LinkZUp(&(elements	[x][y][z-1]));
				if (z < effbounds.z)		el->LinkZDown(&(elements[x][y][z+1]));
				if (x > 0 && y > 0)						el->LinkUpLeft(&(elements[x-1][y-1][z]));
				if (x < effbounds.x && y > 0)			el->LinkUpRight(&(elements[x+1][y-1][z]));
				if (x > 0 && y < effbounds.y)			el->LinkDownLeft(&(elements[x-1][y+1][z]));
				if (x < effbounds.x && y < effbounds.y)	el->LinkDownRight(&(elements[x+1][y+1][z]));

				// Copy data from the source element space, if required
				if (havesource && x<srcsize.x && y<srcsize.y && z<srcsize.z) {
					src = copysource->GetElementDirect(x, y, z);
					if (src) ComplexShipElement::CopyData(src, el);
				}
			}
		}
	}

	// Return success
	return ErrorCodes::NoError;
}

// Deallocates the memory used to represent a three-dimensional space of elements.  Note that this relies on
// the elementsize parameter being consistent since the memory was allocated, otherwise this could result
// in memory leak or attempts to deallocate beyond the previously-allocated space
Result ComplexShipElement::DeallocateElementStorage(ComplexShipElement ****elements, INTVECTOR3 elementsize)
{
	// Sanity check to make sure we have been passed valid parameters
	if (!(*elements)) return ErrorCodes::NoError;

	// We must deallocate from the 'lowest' dimension (z) up to y and finally x.  Loop into each dimension in turn
	for (int x = 0; x < elementsize.x; x++) 
	{
		for (int y = 0; y < elementsize.y; y++) 
		{
			// Deallocate the z dimensions within this xy element
			delete[] (*elements)[x][y];
		}

		// Deallocate the y dimensions within this x element, now that all z dimensions have been removed
		delete[] (*elements)[x];
	}

	// Finally deallocate the x dimensions now that all y & z dimensions within them have been removed
	delete[] (*elements);
	(*elements) = NULL;

	// Return success
	return ErrorCodes::NoError;
}

// Rotates an element space by the specified angle, modifying both the element space and the element size vector
Result ComplexShipElement::RotateElementSpace(ComplexShipElement ****pElements, INTVECTOR3 *pElementsize, Rotation90Degree rot)
{
	Result result;
	INTVECTOR3 newsize;
	ComplexShipElement *src, *dest = NULL;
	ComplexShipElement ***elements, ***newspace;

	// Parameter check
	if (!pElements || !pElementsize) return ErrorCodes::CannotRotateNullElementSpace;
	elements = (*pElements);
	INTVECTOR3 &elementsize = (*pElementsize);
	if (!elements) return ErrorCodes::CannotRotateNullElementSpace;

	// Dimensions may be changing based on rotation; calculate them here
	if (rot == Rotation90Degree::Rotate0 || rot == Rotation90Degree::Rotate180)
		newsize = elementsize;
	else if (rot == Rotation90Degree::Rotate90 || rot == Rotation90Degree::Rotate270)
		newsize = INTVECTOR3(elementsize.y, elementsize.x, elementsize.z);
	else return ErrorCodes::CannotRotateElementSpaceByInvalidParameter;

	// Create and initialise a new element space
	newspace = NULL;
	result = ComplexShipElement::CreateElementSpace(NULL, &newspace, newsize);
	if (result != ErrorCodes::NoError) return ErrorCodes::ErrorWhileAllocatingRotatedElementSpace;

	// Now copy data across to the new section based on its orientation
	for (int x=0; x<elementsize.x; x++) {
		for (int y=0; y<elementsize.y; y++) {
			for (int z=0; z<elementsize.z; z++)
			{
				// Determine the source & destination elements based on relative orientation
				src = &(elements[x][y][z]);
				if (rot == Rotation90Degree::Rotate0) 
					dest = &(newspace[x][y][z]);
				else if (rot == Rotation90Degree::Rotate90)
					dest = &(newspace[(newsize.x-1)-y][x][z]);
				else if (rot == Rotation90Degree::Rotate180)
					dest = &(newspace[(elementsize.x-1)-x][(elementsize.y-1)-y][z]);
				else if (rot == Rotation90Degree::Rotate270)
					dest = &(newspace[y][(newsize.y-1)-x][z]);
					
				// Copy relevant data from source to destination element
				ComplexShipElement::CopyData(src, dest);
				dest->SetParent(src->GetParent());
			}
		}
	}

	// We can now deallocate the source element space
	ComplexShipElement::DeallocateElementStorage(pElements, elementsize);

	// Repoint the original pointer to this new block of element space, and also update the element size pointer
	(*pElements) = newspace;
	pElementsize->x = newsize.x; pElementsize->y = newsize.y; pElementsize->z = newsize.z;

	// Return success
	return ErrorCodes::NoError;
}

// Rotate all the attach points on this element by the specified angle
void ComplexShipElement::RotateElementAttachPoints(ComplexShipElement *el, Rotation90Degree rotation)
{
	// Process each attach point in turn
	AttachPointCollection *points = el->GetAttachPoints();
	std::vector<AttachPointCollection>::size_type n = el->GetAttachPointCount();
	for (std::vector<AttachPointCollection>::size_type i = 0; i < n; ++i)
	{
		// Change the edge value to reflect the rotation being applied
		points->at(i).Edge = GetRotatedDirection(points->at(i).Edge, rotation);
	}
}

// Returns the type of attachment allowed on the specified edge of this element.  Returns "NotAllowed" by default if none is specified
ComplexShipElement::AttachType ComplexShipElement::GetAttachPoint(Direction edge)
{
	// Look at each possible attach point in turn
	std::vector<AttachPointCollection>::size_type n = m_attachpoints.size();
	for (std::vector<AttachPointCollection>::size_type i = 0; i < n; ++i)
		if (m_attachpoints[i].Edge == edge) return m_attachpoints[i].Type;

	// If no attachment point is specified then we will allow no attachment by default
	return AttachType::NoneAllowed;
}

// Static method to test whether an attachment is possible from the 'elementedge' edge of 'element' into 'neighbourelement'
bool ComplexShipElement::AttachmentIsCompatible(ComplexShipElement *element, Direction elementedge,
												ComplexShipElement *neighbourelement)
{
	// If there is no active neighbouring element then we automatically pass the test
	if (!neighbourelement || !neighbourelement->IsActive()) return true;

	// However, if the element in question does not exist (shouldn't ever happen) then automatically return false
	if (!element) return false;

	// Now retrieve the attach point type on the edge of these neighbouring elements; test each to see if it does not
	// allow attachments as we go, to avoid retrieving the second attachment type if the first already disallows it
	ComplexShipElement::AttachType type1 = element->GetAttachPoint(elementedge);
	if (type1 == AttachType::NoneAllowed) return false;

	ComplexShipElement::AttachType type2 = neighbourelement->GetAttachPoint(GetRotatedDirection(elementedge, Rotation90Degree::Rotate180));
	if (type1 == AttachType::NoneAllowed) return false;

	// Otherwise test whether the types are compatible; either equal, or meet some other special criteria
	return (type1 == type2);
}

ComplexShipElement *ComplexShipElement::Copy(ComplexShipElement *element)
{
	// Debug purposes: record the number of elements being created
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_con;
	#endif

	// Make a copy of this element via copy constructor
	ComplexShipElement *e = new ComplexShipElement(*element);

	// Copy all attach points defined on this element
	//ComplexShipElement::AttachPointCollection *points = element->GetAttachPoints();
	//for (int i=0; i<points->size(); i++) e->AddAttachPoint(points->at(i));

	// The copy operation should not transfer any objects/terrain
	e->ClearAllObjects();
	e->ClearAllTerrain();

	// Return the copied element
	return e;
}

// This method copies element data from one instance to another.  It does not delete or create any objects
void ComplexShipElement::CopyData(ComplexShipElement *src, ComplexShipElement *dest)
{
	// Copy key fields from source to destination
	dest->SetHealth(src->GetHealth());

	// Copy the element properties 
	dest->SetProperties(src->GetProperties());

	// TODO: Copy damage modifiers

	// Copy all attach points defined on this element
	dest->SetAttachPoints(src->GetAttachPoints());
}

// Compares the equality of two elements
bool ComplexShipElement::Equals(ComplexShipElement *e)
{
	// Make sure we have an item for comparison
	if (!e) return false;

	// First compare equality of all element properties
	for (int i=0; i<ComplexShipElement::PROPERTY::PROPERTY_COUNT; i++)
		if (m_properties[i] != e->GetProperty(i)) return false;

	// Compare the attach points on these elements; make sure we have the same number
	AttachPointCollection::size_type apcount = e->GetAttachPointCount();
	if (m_attachpoints.size() != apcount) return false;

	// Now make sure all attach points are equal
	ComplexShipElement::AttachPointCollection *apoints = e->GetAttachPoints();
	for (AttachPointCollection::size_type i = 0; i < apcount; ++i)
		if (apoints->at(i).Edge != m_attachpoints[i].Edge || apoints->at(i).Type != m_attachpoints[i].Type)
			return false;

	// Check health & constructed state; allow a tiny (epsilon) margin for floating point diffs
	if (fast_abs(e->GetHealth() - m_health) > Game::C_EPSILON) return false;

	// We have passed all checks so return true to show these elements are identical
	return true;
}

// Sets the simulation state associated with this element, and updates any object that is currently within it
void ComplexShipElement::SetSimulationState(iObject::ObjectSimulationState state)
{
	// We only need to take action if this state is different to our current one
	if (state != m_simulationstate)
	{
		// Store the new simulation state for this element
		m_simulationstate = state;

		// TODO: the logic below may be redundant / duplicative if applied by multiple elements to one object/tile that is linked to >1 element

		// Iterate through all the objects in this element
		std::vector<iEnvironmentObject*>::iterator it_end = Objects.end();
		for (std::vector<iEnvironmentObject*>::iterator it = Objects.begin(); it != it_end; ++it)
		{
			// Have the object update its own state from its parent elements (of which this is one)
			if ((*it)) (*it)->UpdateSimulationStateFromParentElements();
		}

		// Iterate through any tiles linked to this element and apply the same state
		iContainsComplexShipTiles::ComplexShipTileCollection::iterator t_it_end = m_tiles[0].end();
		for (iContainsComplexShipTiles::ComplexShipTileCollection::iterator t_it = m_tiles[0].begin(); t_it != t_it_end; ++t_it)
		{
			// Set the new simulation state for this tile
			if ((*t_it) && (*t_it)->GetSimulationState() != state) 
				(*t_it)->UpdateSimulationStateFromParentElements();
		}
	}
}


iSpaceObjectEnvironment *ComplexShipElement::GetParent(void)
{
	return m_parent;
}

void ComplexShipElement::SetParent(iSpaceObjectEnvironment *parent)
{
	m_parent = parent;
}

bool ComplexShipElement::ConnectsInDirection(Direction direction)
{
	switch (direction)
	{
		case Direction::Left:		return m_cleft;
		case Direction::Up:			return m_cup;
		case Direction::Right:		return m_cright;
		case Direction::Down:		return m_cdown;
		case Direction::UpLeft:		return m_cupleft;
		case Direction::UpRight:	return m_cupright; 
		case Direction::DownRight:	return m_cdownright; 
		case Direction::DownLeft:	return m_cdownleft;
		case Direction::ZUp:		return m_czup;
		case Direction::ZDown:		return m_czdown;
		default:					return false;
	}
}

void ComplexShipElement::SetConnectionInDirection(Direction direction, bool connection)
{
	switch (direction)
	{
		case Direction::Left:		m_cleft = connection; break;
		case Direction::Up:			m_cup = connection; break;
		case Direction::Right:		m_cright = connection; break;
		case Direction::Down:		m_cdown = connection; break;
		case Direction::UpLeft:		m_cupleft = connection; break;
		case Direction::UpRight:	m_cupright = connection; break;
		case Direction::DownRight:	m_cdownright = connection; break;
		case Direction::DownLeft:	m_cdownleft = connection; break;
		case Direction::ZUp:		m_czup = connection; break;
		case Direction::ZDown:		m_czdown = connection; break;
		default:					return;
	}
}

ComplexShipElement * ComplexShipElement::GetNeighbour(Direction direction)
{
	switch (direction)
	{
		case Direction::Left:		return m_left;
		case Direction::Up:			return m_up;
		case Direction::Right:		return m_right;
		case Direction::Down:		return m_down;
		case Direction::UpLeft:		return m_upleft;
		case Direction::UpRight:	return m_upright; 
		case Direction::DownLeft:	return m_downleft;
		case Direction::DownRight:	return m_downright;
		case Direction::ZUp:		return m_zup;
		case Direction::ZDown:		return m_zdown;
		default:					return NULL;
	}
}

void ComplexShipElement::AllocateNavPointPositionData(int n)
{
	// Deallocate any memory that has already been allocated
	if (m_navnodepositions) { free(m_navnodepositions); m_navnodepositions = NULL; }

	// Validate the parameter; n = number of nav point positions to allocate
	if (n <= 0) return;

	// Allocate space for the nav point data
	m_navnodepositions = (ComplexShipElement::NavNodePos*)malloc(sizeof(ComplexShipElement::NavNodePos) * n);
	m_numnavnodepositions = n;
}

void ComplexShipElement::AllocateNavPointConnectionData(int n)
{
	// Deallocate any memory that has already been allocated
	if (m_navnodeconnections) { free(m_navnodeconnections); m_navnodeconnections = NULL; }

	// Validate the parameter; n = number of nav point connections to allocate
	if (n <= 0) return;

	// Allocate space for the nav point connection data
	m_navnodeconnections = (ComplexShipElement::NavNodeConnection*)malloc(sizeof(ComplexShipElement::NavNodeConnection) * n);
	m_numnavnodeconnections = n;
}

string ComplexShipElement::AttachTypeToString(ComplexShipElement::AttachType type)
{
	switch (type) {
		case ComplexShipElement::AttachType::Standard:				return "standard";
		case ComplexShipElement::AttachType::TurretModule:			return "turret";
		default:													return "disallowed";
	}
}
ComplexShipElement::AttachType ComplexShipElement::AttachTypeFromString(string type)
{
	// All comparisons are case-insensitive
	string s = StrLower(type);
	if		(s == "standard")	return ComplexShipElement::AttachType::Standard;
	else if	(s == "turret")		return ComplexShipElement::AttachType::TurretModule;
	else							return ComplexShipElement::AttachType::NoneAllowed;
}

// Static method to convert from a string property name to the property itself
string ComplexShipElement::TranslatePropertyToName(ComplexShipElement::PROPERTY prop)
{
	switch (prop)
	{
		case ComplexShipElement::PROPERTY::PROP_ACTIVE:				return "active";
		case ComplexShipElement::PROPERTY::PROP_BUILDABLE:			return "buildable";
		case ComplexShipElement::PROPERTY::PROP_WALKABLE:			return "walkable";
		case ComplexShipElement::PROPERTY::PROP_POWER_CABLES:		return "powercables";
		default:													return "";
	}
}

// Static method to convert from element properties to their string name representation
ComplexShipElement::PROPERTY ComplexShipElement::TranslatePropertyFromName(string name)
{
	// All comparisons are case-insensitive
	string s = StrLower(name);

	if		(s == "active")				return ComplexShipElement::PROPERTY::PROP_ACTIVE;	
	else if (s == "buildable")			return ComplexShipElement::PROPERTY::PROP_BUILDABLE;
	else if (s == "walkable")			return ComplexShipElement::PROPERTY::PROP_WALKABLE;
	else if (s == "powercables")		return ComplexShipElement::PROPERTY::PROP_POWER_CABLES;
	else								return ComplexShipElement::PROPERTY::PROP_UNKNOWN;
}

// Returns the coordinates of a point on the specified edge of an element
XMFLOAT3 ComplexShipElement::GetEdgePosition(Direction direction)
{
	static const float midpoint = Game::C_CS_ELEMENT_SCALE / 2.0f;

	// Return a different position depending on the edge specified
	switch (direction)
	{
		case Direction::Left:			return XMFLOAT3(0, 0, midpoint);
		case Direction::Up:				return XMFLOAT3(midpoint, 0, 0);
		case Direction::Right:			return XMFLOAT3(Game::C_CS_ELEMENT_SCALE, 0, midpoint);
		case Direction::Down:			return XMFLOAT3(midpoint, 0, Game::C_CS_ELEMENT_SCALE);
		case Direction::UpLeft:			return XMFLOAT3(0, 0, 0);
		case Direction::UpRight:		return XMFLOAT3(Game::C_CS_ELEMENT_SCALE, 0, 0);
		case Direction::DownRight:		return XMFLOAT3(Game::C_CS_ELEMENT_SCALE, 0, Game::C_CS_ELEMENT_SCALE);
		case Direction::DownLeft:		return XMFLOAT3(0, 0, Game::C_CS_ELEMENT_SCALE);
		case Direction::ZUp:			return XMFLOAT3(midpoint, Game::C_CS_ELEMENT_SCALE, midpoint);
		case Direction::ZDown:			return XMFLOAT3(midpoint, 0, midpoint);
		default:						return XMFLOAT3(midpoint, midpoint, midpoint);
	}
}

// Returns the coordinates of the point on the specified edge of an element
XMFLOAT3 ComplexShipElement::GetAdjacentElementCentrePosition(Direction direction)
{
	static const float midpoint = Game::C_CS_ELEMENT_SCALE / 2.0f;

	// Return a different position depending on the direction specified
	switch (direction)
	{
		case Direction::Left:			return XMFLOAT3(-midpoint, 0, midpoint);
		case Direction::Up:				return XMFLOAT3(midpoint, 0, -midpoint);
		case Direction::Right:			return XMFLOAT3(Game::C_CS_ELEMENT_SCALE + midpoint, 0, midpoint);
		case Direction::Down:			return XMFLOAT3(midpoint, 0, Game::C_CS_ELEMENT_SCALE + midpoint);
		case Direction::ZUp:			return XMFLOAT3(midpoint, Game::C_CS_ELEMENT_SCALE + midpoint, midpoint);
		case Direction::ZDown:			return XMFLOAT3(midpoint, -midpoint, midpoint);
		default:						return XMFLOAT3(midpoint, midpoint, midpoint);	// Note: this doesn't mean much for an adjacent element, would cause issues
	}
}

// Determines the number of walkable connections from this element
int ComplexShipElement::DetermineNumberOfWalkableConnections(void)
{
	return (m_cleft ? 1 : 0) + (m_cup ? 1 : 0) + (m_cright ? 1 : 0) + 
		   (m_cdown ? 1 : 0) + (m_czup ? 1 : 0) + (m_czdown ? 1 : 0) + 
		   (m_cupleft ? 1 : 0) + (m_cupright ? 1 : 0) + 
		   (m_cdownleft ? 1 : 0) + (m_cdownright ? 1 : 0);
}

// Method to initialise static data, for use in any static runtime calls
void ComplexShipElement::InitialiseStaticData(void)
{
	// Initialise a vector of default property values.  Initial value is 'false' unless specified otherwise
	ComplexShipElement::DefaultPropertyValues = vector<bool>(ComplexShipElement::PROPERTY::PROPERTY_COUNT, false);

	// Specify a default for any properties that do not default to 'false'
	// e.g. ComplexShipElement::DefaultPropertyValues[ComplexShipElement::PROPERTY::______] = true;
}



