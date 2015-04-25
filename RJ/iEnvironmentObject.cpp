#include "iSpaceObjectEnvironment.h"
#include "SimulationStateManager.h"
#include "iActiveObject.h"

#include "iEnvironmentObject.h"


// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void iEnvironmentObject::InitialiseCopiedObject(iEnvironmentObject *source)
{
	// Pass control to all base classes
	iActiveObject::InitialiseCopiedObject((iActiveObject*)source);
}

// Moves this object into a new environment
void iEnvironmentObject::MoveIntoEnvironment(iSpaceObjectEnvironment *env)
{
	// If we have a current environment, we need to inform it and the state manager that we are leaving the environment
	if (m_parent)
	{
		m_parent->ObjectLeavingEnvironment(this);
		Game::StateManager.ObjectLeavingInteriorEnvironment(this, m_parent);
	}

	// Store the new environment
	m_parent = env;

	// Inform the new environment and state manager that we are entering the environment, assuming it is not null
	if (m_parent)
	{
		m_parent->ObjectEnteringEnvironment(this);
		Game::StateManager.ObjectEnteringInteriorEnvironment(this, m_parent);
	}

	// Move the object to be within the environment (although this may not be accessible space) as an initial state
	this->SetEnvironmentPositionAndOrientation(NULL_VECTOR, ID_QUATERNION);
}

// Sets the position of this object relative to its parent environment, recalculating all derived fields in the process
void iEnvironmentObject::SetEnvironmentPosition(const D3DXVECTOR3 & pos)
{
	// Store the new relative position
	m_envposition = pos;

	// Recalculate all dependent data
	RecalculateEnvironmentPositionData();
}

// Sets the orientation of this object relative to its parent environment, recalculating all derived fields in the process
void iEnvironmentObject::SetEnvironmentOrientation(const D3DXQUATERNION & orient)
{
	// Store the new relative orientation
	m_envorientation = orient;

	// Recalculate all dependent data
	RecalculateEnvironmentOrientationData();
}

// Sets the position & orientation of this object relative to its parent environment, recalculating all derived fields in the process
void iEnvironmentObject::SetEnvironmentPositionAndOrientation(const D3DXVECTOR3 & pos, const D3DXQUATERNION & orient)
{
	// Store the new relative position & orientation
	m_envposition = pos;
	m_envorientation = orient;

	// Recalculate all dependent data
	RecalculateEnvironmentPositionAndOrientationData();
}

void iEnvironmentObject::RecalculateEnvironmentPositionData(void)
{
	if (m_parent)
	{
		// Transform the relative position by the parent world matrix to get this object's absolute position
		D3DXVECTOR3 pos;
		D3DXVec3TransformCoord(&pos, &m_envposition, m_parent->GetZeroPointWorldMatrix());
		SetPosition(pos);

		// Also determine the element range (usually just 1 element) that this object now exists in.  Swap Y and Z coords since we are
		// moving from space to environment coordinates
		INTVECTOR3 min_element = INTVECTOR3((int)floorf((m_envposition.x - m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP),
											(int)floorf((m_envposition.z - m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP),
											(int)floorf((m_envposition.y - m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP));
		INTVECTOR3 max_element = INTVECTOR3((int)floorf((m_envposition.x + m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP),
											(int)floorf((m_envposition.z + m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP),
											(int)floorf((m_envposition.y + m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP));

		// If the element range has changed, update our record and trigger an update in the parent environment mappings
		if (min_element != m_parent_element_min || max_element != m_parent_element_max)
		{
			// Trigger an update of the environment record, that tracks the location of objects within it
			m_parent->ObjectMoved(this, m_parent_element_min, m_parent_element_max, min_element, max_element);

			// Store the new element range
			m_parent_element_min = min_element;
			m_parent_element_max = max_element;

			// Store a flag indicating whether we now span multiple elements, for render-time efficiency
			m_multielement = (m_parent_element_min != m_parent_element_max);

			// Adopt the simulation state of these new elements.  We take the highest level of simulation if it differs across the element range
			UpdateSimulationStateFromParentElements();
		}
	}
	else
	{
		// If we have no parent object, absolute position == relative position
		SetPosition(m_envposition);
	}
}

void iEnvironmentObject::RecalculateEnvironmentOrientationData(void)
{
	if (m_parent)
	{
		// Multiply in the relative orientation to the parent's to get this object's absolute orientation
		SetOrientation(m_envorientation * m_parent->GetOrientation());
	}
	else
	{
		// If we have no parent object, absolute position == relative position
		SetOrientation(m_envorientation);
	
	}

	// Recalculate intermediate orientation matrices based on our current state, for more efficient runtime performance
	D3DXMatrixRotationQuaternion(&m_orientationmatrix, &m_envorientation);			// Cache the (environment-relative) orientation matrix for this object
	D3DXMatrixInverse(&m_inverseorientationmatrix, NULL, &m_orientationmatrix);		// Cache the inverse orientation matrix, (also relative to the current environment)
}

void iEnvironmentObject::RecalculateEnvironmentPositionAndOrientationData(void)
{
	if (m_parent)
	{
		// Derive absolute position & orientation based on the parent object's state
		D3DXVECTOR3 pos;
		D3DXVec3TransformCoord(&pos, &m_envposition, m_parent->GetZeroPointWorldMatrix());
		SetPosition(pos);
		SetOrientation(m_envorientation * m_parent->GetOrientation());

		// Also determine the element range (usually just 1 element) that this object now exists in.  Swap Y and Z coords since
		// we are moving from space to environment coordinates
		INTVECTOR3 min_element = INTVECTOR3((int)floorf((m_envposition.x - m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP),
											(int)floorf((m_envposition.z - m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP),
											(int)floorf((m_envposition.y - m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP));
		INTVECTOR3 max_element = INTVECTOR3((int)floorf((m_envposition.x + m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP),
											(int)floorf((m_envposition.z + m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP),
											(int)floorf((m_envposition.y + m_collisionsphereradius) * Game::C_CS_ELEMENT_SCALE_RECIP));

		// If the element range has changed, update our record and trigger an update in the parent environment mappings
		if (min_element != m_parent_element_min || max_element != m_parent_element_max)
		{
			// Trigger an update of the envionrment record, that tracks the location of objects within it
			m_parent->ObjectMoved(this, m_parent_element_min, m_parent_element_max, min_element, max_element);
		
			// Store the new element range
			m_parent_element_min = min_element;
			m_parent_element_max = max_element;

			// Store a flag indicating whether we now span multiple elements, for render-time efficiency
			m_multielement = (m_parent_element_min != m_parent_element_max);

			// Adopt the simulation state of these new elements.  We take the highest level of simulation if it differs across the element range
			UpdateSimulationStateFromParentElements();
		}
	}
	else
	{
		// If we have no parent object, absolute values == relative values
		SetPosition(m_envposition);
		SetOrientation(m_envorientation);
	}

	// Recalculate intermediate orientation matrices based on our current state, for more efficient runtime performance
	D3DXMatrixRotationQuaternion(&m_orientationmatrix, &m_envorientation);			// Cache the (environment-relative) orientation matrix for this object
	D3DXMatrixInverse(&m_inverseorientationmatrix, NULL, &m_orientationmatrix);		// Cache the inverse orientation matrix, (also relative to the current environment)
}


// Adopts the simulation state of our parent elements.  Takes the "most" simulated state if this differs across the element range
void iEnvironmentObject::UpdateSimulationStateFromParentElements(void)
{
	// Parameter check; make sure we have a valid parent environment before proceeding
	if (!m_parent) return;

	// We will not simulate the object by default
	iObject::ObjectSimulationState state = iObject::ObjectSimulationState::NoSimulation;

	// Loop across the range of elements that this object exists in
	ComplexShipElement *el; iObject::ObjectSimulationState elstate;
	for (int x = m_parent_element_min.x; x <= m_parent_element_max.x; ++x)
	{
		for (int y = m_parent_element_min.y; y <= m_parent_element_max.y; ++y)
		{
			for (int z = m_parent_element_min.z; z <= m_parent_element_max.z; ++z)
			{
				// Attempt to retrieve the element at this location
				el = m_parent->GetElement(x, y, z);
				if (!el) continue;

				// Get the simulation state of this element, and upgrade our own state if the element state is "greater"
				elstate = el->GetSimulationState();
				if (iObject::CompareSimulationStates(elstate, state) == ComparisonResult::GreaterThan)
				{
					state = elstate;
				}
			}
		}
	}

	// We can now set the simulation state of this object.  No action will be taken if the state has not changed
	SetSimulationState(state);
}

// Event raised whenever the object has a significant collision with the terrain.  "Significant" denotes impacts greater than 
// a defined threshold, so excluding e.g. normal floor collisions
void iEnvironmentObject::CollisionWithTerrain(const GamePhysicsEngine::TerrainImpactData & impact)
{

}

// Add delta orientation; relative to the object environment
void iEnvironmentObject::AddDeltaOrientation(const D3DXQUATERNION &dq)
{
	// Add the delta value
	m_envorientation *= dq;

	// Check whether we have passed the threshold number of quaternion operations that would necessitate a re-normalisation
	if (++m_orientchanges > QUATERNION_NORMALISATION_THRESHOLD)
	{
		// Normalise the orientation and reset our counter for the next normalisation
		D3DXQuaternionNormalize(&m_envorientation, &m_envorientation);
		m_orientchanges = 0;
	}

	// Recalculate all derived fields
	RecalculateEnvironmentOrientationData();
}

// Performs all physics simulation for this environment object
void iEnvironmentObject::SimulateObjectPhysics(void)
{
	// Compose a local momentum change vector during these operations, and then apply one transform at the end
	D3DXVECTOR3 lm_delta = NULL_VECTOR;

	// Apply gravity to the object, if it is in a non-zero gravity environment
	ComplexShipElement *el = m_parent->GetElement(Game::PhysicalPositionToElementLocation(m_envposition));
	if (el && el->GetGravityStrength() > Game::C_EPSILON)
	{
		// Apply this downward (relative to the environment) gravity force to the object
		lm_delta.y = -(el->GetGravityStrength() * Game::TimeFactor);
	}

	// Apply drag in the local x & z dimensions, to quickly slow the entity if it is not trying to move
	float drag = (/*Game::C_ENVIRONMENT_MOVE_DRAG_FACTOR*/ 18.0f * Game::TimeFactor);
	
	if		(PhysicsState.LocalMomentum.x > 0.0f) lm_delta.x = -min(drag, PhysicsState.LocalMomentum.x);
	else if (PhysicsState.LocalMomentum.x < 0.0f) lm_delta.x = min(drag, -PhysicsState.LocalMomentum.x);

	if		(PhysicsState.LocalMomentum.z > 0.0f) lm_delta.z = -min(drag, PhysicsState.LocalMomentum.z);
	else if (PhysicsState.LocalMomentum.z < 0.0f) lm_delta.z = min(drag, -PhysicsState.LocalMomentum.z);


	// We have now composed all dimensions of the delta local momentum vector; apply it to the object now
	ApplyLocalLinearForceDirect(lm_delta);

	// If we have momentum then apply the change to our position now
	if (!IsZeroVector(PhysicsState.WorldMomentum))
	{
		// Move the object based on its external momentum, relative to the environment
		SetEnvironmentPosition(m_envposition + (PhysicsState.WorldMomentum * Game::TimeFactor));
	}
}





