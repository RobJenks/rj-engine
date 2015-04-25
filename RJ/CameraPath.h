#pragma once

#ifndef __CameraPathH__
#define __CameraPathH__

#include <vector>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "Utility.h"

class CameraPath
{
public:
	// Enumeration of possible path modes
	enum CameraPathMode				{ Normal = 0, Repeating, RepeatingInReverse };

	// Enumeration of possible actions to be taken following completion of the path
	enum CameraPathCompletionAction	{ ReleaseOnCompletion = 0, FixInPositionOnCompletion, StartNewPathOnCompletion };

	// Struct holding information on a node in the camera path
	struct CameraPathNode
	{
		D3DXVECTOR3					Position;				// Position of the node
		D3DXQUATERNION				Orientation;			// Orientation of the camera at this node
		float						Time;					// Time at this node (secs), used for interpolation
		iSpaceObject *				Object;					// Object that this node is relative to, if applicable

		CameraPathNode(const D3DXVECTOR3 & pos, const D3DXQUATERNION & orient, float time)
		{ Position = pos; Orientation = orient; Time = time; Object = NULL; }

		CameraPathNode(const D3DXVECTOR3 & relativepos, const D3DXQUATERNION & relativeorient, iSpaceObject *obj, float time)
		{ Position = relativepos; Orientation = relativeorient; Time = time; Object = obj; }
	};

	// Starts the path, setting all values to their starting values.  "reverse" flag determines whether we start at beginning or end
	CMPINLINE void					StartPath(void)									{ StartPath(false); }
	void							StartPath(bool reverse);

	// Advances the path along its route.  Returns a flag indicating whether the path has now completed
	bool							Advance(float timefactor);

	// Flag determining whether the path will run once, repeatedly, or repeatedly in reverse
	CameraPath::CameraPathMode		GetPathMode(void)								{ return m_pathmode; }
	void							SetPathMode(CameraPath::CameraPathMode mode)	{ m_pathmode = mode; }

	// Flag determining the action that should be taken (if possible) upon completion of the path
	CameraPathCompletionAction		GetPathCompletionAction(void)								{ return m_completionaction; }
	void							SetPathCompletionAction(CameraPathCompletionAction action)	{ m_completionaction = action; }
	CameraPath *					GetPathToBeInitiatedOnCompletion(void)						{ return m_completionpath; }
	void							SetPathToBeInitiatedOnCompletion(CameraPath *path)			{ m_completionpath = path; }

	// Retrieves pointers to the current camera position & orientation
	CMPINLINE const D3DXVECTOR3 * 	 GetCurrentCameraPosition(void) const			{ return &m_camerapos; }
	CMPINLINE const D3DXQUATERNION * GetCurrentCameraOrientation(void) const		{ return &m_cameraorient; }

	// Flag indicating whether the path is running in reverse
	CMPINLINE bool					IsRunningInReverse(void)						{ return m_reversepath; }

	// Retrieve the current path time 
	CMPINLINE float					GetPathTime(void)								{ return m_time; }

	// Methods to query, add or update nodes
	void							AddNode(const D3DXVECTOR3 & position, const D3DXQUATERNION & orientation, float time);
	void							AddNode(const D3DXVECTOR3 & relativeposition, const D3DXQUATERNION & relativeorientation, iSpaceObject *object, float time);
	void							RemoveNode(int index);
	void							ClearNodes(void);
	CMPINLINE int					GetNodeCount(void)									{ return m_nodecount; }
	CMPINLINE vector<CameraPathNode>::const_iterator	GetNodeIteratorStart(void)		{ return m_nodes.begin(); }
	CMPINLINE vector<CameraPathNode>::const_iterator	GetNodeIteratorEnd(void)		{ return m_nodes.end(); }

	// Default constructor
	CameraPath(void);

	// Default destructor
	~CameraPath(void);

private:

	CameraPathMode					m_pathmode;				// The mode that this path will run in

	vector<CameraPathNode>			m_nodes;				// Vector of nodes in the camera path
	int								m_nodecount;			// Count of the number of nodes in the path

	int								m_index;				// Index of the node we are currently travelling towards
	bool							m_reversepath;			// Flag indicating whether we are travelling forwards (false, default) or in reverse (true)

	float							m_time;					// Current time value along the path

	D3DXVECTOR3						m_camerapos;			// Current position of the camera
	D3DXQUATERNION					m_cameraorient;			// Current orientation of the camera

	CameraPathCompletionAction		m_completionaction;		// Action to be taken upon completion of the path
	CameraPath *					m_completionpath;		// Path to be followed on completion of this one, if the completion action is set appropriately
};




#endif