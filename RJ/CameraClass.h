#pragma once

#ifndef __CameraClassH__
#define __CameraClassH__

#include "DX11_Core.h" //#include "FullDX11.h

#include "CompilerSettings.h"
#include "Player.h"
#include "Ships.h"
#include "ErrorCodes.h"
class CameraPath;

class CameraClass
{
public:

	// Enumeration of possible camera states
	enum CameraState					{ NormalCamera = 0, FixedCamera, PathCamera, DebugCamera };
	
	// Initialisation function for this camera component
	Result								Initialise(void);

	// Calculates a view matrix, dependent on parameters and the current player state
	void								CalculateViewMatrix(void);
	
	// Calculates the camera view matrix from supplied positonal data
	void CalculateViewMatrixFromPositionData
		(const D3DXVECTOR3 *position, const D3DXQUATERNION *orientation, const D3DXMATRIX *offsetmatrix);

	// Returns the calculated view matrix
	CMPINLINE D3DXMATRIX				*GetViewMatrix(void) { return &m_view; }
	CMPINLINE void						GetViewMatrix(D3DXMATRIX &view) { view = m_view; }

	// Returns the inverse view matrix
	CMPINLINE D3DXMATRIX				*GetInverseViewMatrix(void) { return &m_invview; }
	CMPINLINE void						GetInverseViewMatrix(D3DXMATRIX &invview) const { invview = m_invview; }

	// Decomposes the view matrix into each component, which can be used by other methods for rendering
	void								DecomposeViewMatrix(void);
	CMPINLINE D3DXQUATERNION*			GetDecomposedViewRotation(void)		{ return &m_vrot; }
	CMPINLINE D3DXVECTOR3*				GetDecomposedViewTranslation(void)	{ return &m_vtrans; }

	// Returns the basis vectors that make up the current view matrix
	const CMPINLINE D3DXVECTOR3*		GetViewUpBasisVector(void) const		{ return &m_vup; }
	const CMPINLINE D3DXVECTOR3*		GetViewRightBasisVector(void) const		{ return &m_vright; }
	const CMPINLINE D3DXVECTOR3*		GetViewForwardBasisVector(void) const	{ return &m_vforward; }

	// Returns the yaw and pitch values derived from orientation of basis vectors against the axes
	CMPINLINE float						GetCameraYaw(void) const		{ return m_yaw; }
	CMPINLINE float						GetCameraPitch(void) const		{ return m_pitch; }

	// Return the basic position/orientation components specified the last time the view matrix was calculated
	CMPINLINE D3DXVECTOR3				GetPosition(void) const			{ return m_position; }
	CMPINLINE D3DXQUATERNION			GetOrientation(void) const		{ return m_orientation; }

	// Methods to change (and retrieve) the current camera state
	CMPINLINE CameraClass::CameraState	GetCameraState(void) const		{ return m_camerastate; }
	void								ReleaseCamera(void);
	void								FixCamera(const D3DXVECTOR3 & position, const D3DXQUATERNION & orientation);
	void								StartCameraPath(CameraPath *path);

	// Flag indicating whether the current path has finished executing
	CMPINLINE bool						PathHasCompleted(void)			{ return m_pathcomplete; }

	// Method that is called following completion of a camera path, that will put the camera into the desired post-path state
	void								HandleEndOfCameraPath(void);

	// Methods to activate and deactivate the debug camera
	void								ActivateDebugCamera(void);
	void								DeactivateDebugCamera(void);

	// Return or set the current fixed camera position and orientation
	CMPINLINE D3DXVECTOR3				GetFixedCameraPosition(void) const							{ return m_fixedposition; }
	CMPINLINE D3DXQUATERNION			GetFixedCameraOrientation(void) const						{ return m_fixedorientation; }
	CMPINLINE void						SetFixedCameraPosition(const D3DXVECTOR3 & position)		{ m_fixedposition = position; }
	CMPINLINE void						SetFixedCameraOrientation(const D3DXQUATERNION & orient)	{ m_fixedorientation = orient; }

	// Return or set the current debug camera position and orientation
	CMPINLINE D3DXVECTOR3				GetDebugCameraPosition(void) const							{ return m_debugposition; }
	CMPINLINE D3DXQUATERNION			GetDebugCameraOrientation(void) const						{ return m_debugorientation; }
	CMPINLINE void						SetDebugCameraPosition(const D3DXVECTOR3 & position)		{ m_debugposition = position; }
	CMPINLINE void						SetDebugCameraOrientation(const D3DXQUATERNION & orient)	{ m_debugorientation = orient; }
	void								DebugCameraYaw(float radians);
	void								DebugCameraPitch(float radians);
	void								DebugCameraRoll(float radians);
	CMPINLINE void						AddDeltaDebugCameraPosition(const D3DXVECTOR3 & delta)		{ m_debugposition += delta; }
	CMPINLINE void						AddDeltaDebugCameraOrientation(const D3DXQUATERNION & rot)	
	{ 
		m_debugorientation *= rot; 
		D3DXQuaternionNormalize(&m_debugorientation, &m_debugorientation);
	}


	// Zooms the camera to an overhead view of the specified space object.  Returns true if the path was started sucessfully
	bool								ZoomToOverheadShipView(iSpaceObject *target);
	bool								ZoomToOverheadShipView(iSpaceObject *target, float time);
	bool								ZoomToOverheadShipView(iSpaceObject *target, float distance, float time);

	// Accessor/modifier methods for the ship class environment parameter
	CMPINLINE Ships::Class				GetShipClass(void) { return m_shipclass; }
	CMPINLINE void						SetShipClass(Ships::Class sclass) { m_shipclass = sclass; }

	// Sets the currently-active ship; if the ship has changed since the last cycle, this will reset any ship-dependent tracking data
	void								UpdateChaseCamera(const Ship *target);

	CameraClass(void);
	~CameraClass(void);


private:

	CameraState							m_camerastate;					// Current state of the camera; determines how view matrices are derived
	Ships::Class						m_shipclass;

	D3DXMATRIX							m_view;							// The view matrix
	D3DXMATRIX							m_invview;						// The inverse view matrix

	D3DXQUATERNION						m_vrot;							// Decomposed from m_view: rotation component
	D3DXVECTOR3							m_vtrans;						// Decomposed from m_view: translation component

	D3DXVECTOR3							m_vforward, m_vup, m_vright;	// Decomposed from m_view: forward/up/right basis vectors

	D3DXVECTOR3							m_position;						// Current position of the camera
	D3DXQUATERNION						m_orientation;					// Current orientation of the camera
	D3DXMATRIX							m_offsetmatrix;					// Current offset matrix for the camera

	D3DXVECTOR3							m_fixedposition;				// Camera position when in fixed camera mode
	D3DXQUATERNION						m_fixedorientation;				// Camera orientation when in fixed camera mode

	D3DXVECTOR3							m_debugposition;				// Camera position when in debug camera mode
	D3DXQUATERNION						m_debugorientation;				// Camera orientation when in debug camera mode

	CameraPath *						m_camerapath;					// Current path that the camera is following
	bool								m_pathcomplete;					// Flag indicating when the current path has completed

	float								m_yaw, m_pitch;					// Current camera yaw & pitch

	D3DXMATRIX							m_rot, m_trans, m_inter;		// Interim calculation matrices

};


#endif