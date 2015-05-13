#include "ErrorCodes.h"
#include "iSpaceObject.h"
#include "Octree.h"
#include "GameSpatialPartitioningTrees.h"
#include "CapitalShipPerimeterBeacon.h"

// Default constructor
CapitalShipPerimeterBeacon::CapitalShipPerimeterBeacon(void)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::CapitalShipPerimeterBeaconObject);

	// Initialise key fields to default values
	m_parentship = NULL;
	m_parentshipid = 0;
	Active = false;
	BeaconPos = NULL_VECTOR;
}

// Links this beacon to the specified parent ship, creating space object attachments to keep it in place during movement
void CapitalShipPerimeterBeacon::AssignToShip(iSpaceObject *ship, D3DXVECTOR3 position)
{
	// If we already have a parent ship then we need to break links with it now
	if (m_parentship)
	{
		// Attempt to break any attachment that currently exists between a parent and this object
		this->DetachFromParent();

		// Clear our pointers to the parent ship
		m_parentship = NULL;
		m_parentshipid = 0;
		BeaconPos = NULL_VECTOR;
		Active = false;
	}

	// If we are already in the spatial partitioning tree then also remove first
	if (m_treenode) m_treenode->RemoveItem(this);

	// Now first make sure the ship is a valid one
	if (!ship) return;

	// Attempt to link the beacon to this ship at the specified point
	//Result result = SpaceObjectAttachment::Create(ship, this, position, ID_QUATERNION);
	//if (result != ErrorCodes::NoError) return;

	// Apply the attachment, which will move the beacon into the ship's reference frame and calculate its world position
	//if (m_attachparent) m_attachparent->ApplyAttachment();

	// We have succeeded in linking this beacon to the ship, so set relevant parent pointers now
	m_parentship = ship;
	m_parentshipid = ship->GetID();
	BeaconPos = position;

	// Add this beacon to the spatial partitioning tree
	//Game::SpatialPartitioningTree->AddItem(this, m_position);
}

// Shutdown method for a beacon, to break any link to a parent ship and deallocate all resources 
void CapitalShipPerimeterBeacon::Shutdown(void)
{
	// If we are linked to a ship then attempt to cleanly break the attachment before being deallocated
	if (m_parentship)
	{
		// Break the attachment to this parent ship
		this->DetachFromParent();
		m_parentship = NULL;
	}
}

// Destructor; attempts to break the attachment to any parent ship before deallocating the beacon
CapitalShipPerimeterBeacon::~CapitalShipPerimeterBeacon(void)
{
	// If we are linked to a ship then attempt to cleanly break the attachment before being deallocated
	// Logic repeated from shutdown method to make sure this is always called
	if (m_parentship)
	{
		// Break the attachment to this parent ship
		this->DetachFromParent();
		m_parentship = NULL;
	}
}
