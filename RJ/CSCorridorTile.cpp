#include "Utility.h"
#include "GameDataExtern.h"
#include "XMLGenerator.h"
#include "ComplexShipTileClass.h"
#include "ComplexShipTileDefinition.h"

#include "CSCorridorTile.h"
class ComplexShip;

// Default constructor
CSCorridorTile::CSCorridorTile(void)
{	
	// Corridor tiles will not be simulated per frame
	DeactivateSimulation();

	// Initialise default values
}

// The pre-parent-link event exposed by the base tile class
void CSCorridorTile::BeforeLinkToParent(ComplexShip *ship)
{

}

// The post-parent-link event exposed by the base tile class
void CSCorridorTile::AfterLinkToParent(ComplexShip *ship)
{

}

// The pre-parent-unlink event exposed by the base tile class
void CSCorridorTile::BeforeUnlinkFromParent(void)
{

}

// The post-parent-ulink event exposed by the base tile class
void CSCorridorTile::AfterUnlinkFromParent(ComplexShip *oldship)
{

}


// Apply the contents of the tile to its parent objects.  Called upon linking, plus on repair of the ship.  Inherited virtual.
void CSCorridorTile::ApplyTileSpecific(ComplexShipElement *el)
{

}

// Virtual inherited method to make a copy of this tile and return it
ComplexShipTile *CSCorridorTile::Copy(void) const
{
	// Call the copy constructor, which will in turn call the base class copy constructor, to create a copy of this tile
	return new CSCorridorTile(*this);
}

// Copy constructor
CSCorridorTile::CSCorridorTile(const CSCorridorTile &C) : ComplexShipTile(C)
{
	// Copy any class-specific properties 
}

// Default destructor
CSCorridorTile::~CSCorridorTile(void)
{
}


// Static class method to generate XML data for a corridor tile
TiXmlElement *CSCorridorTile::GenerateXML(void)
{
	// Create a new node to hold this data
	TiXmlElement *node = new TiXmlElement(D::NODE_ComplexShipTile);
	node->SetAttribute("code", m_definition->GetCode().c_str());

	// First, have the base class generate XML for all common tile properties
	TiXmlElement *base = ComplexShipTile::GenerateBaseClassXML(this);
	if (!base) { delete node; return NULL; }
	node->LinkEndChild(base);

	// Now generate XML for all properties specific to this tile class
	/* No corridor tile-specific properties right now */

	// Return a reference to the new node
	return node;
}