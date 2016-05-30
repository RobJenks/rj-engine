#include "GameVarsExtern.h"
#include "RJMain.h"
#include "CoreEngine.h"
#include "CameraClass.h"
#include "VolumetricLine.h"
#include "Player.h"
#include "ComplexShip.h"

#include "RJMain.h"		// DBG
#include "SimpleShip.h"	// DBG

#include "UI_ShipBuilder.h"

// Constant values
const float	UI_ShipBuilder::MOUSE_DRAG_DISTANCE_MODIFIER = 0.5f;
const float	UI_ShipBuilder::MOUSE_DRAG_ROTATION_SPEED = TWOPI;
const float UI_ShipBuilder::MOUSE_DRAG_PAN_SPEED = 25.0f;
const float UI_ShipBuilder::ZOOM_SPEED = 1.0f;
const float UI_ShipBuilder::ZOOM_INCREMENT_SCALE = 2.0f;			
const float UI_ShipBuilder::ZOOM_INCREMENT_TIME = 0.25f; 
const AXMVECTOR UI_ShipBuilder::DEFAULT_CAMERA_ROTATION = XMQuaternionRotationAxis(RIGHT_VECTOR, -PIOVER2);
const float UI_ShipBuilder::DEFAULT_ZOOM_LEVEL = 4.0f;
const float UI_ShipBuilder::MIN_ZOOM_LEVEL = 3.0f;
const float	UI_ShipBuilder::CAMERA_REVERT_TIME = 0.5f;
const float UI_ShipBuilder::COMPONENT_FADE_TIME = 0.5f;
const float UI_ShipBuilder::COMPONENT_FADE_OUT_ALPHA = 0.35f;

// Default constructor
UI_ShipBuilder::UI_ShipBuilder(void)
{
	// Most initialisation takes place per-activation in the Activate() method
}

// Initialisation method, called once the UI component has been created
Result UI_ShipBuilder::InitialiseController(Render2DGroup *render, UserInterface *ui)
{
	// Retrieve references to the key UI controls
	if (m_render)
	{
		// m_xxx = m_render->Components.<ComponentType>.GetItem("xxx");
	}

	// Return success
	return ErrorCodes::NoError;
}

// Method that is called when the UI controller becomes active
void UI_ShipBuilder::Activate(void)
{
	// Pause the game while the model builder is active
	Game::Application.Pause();

	// We use a fixed camera in the editor
	//Game::Engine->GetCamera()->FixCamera(NULL_VECTOR, ID_QUATERNION);

	// We do not start with a target ship
	m_ship = NULL;

	// Initialise per-run values 
	m_mode = EditorMode::ShipSectionMode;
	m_centre = NULL_VECTOR;
	m_camerastate = SBCameraState::Normal;
	m_camera_release = 0U;
	m_camera_rotate = DEFAULT_CAMERA_ROTATION;
	m_zoomlevel = MIN_ZOOM_LEVEL;
	m_zoom_increment_amount = 0.0f;
	m_zoom_increment_end = 0.0f;
	m_reverting_camera = false;
	m_reverttimeremaining = 0.0f;
	m_reverting_from = ID_QUATERNION;
	m_revert_centre_from = NULL_VECTOR;
	m_revert_zoom_from = UI_ShipBuilder::DEFAULT_ZOOM_LEVEL;
	m_rmb_down_start_centre = m_centre;
	m_deck = 0;

	// Set default starting editor mode
	SetEditorMode(UI_ShipBuilder::EditorMode::ShipSectionMode);
}

// Method to perform per-frame logic and perform rendering of the UI controller (excluding 2D render objects, which will be handled by the 2D render manager)
void UI_ShipBuilder::Render(void)
{
	// Perform any updates of the camera required since the previous frame
	PerformCameraUpdate();

	// Render the editor grid, depending on editor mode
	RenderEditorGrid();

}


// Method that is called when the UI controller is deactivated
void UI_ShipBuilder::Deactivate(void)
{
	// Remove any reference to the target ship
	m_ship = NULL;

	// Unpause the game once the model builder is deactivated
	Game::Application.Unpause();
}


// Sets the ship under construction and focuses the view on it
void UI_ShipBuilder::SetShip(ComplexShip *ship)
{
	// Parameter check
	if (!ship || !ship->GetSpaceEnvironment()) { InitialiseForShip(NULL); return; }

	// Initialise the UI and focus on the target ship
	InitialiseForShip(ship);
}


// Initialise the UI and focus on the target ship
void UI_ShipBuilder::InitialiseForShip(ComplexShip *ship)
{
	// Store the ship being constructed, and do a final parameter check (although should always be valid and non-null)
	m_ship = ship;
	if (!m_ship || !ship->GetSpaceEnvironment()) { m_ship = NULL; return; }

	// Initialise a camera transition to overhead view of the ship
	Game::Engine->GetCamera()->ZoomToOverheadShipView(m_ship, GetDefaultZoomLevel(), Game::C_DEFAULT_ZOOM_TO_SHIP_SPEED);
	LockCamera(Game::C_DEFAULT_ZOOM_TO_SHIP_SPEED);

	// Set the zoom level based on this ship
	SetZoom(GetDefaultZoomLevel());

	// Initialise the UI to ship tile mode
	SetEditorMode(UI_ShipBuilder::EditorMode::ShipSectionMode);
}


// Sets the currently-active editor mode
void UI_ShipBuilder::SetEditorMode(EditorMode mode)
{
	// If the mode is not changing then we don't need to do anything 
	//if (mode == m_mode) return;

	// Raise the deactivation event for the current editor mode
	EditorModeDeactivated(m_mode, mode);

	// Now raise the activation event for the new editor mode
	EditorModeActivated(mode, m_mode);
}


// Event triggered when an editor mode is deactivated.  "mode" is the mode being deactivated
void UI_ShipBuilder::EditorModeDeactivated(EditorMode mode, EditorMode next_mode)
{

}

// Event triggered when an editor mode is activated.  "mode" is the mode being activated
void UI_ShipBuilder::EditorModeActivated(EditorMode mode, EditorMode previous_mode)
{
	switch (mode)
	{
		case EditorMode::ShipSectionMode:		ActivateSectionMode(previous_mode);			break;
		case EditorMode::TileMode:				ActivateTileMode(previous_mode);			break;
		case EditorMode::ObjectMode:			ActivateObjectMode(previous_mode);			break;
		default:
			return;
	}
}

// Activate the specified editor mode
void UI_ShipBuilder::ActivateSectionMode(EditorMode previous_mode)
{
	// Remove any fade effect from the ship or its contents when building entire ship sections
	if (m_ship)
	{ 
		m_ship->FadeToAlpha(UI_ShipBuilder::COMPONENT_FADE_TIME, 1.0f, true);
		m_ship->FadeAllTiles(UI_ShipBuilder::COMPONENT_FADE_TIME, 1.0f, true);
		m_ship->ForceRenderingOfInterior(false);
	}
}

// Activate the specified editor mode
void UI_ShipBuilder::ActivateTileMode(EditorMode previous_mode)
{
	// Fade out the ship exterior, leaving all tiles at full alpha
	if (m_ship)
	{
		m_ship->FadeToAlpha(UI_ShipBuilder::COMPONENT_FADE_TIME, UI_ShipBuilder::COMPONENT_FADE_OUT_ALPHA, true);
		m_ship->FadeAllTiles(UI_ShipBuilder::COMPONENT_FADE_TIME, 1.0f, true);
		m_ship->ForceRenderingOfInterior(true);
	}
}

// Activate the specified editor mode
void UI_ShipBuilder::ActivateObjectMode(EditorMode previous_mode)
{
	// Fade out the ship exterior and tile exteriors
	if (m_ship)
	{
		m_ship->FadeToAlpha(UI_ShipBuilder::COMPONENT_FADE_TIME, UI_ShipBuilder::COMPONENT_FADE_OUT_ALPHA, true);
		m_ship->FadeAllTiles(UI_ShipBuilder::COMPONENT_FADE_TIME, UI_ShipBuilder::COMPONENT_FADE_OUT_ALPHA, true);
		m_ship->ForceRenderingOfInterior(true);
	}
}

// Locks the camera for the specified period of time, after which it will be released again to the user
void UI_ShipBuilder::LockCamera(unsigned int time_ms)
{
	// Parameter check for safety; can only lock the camera between 1ms and 10 minutes
	if (time_ms <= 0 || time_ms > (1000 * 60 * 60 * 10)) return;

	// Set the camera state to prevent any user input
	m_camerastate = SBCameraState::Locked;

	// Record the time at which the camera can be released again; if we are already locking it for
	// a longer period of time then retain that longer period
	m_camera_release = max(Game::PersistentClockMs + time_ms, m_camera_release);
}

// Locks the camera for the specified period of time, after which it will be released again to the user
void UI_ShipBuilder::LockCamera(float time_secs)
{
	LockCamera((unsigned int)std::ceilf(time_secs * 1000.0f));
}


// Releases the camera back to normal user control again
void UI_ShipBuilder::ReleaseCamera(void)
{
	// Restore the camera state
	m_camerastate = SBCameraState::Normal;

	// Reset the counter for good measure (although not required)
	m_camera_release = 0U;
}

// Zoom speed per second is based upon the size of the target ship
float UI_ShipBuilder::GetZoomSpeedPerSecond(void) const		
{ 
	return (m_ship ? (m_ship->GetCollisionSphereRadius() * ZOOM_SPEED) : 1.0f); 
}

// Returns the size of each zoom increment to be applied every ZOOM_INCREMENT_TIME secs
float UI_ShipBuilder::GetZoomIncrement(void) const
{
	return (m_ship ? (m_ship->GetCollisionSphereRadius() * ZOOM_INCREMENT_SCALE) : 1.0f);
}

// Default zoom level is based upon the size of the target ship
float UI_ShipBuilder::GetDefaultZoomLevel(void) const			
{ 
	return (m_ship ? (m_ship->GetCollisionSphereRadius() * DEFAULT_ZOOM_LEVEL) : MIN_ZOOM_LEVEL); 
}

// Minimum zoom level is based upon the size of the target ship
float UI_ShipBuilder::GetMinZoomLevel(void) const
{
	return (m_ship ? (m_ship->GetCollisionSphereRadius() + 1.0f) : MIN_ZOOM_LEVEL);
}

// Set the zoom level of the camera directly
void UI_ShipBuilder::SetZoom(float zoom)
{
	// Validate and then update the zoom level
	m_zoomlevel = max(GetMinZoomLevel(), zoom);
}

// Zooms the view by the specified amount
void UI_ShipBuilder::PerformZoom(float zoom)
{
	// Validate and then update the zoom level
	SetZoom(m_zoomlevel + zoom);
}

// Starts a new zoom increment that will be applied over the next ZOOM_INCREMENT_TIME secs
void UI_ShipBuilder::PerformZoomIncrement(float increment)
{
	if (ZoomIncrementInProgress())
		m_zoom_increment_amount = (increment * 0.75f + m_zoom_increment_amount * 0.25f);
	else
		m_zoom_increment_amount = increment;
	
	m_zoom_increment_amount = min(m_zoom_increment_amount, GetZoomIncrement() * 5.0f);
	m_zoom_increment_end = Game::PersistentClockTime + UI_ShipBuilder::ZOOM_INCREMENT_TIME;
}

// Revert the camera back to base position/orientation/zoom
void UI_ShipBuilder::RevertCamera(void)
{
	m_reverting_camera = true;
	m_reverting_from = m_camera_rotate;
	m_revert_centre_from = m_centre;
	m_revert_zoom_from = m_zoomlevel;
	m_reverttimeremaining = UI_ShipBuilder::CAMERA_REVERT_TIME;
}

// Perform any updates of the camera required since the previous frame
void UI_ShipBuilder::PerformCameraUpdate(void)
{
	// If the camera is currently locked, test whether it can now be released
	if (m_camerastate == SBCameraState::Locked && Game::PersistentClockMs >= m_camera_release) ReleaseCamera();
	
	// If the camera is currently reverting back to base, calculate the delta for this frame here
	if (m_reverting_camera)
	{
		// Perform linear interpolation between current and target yaw/pitch/zoom, then apply the changes immediately
		float revert_pc = 1.0f - (m_reverttimeremaining / UI_ShipBuilder::CAMERA_REVERT_TIME);
		revert_pc = clamp(revert_pc, 0.0f, 1.0f);

		// Get the quaternion between our current orientation and the default orientation, and interpolate based on revert %
		m_camera_rotate = XMQuaternionSlerp(m_reverting_from, UI_ShipBuilder::DEFAULT_CAMERA_ROTATION, revert_pc);

		// Perform linear interpolation from the current centre point back to the origin
		m_centre = XMVectorLerp(m_revert_centre_from, NULL_VECTOR, revert_pc);

		// We should also revert back to the default zoom level, if we aren't there already, based on the same revert timer
		SetZoom(m_revert_zoom_from + ((GetDefaultZoomLevel() - m_revert_zoom_from) * revert_pc));

		// Decrement the revert timer and deactivate it if we have reached the target
		m_reverttimeremaining -= Game::PersistentTimeFactor;
		if (m_reverttimeremaining <= 0.0f)
		{
			m_reverting_camera = false;
			m_reverttimeremaining = 0.0f;
			m_camera_rotate = DEFAULT_CAMERA_ROTATION;
			m_centre = NULL_VECTOR;
			SetZoom(GetDefaultZoomLevel());
		}
	}

	// If the camera is not locked, update it based on the current camera parameters
	if (m_camerastate != SBCameraState::Locked && Game::Engine->GetCamera()->GetCameraState() != CameraClass::CameraState::DebugCamera)
	{
		// Process any pending zoom actions
		if (ZoomIncrementInProgress())
		{
			PerformZoom(m_zoom_increment_amount * (Game::PersistentTimeFactor / UI_ShipBuilder::ZOOM_INCREMENT_TIME));
		}

		// The camera position will be a vector (0,0,zoomlevel) transformed by the yaw/pitch about (0,0,0), translated by the centre point
		XMVECTOR ship_centre = XMVectorAdd(m_ship->GetPosition(), m_centre);
		XMVECTOR base_pos = XMVectorSetZ(NULL_VECTOR, m_zoomlevel);
		XMVECTOR cam_pos = XMVectorAdd(XMVector3Rotate(base_pos, m_camera_rotate), ship_centre);
		
		// Set the camera orientation to face the centre point
		XMVECTOR cam_orient = QuaternionBetweenVectors(FORWARD_VECTOR, XMVector3NormalizeEst(XMVectorSubtract(ship_centre, cam_pos)));
		
		// Set the camera position and orientation
		Game::Engine->GetCamera()->FixCamera(cam_pos, cam_orient);
	}
}

// Method to process user input into the active UI controller
void UI_ShipBuilder::ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse)
{
	// Process keyboard and mouse input
	ProcessKeyboardInput(keyboard);
	ProcessMouseInput(mouse, keyboard);

}

// Process all keyboard input to this UI controller
void UI_ShipBuilder::ProcessKeyboardInput(GameInputDevice *keyboard)
{
	BOOL *keys = keyboard->GetKeys();

	// Revert the camera to base position/orientation/zoom if the home key is pressed
	if (keys[DIK_HOME])
	{
		RevertCamera();
		keyboard->LockKey(DIK_HOME);
	}

	// Zoom controls
	if (keys[DIK_PGUP])			ZoomIn();
	else if (keys[DIK_PGDN])	ZoomOut();
	
	// Controls to change editor mode
	if (keys[DIK_1])			{ SetEditorMode(UI_ShipBuilder::EditorMode::ShipSectionMode);	keyboard->LockKey(DIK_1); }
	else if (keys[DIK_2])		{ SetEditorMode(UI_ShipBuilder::EditorMode::TileMode);			keyboard->LockKey(DIK_2); }
	else if (keys[DIK_3])		{ SetEditorMode(UI_ShipBuilder::EditorMode::ObjectMode);		keyboard->LockKey(DIK_3); }

	// Consume all keys within this UI so they are not passed down to the main application
	keyboard->ConsumeAllKeys();
}


// Process all keyboard input to this UI controller
void UI_ShipBuilder::ProcessMouseInput(GameInputDevice *mouse, GameInputDevice *keyboard)
{
	// If the RMB is being held down, and we weren't clicking on a component, then 
	// the user is performing a camera action
	if (m_rmb_down && m_rmb_down_component == NULL)
	{
		bool shift = keyboard->ShiftDown(); 
		bool ctrl = keyboard->CtrlDown();

		// Take different action based on modifier keys
		if (!shift && !ctrl)
		{
			// If the user is not holding shift or control down, they are rotating the camera view
			RotateCameraBasedOnUserInput(mouse->GetRMBStartPosition(), mouse->GetCursor());
		}
		else if (shift && !ctrl)
		{
			// If the user is holding shift they are panning the view
			PanCameraBasedOnUserInput(mouse->GetRMBStartPosition(), mouse->GetCursor());
		}
	}

	// If the mouse wheel is being used then zoom the camera
	long z = mouse->GetZDelta();
	if (z > 0) ZoomInIncrement();
	else if (z < 0) ZoomOutIncrement();
}

// Event raised when the RMB is first depressed
void UI_ShipBuilder::ProcessRightMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component)
{
	// Store the current camera centre point when the RMB is first depressed, for use when panning the camera
	if (m_rmb_down_component == NULL) m_rmb_down_start_centre = m_centre;

}

// Method to handle the mouse move event
void UI_ShipBuilder::ProcessMouseMoveEvent(INTVECTOR2 location)
{

}

// Determines the correct positioning for the camera based on current mouse events
void UI_ShipBuilder::RotateCameraBasedOnUserInput(const INTVECTOR2 & startlocation, const INTVECTOR2 & currentlocation)
{
	// We do not want to move the camera based on user input if it us currently being moved by the system
	if (m_camerastate == UI_ShipBuilder::SBCameraState::Locked || m_reverting_camera) return;

	// The distance moved in the X axis will correspond to yaw, and distance in the Y axis to pitch
	float yaw = (currentlocation.x - startlocation.x) / ((float)Game::ScreenWidth * UI_ShipBuilder::MOUSE_DRAG_DISTANCE_MODIFIER);
	float pitch = (currentlocation.y - startlocation.y) / ((float)Game::ScreenHeight * UI_ShipBuilder::MOUSE_DRAG_DISTANCE_MODIFIER);

	// Constrain the yaw and pitch to be within (-1.0 to +1.0), then scale to (time-scaled) rotation speed
	yaw = clamp(yaw, -1.0f, 1.0f) * UI_ShipBuilder::MOUSE_DRAG_ROTATION_SPEED * Game::PersistentTimeFactor;
	pitch = clamp(pitch, -1.0f, 1.0f) * UI_ShipBuilder::MOUSE_DRAG_ROTATION_SPEED * Game::PersistentTimeFactor;

	// Update the camera orientation based on these delta values
	m_camerastate = SBCameraState::Rotating;
	m_camera_rotate = XMQuaternionNormalizeEst(XMQuaternionMultiply(XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f), m_camera_rotate));
}


// Pans the camera based on user mouse input
void UI_ShipBuilder::PanCameraBasedOnUserInput(const INTVECTOR2 & startlocation, const INTVECTOR2 & currentlocation)
{
	// We do not want to move the camera based on user input if it us currently being moved by the system
	if (m_camerastate == UI_ShipBuilder::SBCameraState::Locked || m_reverting_camera) return;

	// Get local distance moved in X and Y dimensions
	float x_mv = (currentlocation.x - startlocation.x) / ((float)Game::ScreenWidth * UI_ShipBuilder::MOUSE_DRAG_DISTANCE_MODIFIER);
	float y_mv = (currentlocation.y - startlocation.y) / ((float)Game::ScreenHeight * UI_ShipBuilder::MOUSE_DRAG_DISTANCE_MODIFIER);

	// Constrain the values to be within (-1.0 to +1.0), then scale using pan speed.  Invert direction where required
	x_mv = -1.0f * clamp(x_mv, -1.0f, 1.0f) * UI_ShipBuilder::MOUSE_DRAG_PAN_SPEED;
	y_mv = +1.0f * clamp(y_mv, -1.0f, 1.0f) * UI_ShipBuilder::MOUSE_DRAG_PAN_SPEED;

	// Transform the pan vector into rotated world space 
	XMVECTOR pan = XMVector3Rotate(XMVectorSet(x_mv, y_mv, 0.0f, 0.0f), GetCameraOrientation());

	// Add this delta to the original centre point, when the current panning action started
	m_centre = XMVectorAdd(m_rmb_down_start_centre, pan);
}

// Internal methods to get the current position/orientation of the game camera
XMVECTOR UI_ShipBuilder::GetCameraPosition(void) const			{ return Game::Engine->GetCamera()->GetFixedCameraPosition(); }
XMVECTOR UI_ShipBuilder::GetCameraOrientation(void) const		{ return Game::Engine->GetCamera()->GetFixedCameraOrientation(); }


// Render the editor grid, depending on editor mode
void UI_ShipBuilder::RenderEditorGrid(void)
{
	static const int EXTEND_GRID = 3;
	const INTVECTOR3 &elsize = m_ship->GetElementSize();
	VolumetricLine vol_line = VolumetricLine(NULL_VECTOR, NULL_VECTOR, XMFLOAT4(0.75f, 0.75f, 0.75f, 0.35f), 0.5f, NULL);
	
	// Determine the local/world start and end positions
	XMVECTOR local_start_pos = Game::ElementLocationToPhysicalPosition(INTVECTOR3(-EXTEND_GRID, -EXTEND_GRID, m_ship->GetDeckIndex(m_deck)));
	XMVECTOR start_pos = XMVector3TransformCoord(local_start_pos, m_ship->GetZeroPointWorldMatrix());
	XMVECTOR end_pos = XMVectorAdd(start_pos, XMVector3TransformCoord(XMVectorSetZ(NULL_VECTOR, 
		Game::ElementLocationToPhysicalPosition(elsize.z + EXTEND_GRID + EXTEND_GRID)), m_ship->GetOrientationMatrix()));

	// Also determine the world space adjustment required to transition between elements
	XMVECTOR incr = XMVector3TransformCoord(Game::ElementLocationToPhysicalPosition(INTVECTOR3(1, 0, 0)), m_ship->GetOrientationMatrix());

	// Generate 'vertical' lines at each x coordinate first
	XMVECTOR add_vec = NULL_VECTOR;
	for (int x = -EXTEND_GRID; x < (elsize.x + EXTEND_GRID); ++x)
	{
		vol_line.P1 = XMVectorAdd(start_pos, add_vec);
		vol_line.P2 = XMVectorAdd(end_pos, add_vec);
		Game::Engine->RenderVolumetricLine(vol_line);
		OutputDebugString(concat("Adding V line: ")(Vector3ToString(vol_line.P1))(" to ")(Vector3ToString(vol_line.P2))("\n").str().c_str());
		add_vec = XMVectorAdd(add_vec, incr);
	}
	
	// Recalculate some fields for the other dimension
	end_pos = XMVectorAdd(start_pos, XMVector3TransformCoord(XMVectorSetX(NULL_VECTOR,
		Game::ElementLocationToPhysicalPosition(elsize.x + EXTEND_GRID + EXTEND_GRID)), m_ship->GetOrientationMatrix()));
	incr = XMVector3TransformCoord(Game::ElementLocationToPhysicalPosition(INTVECTOR3(0, 1, 0)), m_ship->GetOrientationMatrix());
	add_vec = NULL_VECTOR;

	// Now generate 'horizontal' lines at each y coordinate
	for (int y = -EXTEND_GRID; y < (elsize.y + EXTEND_GRID); ++y)
	{
		vol_line.P1 = XMVectorAdd(start_pos, add_vec);
		vol_line.P2 = XMVectorAdd(end_pos, add_vec);
		Game::Engine->RenderVolumetricLine(vol_line);
		OutputDebugString(concat("Adding H line: ")(Vector3ToString(vol_line.P1))(" to ")(Vector3ToString(vol_line.P2))("\n").str().c_str());
		add_vec = XMVectorAdd(add_vec, incr);
	}
}




