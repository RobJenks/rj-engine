#include "FastMath.h"
#include "Utility.h"
#include "ErrorCodes.h"
#include "ComplexShipTile.h"
#include "iSpaceObjectEnvironment.h"
#include "ElementStateDefinition.h"
#include "Damage.h"

#include "ComplexShipElement.h"

#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
	long ComplexShipElement::inst_con = 0;
	long ComplexShipElement::inst_des = 0;
#endif

// Static variables
const bitstring ComplexShipElement::NULL_PROPERTIES = (bitstring)0U;

// Constant indicating that all property values are set
const bitstring ComplexShipElement::ALL_PROPERTIES = (bitstring)((unsigned int)ComplexShipElement::PROPERTY::PROPERTY_MAX - 1U);

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
	m_properties = ComplexShipElement::NULL_PROPERTIES;
	m_tile = NULL;
	m_health = 1.0f;
	m_strength = 1000.0f;
	m_gravity = 0.0f;

	// No attach points by default
	memset(m_attachpoints, 0, sizeof(bitstring) * ComplexShipElement::AttachType::_AttachTypeCount);

	// No connections or adjacency info by default
	m_connections = 0U;
	memset(m_adj, 0, sizeof(ComplexShipElement*) * Direction::_Count);

	// No nav node positions or connections are specified by default
	m_navnodepositions = NULL; m_navnodeconnections = NULL;
	m_numnavnodepositions = 0; m_numnavnodeconnections = 0;
	m_customnavdata = false;
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


// Add to the element state (e.g. apply additional properties)
void ComplexShipElement::ApplyElementState(const ElementStateDefinition::ElementState & state)
{
	// Update element properties
	SetProperty((PROPERTY)state.Properties);
}

// Overwrite the entire element state with another
void ComplexShipElement::OverwriteElementState(const ElementStateDefinition::ElementState & state)
{
	// Update element properties
	SetProperties(state.Properties);
}

// Reset the element state
void ComplexShipElement::ResetElementState(void)
{
	SetProperties(NULL_PROPERTIES);
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
	DeallocateNavPointPositionData();
	
	// Validate the parameter; n = number of nav point positions to allocate
	if (n <= 0) return;

	// Allocate space for the nav point data
	m_navnodepositions = (ComplexShipElement::NavNodePos*)malloc(sizeof(ComplexShipElement::NavNodePos) * n);
	m_numnavnodepositions = n;
}

void ComplexShipElement::DeallocateNavPointPositionData(void)
{
	if (m_navnodepositions) SafeFree(m_navnodepositions);
	m_numnavnodepositions = 0;
}

void ComplexShipElement::AllocateNavPointConnectionData(int n)
{
	// Deallocate any memory that has already been allocated
	DeallocateNavPointConnectionData();

	// Validate the parameter; n = number of nav point connections to allocate
	if (n <= 0) return;

	// Allocate space for the nav point connection data
	m_navnodeconnections = (ComplexShipElement::NavNodeConnection*)malloc(sizeof(ComplexShipElement::NavNodeConnection) * n);
	m_numnavnodeconnections = n;
}

void ComplexShipElement::DeallocateNavPointConnectionData(void)
{
	if (m_navnodeconnections) SafeFree(m_navnodeconnections);
	m_numnavnodeconnections = 0;
}

string ComplexShipElement::AttachTypeToString(ComplexShipElement::AttachType type)
{
	switch (type) {
		case ComplexShipElement::AttachType::Standard:				return "Standard";
		case ComplexShipElement::AttachType::TurretModule:			return "Turret";
		default:													return "Disallowed";
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
		case ComplexShipElement::PROPERTY::PROP_ACTIVE:				return "Active";
		case ComplexShipElement::PROPERTY::PROP_BUILDABLE:			return "Buildable";
		case ComplexShipElement::PROPERTY::PROP_OUTER_HULL_ELEMENT:	return "OuterHullElement";
		case ComplexShipElement::PROPERTY::PROP_WALKABLE:			return "Walkable";
		case ComplexShipElement::PROPERTY::PROP_TRANSMITS_POWER:	return "TransmitsPower";
		case ComplexShipElement::PROPERTY::PROP_TRANSMITS_DATA:		return "TransmitsData";
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
	else if (s == "outerhullelement")	return ComplexShipElement::PROPERTY::PROP_OUTER_HULL_ELEMENT;
	else if (s == "walkable")			return ComplexShipElement::PROPERTY::PROP_WALKABLE;
	else if (s == "transmitspower")		return ComplexShipElement::PROPERTY::PROP_TRANSMITS_POWER;
	else if (s == "transmitsdata")		return ComplexShipElement::PROPERTY::PROP_TRANSMITS_DATA;

	// This usually means something has gone wrong.  'Count' property is not used
	// for anything so setting it will not have any effect on the element state
	else								return ComplexShipElement::PROPERTY::PROPERTY_MAX;
}

// Static method to parse a property string and determine the final element state
ComplexShipElement::PROPERTY ComplexShipElement::ParsePropertyString(const std::string & prop_string)
{
	// Split the string based on a standard delimiter
	// TODO: we could probably do this more efficiently by one manual pass through the string character-
	// by-character, if needed
	std::vector<std::string> props; 
	SplitString(prop_string, '|', true, props);

	// Process each component in turn
	unsigned int state = 0U;
	for (std::string prop : props)
	{
		if (prop.empty()) continue;
		if (prop.find_first_not_of(" 0123456789") == std::string::npos)
		{
			// We couldn't find a non-numeric digit; therefore treat this as a direct state value
			int signed_st = atoi(prop.c_str());
			if (signed_st > 0 && signed_st < ComplexShipElement::PROPERTY::PROPERTY_MAX) 
				SetBit(state, (unsigned int)signed_st);
		}
		else
		{
			// The string contains some non-numeric characters, so attempt to parse it as a property name
			ComplexShipElement::PROPERTY pval = TranslatePropertyFromName(prop);
			if (pval != ComplexShipElement::PROPERTY::PROPERTY_MAX)
				SetBit(state, (unsigned int)pval);
		}
	}

	// Return the final calculated state
	return (ComplexShipElement::PROPERTY)state;
}

// Builds a descriptive property string from the supplied property state value
std::string ComplexShipElement::DeterminePropertyStringDescription(bitstring properties)
{
	concat s("");

	// Iterate through all possible property values (i == {1, 2, 4, 8, ..., 2^(N-1)})
	for (bitstring i = 1; i < PROPERTY::PROPERTY_MAX; i <<= 1)
	{
		if (CheckBit_Any(properties, i)) s(TranslatePropertyToName((PROPERTY)i))(", ");
	}

	// We either have "Prop1, Prop2, ..., PropN, " or "", depending on whether the input contained
	// any valid properties.  In the case of the former, strip the trailing ", " before returning
	std::string result = s.str();
	return (result.empty() ? result : result.substr(0U, result.length() - 2U));	
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



