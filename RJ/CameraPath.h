#pragma once

#ifndef __CameraPathH__
#define __CameraPathH__

#include <vector>
#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "Utility.h"
#include "ObjectReference.h"


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class CameraPath : public ALIGN16<CameraPath>
{
public:
	// Enumeration of possible path modes
	enum CameraPathMode				{ Normal = 0, Repeating, RepeatingInReverse };

	// Enumeration of possible actions to be taken following completion of the path
	enum CameraPathCompletionAction	{ ReleaseOnCompletion = 0, FixInPositionOnCompletion, StartNewPathOnCompletion };

	// Struct holding information on a node in the camera path
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct CameraPathNode : public ALIGN16<CameraPathNode>
	{
		AXMVECTOR						Position;				// Position of the node
		AXMVECTOR						Orientation;			// Orientation of the camera at this node
		float							Time;					// Time at this node (secs), used for interpolation
		ObjectReference<iSpaceObject>	Object;					// Object that this node is relative to, if applicable
		bool							IsObjectRelative;		// Flag indicating whether we should have an object to track

		CameraPathNode(const FXMVECTOR pos, const FXMVECTOR orient, float time)
			: Position(pos), Orientation(orient), Time(time), Object(), IsObjectRelative(false) { }

		CameraPathNode(const FXMVECTOR relativepos, const FXMVECTOR relativeorient, iSpaceObject *obj, float time)
			: Position(relativepos), Orientation(relativeorient), Time(time), Object(obj), IsObjectRelative((obj != NULL)) { }
	};

	// Starts the path, setting all values to their starting values.  "reverse" flag determines whether we start at beginning or end
	CMPINLINE void					StartPath(void)									{ StartPath(false); }
	void							StartPath(bool reverse);

	// Advances the path along its route.  Returns a flag indicating whether the path has now completed
	bool							Advance(void); 
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
	CMPINLINE const XMVECTOR	 	 GetCurrentCameraPosition(void) const			{ return m_camerapos; }
	CMPINLINE const XMVECTOR		 GetCurrentCameraOrientation(void) const		{ return m_cameraorient; }

	// Flag indicating whether the path is running in reverse
	CMPINLINE bool					IsRunningInReverse(void)						{ return m_reversepath; }

	// Retrieve the current path time 
	CMPINLINE float					GetPathTime(void)								{ return m_time; }

	// Flag indicating whether the camera path respects game pause or not.  Default is false
	CMPINLINE bool					IsPausable(void) const							{ return m_pausable; }
	CMPINLINE void					SetIsPausable(bool pausable)					{ m_pausable = pausable; }

	// Methods to query, add or update nodes
	void							AddNode(const FXMVECTOR position, const FXMVECTOR orientation, float time);
	void							AddNode(const FXMVECTOR relativeposition, const FXMVECTOR relativeorientation, iSpaceObject *object, float time);
	void							RemoveNode(int index);
	void							ClearNodes(void);
	CMPINLINE int					GetNodeCount(void)									{ return m_nodecount; }
	CMPINLINE std::vector<CameraPathNode>::const_iterator	GetNodeIteratorStart(void)	{ return m_nodes.begin(); }
	CMPINLINE std::vector<CameraPathNode>::const_iterator	GetNodeIteratorEnd(void)	{ return m_nodes.end(); }

	// Static method to generate a "tracking path" that will simply maintain the camera position & orientation
	// relative to the target object
	static CameraPath *				CreateTrackingPath(iSpaceObject *target, const FXMVECTOR relative_pos, const FXMVECTOR relative_orient);

	// Default constructor
	CameraPath(void);

	// Default destructor
	~CameraPath(void);

private:

	CameraPathMode					m_pathmode;				// The mode that this path will run in

	std::vector<CameraPathNode>		m_nodes;				// Vector of nodes in the camera path
	int								m_nodecount;			// Count of the number of nodes in the path

	int								m_index;				// Index of the node we are currently travelling towards
	bool							m_reversepath;			// Flag indicating whether we are travelling forwards (false, default) or in reverse (true)

	float							m_time;					// Current time value along the path

	bool							m_pausable;				// Flag indicating whether the camera path respects game pause or not.  Default is false

	AXMVECTOR						m_camerapos;			// Current position of the camera
	AXMVECTOR						m_cameraorient;			// Current orientation of the camera

	CameraPathCompletionAction		m_completionaction;		// Action to be taken upon completion of the path
	CameraPath *					m_completionpath;		// Path to be followed on completion of this one, if the completion action is set appropriately
};




#endif