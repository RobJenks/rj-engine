#pragma once

#ifndef __ComplexShipElementH__
#define __ComplexShipElementH__

#include "DX11_Core.h"

#include <vector>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "FastMath.h"
#include "iEnvironmentObject.h"
#include "StaticTerrain.h"
#include "Utility.h"
#include "GameDataExtern.h"
#include "iContainsComplexShipTiles.h"
#include "NavNode.h"

class ComplexShipTile;
class iSpaceObjectEnvironment;
class ComplexShipSection;

#define DEBUG_LOGINSTANCECREATION


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ComplexShipElement : public ALIGN16<ComplexShipElement>
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(ComplexShipElement)

	// Default constructor
	ComplexShipElement(void);

	// Location in element space
	CMPINLINE INTVECTOR3			GetLocation(void) const				{ return m_location; }
	CMPINLINE void					SetLocation(const INTVECTOR3 & loc) { m_location = loc; }
	CMPINLINE void					SetLocation(int x, int y, int z)	{ m_location.x = x; m_location.y = y; m_location.z = z; }

	// ID of the element (corresponding to its index in the element space)
	CMPINLINE int					GetID(void) const					{ return m_id; }
	CMPINLINE void					SetID(int id)						{ m_id = id; }

	// Enumeration of all classes of attachment between elements
	enum AttachType { Standard = 0, TurretModule, _AttachTypeCount };
	
	// Properties of the element, indexed into the m_properties bitstring
	enum PROPERTY 
	{
		// Structural properties; set by e.g. ship hull or sections
		PROP_ACTIVE				= (1 << 0),				// 1:     Is the element active, i.e. can be used in the SD
		PROP_BUILDABLE			= (1 << 1),				// 2:     Can the element have tiles built on it?
		PROP_OUTER_HULL_ELEMENT = (1 << 2),				// 4:     Is this element part of the outer ship hull?

		// More detaileed properties; set by e.g. ship tiles
		PROP_WALKABLE			= (1 << 3),				// 8:     Is the element an (easy, walking) route for player & AI?
		PROP_TRANSMITS_POWER	= (1 << 4),				// 16:    Are there power cables running through this element?
		PROP_TRANSMITS_DATA		= (1 << 5),				// 32:    Are there data cables running through this element?
		
		PROPERTY_COUNT			= (1 << 6)				//        (The total number of properties per element)
	};

	// Struct holding data on a connection from one nav node to another
	struct NavNodeConnection { 
		int Source; int Target; bool IsDirection; int Cost; 
		CMPINLINE NavNodeConnection(int src, int tgt, bool isDir, int cost) { Source = src; Target = tgt; IsDirection = isDir; Cost = cost; }
		CMPINLINE void operator=(const NavNodeConnection & rhs) { Source = rhs.Source; Target = rhs.Target; IsDirection = rhs.IsDirection; Cost = rhs.Cost; }
	};

	// Struct holding data on a nav node position within this element
	struct NavNodePos { 
		INTVECTOR3 Position;  
		float CostModifier;
		int NumConnections;

		CMPINLINE NavNodePos(void) { Position = NULL_INTVECTOR3; NumConnections = 0; CostModifier = 1.0f; }
		CMPINLINE void operator=(const NavNodePos & rhs) { Position = rhs.Position; CostModifier = rhs.CostModifier; NumConnections = rhs.NumConnections; }
	};
	
	// Test the value of a property.  Multiple properties can be provided at once (e.g. GetProperty(Property::Active | Property::Buildable)), 
	// in which case the method will return a value indicating whether all properties are true for the element
	CMPINLINE bool					GetProperty(PROPERTY prop)					{ return CheckBit_All(m_properties, prop); }

	// Set the value of a property.  Multiple properties can be provided, in which case the method will set all at once
	CMPINLINE void					SetProperty(PROPERTY prop)					{ SetBit(m_properties, prop); }
	CMPINLINE void					ClearProperty(PROPERTY prop)				{ ClearBit(m_properties, prop); }
	CMPINLINE void					ClearAllProperties(void)					{ m_properties = 0U; }
	CMPINLINE void					SetPropertyValue(PROPERTY prop, bool value)	{ SetBitState(m_properties, prop, value); }

	// Get or set the full property set in one operation
	CMPINLINE bitstring				GetProperties(void) const { return m_properties; }
	CMPINLINE void					SetProperties(bitstring props) { m_properties = props; }

	// Is the element active?  If not, it cannot be used for anything and is empty space
	CMPINLINE bool					IsActive(void) const	{ return CheckBit_Any(m_properties, ComplexShipElement::PROPERTY::PROP_ACTIVE); }
	CMPINLINE void					SetActive(bool b)		{ SetBitState(m_properties, ComplexShipElement::PROPERTY::PROP_ACTIVE, b); }

	// Can the element be used by the user for construction?  If not, it may still contain other components but is locked and cannot be changed
	CMPINLINE bool					IsBuildable(void) const	{ return CheckBit_Any(m_properties, ComplexShipElement::PROPERTY::PROP_BUILDABLE); }
	CMPINLINE void					SetBuildable(bool b)	{ SetBitState(m_properties, ComplexShipElement::PROPERTY::PROP_BUILDABLE, b); }

	// Is the element walkable?  Used for pathfinding and routing calculations
	CMPINLINE bool					IsWalkable(void) const	{ return CheckBit_Any(m_properties, ComplexShipElement::PROPERTY::PROP_WALKABLE); }

	// Is the element part of the outer environment hull?
	CMPINLINE bool					IsOuterHullElement(void) const	{ return CheckBit_Any(m_properties, ComplexShipElement::PROPERTY::PROP_OUTER_HULL_ELEMENT); }

	// Health of the element.  Ranges from 0.0-1.0.  Element is destroyed at <= 0.0
	CMPINLINE float					GetHealth(void) const			{ return m_health; }
	CMPINLINE void					SetHealth(float h)				{ m_health = h; }

	// Returns a flag indicating whether the element has been destroyed
	CMPINLINE bool					IsDestroyed(void) const			{ return (m_health <= 0.0f); }

	// Inherent strength of the element.  Generally inherited from the hull that contains this element.  This is the
	// base impact resistance when determining the effect of a collider intersection with an environment
	CMPINLINE float					GetStrength(void) const			{ return m_strength; }
	CMPINLINE void					SetStrength(float s)			{ m_strength = s; }

	// Impact resistance of the element.  This is its health-scaled strength value.  I.e. simulates the decrease
	// in element integrity as the hull takes damage
	CMPINLINE float					GetImpactResistance(void) const	{ return (m_strength * m_health); }

	// Get or change the gravity strength at this element
	CMPINLINE float					GetGravityStrength(void) const	{ return m_gravity; }
	CMPINLINE void					ChangeGravityStrength(float G)	{ m_gravity = G; }

	// Indicates whether the element has attachment points of the given type in the specified direction.  Multiple directions 
	// can be specified, in which case this will test whether the element has attachpoints in ALL of those directions
	CMPINLINE bool					HasAttachPoints(ComplexShipElement::AttachType type, DirectionBS direction) const
	{
		return CheckBit_All(m_attachpoints[type], direction);
	}

	// Indicates whether the element has attachment points of the given type in the specified direction.  Multiple directions 
	// can be specified, in which case this will test whether the element has attachpoints in ANY of those directions
	CMPINLINE bool					HasAnyAttachPoints(ComplexShipElement::AttachType type, DirectionBS direction) const
	{
		return CheckBit_Any(m_attachpoints[type], direction);
	}

	// Indicates whether the element has an attach point of the specified type in ANY direction
	CMPINLINE bool					HasAnyAttachPoints(ComplexShipElement::AttachType type) const
	{
		return CheckBit_Any(m_attachpoints[type], DirectionBS_All); 
	}

	// Indicates whether the element has attachment points of ANY type in the specified direction.  Multiple directions 
	// can be specified, in which case this will test whether the element has attachpoints in ALL of those directions
	CMPINLINE bool					HasAttachPointsOfAnyType(DirectionBS direction) const
	{
		return CheckBit_All((m_attachpoints[ComplexShipElement::AttachType::Standard] |
							 m_attachpoints[ComplexShipElement::AttachType::TurretModule]), direction);
	}

	// Indicates whether the element has attachment points of ANY type in the specified direction.  Multiple directions 
	// can be specified, in which case this will test whether the element has attachpoints in ANY of those directions
	CMPINLINE bool					HasAnyAttachPointsOfAnyType(void) const
	{
		return (HasAttachPointsOfAnyType(DirectionBS_All));
	}

	// Returns the full attachment state of the element
	CMPINLINE bitstring				GetAttachmentState(ComplexShipElement::AttachType type) const { return m_attachpoints[(int)type]; }
	CMPINLINE bitstring			  (&GetAttachmentState(void))[ComplexShipElement::AttachType::_AttachTypeCount] { return m_attachpoints; }

	// Sets the full attachment state of the element
	CMPINLINE void					SetAttachmentState(ComplexShipElement::AttachType type, bitstring state) { m_attachpoints[type] = state; }
	CMPINLINE void					SetAttachmentState(bitstring(&state)[ComplexShipElement::AttachType::_AttachTypeCount]) 
	{
		for (int i = 0; i < ComplexShipElement::AttachType::_AttachTypeCount; ++i) m_attachpoints[i] = state[i]; 
	}

	// Retrieve a neighbouring element in three-dimensional element space (or -1, if no neighbour)
	CMPINLINE int					GetNeighbour(Direction direction) const					{ return m_adj[direction]; }
	
	// Link this element to a neighbour in the three-dimensional space
	CMPINLINE void					LinkNeighbour(Direction dir, int neighbour_index)		{ m_adj[dir] = neighbour_index; }

	// Return a constant reference to the Direction::_Count neighbours of this element
	typedef int AdjacencyData[Direction::_Count];
	CMPINLINE AdjacencyData const&	AdjacentElements() const								{ return m_adj; }

	// Indicates that no element is present, when working with element IDs
	static const int				NO_ELEMENT = -1;

	// Rotates the element by the specified angle
	void							RotateElement(Rotation90Degree rotation);

	// Link back to the parent object
	iSpaceObjectEnvironment *		GetParent(void) const;
	void							SetParent(iSpaceObjectEnvironment *parent);

	// Pointer to the tile currently occupying this element, or NULL if none
	ComplexShipTile *				GetTile(void) const										{ return m_tile; }
	void							SetTile(ComplexShipTile *tile)							{ m_tile = tile; }

	// Methods to query the walkable connections to neighbouring walkable elements.  More than
	// one direction can be specified, in which case this will test whether the element connects
	// in ALL specified directions
	CMPINLINE bool					ConnectsInDirection(DirectionBS direction) const		{ return CheckBit_All(m_connections, direction); }

	// Methods to query the walkable connections to neighbouring walkable elements.  More than
	// one direction can be specified, in which case this will test whether the element connects
	// in ANY of the specified directions
	CMPINLINE bool					ConnectsInAnyDirection(DirectionBS direction) const		{ return CheckBit_Any(m_connections, direction); }

	// Methods to set the walkable connections to neighbouring walkable elements.  Multiple directions
	// can be specified, in which case the state of all directions is set at once
	CMPINLINE void					SetConnectionStateInDirection(DirectionBS direction, bool state)
	{
		(state ? SetBit(m_connections, direction) : ClearBit(m_connections, direction));
	}

	// Return or set the entire connection state for an element in one operation
	CMPINLINE bitstring				GetConnectionState(void) const							{ return m_connections; }
	CMPINLINE void					SetConnectionState(bitstring state)						{ m_connections = state; }

	// Determines the number of walkable connections from this element
	int								DetermineNumberOfWalkableConnections(void);

	// Methods to manipulate nav point position data
	CMPINLINE int					GetNavPointPositionCount(void) const	{ return m_numnavnodepositions; }
	CMPINLINE NavNodePos *			GetNavPointPositionData(void)			{ return m_navnodepositions; }
	CMPINLINE int					GetNavPointConnectionCount(void) const	{ return m_numnavnodeconnections; }
	CMPINLINE NavNodeConnection *	GetNavPointConnectionData(void)			{ return m_navnodeconnections; }
	void							AllocateNavPointPositionData(int n);
	void							AllocateNavPointConnectionData(int n);
	void							DeallocateNavPointPositionData(void);
	void							DeallocateNavPointConnectionData(void);
	CMPINLINE const NavNodePos &		GetNavPointPosition(int index) const	{ return m_navnodepositions[index]; }
	CMPINLINE const NavNodeConnection &	GetNavNodeConnection(int index) const	{ return m_navnodeconnections[index]; }

	// The collection of nav nodes within this element.  Remains public to allow direct access
	std::vector<NavNode*>			NavNodes;

	// Flag indicating whether the element has custom-defined nav data, or whether the data is being auto-generated
	CMPINLINE bool					HasCustomNavData(void) const				{ return m_customnavdata; }
	CMPINLINE void					SetCustomNavDataFlag(bool is_custom)		{ m_customnavdata = is_custom; }

	// Default destructor
	~ComplexShipElement(void);

	// Static method to test whether an attachment is possible from the 'elementedge' edge of 'element' into 'neighbourelement'
	static bool	AttachmentIsCompatible(	ComplexShipElement *element, DirectionBS elementedge,
										ComplexShipElement *neighbourelement);

	// Methods to translate between the attachment enumerations and their string representations
	static string							AttachTypeToString(AttachType type);
	static AttachType						AttachTypeFromString(string type);

	// Compares the equality of two elements
	CMPINLINE friend bool operator== (const ComplexShipElement &e1, ComplexShipElement &e2) 
	{ 
		return (e1.GetID() == e2.GetID() && e1.GetParent() == e2.GetParent());
	}
	CMPINLINE friend bool operator!= (const ComplexShipElement &e1, ComplexShipElement &e2) { return !(e1 == e2); }

	// Allow assignment of one element contents to another
	void operator=(const ComplexShipElement & rhs);

	// Copy constructor to create an element based on another
	ComplexShipElement(const ComplexShipElement & source);

	// Static methods to make a copy of elements or element data
	static ComplexShipElement *				Copy(ComplexShipElement *element);
	static void								CopyData(ComplexShipElement *src, ComplexShipElement *dest);

	// Static methods to convert between element properties and their string name representation
	static ComplexShipElement::PROPERTY		TranslatePropertyFromName(string name);
	static string							TranslatePropertyToName(ComplexShipElement::PROPERTY prop);

	// Static method to parse a property string and determine the final element state
	static ComplexShipElement::PROPERTY		ParsePropertyString(const std::string & prop_string);

	// Returns the coordinates of a point on specific adjoining edges or elements
	static XMFLOAT3							GetEdgePosition(Direction direction);
	static XMFLOAT3							GetAdjacentElementCentrePosition(Direction direction);

	// Default property state for a complex ship element
	static const bitstring					DefaultProperties;



private:
	INTVECTOR3						m_location;					// x,y,z location of this element, in element space
	iSpaceObjectEnvironment *		m_parent;					// The parent environment that this element belongs to
	int								m_id;						// Index of the element in its parent element collection
	
	// Pointer to the tile that exists on this element, or NULL if no tile
	ComplexShipTile *				m_tile;

	// Properties of the element, stored as a bitstring for efficiency.  All set to false on initialisation
	bitstring 						m_properties;
	
	// Store an attachment state per attach type.  Each type has a bitstring; 1 for allowed, 0 for not allowed, based on DirectionBS enumeration
	bitstring						m_attachpoints[ComplexShipElement::AttachType::_AttachTypeCount];

	// Health of the element from 0.0-1.0
	float							m_health;
	
	// Inherent strength of the element.  Generally inherited from the hull that contains this element.  This is the
	// base impact resistance when determining the effect of a collider intersection with an environment
	float							m_strength;

	// Other properties of the element
	float							m_gravity;

	// Index of neighbouring elements, or -1 if no neighbour
	int								m_adj[Direction::_Count];

	// Bitstring determining which directions this element connects in
	bitstring						m_connections;

	// Data on the navigation nodes and connections to be created within this element.  
	// If none are specified then default nodes will be generated during pathfinding
	NavNodePos *					m_navnodepositions;
	int								m_numnavnodepositions;
	NavNodeConnection *				m_navnodeconnections;
	int								m_numnavnodeconnections;
	bool							m_customnavdata;

public:

	// Debug variables to log instance creation
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		static long inst_con;
		static long inst_des;
	#endif
};


#endif