#include <unordered_map>
#include <algorithm>
#include "Logging.h"
#include "Ship.h"
#include "SpaceTurret.h"
#include "ProjectileLauncher.h"
#include "BasicProjectileDefinition.h"
#include "SpaceProjectileDefinition.h"
#include "TurretController.h"


// Default constructor
TurretController::TurretController(void)
	: m_parent(NULL), m_turretcount(0U), m_active(false)
{
}


// Perform a full update of the turret collection
void TurretController::Update(std::vector<ObjectReference<iSpaceObject>> & enemy_contacts)
{
	// Check to make sure there is something to update; we will do nothing if there are no turrets
	if (!m_parent || !m_active) return;

	// Take different actions based on simulation state
	iObject::ObjectSimulationState simulationstate = m_parent->SimulationState();
	if (simulationstate == iObject::ObjectSimulationState::FullSimulation)
	{
		/* Full simulation; model individual turret orientations, firing arcs etc */

		// Iterate over every turret and run its full-simulation update
		TurretCollection::iterator it_end = m_turrets.end();
		for (TurretCollection::iterator it = m_turrets.begin(); it != it_end; ++it)
		{
			if ((*it)) (*it)->Update(enemy_contacts);
		}
	}
}

// Sets the control mode of all turrets
void TurretController::SetControlModeOfAllTurrets(SpaceTurret::ControlMode mode)
{
	// Iterate over every turret and set the desired control mode
	TurretCollection::iterator it_end = m_turrets.end();
	for (TurretCollection::iterator it = m_turrets.begin(); it != it_end; ++it)
	{
		if ((*it)) (*it)->SetControlMode(mode);
	}
}

// Set a refernce to the parent object that owns this turret controller
void TurretController::SetParent(Ship *parent)
{
	// Store a reference to the new parent object
	m_parent = parent;

	// Update turret collection status
	RefreshTurretCollection();

	// Notify each turret of its new parent object
	for (TurretCollection::size_type i = 0; i < m_turretcount; ++i)
	{
		if (m_turrets[i]) m_turrets[i]->SetParent(m_parent);
	}
}

// Add a new turret to the collection, assuming it doesn't already exist
bool TurretController::AddTurret(SpaceTurret *turret)
{
	// Parameter check, and make sure this turret is not already in the collection
	if (!turret) return false;
	if (HaveTurret(turret) || HaveTurretByID(turret->GetTurretID()))
	{
		Game::Log << LOG_WARN << "Attempted to add turret " << turret->GetTurretID() << " to turret controller, but it already exists\n";
		return false;
	}

	// Add to the turret collection and the indexed collection
	m_turrets.push_back(turret);
	m_indexed_turrets[turret->GetTurretID()] = turret;

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
	return (turret && RemoveTurretByID(turret->GetTurretID()));
}

// Remove a turret based on its unique turret ID
bool TurretController::RemoveTurretByID(SpaceTurret::TurretID id)
{
	// Attempt to locate in the indexed collection
	SpaceTurret *turret = GetTurretByID(id);
	if (!turret) return false;

	// Also attempt to locate the turret in the main collection
	for (TurretCollection::size_type i = 0; i < m_turretcount; ++i)
	{
		if (m_turrets[i]->GetTurretID() == id)
		{
			// Remove the item
			RemoveFromVectorAtIndex<SpaceTurret*>(m_turrets, i);

			// The turret is no longer owned by our parent object
			turret->SetParent(NULL);

			// Update turret collection status
			RefreshTurretCollection();

			// Return success
			return true;
		}
	}

	// Note: if we got this far, the turret DID exist in the indexed collection but not in 
	// the turret vector.  Raise a warning because something is obviously wrong
	Game::Log << LOG_WARN << "Attmepted to remove turret " << id << " from turret controller; located in indexed collection but NOT found in turret vector\n";

	// We did not find the turret in this collection
	return false;
}


// Remove all turrets from the collection
void TurretController::RemoveAllTurrets(void)
{
	// Notify all turrets that they are no longer owned by our parent object
	for (TurretCollection::size_type i = 0; i < m_turretcount; ++i)
	{
		if (m_turrets[i]) m_turrets[i]->SetParent(NULL);
	}

	// Clear the turret collections
	m_turrets.clear();
	m_indexed_turrets.clear();
	
	// Update turret collection status
	RefreshTurretCollection();
}


// Test whether the specified turret exists in this collection
bool TurretController::HaveTurret(SpaceTurret *turret)
{
	return (turret != NULL && m_indexed_turrets.count(turret->GetTurretID()) != 0);
}

// Refreshes the turret controller status following a change to internal (e.g. turret removed) or external (e.g.
// parent object) changes
void TurretController::RefreshTurretCollection(void)
{
	// Cache the turret collection size for runtime efficiency
	m_turretcount = m_turrets.size();

	// Controller will be considered active if it is managing at least one turret
	m_active = (m_turretcount != 0U);

	// Reset any statistics that are based on the contents of the turret collection
	m_avg_projectile_velocity = 0.0f;

	// Now recalculate those statistics, assuming there are any turrets in the collection
	SpaceTurret *t; ProjectileLauncher *l; int count = 0;
	TurretCollection::const_iterator it_end = m_turrets.end();
	for (TurretCollection::const_iterator it = m_turrets.begin(); it != it_end; ++it)
	{
		// We need to consider each launcher in turn; maintain a count of the total launchers processed
		t = (*it); if (!t) continue;
		int ln = t->GetLauncherCount();
		count += ln;

		for (int li = 0; li < ln; ++li)
		{
			// Process for each launcher in each turret in the collection
			l = t->GetLauncher(li); if (!l) continue;
			m_avg_projectile_velocity += l->GetLaunchVelocity();
		}
	}

	// Calculate average statistics as long as there was at least one valid launcher to process
	if (count != 0)
	{
		m_avg_projectile_velocity /= count;
	}
	else
	{
		m_avg_projectile_velocity = 1.0f;
	}
}

// Sets the current target for all ship turrets 
void TurretController::SetTarget(iSpaceObject *target)
{
	if (!m_active) return;
	std::for_each(m_turrets.begin(), m_turrets.end(),
		[&target](SpaceTurret *t) { if (t) t->SetTarget(target); });
}

// Changes the current target for all ship turrets targeting current_target to new_target
void TurretController::ChangeTarget(iSpaceObject *current_target, iSpaceObject *new_target)
{
	if (!m_active) return;
	std::for_each(m_turrets.begin(), m_turrets.end(),
		[&current_target, &new_target](SpaceTurret *t) { if (t && t->GetTarget() == current_target) t->SetTarget(new_target); });
}

// Sets the designated target for all ship turrets 
void TurretController::SetDesignatedTarget(iSpaceObject *designated_target)
{
	if (!m_active) return;
	std::for_each(m_turrets.begin(), m_turrets.end(),
		[&designated_target](SpaceTurret *t) { if (t) t->DesignateTarget(designated_target); });
}

// Changes the designated target for all ship turrets targeting current_designation to new_designation
void TurretController::ChangeDesignatedTarget(iSpaceObject *current_designation, iSpaceObject *new_designation)
{
	if (!m_active) return;
	std::for_each(m_turrets.begin(), m_turrets.end(),
		[&current_designation, &new_designation](SpaceTurret *t) 
			{ if (t && t->GetDesignatedTarget() == current_designation) t->DesignateTarget(new_designation); });
}

// Fires all turrets that can currently launch a projectile along the specififed target vector.  Turrets
// which are not currently aligned to the vector will do nothing
void TurretController::FireTurretsAlongVector(const FXMVECTOR target_vector)
{
	if (!m_active) return;
	std::for_each(m_turrets.begin(), m_turrets.end(),
		[target_vector](SpaceTurret *t) { if (t && t->IsCannonAlignedToVector(target_vector)) t->Fire(); });
}


// Clears all controller contents without affecting any of the turrets themselves.  Generally used post-clone
// to reset the controller without resetting the parent pointer of the (old) turrets being removed
void TurretController::ForceClearContents(void)
{
	// Simply remove all turret pointers and refresh the internal collection state
	m_turrets.clear();
	m_indexed_turrets.clear();
	RefreshTurretCollection();
}

// Default destructor
TurretController::~TurretController(void)
{
	// Remove any turrets managed by this controller
	RemoveAllTurrets();
}