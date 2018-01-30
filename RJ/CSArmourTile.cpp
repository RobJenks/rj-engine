#include "CSArmourTileDefinition.h"
#include "CSArmourTile.h"

// Default constructor
CSArmourTile::CSArmourTile(void)
{
	// This tile does not require periodic simulation
	DeactivateSimulation();
}


// Virtual inherited method to make a copy of this tile and return it
ComplexShipTile * CSArmourTile::Copy(void) const
{
	// Call the copy constructor, which will in turn call the base class copy constructor, to create a copy of this tile
	return new CSArmourTile(*this);
}

// Virtual & static methods respectively to generate and read XML data representing the tile
TiXmlElement * CSArmourTile::GenerateXML(void)
{
	// Create a new node to hold this data
	TiXmlElement *node = new TiXmlElement("ComplexShipTile");
	node->SetAttribute("code", m_definition->GetCode().c_str());

	// First, have the base class generate XML for all common tile properties
	TiXmlElement *base = ComplexShipTile::GenerateBaseClassXML(this);
	if (!base) { delete node; return NULL; }
	node->LinkEndChild(base);

	// Now generate XML for all properties specific to this tile class
	/* No tile-specific properties right now */

	// Return a reference to the new node
	return node;
}

// Virtual method to read any class-specific data for this tile type
void CSArmourTile::ReadClassSpecificXMLData(TiXmlElement *node)
{

}

// Processes a debug tile command from the console
void CSArmourTile::ProcessDebugTileCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros for convenience
	INIT_DEBUG_TILE_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, Parameter(2) is the already-matched tile ID, and Parameter(3) is the function name
	// We therefore pass Parameter(4) onwards as arguments

	// Accessor methods
	// REGISTER_DEBUG_TILE_ACCESSOR_FN(...)

	// Mutator methods
	REGISTER_DEBUG_TILE_FN(RecalculateParameters)


	// Pass back to our base class if we could not handle the command
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		ComplexShipTile::ProcessDebugTileCommand(command);
}

// Default destructor
CSArmourTile::~CSArmourTile(void)
{

}