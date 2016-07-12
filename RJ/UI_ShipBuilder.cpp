#include "GameVarsExtern.h"
#include "RJMain.h"
#include "CoreEngine.h"
#include "CameraClass.h"
#include "VolumetricLine.h"
#include "Player.h"
#include "ComplexShip.h"
#include "OverlayRenderer.h"
#include "ComplexShipTile.h"
#include "CSCorridorTile.h"

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
const XMFLOAT3 UI_ShipBuilder::TILE_PLACEMENT_COLOUR_VALID = XMFLOAT3(0.5f, 1.0f, 0.5f);
const XMFLOAT3 UI_ShipBuilder::TILE_PLACEMENT_COLOUR_INVALID = XMFLOAT3(1.0, 0.0f, 0.0f);
const XMFLOAT3 UI_ShipBuilder::TILE_PLACEMENT_COLOUR_PLACEMENTERROR = XMFLOAT3(1.0f, 0.4f, 0.4f);

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
	m_level = 0;
	m_revert_dir_light_index = 0;
	m_revert_dir_light_is_overriding = false;
	m_revert_dir_light = LightData();
	m_mouse_is_over_element = false;
	m_mouse_over_element = NULL_INTVECTOR3;
	m_tile_being_placed = NULL;
	m_placing_generic_corridors = false;

	// Initialise any editor-specific render data
	InitialiseRenderData();


	// Set default starting editor mode
	SetEditorMode(UI_ShipBuilder::EditorMode::ShipSectionMode);
}


// Initialise any editor-specific render states and data
void UI_ShipBuilder::InitialiseRenderData(void)
{
	// We want to add a	new directional light that will shine 'out' of the camera
	LightData dirlight = LightData(LightingManagerObject::LightType::Directional, XMFLOAT3(1.0f, 1.0f, 0.82f), 0.3f, 0.05f, 0.1f, FORWARD_VECTOR_F);

	// Check whether we can create a new directional light one, or whether we are at the limit and so 
	// have to create a new one (which will be reverted on exit)
	if (Game::Engine->LightingManager.CanAddNewDirectionalLight())
	{
		// We cam simply add a new light
		Game::Engine->LightingManager.AddDirectionalLight(dirlight);
		m_revert_dir_light_index = Game::Engine->LightingManager.GetDirectionalLightSourceCount() - 1;
		m_revert_dir_light_is_overriding = false;
	}
	else
	{
		// We need to override an existing light; pick the last, rather arbitrarily.  TODO: Potential risk if directional lights
		// are removed part-way through use of the editor
		m_revert_dir_light_index = Game::Engine->LightingManager.GetDirectionalLightSourceCount() - 1;
		m_revert_dir_light = Game::Engine->LightingManager.GetDirectionalLightDataEntry(m_revert_dir_light_index); 
		m_revert_dir_light_is_overriding = true; 
		Game::Engine->LightingManager.UpdateDirectionalLight(m_revert_dir_light_index, dirlight);	
	}

	// Initialise the volumetric line used for rendering the editor grid
	Texture *tex = new Texture(BuildStrFilename(D::IMAGE_DATA_S, "Rendering\\ui_editor_line_1.dds"));
	m_gridline = VolumetricLine(NULL_VECTOR, NULL_VECTOR, XMFLOAT4(1.0f, 1.0f, 1.0f, 0.75f), 0.5f,
		(tex->GetTexture() != NULL ? tex : NULL));

}

// Revert any editor-specific render states and data
void UI_ShipBuilder::RevertRenderData(void)
{
	// We either need to remove the editor directional light, or revert back to the original light, depending on 
	// whether we added a new light or overrode an existing one on initialisation
	if (m_revert_dir_light_is_overriding)
	{
		Game::Engine->LightingManager.UpdateDirectionalLight(m_revert_dir_light_index, m_revert_dir_light);
	}
	else
	{
		Game::Engine->LightingManager.RemoveDirectionalLight(m_revert_dir_light_index);
	}

	// Deallocate the editor gridline objects
	if (m_gridline.RenderTexture)
	{
		SafeDelete(m_gridline.RenderTexture);
	}
}


// Method to perform per-frame logic and perform rendering of the UI controller (excluding 2D render objects, which will be handled by the 2D render manager)
void UI_ShipBuilder::Render(void)
{
	// Perform any updates of the camera required since the previous frame
	PerformCameraUpdate();

	// Perform any rendering updates required for the editor
	PerformRenderUpdate();

	// Render the current selection and any objects part-way through being placed
	RenderCurrentActions();

	// Render the editor grid, depending on editor mode
	RenderEditorGrid();

}


// Updates any editor-specific render data for the current frame
void UI_ShipBuilder::PerformRenderUpdate(void)
{
	// Update the editor directional light to ensure it is always pointing 'out' of the camera
	LightData dirlight = Game::Engine->LightingManager.GetDirectionalLightDataEntry(m_revert_dir_light_index);
	dirlight.Direction = Game::Engine->GetCamera()->GetViewForwardBasisVectorF();
	Game::Engine->LightingManager.UpdateDirectionalLight(m_revert_dir_light_index, dirlight);
}

// Render the current selection and any objects part-way through being placed
void UI_ShipBuilder::RenderCurrentActions()
{
	// Actions to be performed when the user is in tile mode and has the mouse over a valid ship tile
	if (m_mouse_is_over_element && m_mode == UI_ShipBuilder::EditorMode::TileMode)
	{
		// Render a highlighting effect on the element currently being highlighted, if applicable
		//Game::Engine->GetOverlayRenderer()->RenderElementOverlay(m_ship, m_mouse_over_element, XMFLOAT3(128.0f, 255.0f, 255.0f), 255.0f);
		Game::Engine->GetOverlayRenderer()->RenderCuboidAtRelativeElementLocation(m_ship, m_mouse_over_element, ONE_INTVECTOR3, 
			XMFLOAT3(0.785f, 1.0f, 1.0f), 0.3f);

		// If the user is trying to place a ship tile we may also need to render it here
		RenderTilePlacement();
	}
}

// Moves the 'temporary' tile being placed to a new location in the environment, recalculating data as required
void UI_ShipBuilder::MovePlacementTile(const INTVECTOR3 & location)
{
	// Parameter check
	if (!m_tile_being_placed) return;

	// Parameter check
	const INTVECTOR3 & envsize = m_ship->GetElementSize();
	INTVECTOR3 tile_far_el = (location + (m_tile_being_placed->GetElementSize()) - ONE_INTVECTOR3);
	if (location < NULL_INTVECTOR3 || !(location < envsize) || tile_far_el < NULL_INTVECTOR3 || !(tile_far_el < envsize)) return;

	// Set the tile parent to the environment being constructed (though not the reverse, so the environment never
	// knows that it owns this tile)
	m_tile_being_placed->OverrideParentEnvironmentReference(m_ship);

	// Set the tile location.  This will recalculate the tile world matrix etc. as well
	m_tile_being_placed->SetElementLocation(location);
}

// If a tile is currently being placed, renders the tile and performs any other associated rendering
void UI_ShipBuilder::RenderTilePlacement(void)
{
	// If we are not placing a tile then there is nothing to do here
	if (!m_tile_being_placed) return;

	// We can only place a tile if the mouse is currently over an environment element
	if (!m_mouse_is_over_element) return;

	// Test whether we are attempting to place the tile in a valid location.  
	m_tile_placement_issues.clear();
	bool is_valid = TestTilePlacement(m_tile_being_placed, m_mouse_over_element, m_tile_placement_issues);

	// Now perform rendering based on the result
	if (is_valid)
	{
		// If the proposed location is valid then render the tile itself.  We need to set the tile location 
		// accordingly since it is not actually part of the parent ship
		MovePlacementTile(m_mouse_over_element);
		Game::Engine->RenderComplexShipTile(m_tile_being_placed, m_ship);

		// Render the selection highlight to reflect the fact this is a valid placement
		Game::Engine->GetOverlayRenderer()->RenderElementBox(m_ship, m_mouse_over_element, m_tile_being_placed->GetElementSize(),
			UI_ShipBuilder::TILE_PLACEMENT_COLOUR_VALID, 0.75f, 0.4f);
	}
	else
	{
		// Render the selection highlight to reflect the fact this is not a valid placement
		Game::Engine->GetOverlayRenderer()->RenderElementBox(m_ship, m_mouse_over_element, m_tile_being_placed->GetElementSize(),
			UI_ShipBuilder::TILE_PLACEMENT_COLOUR_INVALID, 0.75f, 0.4f);

		// Render each placement error in turn, to show why the tile cannot be placed here
		std::vector<TilePlacementIssue>::const_iterator it_end = m_tile_placement_issues.end();
		for (std::vector<TilePlacementIssue>::const_iterator it = m_tile_placement_issues.begin(); it != it_end; ++it)
		{
			// For now, we don't take any different action based on the issue type
			Game::Engine->GetOverlayRenderer()->RenderCuboidAtRelativeElementLocation(m_ship, (*it).Element, ONE_INTVECTOR3,
				UI_ShipBuilder::TILE_PLACEMENT_COLOUR_PLACEMENTERROR, 0.75f);
		}
	}
	
}

// Tests whether the proposed tile placement is valid.  Returns a flag indicating validity.  Also outputs
// a list of errors to the supplied output vector, if any exist
bool UI_ShipBuilder::TestTilePlacement(	ComplexShipTile *tile, const INTVECTOR3 & location,
										std::vector<TilePlacementIssue> & outPlacementIssues)
{
	// Parameter check
	if (!tile) return false;

	// Start by assuming the placement is valid and then test all conditions that could invalidate it
	bool is_valid = true;

	// Derive some required data
	INTVECTOR3 el;
	const INTVECTOR3 & envsize = m_ship->GetElementSize();
	INTVECTOR3 tile_far_el = (location + (tile->GetElementSize()) - ONE_INTVECTOR3);
	
	// Tile is placed with its (0,0,0) element at the mouse position.  Iterate over the full space
	// of elements to be covered by this tile and validate them
	ComplexShipElement *element;
	for (int x = location.x; x <= tile_far_el.x; ++x)
	{
		for (int y = location.y; y <= tile_far_el.y; ++y)
		{
			for (int z = location.z; z <= tile_far_el.z; ++z)
			{
				// Test whether the element lies within the ship bounds
				el = INTVECTOR3(x, y, z);
				if (el < NULL_INTVECTOR3 || !(el < envsize))
				{
					outPlacementIssues.push_back(TilePlacementIssue(TilePlacementIssueType::OutOfEnvironmentBounds, el));
					is_valid = false;
				}

				// Test whether the element is already occupied (if it exists; 'el' may be out of bounds)
				element = m_ship->GetElement(el);
				if (element && element->GetTile() != NULL)
				{
					outPlacementIssues.push_back(TilePlacementIssue(TilePlacementIssueType::ElementAlreadyOccupied, el));
					is_valid = false;
				}
			}
		}
	}

	// Return the final result 
	return is_valid;
}



// Method that is called when the UI controller is deactivated
void UI_ShipBuilder::Deactivate(void)
{
	// Remove any reference to the target ship
	m_ship = NULL;

	// Revert any editor-specific render data
	RevertRenderData();

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

	// Update the editor mode
	m_mode = mode;

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

	// Adjust which deck of the ship is being modified
	if (keys[DIK_O])			{ MoveUpLevel();												keyboard->LockKey(DIK_O); }
	else if (keys[DIK_L])		{ MoveDownLevel();												keyboard->LockKey(DIK_L); }

	// TODO: DEBUG
	if (keys[DIK_T])			{ 
		m_tile_being_placed = ComplexShipTile::Create("lifesupport_huge_01"); m_tile_being_placed->CompileAndValidateTile();	keyboard->LockKey(DIK_T);
	}

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

	// Determine the ship element (if applicable) currently being selected by the mouse
	m_mouse_is_over_element = m_ship->DetermineElementIntersectedByRay(Game::Mouse.GetWorldSpaceMouseBasicRay(), m_level, m_mouse_over_element);
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
	
	// Determine the local/world start and end positions
	XMVECTOR local_start_pos = Game::ElementLocationToPhysicalPosition(INTVECTOR3(-EXTEND_GRID, -EXTEND_GRID, m_ship->GetDeckIndex(m_level)));
	XMVECTOR start_pos = XMVector3TransformCoord(local_start_pos, m_ship->GetZeroPointWorldMatrix());
	XMVECTOR end_pos = XMVectorAdd(start_pos, XMVector3TransformCoord(XMVectorSetZ(NULL_VECTOR, 
		Game::ElementLocationToPhysicalPosition(elsize.y + EXTEND_GRID + EXTEND_GRID)), m_ship->GetOrientationMatrix()));

	// Also determine the world space adjustment required to transition between elements
	XMVECTOR incr = XMVector3TransformCoord(Game::ElementLocationToPhysicalPosition(INTVECTOR3(1, 0, 0)), m_ship->GetOrientationMatrix());

	// Generate 'vertical' lines at each x coordinate first
	XMVECTOR add_vec = NULL_VECTOR;
	for (int x = -EXTEND_GRID; x <= (elsize.x + EXTEND_GRID); ++x)
	{
		m_gridline.P1 = XMVectorAdd(start_pos, add_vec);
		m_gridline.P2 = XMVectorAdd(end_pos, add_vec);
		Game::Engine->RenderVolumetricLine(m_gridline);
		add_vec = XMVectorAdd(add_vec, incr);
	}
	
	// Recalculate some fields for the other dimension
	end_pos = XMVectorAdd(start_pos, XMVector3TransformCoord(XMVectorSetX(NULL_VECTOR,
		Game::ElementLocationToPhysicalPosition(elsize.x + EXTEND_GRID + EXTEND_GRID)), m_ship->GetOrientationMatrix()));
	incr = XMVector3TransformCoord(Game::ElementLocationToPhysicalPosition(INTVECTOR3(0, 1, 0)), m_ship->GetOrientationMatrix());
	add_vec = NULL_VECTOR;

	// Now generate 'horizontal' lines at each y coordinate
	for (int y = -EXTEND_GRID; y <= (elsize.y + EXTEND_GRID); ++y)
	{
		m_gridline.P1 = XMVectorAdd(start_pos, add_vec);
		m_gridline.P2 = XMVectorAdd(end_pos, add_vec);
		Game::Engine->RenderVolumetricLine(m_gridline);
		add_vec = XMVectorAdd(add_vec, incr);
	}
}

// Adjust the level of the environment currently being modified
void UI_ShipBuilder::MoveToLevel(int level)
{
	// Validate the parameter; constrain to be within the valid range
	const INTVECTOR3 elsize = m_ship->GetElementSize();
	level = clamp(level, 0, elsize.z - 1);

	// Take no action if we are already on the desired level
	if (level == m_level) return;

	// (Adjust visual cues, begin animation to move editor/ship up or down appropriately */

	// Store the new level being edited
	m_level = level;
}


