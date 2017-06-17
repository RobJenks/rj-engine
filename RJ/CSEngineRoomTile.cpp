#include "Modifier.h"
#include "iSpaceObjectEnvironment.h"
#include "Hardpoints.h"
#include "Hardpoint.h"
#include "HpEngine.h"
#include "Engine.h"
#include "CSEngineRoomTileDefinition.h"
#include "CSEngineRoomTile.h"

// Default constructor
CSEngineRoomTile::CSEngineRoomTile(void)
{
	// Activate simulation if this tile does require periodic (as opposed to ad-hoc) simulation
	// ActivateSimulation(...);
}


// Simulation method for this tile
void CSEngineRoomTile::PerformTileSimulation(unsigned int delta_ms)
{
	if (m_powerlevel > 30)
	{
		int a = 1;
	}

	// Update the state of any engines assigned to this engine room
	UpdateEngines();

	// We no longer need to simulate this tile
	SetTileSimulationRequired(false);
}

// Update the state of any engines assigned to this engine room
void CSEngineRoomTile::UpdateEngines(void)
{
	float health_pc = GetHealthPercentage();
	float power_pc = GetPowerSatisfaction();

	// Build required modifier sets.  For now, both thrust and acceleration will vary 
	// linearly with modifiers, so we can use one modifier set and calculation are also simpler
	std::vector<Modifier<float>> modifiers;
	if (health_pc < 1.0f) modifiers.push_back(Modifier<float>(
		Modifier<float>::ModifierType::Multiplicative, health_pc, StandardModifiers::DAMAGE_SUSTAINED));
	if (power_pc < 1.0f) modifiers.push_back(Modifier<float>(
		Modifier<float>::ModifierType::Multiplicative, power_pc, StandardModifiers::INSUFFICIENT_POWER));

	// Apply the modifiers to any engines owned by this tile
	std::vector<std::string>::const_iterator it_end = m_hardpoint_refs.end();
	for (std::vector<std::string>::const_iterator it = m_hardpoint_refs.begin(); it != it_end; ++it)
	{
		Hardpoint *hp = m_parent->GetHardpoints().Get(*it);
		if (!hp) continue;
		if (!hp->GetType() == Equip::Class::Engine) continue;

		HpEngine *hpe = (HpEngine*)hp;
		Engine *engine = hpe->GetEngine();
		if (!engine) continue;
		
		engine->SetMinThrustModifiers(modifiers);
		engine->SetMaxThrustModifiers(modifiers);
		engine->SetAccelerationModifiers(modifiers);

		// Trigger a notification event for this hardpoint
		hpe->NotifyParentOfThrustChange();
	}
}

// Virtual inherited method to make a copy of this tile and return it
ComplexShipTile * CSEngineRoomTile::Copy(void) const
{
	// Call the copy constructor, which will in turn call the base class copy constructor, to create a copy of this tile
	return new CSEngineRoomTile(*this);
}

// Virtual & static methods respectively to generate and read XML data representing the tile
TiXmlElement * CSEngineRoomTile::GenerateXML(void)
{
	// Create a new node to hold this data
	TiXmlElement *node = new TiXmlElement(D::NODE_ComplexShipTile);
	node->SetAttribute("code", m_definition->GetCode().c_str());

	// First, have the base class generate XML for all common tile properties
	TiXmlElement *base = ComplexShipTile::GenerateBaseClassXML(this);
	if (!base) { delete node; return NULL; }
	node->LinkEndChild(base);

	// Now generate XML for all properties specific to this tile class
	/* No tile-specific properties right now */
	// TODO: NOT TRUE

	// Return a reference to the new node
	return node;
}

// Virtual method to read any class-specific data for this tile type
void CSEngineRoomTile::ReadClassSpecificXMLData(TiXmlElement *node)
{

}

// Processes a debug tile command from the console
void CSEngineRoomTile::ProcessDebugTileCommand(GameConsoleCommand & command)
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
CSEngineRoomTile::~CSEngineRoomTile(void)
{

}