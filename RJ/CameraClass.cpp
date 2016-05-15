#include <stdlib.h>
#include "FastMath.h"
#include "Ship.h"
#include "SimpleShip.h"
#include "Ships.h"
#include "Player.h"
#include "CoreEngine.h"
#include "CameraPath.h"
#include "ViewFrustrum.h"

#include "CameraClass.h"

CameraClass::CameraClass(void)
{
	// Set default parameters
	m_camerastate = CameraClass::CameraState::NormalCamera;
	m_shipclass = Ships::Class::Simple;
	m_position = m_fixedposition = m_debugposition = NULL_VECTOR;
	m_orientation = m_fixedorientation = m_debugorientation = ID_QUATERNION;
	m_view = m_invview = ID_MATRIX;
	m_vright = m_vup = m_vforward = NULL_VECTOR;
	m_vrightf = m_vupf = m_vforwardf = NULL_FLOAT3;
	m_camerapath = NULL;
}

CameraClass::~CameraClass(void)
{
}

Result CameraClass::Initialise()
{
	// No initialisation required; return success
	return ErrorCodes::NoError;
}

void CameraClass::CalculateViewMatrix(void)
{
	// Test camera mode - in "normal" state the camera is under the control of the player
	if (m_camerastate == CameraClass::CameraState::NormalCamera)
	{
		// If the player is piloting a ship then we need to apply the additional camera offset matrix
		if (Game::CurrentPlayer->GetState() == Player::StateType::ShipPilot && Game::CurrentPlayer->GetPlayerShip() &&
			Game::CurrentPlayer->GetPlayerShip()->GetShipClass() == Ships::Class::Simple)
		{
			// Get a reference to the player ship; we know it is a SimpleShip
			SimpleShip *ship = (SimpleShip*)Game::CurrentPlayer->GetPlayerShip();

			// Derive the camera offset matrix
			XMMATRIX camoffset = ship->DeriveActualCameraMatrix();

			// Calculate the view matrix based on positional data & the additional offset matrix.  We can use the chase camera
			// orientation which was calculated above based on the current ship movement
			CalculateViewMatrixFromPositionData(ship->GetPosition(), 
												ship->GetUnadjustedOrientation(), camoffset);
		}
		else
		{
			// Otherwise we apply no offset and simply calculate a position/orientation-based view matrix
			CalculateViewMatrixFromPositionData(Game::CurrentPlayer->GetPosition(), 
												Game::CurrentPlayer->GetOrientation(), 
												Game::CurrentPlayer->GetViewOffsetMatrix());
		}
	}
	else if (m_camerastate == CameraClass::CameraState::FixedCamera)
	{
		// If we are using a fixed camera then simply calculate the view matrix based on the fixed position/orientation
		CalculateViewMatrixFromPositionData(m_fixedposition, m_fixedorientation, ID_MATRIX);
	}
	else if (m_camerastate == CameraClass::CameraState::PathCamera)
	{
		// Advance the camera along its path.  Method will return true if the path is now complete
		if (m_camerapath->Advance() == true) 
		{
			// If the path has ended then handle the completion event, and release or fix the camera for next frame
			HandleEndOfCameraPath();
		}
		else
		{
			// Otherwise calculate the view matrix based upon the current camera path position & orientation
			CalculateViewMatrixFromPositionData(m_camerapath->GetCurrentCameraPosition(), 
												m_camerapath->GetCurrentCameraOrientation(), ID_MATRIX);
		}
	}
	else if (m_camerastate == CameraClass::CameraState::DebugCamera)
	{
		// Calculate a view matrix based upon the debug camera position
		CalculateViewMatrixFromPositionData(m_debugposition, m_debugorientation, ID_MATRIX);
	}
	else
	{
		// Error case; revert the camera to normal
		m_camerastate = CameraClass::CameraState::NormalCamera;
	}
}

void CameraClass::CalculateViewMatrixFromPositionData(const FXMVECTOR position, const FXMVECTOR orientation, const CXMMATRIX offsetmatrix)
{
	// Store location/orientation values for use throughout the frame
	m_position = position;
	m_orientation = orientation;
	m_offsetmatrix = offsetmatrix;
		
	// First convert the orientation D3DXQUATERNION into a rotation matrix
	m_rot = XMMatrixRotationQuaternion(orientation);
			
	// Generate translation matrix for the position vector
	m_trans = XMMatrixTranslationFromVector(m_position);

	// Calculate the inverse view matrix by performing (translation[location] > rotation[orient] > translation [camoffset])
	m_inter = XMMatrixMultiply(m_rot, m_trans);
	m_invview = XMMatrixMultiply(offsetmatrix, m_inter);

	// Invert since view matrix is transform from world space into the camera coordinate system, then we are done
	m_view = XMMatrixInverse(NULL, m_invview);

	// Decompose into components that other methods can use for rendering; more efficient to do once here
	DecomposeViewMatrix();
}

void CameraClass::DecomposeViewMatrix(void)
{
	// Decompose view matrix into the constituent parts, which can then be used by other methods as required
	XMVECTOR tmp;
	XMMatrixDecompose(&tmp, &m_vrot, &m_vtrans, m_view);

	// Also decompose the view matrix into its component basis vectors and store them
	// Since these are stored in columns, we take the transpose and then use direct row-access methods to retrieve them
	// m_vright =		D3DXVECTOR3(m_view._11, m_view._21, m_view._31);
	// m_vup =			D3DXVECTOR3(m_view._12, m_view._22, m_view._32);
	// m_vforward =		D3DXVECTOR3(m_view._13, m_view._23, m_view._33);
	XMMATRIX viewT = XMMatrixTranspose(m_view);
	m_vright =		XMVectorSetW(viewT.r[0], 0.0f);
	m_vup =			XMVectorSetW(viewT.r[1], 0.0f);
	m_vforward =	XMVectorSetW(viewT.r[2], 0.0f);

	// Also store local float representations of the basis vectors for more efficient per-component access
	XMStoreFloat3(&m_vrightf, m_vright);
	XMStoreFloat3(&m_vupf, m_vup);
	XMStoreFloat3(&m_vforwardf, m_vforward);

	// Use the forward basis vector (which we can assume is normalised, if DX is following correct matrix algebra,
	// to derive the camera yaw and pitch values.  Note roll cannot be derived & we don't need it here

// TODO: DEBUG:	
m_yaw = atan2(m_vforwardf.x, m_vforwardf.z);
m_pitch = atan2(m_vforwardf.y, sqrtf(m_vforwardf.x*m_vforwardf.x + m_vforwardf.z*m_vforwardf.z));	

}

// Releases the camera and puts it back into normal (player controlled) state
void CameraClass::ReleaseCamera(void)
{
	// Check whether we are currently following a path; if so, dispose of the path first
	if (m_camerastate == CameraClass::CameraState::PathCamera && m_camerapath)
	{
		delete m_camerapath;
		m_camerapath = NULL;
	}

	// Reset the camera state
	m_camerastate = CameraClass::CameraState::NormalCamera;
}

// Fixes the camera in a specific position & orientation
void CameraClass::FixCamera(const FXMVECTOR position, const FXMVECTOR orientation)
{
	// Check whether we are currently following a path; if so, dispose of the path first
	if (m_camerastate == CameraClass::CameraState::PathCamera && m_camerapath)
	{
		delete m_camerapath;
		m_camerapath = NULL;
	}

	// Store the fixed camera parameters
	m_fixedposition = position;
	m_fixedorientation = orientation;

	// Change the camera state
	m_camerastate = CameraClass::CameraState::FixedCamera;
}

// Starts the camera travelling along a pre-constructed path
void CameraClass::StartCameraPath(CameraPath *path)
{
	// Check whether we are currently following a path; if so, dispose of the path first
	if (m_camerastate == CameraClass::CameraState::PathCamera && m_camerapath)
	{
		delete m_camerapath;
		m_camerapath = NULL;
	}

	// Store the path that has been provided
	m_camerapath = path;

	// If the path is not valid then release the camera and quit here
	if (!m_camerapath) { ReleaseCamera(); return; }

	// Change the camera mode 
	m_camerastate = CameraClass::CameraState::PathCamera;
		
	// Initialise the path
	m_camerapath->StartPath();

	// Reset the path completion flag, which will be set to "true" once the path completes
	m_pathcomplete = false;
}

// Method that is called following completion of a camera path, that will put the camera into the desired post-path state
void CameraClass::HandleEndOfCameraPath(void)
{
	// Parameter check; make sure we actually have a path defined and are following it
	if (m_camerastate != CameraClass::CameraState::PathCamera || m_camerapath == NULL)
	{
		// This is an error.  Release the camera for safety
		ReleaseCamera();
		return;
	}

	// Set the flag which indicates the current path has completed
	m_pathcomplete = true;

	// There are multiple options upon completion of the path
	if (m_camerapath->GetPathCompletionAction() == CameraPath::CameraPathCompletionAction::FixInPositionOnCompletion)
	{
		// If we want to stay in position upon path completion then fix the camera in its current position now.  This will deallocate the current path
		FixCamera(m_position, m_orientation);
		return;
	}
	else if (m_camerapath->GetPathCompletionAction() == CameraPath::CameraPathCompletionAction::StartNewPathOnCompletion)
	{
		// If we want to start a new path on completion then make sure we have a valid one to transition into
		if (m_camerapath->GetPathToBeInitiatedOnCompletion() != NULL && m_camerapath->GetPathToBeInitiatedOnCompletion() != m_camerapath)
		{
			// Start the new path.  This will automatically deallocate the current path
			StartCameraPath(m_camerapath->GetPathToBeInitiatedOnCompletion());
			return;
		}
		else
		{
			// If there is no path to transition into then we must just release the camera.  This will deallocate the path.
			ReleaseCamera();
			return;
		}
	}
	else
	{
		// If we want to release the camera, or in any other case not covered, simply return camera control back to the user.  Will deallocate the path.
		ReleaseCamera();
		return;
	}
}

// Activates the debug camera
void CameraClass::ActivateDebugCamera(void)
{
	// The debug camera will begin at the current camera position and orientation
	m_debugposition = m_position;
	m_debugorientation = m_orientation;

	// Update the camera state
	m_camerastate = CameraClass::CameraState::DebugCamera;
}


// Deactivates the debug camera, releasing the camera back to normal operation
void CameraClass::DeactivateDebugCamera(void)
{
	ReleaseCamera();
}

// Rotate the debug camera about its local up vector
void CameraClass::DebugCameraYaw(float radians)
{
	// Determine the local up vector
	XMVECTOR up = XMVector3Rotate(UP_VECTOR, m_debugorientation);
	
	// Generate a rotation quaternion about this local axis
	XMVECTOR yaw = XMQuaternionRotationAxis(up, radians);

	// Apply the delta quaternion to our current orientation
	AddDeltaDebugCameraOrientation(yaw);
}

// Rotate the debug camera about its local right vector
void CameraClass::DebugCameraPitch(float radians)
{
	// Determine the local right vector
	XMVECTOR right = XMVector3Rotate(RIGHT_VECTOR, m_debugorientation);

	// Generate a rotation quaternion about this local axis
	XMVECTOR pitch = XMQuaternionRotationAxis(right, radians);

	// Apply the delta quaternion to our current orientation
	AddDeltaDebugCameraOrientation(pitch);
}

// Roll the debug camera about its local forward vector
void CameraClass::DebugCameraRoll(float radians)
{
	// Determine the local forward vector
	XMVECTOR forward = XMVector3Rotate(FORWARD_VECTOR, m_debugorientation);

	// Generate a rotation quaternion about this local axis
	XMVECTOR roll = XMQuaternionRotationAxis(forward, radians);

	// Apply the delta quaternion to our current orientation
	AddDeltaDebugCameraOrientation(roll);
}
// Zooms the camera to an overhead view of the specified space object.  Returns true if the path was started sucessfully
bool CameraClass::ZoomToOverheadShipView(iSpaceObject *target)
{
	// Parameter check
	if (!target) return false;

	// Call the overloaded method with a default path time
	return ZoomToOverheadShipView(target, Game::C_DEFAULT_ZOOM_TO_SHIP_SPEED);
}

// Zooms the camera to an overhead view of the specified space object.  Returns true if the path was started sucessfully
bool CameraClass::ZoomToOverheadShipView(iSpaceObject *target, float time)
{
	// Parameter check
	if (!target || time < Game::C_EPSILON) return false;

	// Determine the distance we need to zoom from this target.  First test trivial case if object has no size
	XMVECTOR objectsize = target->GetSize();
	if (!XMVector3GreaterOrEqual(objectsize, Game::C_EPSILON_V))
	{
		return ZoomToOverheadShipView(target, Game::C_DEFAULT_ZOOM_TO_SHIP_OVERHEAD_DISTANCE, time);
	}
	else
	{
		// If the ship has a valid size then we want to determine based on its size.  We will use the formula
		//		d = (s/2) / tan(a/2)
		// where
		//		d = distance to move from the ship
		//		s = size of the object (z dimension will be considered)
		//		a = camera FOV.  (use precalculated tan(FOV/2) exposed by frustum in GetTanOfHalfFOV())
		float distance = ((XMVectorGetZ(objectsize) * 1.25f) / 2.0f) / Game::Engine->GetViewFrustrum()->GetTanOfHalfFOV();
		return ZoomToOverheadShipView(target, distance, time);
	}
}

// Zooms the camera to an overhead view of the specified space object.  Returns true if the path was started sucessfully
bool CameraClass::ZoomToOverheadShipView(iSpaceObject *target, float distance, float time)
{
	// Parameter check
	if (!target || time < Game::C_EPSILON || distance < Game::C_EPSILON) return false;

	// Create a path for this zoom.  Camera will go from current pos (node 1) to a point on the outer radius of the
	// target IF REQUIRED (node 2), and finally to an overhead view of the ship itself (node 3)
	CameraPath *path = new CameraPath();

	// Node 1 = the current position & orientation of the camera
	path->AddNode(m_position, m_orientation, 0.0f);

	// Get the vector from camera > centre of target in world space
	XMVECTOR localcentre = NULL_VECTOR; 
	XMVECTOR targetpos = XMVector3TransformCoord(localcentre, target->GetWorldMatrix());
	XMVECTOR targetvec = XMVectorSubtract(targetpos, m_position);
	XMVECTOR targetvec_norm = XMVector3NormalizeEst(targetvec);

	// Specify constants for the boundary distance, and the portion of total time allocated to getting to that boundary
	float boundarydist = distance * 1.0f;
	float boundary_time_pc = 0.5f;

	// Test whether we are outside the ship boundary radius, and therefore need to create node 2
	float disttotarget = XMVectorGetX(XMVector3LengthEst(targetvec));
	if (disttotarget > boundarydist)
	{
		// Node 2 = a point on the boundary radius surrounding the target ship
		
		// We can get a vector to the point on the boundary by negating the normalised (unit length) target vector
		// and multiplying by the boundary distance (to get vector from target to boundary point).  We then transform
		// by the inverse world matrix of the target to get the coordinates in local target space
		XMVECTOR boundaryvec = XMVectorScale(targetvec_norm, -boundarydist);
		boundaryvec = XMVector3TransformCoord(boundaryvec, target->GetInverseWorldMatrix());

		// Adjust the local boundary vector so that it is at the target distance above the ship
		boundaryvec = XMVectorSetY(boundaryvec, distance);

		// The required change in orientation will be the quaternion between the basis (fwd) vector and the 
		// vector from boundary point to target (the negation of boundaryvec, which is target>boundary)
		XMVECTOR orienttotarget = QuaternionBetweenVectors(BASIS_VECTOR, XMVectorNegate(boundaryvec));

		// Create the target-relative node using these delta vector/quaternion values
		path->AddNode(boundaryvec, orienttotarget, target, (time * boundary_time_pc));
	}
	else
	{
		// If we do NOT need to travel to the boundary point, scale the path time down accordingly
		time -= (time * boundary_time_pc);
	}

	// Node 3 = final position above the target object.  Will be a relative node, tied to the target object
	
	// Simply specify the relative transformation required from target pos/orient
	XMVECTOR overheadpos = XMVectorSetY(localcentre, distance);
	XMVECTOR overheadorient = XMQuaternionRotationRollPitchYaw(PIOVER2, 0.0f, 0.0f);

	// Create the final node above the target object
	path->AddNode(overheadpos, overheadorient, target, time);

	// Set the path properties
	path->SetPathMode(CameraPath::CameraPathMode::Normal);												// Normal linear path
	path->SetPathCompletionAction(CameraPath::CameraPathCompletionAction::FixInPositionOnCompletion);	// Fix in position once we reach the ship

	// We can now start the camera on this path and return true to indicate it has started
	StartCameraPath(path);
	return true;
}








