#include "iSpaceObjectEnvironment.h"
#include "SimulationStateManager.h"
#include "iActiveObject.h"
#include "ObjectReference.h"
#include "EnvironmentTree.h"

#include "iEnvironmentObject.h"


#include "CoreEngine.h"
#include "OverlayRenderer.h"


// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void iEnvironmentObject::InitialiseCopiedObject(iEnvironmentObject *source)
{
	// Pass control to all base classes
	iActiveObject::InitialiseCopiedObject(source);
}

// Moves this object into a new environment
void iEnvironmentObject::MoveIntoEnvironment(iSpaceObjectEnvironment *env)
{
	// If we have a current environment, we need to inform it and the state manager that we are leaving the environment
	if (m_parent())
	{
		m_parent()->ObjectLeavingEnvironment(this);
		Game::StateManager.ObjectLeavingInteriorEnvironment(this, m_parent());
	}

	// Store the new environment
	m_parent = env;

	// Inform the new environment and state manager that we are entering the environment, assuming it is not null
	if (env)
	{
		m_parent()->ObjectEnteringEnvironment(this);
		Game::StateManager.ObjectEnteringInteriorEnvironment(this, m_parent());
	}

	// Move the object to be within the environment (although this may not be accessible space) as an initial state
	this->SetEnvironmentPositionAndOrientation(NULL_VECTOR, ID_QUATERNION);
}

// Sets the position of this object relative to its parent environment, recalculating all derived fields in the process
void iEnvironmentObject::SetEnvironmentPosition(const FXMVECTOR pos)
{
	// Store the new relative position
	m_envposition = pos;

	// Recalculate all dependent data
	RecalculateEnvironmentPositionData();
}

// Sets the orientation of this object relative to its parent environment, recalculating all derived fields in the process
void iEnvironmentObject::SetEnvironmentOrientation(const FXMVECTOR orient)
{
	// Store the new relative orientation
	m_envorientation = orient;

	// Recalculate all dependent data
	RecalculateEnvironmentOrientationData();
}

// Changes the orientation of this object relative to its parent environment, recalculating all derived fields in the process
void iEnvironmentObject::ChangeEnvironmentOrientation(const FXMVECTOR orient_delta)
{
	// Store the new relative orientation
	m_envorientation = XMQuaternionMultiply(m_envorientation, orient_delta);

	// Recalculate all dependent data
	RecalculateEnvironmentOrientationData();
}

// Sets the position & orientation of this object relative to its parent environment, recalculating all derived fields in the process
void iEnvironmentObject::SetEnvironmentPositionAndOrientation(const FXMVECTOR pos, const FXMVECTOR orient)
{
	// Store the new relative position & orientation
	m_envposition = pos;
	m_envorientation = orient;

	// Recalculate all dependent data
	RecalculateEnvironmentPositionAndOrientationData();
}

void iEnvironmentObject::RecalculateEnvironmentPositionData(void)
{
	if (m_parent())
	{
		// Transform the relative position by the parent world matrix to get this object's absolute position
		SetPosition(XMVector3TransformCoord(m_envposition, m_parent()->GetZeroPointWorldMatrix()));

		// Determine the object element location, and update relevant data if we have just moved between elements
		const INTVECTOR3 element_location = Game::PhysicalPositionToElementLocation(m_envposition);
		if (element_location != m_element_location && m_env_treenode)
		{
			m_element_location = element_location;
			if (element_location >= NULL_INTVECTOR3 && element_location < m_parent()->GetElementSize())
			{
				m_within_env = true;
				m_env_treenode->ItemMoved(this);
			}
			else
			{
				m_within_env = false;
			}
		}
	}
	else
	{
		// If we have no parent object, absolute position == relative position
		SetPosition(m_envposition);
		m_within_env = false;
	}
}

void iEnvironmentObject::RecalculateEnvironmentOrientationData(void)
{
	if (m_parent())
	{
		// Multiply in the relative orientation to the parent's to get this object's absolute orientation
		SetOrientation(XMQuaternionMultiply(m_envorientation, m_parent()->GetOrientation()));
	}
	else
	{
		// If we have no parent object, absolute position == relative position
		SetOrientation(m_envorientation);
	}

	// Recalculate intermediate orientation matrices based on our current state, for more efficient runtime performance
	m_orientationmatrix = XMMatrixRotationQuaternion(m_envorientation);			// Cache the (environment-relative) orientation matrix for this object
	m_inverseorientationmatrix = XMMatrixInverse(NULL, m_orientationmatrix);	// Cache the inverse orientation matrix, (also relative to the current environment)
}

void iEnvironmentObject::RecalculateEnvironmentPositionAndOrientationData(void)
{
	if (m_parent())
	{
		// Derive absolute position & orientation based on the parent object's state
		SetPosition(XMVector3TransformCoord(m_envposition, m_parent()->GetZeroPointWorldMatrix()));
		SetOrientation(XMQuaternionMultiply(m_envorientation, m_parent()->GetOrientation()));

		// Determine the object element location, and update relevant data if we have just moved between elements
		const INTVECTOR3 element_location = Game::PhysicalPositionToElementLocation(m_envposition);
		if (element_location != m_element_location && m_env_treenode)
		{
			m_element_location = element_location;
			if (element_location >= NULL_INTVECTOR3 && element_location < m_parent()->GetElementSize())
			{
				m_within_env = true;
				m_env_treenode->ItemMoved(this);
			}
			else
			{
				m_within_env = false;
			}
		}
	}
	else
	{
		// If we have no parent object, absolute values == relative values
		SetPosition(m_envposition);
		SetOrientation(m_envorientation);
		m_within_env = false;
	}

	// Recalculate intermediate orientation matrices based on our current state, for more efficient runtime performance
	m_orientationmatrix = XMMatrixRotationQuaternion(m_envorientation);			// Cache the (environment-relative) orientation matrix for this object
	m_inverseorientationmatrix = XMMatrixInverse(NULL, m_orientationmatrix);	// Cache the inverse orientation matrix, (also relative to the current environment)
}


// Event raised whenever the object has a significant collision with the terrain.  "Significant" denotes impacts greater than 
// a defined threshold, so excluding e.g. normal floor collisions
void iEnvironmentObject::CollisionWithTerrain(const GamePhysicsEngine::TerrainImpactData & impact)
{

}

// Performs all physics simulation for this environment object
void iEnvironmentObject::SimulateObjectPhysics(void)
{
	// Get a pointer to the parent object
	iSpaceObjectEnvironment *parent = m_parent();

	// Compose a local momentum change vector during these operations, and then apply one transform at the end
	XMFLOAT3 lm, lm_delta = NULL_FLOAT3;
	XMStoreFloat3(&lm, PhysicsState.LocalMomentum);

	// Apply gravity to the object, if it is in a non-zero gravity environment
	if (parent)
	{
		ComplexShipElement *el = parent->GetElement(Game::PhysicalPositionToElementLocation(m_envposition));
		if (el && el->GetGravityStrength() > Game::C_EPSILON)
		{
			// Apply this downward (relative to the environment) gravity force to the object
			lm_delta.y = -(el->GetGravityStrength() * Game::TimeFactor);
		}
	}

	// Apply drag in the local x & z dimensions, to quickly slow the entity if it is not trying to move
	float drag = (/*Game::C_ENVIRONMENT_MOVE_DRAG_FACTOR*/ 18.0f * Game::TimeFactor);
	
	if		(lm.x > 0.0f) lm_delta.x = -min(drag, lm.x);
	else if (lm.x < 0.0f) lm_delta.x = min(drag, -lm.x);

	if		(lm.z > 0.0f) lm_delta.z = -min(drag, lm.z);
	else if (lm.z < 0.0f) lm_delta.z = min(drag, -lm.z);

	// We have now composed all dimensions of the delta local momentum vector; apply it to the object now
	ApplyLocalLinearForceDirect(XMLoadFloat3(&lm_delta));

	// If we have momentum then apply the change to our position now
	if (!IsZeroVector3(PhysicsState.WorldMomentum))
	{
		// Move the object based on its external momentum, relative to the environment
		SetEnvironmentPosition(XMVectorAdd(	m_envposition, XMVectorScale(PhysicsState.WorldMomentum, Game::TimeFactor)));
	}
}


// Shut down the environment object, notifying any parent environment of the change
void iEnvironmentObject::Shutdown(void)
{
	// Notify any parent environment that this object is being removed
	MoveIntoEnvironment(NULL);

	// Pass control to the base class
	iObject::Shutdown();
}

// Custom debug string function
std::string	iEnvironmentObject::DebugString(void) const
{
	return iObject::DebugString(concat("Env=")(m_parent() ? m_parent()->GetInstanceCode() : "(NULL)").str());
}


// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void iEnvironmentObject::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(GetParentEnvironment)
	REGISTER_DEBUG_ACCESSOR_FN(GetEnvironmentPosition)
	REGISTER_DEBUG_ACCESSOR_FN(GetEnvironmentOrientation)
	REGISTER_DEBUG_ACCESSOR_FN(GetElementLocation)
	REGISTER_DEBUG_ACCESSOR_FN(IsWithinEnvironment)
	REGISTER_DEBUG_ACCESSOR_FN(GetEnvironmentTreeNode)
	REGISTER_DEBUG_ACCESSOR_FN(IsOnGround)


	// Mutator methods
	REGISTER_DEBUG_FN(SetEnvironmentPosition, XMVectorSet(command.ParameterAsFloat(2), command.ParameterAsFloat(3), command.ParameterAsFloat(4), 0.0f))
	REGISTER_DEBUG_FN(SetEnvironmentOrientation, XMVectorSet(command.ParameterAsFloat(2), command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5)))
	REGISTER_DEBUG_FN(ChangeEnvironmentOrientation, XMVectorSet(command.ParameterAsFloat(2), command.ParameterAsFloat(3), command.ParameterAsFloat(4), command.ParameterAsFloat(5)))
	REGISTER_DEBUG_FN(AddDeltaPosition, XMVectorSet(command.ParameterAsFloat(2), command.ParameterAsFloat(3), command.ParameterAsFloat(4), 0.0f))
	REGISTER_DEBUG_FN(SimulateObjectPhysics)
	REGISTER_DEBUG_FN(SetGroundFlag, command.ParameterAsBool(2))
	

	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		iActiveObject::ProcessDebugCommand(command);

}


