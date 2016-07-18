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
const bitstring ComplexShipElement::DefaultPropertyValues = 0U;

// Default constructor
ComplexShipElement::ComplexShipElement(void)
{
	// Debug purposes: record the number of elements being created
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_con;
	#endif

	// Initialise key values & pointers to default values
	m_id = -1;
	m_location = NULL_INTVECTOR3;
	m_parent = NULL;
	m_properties = ComplexShipElement::DefaultPropertyValues;
	m_tile = NULL;
	m_health = 1.0f;
	m_gravity = 0.0f;

	// No attach points by default
	memset(m_attachpoints, 0, sizeof(bitstring) * ComplexShipElement::AttachType::_AttachTypeCount);

	// No connections or adjacency info by default
	m_connections = 0U;
	memset(m_adj, 0, sizeof(ComplexShipElement*) * Direction::_Count);

	// No nav node positions or connections are specified by default
	m_navnodepositions = NULL; m_navnodeconnections = NULL;
	m_numnavnodepositions = 0; m_numnavnodeconnections = 0;
}

// Default desctructor
ComplexShipElement::~ComplexShipElement(void)
{
	// Debug purposes: record the number of elements being deallocated
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_des;
	#endif

	// Deallocate the memory assigned for nav point position & connection data
	SafeFree(m_navnodepositions); m_numnavnodepositions = 0;
	SafeFree(m_navnodeconnections); m_numnavnodeconnections = 0;
}


// Rotates the element by the specified angle
void ComplexShipElement::RotateElement(Rotation90Degree rotation)
{
	// Trivial case
	if (rotation == Rotation90Degree::Rotate0) return;

	// Rotate all attach points
	for (int i = 0; i < (int)ComplexShipElement::AttachType::_AttachTypeCount; ++i)
	{
		m_attachpoints[i] = RotateDirectionBitString(m_attachpoints[i], rotation);
	}

	// Rotate the connection data
	RotateDirectionBitString(m_connections, rotation);

	// Update element links
	int adj[Direction::_Count];
	memcpy(adj, m_adj, sizeof(int) * Direction::_Count);
	for (int i = 0; i < Direction::_Count; ++i)
	{
		m_adj[i] = adj[GetRotatedDirection((Direction)i, rotation)];
	}

	// Update nav node positions & connections

}

// Static method to test whether an attachment is possible from the 'elementedge' edge of 'element' into 'neighbourelement'
bool ComplexShipElement::AttachmentIsCompatible(ComplexShipElement *element, DirectionBS elementedge,
												ComplexShipElement *neighbourelement)
{
	// If the element in question does not exist (shouldn't ever happen) then automatically return false
	if (!element) return false;
	
	// If there is no active neighbouring element then we automatically pass the test
	if (!neighbourelement || !neighbourelement->IsActive()) return true;

	// Test whether the elements have compatible attach points at their intersection point
	for (int i = 0; i < (int)ComplexShipElement::AttachType::_AttachTypeCount; ++i)
	{
		if (element->HasAttachPoints((ComplexShipElement::AttachType)i, elementedge) &&
			neighbourelement->HasAttachPoints(	(ComplexShipElement::AttachType)i,
												GetRotatedBSDirection(elementedge, Rotation90Degree::Rotate180)))
		{
			return true;
		}
	}

	// There were no compatible attach points on both elements
	return false;
}


// Allow assignment of one element contents to another
void ComplexShipElement::operator=(const ComplexShipElement & rhs)
{
	// Copy all relevant data from the source object
	m_id = rhs.GetID();
	m_location = rhs.GetLocation();
	m_parent = rhs.GetParent();
	m_properties = rhs.GetProperties();
	m_health = rhs.GetHealth();
	m_gravity = rhs.GetGravityStrength();
	m_connections = rhs.GetConnectionState();
	m_tile = rhs.GetTile();
	for (int i = 0; i < ComplexShipElement::AttachType::_AttachTypeCount; ++i)
		m_attachpoints[i] = rhs.GetAttachmentState((ComplexShipElement::AttachType)i);
	for (int i = 0; i < Direction::_Count; ++i)
		m_adj[i] = rhs.GetNeighbour((Direction)i);

	// Copy nav network data
	if (false)	// TODO: DEBUG: TO BE REINSTATED
	{
		NavNodes = rhs.NavNodes;
		AllocateNavPointPositionData(rhs.GetNavPointPositionCount());
		AllocateNavPointConnectionData(rhs.GetNavPointConnectionCount());
		for (int i = 0; i < m_numnavnodepositions; ++i)
			m_navnodepositions[i] = rhs.GetNavPointPosition(i);
		for (int i = 0; i < m_numnavnodeconnections; ++i)
			m_navnodeconnections[i] = rhs.GetNavNodeConnection(i);
	}
}

// Copy constructor to create an element based on another
ComplexShipElement::ComplexShipElement(const ComplexShipElement & source)
{
	// Debug purposes: record the number of elements being created
#	if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		++inst_con;
#	endif

	// Use assignment operator to copy the element
	(*this) = source;
}

iSpaceObjectEnvironment *ComplexShipElement::GetParent(void) const
{
	return m_parent;
}

void ComplexShipElement::SetParent(iSpaceObjectEnvironment *parent)
{
	m_parent = parent;
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
	else						return ComplexShipElement::AttachType::_AttachTypeCount;
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
	return ((ConnectsInDirection(DirectionBS::Left_BS) ? 1 : 0) +
			(ConnectsInDirection(DirectionBS::Up_BS) ? 1 : 0) +
			(ConnectsInDirection(DirectionBS::Right_BS) ? 1 : 0) +
			(ConnectsInDirection(DirectionBS::Down_BS) ? 1 : 0) +
			(ConnectsInDirection(DirectionBS::UpLeft_BS) ? 1 : 0) +
			(ConnectsInDirection(DirectionBS::UpRight_BS) ? 1 : 0) +
			(ConnectsInDirection(DirectionBS::DownRight_BS) ? 1 : 0) +
			(ConnectsInDirection(DirectionBS::DownLeft_BS) ? 1 : 0) +
			(ConnectsInDirection(DirectionBS::ZUp_BS) ? 1 : 0) +
			(ConnectsInDirection(DirectionBS::ZDown_BS) ? 1 : 0));
}



