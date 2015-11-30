#include "iSpaceObject.h"
#include "TurretController.h"

// Default constructor
TurretController::TurretController(void)
	: m_parent(NULL), m_turretcount(0), m_active(false)
{
}


// Perform a full update of the turret collection
void TurretController::Update(std::vector<iSpaceObject*> & enemy_contacts)
{
	// Check to make sure there is something to update; we will do nothing if there are no turrets
	if (!m_parent || !m_active) return;

	// Take different actions based on simulation state
	iObject::ObjectSimulationState simulationstate = m_parent->SimulationState();
	if (simulationstate == iObject::ObjectSimulationState::FullSimulation)
	{
		/* Full simulation; model individual turret orientations, firing arcs etc */

		// Iterate over every turret and run its full-simulation update
		TurretCollection::iterator it_end = Turrets.end();
		for (TurretCollection::iterator it = Turrets.begin(); it != it_end; ++it)
		{
			if ((*it)) (*it)->Update(enemy_contacts);
		}
	}
}

// Sets the control mode of all turrets
void TurretController::SetControlModeOfAllTurrets(SpaceTurret::ControlMode mode)
{
	// Iterate over every turret and set the desired control mode
	TurretCollection::iterator it_end = Turrets.end();
	for (TurretCollection::iterator it = Turrets.begin(); it != it_end; ++it)
	{
		if ((*it)) (*it)->SetControlMode(mode);
	}
}

// Set a refernce to the parent object that owns this turret controller
void TurretController::SetParent(iSpaceObject *parent)
{
	// Store a reference to the new parent object
	m_parent = parent;

	// Update turret collection status
	RefreshTurretCollection();

	// Notify each turret of its new parent object
	for (int i = 0; i < m_turretcount; ++i)
	{
		Turrets[i]->SetParent(m_parent);
	}
}

// Add a new turret to the collection, assuming it doesn't already exist
bool TurretController::AddTurret(SpaceTurret *turret)
{
	// Parameter check, and make sure this turret is not already in the collection
	if (!turret || HaveTurret(turret)) return false;

	// Add to the turret collection
	Turrets.push_back(turret);

	// Notify the turret of its parent object
	turret->SetParent(m_parent);

	// Update turret collection status
	RefreshTurretCollection();

	// Return success
	return true;
}

// Remove the specified turret from the collection, if it exists
bool TurretController::RemoveTurret(SpaceTurret *turret)
{
	// Parameter check
	if (!turret) return false;

	// Attempt to locate the turret
	for (int i = 0; i < m_turretcount; ++i)
	{
		if (Turrets[i] == turret)
		{
			// Remove the item
			RemoveFromVectorAtIndex<SpaceTurret*>(Turrets, i);

			// The turret is no longer owned by our parent object
			turret->SetParent(NULL);

			// Update turret collection status
			RefreshTurretCollection();

			// Return success
			return true;
		}
	}

	// We did not find the turret in this collection
	return false;
}

// Remove all turrets from the collection
void TurretController::RemoveAllTurrets(void)
{
	// Notify all turrets that they are no longer owned by our parent object
	for (int i = 0; i < m_turretcount; ++i)
	{
		Turrets[i]->SetParent(NULL);
	}

	// Clear the turret collection
	Turrets.clear();
	
	// Update turret collection status
	RefreshTurretCollection();
}


// Test whether the specified turret exists in this collection
bool TurretController::HaveTurret(SpaceTurret *turret)
{
	TurretCollection::const_iterator it_end = Turrets.end();
	for (TurretCollection::const_iterator it = Turrets.begin(); it != it_end; ++it)
	{
		if ((*it) == turret) return true;
	}
	return false;
}

// Refreshes the turret controller status following a change to internal (e.g. turret removed) or external (e.g.
// parent object) changes
void TurretController::RefreshTurretCollection(void)
{
	// Cache the turret collection size for runtime efficiency
	m_turretcount = (int)Turrets.size();

	// Controller will be considered if it is managing at least one turret
	m_active = (m_turretcount != 0);
}

// Clears all controller contents without affecting any of the turrets themselves.  Generally used post-clone
// to reset the controller without resetting the parent pointer of the (old) turrets being removed
void TurretController::ForceClearContents(void)
{
	// Simply remove all turret pointers and refresh the internal collection state
	Turrets.clear();
	RefreshTurretCollection();
}

// Default destructor
TurretController::~TurretController(void)
{
	// Remove any turrets managed by this controller
	RemoveAllTurrets();
}