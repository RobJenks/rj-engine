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
#include "Damage.h"
#include "NavNode.h"

class ComplexShipTile;
class iSpaceObjectEnvironment;
class ComplexShipSection;

#define DEBUG_LOGINSTANCECREATION


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ComplexShipElement : public iContainsComplexShipTiles, public ALIGN16<ComplexShipElement>
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(ComplexShipElement)

	// Default constructor
	ComplexShipElement(void);
	
	// Default destructor
	~ComplexShipElement(void);

	// Enumeration of all classes of attachment between elements
	enum AttachType { Standard = 0, TurretModule, _AttachTypeCount };
	
	// Properties of the element, indexed into the m_properties bitstring
	enum PROPERTY {
		PROP_UNKNOWN		= (1 << 0),
		PROP_ACTIVE			= (1 << 1),				// Is the element active, i.e. can be used in the SD
		PROP_BUILDABLE		= (1 << 2),				// Can the element have tiles built on it?
		PROP_WALKABLE		= (1 << 3),				// Is the element an (easy, walking) route for player & AI?
		PROP_POWER_CABLES	= (1 << 4),				// Are there power cables running through this element?
		PROPERTY_COUNT		= (1 << 5)				// (The total number of properties per element)
	};

	// Struct holding data on a connection from one nav node to another
	struct NavNodeConnection { 
		int Source; int Target; bool IsDirection; int Cost; 
		NavNodeConnection(int src, int tgt, bool isDir, int cost) { Source = src; Target = tgt; IsDirection = isDir; Cost = cost; }
	};

	// Struct holding data on a nav node position within this element
	struct NavNodePos { 
		INTVECTOR3 Position;  
		float CostModifier;
		int NumConnections;
		NavNodePos(void) { Position = NULL_INTVECTOR3; NumConnections = 0; CostModifier = 1.0f; }
	};
	
	// Test the value of a property.  Multiple properties can be provided at once (e.g. GetProperty(Property::Active | Property::Buildable)), 
	// in which case the method will return a value indicating whether all properties are true for the element
	CMPINLINE bool					GetProperty(PROPERTY prop)				{ return CheckBit_All(m_properties, prop); }

	// Set the value of a property.  Multiple properties can be provided, in which case the method will set all at once
	CMPINLINE void					SetProperty(PROPERTY prop, bool value)	{ SetBitState(m_properties, prop, value); }

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

	// Location in element space
	CMPINLINE INTVECTOR3			GetLocation(void)					{ return m_location; }
	CMPINLINE void					SetLocation(const INTVECTOR3 & loc) { m_location = loc; }
	CMPINLINE void					SetLocation(int x, int y, int z)	{ m_location.x = x; m_location.y = y; m_location.z = z; }

	// Other key fields relating to this element
	CMPINLINE float					GetHealth(void)					{ return m_health; }
	CMPINLINE void					SetHealth(float h)				{ m_health = h; }

	// Get or change the gravity strength at this element
	CMPINLINE float					GetGravityStrength(void) const	{ return m_gravity; }
	CMPINLINE void					ChangeGravityStrength(float G)	{ m_gravity = G; }

	// Indicates whether the element has attachment points of the given type in the specified direction.  Multiple directions 
	// can be specified, in which case this will test whether the element has attachpoints in ALL of those directions
	CMPINLINE bool					HasAttachPoints(ComplexShipElement::AttachType type, DirectionBS direction)
	{
		return CheckBit_All(m_attachpoints[type], direction);
	}

	// Indicates whether the element has attachment points of the given type in the specified direction.  Multiple directions 
	// can be specified, in which case this will test whether the element has attachpoints in ANY of those directions
	CMPINLINE bool					HasAnyAttachPoints(ComplexShipElement::AttachType type, DirectionBS direction)
	{
		return CheckBit_Any(m_attachpoints[type], direction);
	}

	// Indicates whether the element has an attach point of the specified type in ANY direction
	CMPINLINE bool					HasAnyAttachPoints(ComplexShipElement::AttachType type)
	{
		return CheckBit_Any(m_attachpoints[type], DirectionBS_All); 
	}

	// Indicates whether the element has attachment points of ANY type in the specified direction.  Multiple directions 
	// can be specified, in which case this will test whether the element has attachpoints in ALL of those directions
	CMPINLINE bool					HasAttachPointsOfAnyType(DirectionBS direction)
	{
		return CheckBit_All(m_attachpoints[type], direction);
	}

	// Indicates whether the element has attachment points of ANY type in the specified direction.  Multiple directions 
	// can be specified, in which case this will test whether the element has attachpoints in ANY of those directions
	CMPINLINE bool					HasAnyAttachPointsOfAnyType(DirectionBS direction)
	{
		return CheckBit_Any(m_attachpoints[type], direction);
	}

	AttachType										GetAttachPoint(DirectionBS edge);
	AttachPointCollection *							GetAttachPoints(void) { return &m_attachpoints; }
	CMPINLINE AttachPointCollection::size_type		GetAttachPointCount(void) { return m_attachpoints.size(); }
	CMPINLINE void									AddAttachPoint(ElementAttachPoint attach) { m_attachpoints.push_back(attach); }
	void											SetAttachPoints(AttachPointCollection *apc) { m_attachpoints = *apc; }

	// Methods to retrieve neighbouring elements in the three-dimensional space
	CMPINLINE ComplexShipElement *	GetLeft(void)		{ return m_left; }
	CMPINLINE ComplexShipElement *	GetUp(void)			{ return m_up; }
	CMPINLINE ComplexShipElement *	GetRight(void)		{ return m_right; }
	CMPINLINE ComplexShipElement *	GetDown(void)		{ return m_down; }
	CMPINLINE ComplexShipElement *	GetZUp(void)		{ return m_zup; }
	CMPINLINE ComplexShipElement *	GetZDown(void)		{ return m_zdown; }
	CMPINLINE ComplexShipElement *	GetUpLeft(void)		{ return m_upleft; }
	CMPINLINE ComplexShipElement *	GetUpRight(void)	{ return m_upright; }
	CMPINLINE ComplexShipElement *	GetDownLeft(void)	{ return m_downleft; }
	CMPINLINE ComplexShipElement *	GetDownRight(void)	{ return m_downright; }
	ComplexShipElement *			GetNeighbour(Direction direction);

	// Methods to link this element to its neighbours in the three-dimensional space
	CMPINLINE void					LinkLeft(ComplexShipElement *el)		{ m_left = el; }
	CMPINLINE void					LinkUp(ComplexShipElement *el)			{ m_up = el; }
	CMPINLINE void					LinkRight(ComplexShipElement *el)		{ m_right = el; }
	CMPINLINE void					LinkDown(ComplexShipElement *el)		{ m_down = el; }
	CMPINLINE void					LinkZUp(ComplexShipElement *el	)		{ m_zup = el; }
	CMPINLINE void					LinkZDown(ComplexShipElement *el)		{ m_zdown = el; }
	CMPINLINE void					LinkUpLeft(ComplexShipElement *el)		{ m_upleft = el; }
	CMPINLINE void					LinkUpRight(ComplexShipElement *el)		{ m_upright = el; }
	CMPINLINE void					LinkDownLeft(ComplexShipElement *el)	{ m_downleft = el; }
	CMPINLINE void					LinkDownRight(ComplexShipElement *el)	{ m_downright = el; }

	// Link back to the parent object
	iSpaceObjectEnvironment *		GetParent(void);
	void							SetParent(iSpaceObjectEnvironment *parent);

	// Methods triggered when a tile is added or removed from this object
	void							ShipTileAdded(ComplexShipTile *tile);
	void							ShipTileRemoved(ComplexShipTile *tile);

	// Updates this element based on all attached tiles
	void							UpdateElementBasedOnTiles(void);
	
	// Methods to query the walkable connections to neighbouring walkable elements
	CMPINLINE bool				ConnectsLeft(void)		{ return m_cleft; }
	CMPINLINE bool				ConnectsUp(void)		{ return m_cup; }
	CMPINLINE bool				ConnectsRight(void)		{ return m_cright; }
	CMPINLINE bool				ConnectsDown(void)		{ return m_cdown; }
	CMPINLINE bool				ConnectsZUp(void)		{ return m_czup; }
	CMPINLINE bool				ConnectsZDown(void)		{ return m_czdown; }
	CMPINLINE bool				ConnectsUpLeft(void)	{ return m_cupleft; }
	CMPINLINE bool				ConnectsUpRight(void)	{ return m_cupright; }
	CMPINLINE bool				ConnectsDownLeft(void)	{ return m_cdownleft; }
	CMPINLINE bool				ConnectsDownRight(void)	{ return m_cdownright; }
	bool						ConnectsInDirection(Direction direction);

	// Methods to set the walkable connections to neighbouring walkable elements
	CMPINLINE void				SetConnectionLeft(bool connects)		{ m_cleft = connects; }
	CMPINLINE void				SetConnectionUp(bool connects)			{ m_cup = connects; }
	CMPINLINE void				SetConnectionRight(bool connects)		{ m_cright = connects; }
	CMPINLINE void				SetConnectionDown(bool connects)		{ m_cdown = connects; }
	CMPINLINE void				SetConnectionZUp(bool connects)			{ m_czup = connects; }
	CMPINLINE void				SetConnectionZDown(bool connects)		{ m_czdown = connects; }
	CMPINLINE void				SetConnectionUpLeft(bool connects)		{ m_cupleft = connects; }
	CMPINLINE void				SetConnectionUpRight(bool connects)		{ m_cupright = connects; }
	CMPINLINE void				SetConnectionDownLeft(bool connects)	{ m_cdownleft = connects; }
	CMPINLINE void				SetConnectionDownRight(bool connects)	{ m_cdownright = connects; }
	void						SetConnectionInDirection(Direction direction, bool connection);

	// Determines the number of walkable connections from this element
	int							DetermineNumberOfWalkableConnections(void);

	// Methods to manipulate nav point position data
	CMPINLINE int					GetNavPointPositionCount(void)			{ return m_numnavnodepositions; }
	CMPINLINE NavNodePos *			GetNavPointPositionData(void)			{ return m_navnodepositions; }
	CMPINLINE int					GetNavPointConnectionCount(void)		{ return m_numnavnodeconnections; }
	CMPINLINE NavNodeConnection *	GetNavPointConnectionData(void)			{ return m_navnodeconnections; }
	void							AllocateNavPointPositionData(int n);
	void							AllocateNavPointConnectionData(int n);

	// The collection of nav nodes within this elemnet.  Remains public to allow direct access
	std::vector<NavNode*>			NavNodes;

	// Static methods to allocate, initialise and deallocate a three-dimensional space of these elements
	static Result CreateElementSpace		(iSpaceObjectEnvironment *parent, ComplexShipElement ****elements, INTVECTOR3 elementsize);
	static Result CopyElementSpace			(iSpaceObjectEnvironment *src, iSpaceObjectEnvironment *target);
	static Result AllocateElementStorage	(ComplexShipElement ****elements, INTVECTOR3 elementsize);
	static Result InitialiseElementStorage	(ComplexShipElement ***elements, INTVECTOR3 elementsize, iSpaceObjectEnvironment *parent, iSpaceObjectEnvironment *copysource);
	static Result DeallocateElementStorage	(ComplexShipElement ****elements, INTVECTOR3 elementsize);
	
	// Rotate an entire element space by the specified angle
	static Result RotateElementSpace	   (ComplexShipElement ****pElements, INTVECTOR3 *pElementsize, Rotation90Degree rot);

	// Rotate all the attach points on an element by the specified angle
	static void RotateElementAttachPoints	(ComplexShipElement *el, Rotation90Degree rotation);

	// Static method to test whether an attachment is possible from the 'elementedge' edge of 'element' into 'neighbourelement'
	static bool	AttachmentIsCompatible(	ComplexShipElement *element, Direction elementedge,
										ComplexShipElement *neighbourelement);

	// Methods to translate between the attachment enumerations and their string representations
	static string							AttachTypeToString(AttachType type);
	static AttachType						AttachTypeFromString(string type);

	// Compares the equality of two elements
	bool									Equals(ComplexShipElement *e);

	// Static methods to make a copy of elements or element data
	static ComplexShipElement *				Copy(ComplexShipElement *element);
	static void								CopyData(ComplexShipElement *src, ComplexShipElement *dest);

	// Static methods to convert between element properties and their string name representation
	static ComplexShipElement::PROPERTY		TranslatePropertyFromName(string name);
	static string							TranslatePropertyToName(ComplexShipElement::PROPERTY prop);

	// Returns the coordinates of a point on specific adjoining edges or elements
	static XMFLOAT3							GetEdgePosition(Direction direction);
	static XMFLOAT3							GetAdjacentElementCentrePosition(Direction direction);

	// Method to initialise static data, for use in any static runtime calls
	static void								InitialiseStaticData(void);
	static std::vector<bool>				DefaultPropertyValues;



private:
	INTVECTOR3						m_location;					// x,y,z location of this element, in element space
	iSpaceObjectEnvironment *		m_parent;					// The parent environment that this element belongs to
	int								m_id;						// Index of the element in its parent element collection
	
	// Properties of the element, stored as a bitstring for efficiency.  All set to false on initialisation
	bitstring 						m_properties;
	
	// Store an attachment state per attach type.  Each type has a bitstring; 1 for allowed, 0 for not allowed, based on DirectionBS enumeration
	bitstring						m_attachpoints[ComplexShipElement::AttachType::_AttachTypeCount];

	// Health of the element from 0.0-1.0
	float							m_health;
	
	// Other properties of the element
	float							m_gravity;

	// Criteria around how this element can attach to neighbouring cells
	ComplexShipElement *			m_left;						// Pointer to adjacent elements
	ComplexShipElement *			m_right;					// Pointer to adjacent elements
	ComplexShipElement *			m_up;						// Pointer to adjacent elements
	ComplexShipElement *			m_down;						// Pointer to adjacent elements
	ComplexShipElement *			m_zup;						// Pointer to adjacent elements
	ComplexShipElement *			m_zdown;					// Pointer to adjacent elements
	ComplexShipElement *			m_upleft;					// Pointer to adjacent elements
	ComplexShipElement *			m_upright;					// Pointer to adjacent elements
	ComplexShipElement *			m_downright;				// Pointer to adjacent elements
	ComplexShipElement *			m_downleft;					// Pointer to adjacent elements

	// Flags determining which directions this element will connect in
	bool m_cleft, m_cup, m_cright, m_cdown, m_cupleft, m_cupright, m_cdownright, m_cdownleft, m_czup, m_czdown;

	// Data on the navigation nodes and connections to be created within this element.  
	// If none are specified then default nodes will be generated during pathfinding
	NavNodePos *					m_navnodepositions;
	int								m_numnavnodepositions;
	NavNodeConnection *				m_navnodeconnections;
	int								m_numnavnodeconnections;

public:
	// Debug variables to log instance creation
	#if defined(_DEBUG) && defined(DEBUG_LOGINSTANCECREATION) 
		static long inst_con;
		static long inst_des;
	#endif
};


#endif