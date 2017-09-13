#pragma once

#ifndef __CameraClassH__
#define __CameraClassH__

#include "DX11_Core.h" //#include "FullDX11.h

#include "CompilerSettings.h"
#include "Player.h"
#include "ErrorCodes.h"
class iSpaceObject;
class CameraPath;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class CameraClass : public ALIGN16<CameraClass>
{
public:

	// Enumeration of possible camera states
	enum CameraState					{ NormalCamera = 0, FixedCamera, PathCamera, DebugCamera };

	// Enumeration of possible zoom-to-overhead-view completion actions
	enum ZoomToOverheadCompletionAction	{ ReleaseCameraAfterOverheadZoom = 0, FixInSpaceAfterOverheadZoom, FixOverShipAfterOverheadZoom };

	// Initialisation function for this camera component
	Result								Initialise(void);

	// Calculates a view matrix, dependent on parameters and the current player state
	void								CalculateViewMatrix(void);
	
	// Calculates the camera view matrix from supplied positonal data
	void CalculateViewMatrixFromPositionData
		(const FXMVECTOR position, const FXMVECTOR orientation, const CXMMATRIX offsetmatrix);

	// Returns the calculated view matrix
	CMPINLINE XMMATRIX *				GetViewMatrix(void) { return &m_view; }
	CMPINLINE void						GetViewMatrix(XMMATRIX &view) { view = m_view; }

	// Returns the inverse view matrix
	CMPINLINE XMMATRIX *				GetInverseViewMatrix(void) { return &m_invview; }
	CMPINLINE void						GetInverseViewMatrix(XMMATRIX &invview) const { invview = m_invview; }

	// Decomposes the view matrix into each component, which can be used by other methods for rendering
	void								DecomposeViewMatrix(void);
	CMPINLINE XMVECTOR					GetDecomposedViewRotation(void)		{ return m_vrot; }
	CMPINLINE XMVECTOR					GetDecomposedViewTranslation(void)	{ return m_vtrans; }

	// Returns the basis vectors that make up the current view matrix
	const CMPINLINE XMVECTOR			GetViewUpBasisVector(void) const		{ return m_vup; }
	const CMPINLINE XMVECTOR			GetViewRightBasisVector(void) const		{ return m_vright; }
	const CMPINLINE XMVECTOR			GetViewForwardBasisVector(void) const	{ return m_vforward; }
	const CMPINLINE XMFLOAT3			GetViewUpBasisVectorF(void) const		{ return m_vupf; }
	const CMPINLINE XMFLOAT3			GetViewRightBasisVectorF(void) const	{ return m_vrightf; }
	const CMPINLINE XMFLOAT3			GetViewForwardBasisVectorF(void) const	{ return m_vforwardf; }

	// Returns the camera heading
	const CMPINLINE XMVECTOR			GetCameraHeading(void) const			{ return GetViewForwardBasisVector(); }
	const CMPINLINE XMFLOAT3			GetCameraHeadingF(void) const			{ return GetViewForwardBasisVectorF(); }

	// Returns the yaw and pitch values derived from orientation of basis vectors against the axes
	CMPINLINE float						GetCameraYaw(void) const		{ return m_yaw; }
	CMPINLINE float						GetCameraPitch(void) const		{ return m_pitch; }

	// Return the basic position/orientation components specified the last time the view matrix was calculated
	CMPINLINE XMVECTOR					GetPosition(void) const			{ return m_position; }
	CMPINLINE XMFLOAT3					GetPositionF(void) const		{ return m_positionf; }
	CMPINLINE XMVECTOR					GetOrientation(void) const		{ return m_orientation; }

	// Methods to change (and retrieve) the current camera state
	CMPINLINE CameraClass::CameraState	GetCameraState(void) const		{ return m_camerastate; }
	void								ReleaseCamera(void);
	void								FixCamera(const FXMVECTOR position, const FXMVECTOR orientation);
	void								StartCameraPath(CameraPath *path);

	// Flag indicating whether the current path has finished executing
	CMPINLINE bool						PathHasCompleted(void)			{ return m_pathcomplete; }

	// Method that is called following completion of a camera path, that will put the camera into the desired post-path state
	void								HandleEndOfCameraPath(void);

	// Methods to activate and deactivate the debug camera
	void								ActivateDebugCamera(void);
	void								DeactivateDebugCamera(void);

	// Return or set the current fixed camera position and orientation
	CMPINLINE XMVECTOR					GetFixedCameraPosition(void) const							{ return m_fixedposition; }
	CMPINLINE XMVECTOR					GetFixedCameraOrientation(void) const						{ return m_fixedorientation; }
	CMPINLINE void						SetFixedCameraPosition(const FXMVECTOR position)			{ m_fixedposition = position; }
	CMPINLINE void						SetFixedCameraOrientation(const FXMVECTOR orient)			{ m_fixedorientation = orient; }

	// Return or set the current debug camera position and orientation
	CMPINLINE XMVECTOR					GetDebugCameraPosition(void) const							{ return m_debugposition; }
	CMPINLINE XMVECTOR					GetDebugCameraOrientation(void) const						{ return m_debugorientation; }
	CMPINLINE void						SetDebugCameraPosition(const FXMVECTOR position)			{ m_debugposition = position; }
	CMPINLINE void						SetDebugCameraOrientation(const FXMVECTOR orient)			{ m_debugorientation = orient; }
	void								DebugCameraYaw(float radians);
	void								DebugCameraPitch(float radians);
	void								DebugCameraRoll(float radians);
	CMPINLINE void						AddDeltaDebugCameraPosition(const FXMVECTOR delta)			{ m_debugposition += delta; }
	CMPINLINE void						AddDeltaDebugCameraOrientation(const FXMVECTOR rot)
	{ 
		m_debugorientation = XMQuaternionNormalizeEst(XMQuaternionMultiply(m_debugorientation, rot));
	}


	// Zooms the camera to an overhead view of the specified space object.  Returns true if the path was started sucessfully
	bool								ZoomToOverheadShipView(iSpaceObject *target);
	bool								ZoomToOverheadShipView(iSpaceObject *target, ZoomToOverheadCompletionAction on_complete);
	bool								ZoomToOverheadShipView(iSpaceObject *target, ZoomToOverheadCompletionAction on_complete, float time);
	bool								ZoomToOverheadShipView(iSpaceObject *target, ZoomToOverheadCompletionAction on_complete, float distance, float time);

	// Sets the currently-active ship; if the ship has changed since the last cycle, this will reset any ship-dependent tracking data
	void								UpdateChaseCamera(const Ship *target);

	CameraClass(void);
	~CameraClass(void);


private:

	CameraState							m_camerastate;					// Current state of the camera; determines how view matrices are derived

	AXMMATRIX							m_view;							// The view matrix
	AXMMATRIX							m_invview;						// The inverse view matrix

	AXMVECTOR							m_vrot;							// Decomposed from m_view: rotation component
	AXMVECTOR							m_vtrans;						// Decomposed from m_view: translation component

	AXMVECTOR							m_vforward, m_vup, m_vright;	// Decomposed from m_view: forward/up/right basis vectors
	XMFLOAT3							m_vforwardf, m_vupf, m_vrightf;	// Local float copies of the camera basis vectors

	AXMVECTOR							m_position;						// Current position of the camera
	AXMVECTOR							m_orientation;					// Current orientation of the camera
	AXMMATRIX							m_offsetmatrix;					// Current offset matrix for the camera

	AXMVECTOR							m_fixedposition;				// Camera position when in fixed camera mode
	AXMVECTOR							m_fixedorientation;				// Camera orientation when in fixed camera mode

	AXMVECTOR							m_debugposition;				// Camera position when in debug camera mode
	AXMVECTOR							m_debugorientation;				// Camera orientation when in debug camera mode

	CameraPath *						m_camerapath;					// Current path that the camera is following
	bool								m_pathcomplete;					// Flag indicating when the current path has completed

	float								m_yaw, m_pitch;					// Current camera yaw & pitch

	AXMMATRIX							m_rot, m_trans, m_inter;		// Interim calculation matrices

	XMFLOAT3							m_positionf;					// Key data replicated in other structures for runtime efficiency

};


#endif