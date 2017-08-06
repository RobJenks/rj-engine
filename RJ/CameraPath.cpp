#include <vector>
#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "Utility.h"
#include "FastMath.h"
#include "iSpaceObject.h"

#include "CameraPath.h"

CameraPath::CameraPath(void)
{
	// Set default values
	m_pathmode = CameraPath::CameraPathMode::Normal;
	m_time = 0.0f;
	m_index = 0;
	m_nodecount = 0;
	m_reversepath = false;
	m_pausable = false;
	m_camerapos = NULL_VECTOR;
	m_cameraorient = ID_QUATERNION;
	m_completionaction = CameraPath::CameraPathCompletionAction::ReleaseOnCompletion;
	m_completionpath = NULL;
}

// Starts the path, setting all values to their starting values.  "reverse" flag determines whether we start at beginning or end
void CameraPath::StartPath(bool reverse)
{
	// Default the node-derived parameters to defaults, in case we are missing nodes
	m_time = 0.0f;
	m_camerapos = NULL_VECTOR;
	m_cameraorient = ID_QUATERNION;

	// Start at either the start or end of the path depending on the path direction
	if (!reverse)
	{
		m_index = 0;
		m_reversepath = false;
	}
	else
	{
		m_index = m_nodecount - 1;
		m_reversepath = true;
	}

	// Assuming we have at least one node, take the starting values from that node
	if (m_nodecount > 0)
	{
		m_time = m_nodes[m_index].Time;
		m_camerapos = m_nodes[m_index].Position;
		m_cameraorient = m_nodes[m_index].Orientation;
	}

	// Advance the path by a miniscule amount to enable calculation of derived spatial fields in Advance()
	Advance(Game::C_EPSILON);
}

// Advances the path along its route.  Returns a flag indicating whether the path has now completed
bool CameraPath::Advance(void)
{
	// Advance the path using a timefactor based on the current path parameters
	return Advance((m_pausable ? Game::TimeFactor : Game::PersistentTimeFactor));
}

// Advances the path along its route.  Returns a flag indicating whether the path has now completed.  Allows
// time factor to be explicitly provided, for example to progress the path faster/slower than normal
bool CameraPath::Advance(float timefactor)
{
	// Test whether the path is still valid
	if (m_nodecount < 2) return true;				// There is no point in a path with less than two nodes

	// Test whether the path is completed (in either forward or reverse direction)
	if ( (m_index >= m_nodecount && !m_reversepath) || (m_index < 0 && m_reversepath) )
	{
		// Take different action depending on the path mode
		if (m_pathmode == CameraPath::CameraPathMode::Repeating)
		{
			// Restart the path from the beginning
			StartPath(false);
		}
		else if (m_pathmode == CameraPath::CameraPathMode::RepeatingInReverse)
		{
			// Reverse the path 
			if (!m_reversepath)		StartPath(true);
			else					StartPath(false);
		}
		else
		{
			// If this is not a repeating path then report that it has completed
			return true;
		}
	}

	// Now advance path time by the specified delta
	m_time += (m_reversepath ? -timefactor : timefactor);

	// Test whether we have now advanced beyond the current target node
	if ( (!m_reversepath && m_time > m_nodes[m_index].Time) || (m_reversepath && m_time < m_nodes[m_index].Time) )
	{
		// Increment the current index.  If this pushes us off the end of the path, return now and wait for next evaluation
		// Will then be handled by the completion logic above in the next frame
		m_index += (m_reversepath ? -1 : +1);
		if ( m_index < 0 || m_index >= m_nodecount ) return false;
	}

	// Validity check; at this point we should always have one node "behind" us in the path.  If not then end the path
	if ( (!m_reversepath && m_index <= 0) || (m_reversepath && m_index >= (m_nodecount-1)) ) return false;

	// Target position & orientation will always be those held in node[index].  Transform from object to world space if required
	XMVECTOR targetpos, targetorient; 
	if (m_nodes[m_index].IsObjectRelative)
	{
		// Get the target object.  If it no longer exists (e.g. was destroyed) then terminate the path immediately
		iSpaceObject *obj = m_nodes[m_index].Object();
		if (obj == NULL) return true;

		targetpos = XMVector3TransformCoord(m_nodes[m_index].Position, obj->GetWorldMatrix());
		targetorient = XMQuaternionMultiply(m_nodes[m_index].Orientation, obj->GetOrientation());
	}
	else
	{
		targetpos = m_nodes[m_index].Position;
		targetorient = m_nodes[m_index].Orientation;
	}

	// Determine which node we are interpolating from, towards node[index]
	int previndex; float pc;
	if (!m_reversepath)
	{
		previndex = m_index - 1;			// We are interpolating between the previous node and the current one
		pc = (m_time - m_nodes[m_index-1].Time) / (m_nodes[m_index].Time - m_nodes[m_index-1].Time);
	}
	else
	{
		previndex = m_index + 1;			// We are interpolating between the 'next' node and the current one when running in reverse
		pc = 1.0f - ((m_time - m_nodes[m_index].Time) / (m_nodes[m_index+1].Time - m_nodes[m_index].Time));	// % is inverted, so (i+1 > i)
	}

	// Now get the source position & orientation, again transforming from local space if required
	XMVECTOR sourcepos, sourceorient; 
	if (m_nodes[previndex].IsObjectRelative)
	{
		// Get the target object.  If it no longer exists (e.g. was destroyed) then terminate the path immediately
		iSpaceObject *obj = m_nodes[previndex].Object();
		if (obj == NULL) return true;

		sourcepos = XMVector3TransformCoord(m_nodes[previndex].Position, obj->GetWorldMatrix());
		sourceorient = XMQuaternionMultiply(m_nodes[previndex].Orientation, obj->GetOrientation());
	}
	else
	{
		sourcepos = m_nodes[previndex].Position;
		sourceorient = m_nodes[previndex].Orientation;
	}

	// We can now interpolate between the two relevant path nodes to get the current camera state
	m_camerapos = XMVectorLerp(sourcepos, targetpos, pc);
	m_cameraorient = XMQuaternionSlerp(sourceorient, targetorient, pc);

	// Return false to indicate that the path is still running
	return false;
}

// Adds a node to the end of the path.  Overloaded method that creates an absolute position/orient node
void CameraPath::AddNode(const FXMVECTOR position, const FXMVECTOR orientation, float time)
{
	// Call the overloaded function with no relative-position object
	AddNode(position, orientation, NULL, time);
}

// Adds a node to the end of the path.  Adds a node that is positioned/oriented relative to the specified object
void CameraPath::AddNode(const FXMVECTOR relativeposition, const FXMVECTOR relativeorientation, iSpaceObject *object, float time)
{
	// Make sure the time values are progressing sequentially; disallow a node that would go back in time
	if (m_nodecount > 0)
		if (time <= m_nodes[m_nodecount-1].Time)
			return;

	// Otherwise push a new node onto the path and update the path properties
	m_nodes.push_back(CameraPath::CameraPathNode(relativeposition, relativeorientation, object, time));
	m_nodecount = (int)m_nodes.size();
}

// Removes a node at the specified index
void CameraPath::RemoveNode(int index)
{
	// Make sure the index is valid
	if (index < 0 || index >= m_nodecount) return;

	// Remove from the vector at this index and update the path properties
	RemoveFromVectorAtIndex<CameraPath::CameraPathNode>(m_nodes, index);
	m_nodecount = (int)m_nodes.size();
}

// Clears all nodes from the path
void CameraPath::ClearNodes(void)
{
	m_nodes.clear();
	m_nodecount = 0;
}

// Static method to generate a "tracking path" that will simply maintain the camera position & orientation
// relative to the target object
CameraPath* CameraPath::CreateTrackingPath(iSpaceObject *target, const FXMVECTOR relative_pos, const FXMVECTOR relative_orient)
{
	// Parameter check; we need a target object
	if (!target) return NULL;

	// Create a path which will continually track the target object, without changing its relative pos/orient
	CameraPath *path = new CameraPath();
	path->AddNode(relative_pos, relative_orient, target, 0.0f);
	path->AddNode(XMVectorAdd(relative_pos, Game::C_EPSILON_V), relative_orient, target, 100000.0f);
	path->SetPathMode(CameraPath::CameraPathMode::RepeatingInReverse);
	path->SetIsPausable(false);
	path->SetPathCompletionAction(CameraPath::CameraPathCompletionAction::ReleaseOnCompletion);		// Will not happen until directly terminated, however
	return path;
}

// Default destructor
CameraPath::~CameraPath(void)
{
}






