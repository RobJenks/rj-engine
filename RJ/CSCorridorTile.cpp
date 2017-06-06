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

// Apply the contents of the tile to its parent objects.  Called upon linking, plus on repair of the ship.  Inherited virtual.
void CSCorridorTile::ApplyTileSpecific(void)
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


// Processes a debug tile command from the console
void CSCorridorTile::ProcessDebugTileCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros for convenience
	INIT_DEBUG_TILE_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, Parameter(2) is the already-matched tile ID, and Parameter(3) is the function name
	// We therefore pass Parameter(4) onwards as arguments

	// Accessor methods
		
	// Mutator methods
	REGISTER_DEBUG_TILE_FN(PerformTileSimulation, (unsigned int)command.ParameterAsInt(4))

	// Pass back to our base class if we could not handle the command
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		ComplexShipTile::ProcessDebugTileCommand(command);

}


