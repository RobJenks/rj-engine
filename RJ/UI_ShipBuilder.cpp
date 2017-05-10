#include "GameVarsExtern.h"
#include "RJMain.h"
#include "CoreEngine.h"
#include "CameraClass.h"
#include "GameDataExtern.h"
#include "VolumetricLine.h"
#include "Player.h"
#include "ComplexShip.h"
#include "iSpaceObjectEnvironment.h"
#include "SpaceSystem.h"
#include "BasicProjectile.h"
#include "BasicProjectileDefinition.h"
#include "OverlayRenderer.h"
#include "ComplexShipTile.h"
#include "CSCorridorTile.h"
#include "TileAdjacency.h"
#include "UserInterface.h"
#include "Render2DGroup.h"
#include "UITextBox.h"
#include "Light.h"
#include "FactionManagerObject.h"
#include "GamePhysicsEngine.h"
#include "SpaceProjectile.h"
#include "ElementIntersection.h"
#include "LightSource.h"
#include "GameUniverse.h"
#include "UIComponentGroup.h"
#include "UIButton.h"
#include "TextBlock.h"
#include "XMLGenerator.h"
#include "DataOutput.h"
#include "SimpleShip.h"	// DBG
#include "GameConsole.h"// DBG

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

// Location of the custom ship directory
const char * UI_ShipBuilder::CUSTOM_SHIP_DIRECTORY = "/Ships/CustomDesigns";

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
	m_mode = EditorMode::GeneralMode;
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
	m_mouse_is_over_element = false;
	m_mouse_over_element = NULL_INTVECTOR3;
	m_tile_being_placed = NULL;
	m_tile_placement_is_valid = false;
	m_placing_generic_corridors = false;
	m_intersect_marker_start = m_intersect_marker_end = NULL;
	m_intersect_test_proj = NULL;
	m_selected_intersection_marker = NULL;
	m_intersect_proj_mass = m_intersect_proj_vel = m_intersect_proj_radius = m_intersect_proj_hardness = 1.0f;

	// Initialise any editor-specific render data
	InitialiseRenderData();


	// Set default starting editor mode
	SetEditorMode(UI_ShipBuilder::EditorMode::GeneralMode);
}


// Initialise any editor-specific render states and data
void UI_ShipBuilder::InitialiseRenderData(void)
{
	// Initialise the volumetric line used for rendering the editor grid
	Texture *tex = new Texture(BuildStrFilename(D::IMAGE_DATA_S, "Rendering\\ui_editor_line_1.dds"));
	m_gridline = VolumetricLine(NULL_VECTOR, NULL_VECTOR, XMFLOAT4(1.0f, 1.0f, 1.0f, 0.75f), 0.5f,
		(tex->GetTexture() != NULL ? tex : NULL));

	// Initialise the volumetric line used for rendering the collider trajectory in structural test mode
	tex = new Texture(BuildStrFilename(D::IMAGE_DATA_S, "Rendering\\ui_intersection_test_trajectory.dds"));
	m_intersect_test_trajectory = VolumetricLine(NULL_VECTOR, NULL_VECTOR, XMFLOAT4(1.0f, 0.243f, 0.11f, 0.75f), 0.5f,
		(tex->GetTexture() != NULL ? tex : NULL));

}

// Revert any editor-specific render states and data
void UI_ShipBuilder::RevertRenderData(void)
{
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

	// Render the editor grid, depending on editor mode
	RenderEditorGrid();

	// Perform any editor-mode-specific rendering
	switch (m_mode)
	{
		case EditorMode::GeneralMode:			/* Nothing for now */
		case EditorMode::ShipSectionMode:		/* Nothing for now */						break;
		case EditorMode::TileMode:				PerformTileModeRendering();					break;
		case EditorMode::ObjectMode:			/* Nothing for now */						break;
		case EditorMode::StructuralTestMode:	PerformStructuralTestModeRendering();		break;
	}
}


// Updates any editor-specific render data for the current frame
void UI_ShipBuilder::PerformRenderUpdate(void)
{
	// Override lighting with a basic camera-oriented lighting override
	Game::Engine->LightingManager.ApplyStandardCameraFacingLightOverride();
}

// Perform editor-mode-specific rendering
void UI_ShipBuilder::PerformTileModeRendering(void)
{
	// Actions to be performed when the user has the mouse over a valid ship tile
	if (m_mouse_is_over_element)
	{
		// Render a highlighting effect on the element currently being highlighted, if applicable
		//Game::Engine->GetOverlayRenderer()->RenderElementOverlay(m_ship, m_mouse_over_element, XMFLOAT3(128.0f, 255.0f, 255.0f), 255.0f);
		Game::Engine->GetOverlayRenderer()->RenderCuboidAtRelativeElementLocation(m_ship, m_mouse_over_element, ONE_INTVECTOR3, 
			XMFLOAT3(0.785f, 1.0f, 1.0f), 0.3f);

		// If the user is trying to place a ship tile we may also need to render it here
		RenderTilePlacement();
	}
}

// Perform editor-mode-specific rendering
void UI_ShipBuilder::PerformStructuralTestModeRendering(void)
{
	// If the two intersection test markers are visible, draw the trajectory between them
	if (m_intersect_marker_start && m_intersect_marker_end)
	{
		m_intersect_test_trajectory.P1 = m_intersect_marker_start->GetPosition();
		m_intersect_test_trajectory.P2 = m_intersect_marker_end->GetPosition();
		Game::Engine->RenderVolumetricLine(m_intersect_test_trajectory);
	}

	// Render any intersection test results
	if (m_ship)
	{
		XMFLOAT4 el_colour = NULL_FLOAT4;
		SimulatedEnvironmentCollision::const_iterator it_end = m_simulated_intersection.end();
		for (SimulatedEnvironmentCollision::const_iterator it = m_simulated_intersection.begin(); it != it_end; ++it)
		{
			// Get a reference to the element
			const SimulatedEnvironmentCollisionEvent & evt = (*it);
			const ComplexShipElement *el = m_ship->GetElement(evt.ElementID);
			if (!el) continue;

			// Determine the overlay effect required based on the event details
			if (evt.EventType == SimulatedEnvironmentCollisionEventType::ElementDestroyed)
			{
				el_colour = XMFLOAT4(0.8f*3.0f, 1.0f*3.0f, 1.0f*3.0f, 0.8f);						// Blueish-white (scaled 3x intensity)
			}
			else if (evt.EventType == SimulatedEnvironmentCollisionEventType::ElementDamaged)
			{
				el_colour = XMFLOAT4(1.0f, clamp(evt.Value, 0.0f, 1.0f) * 0.9f, 0.0f, 0.8f);		// Scaled colour from 255,0,0 (red) to 255,230,0 (dark yellow)
			}

			// Render an overlay on this element
			Game::Engine->GetOverlayRenderer()->RenderElement3DOverlay(m_ship, el->GetLocation(), el_colour);
		}
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

	// Perform rendering based on whether the current tile placement is valid
	if (m_tile_placement_is_valid)
	{
		// Make sure the tile location is correct since the tile is not actually part of the parent ship
		MovePlacementTile(m_mouse_over_element);

		// Render the tile 
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


// Place the selected tile at its current location, assuming the placement is valid
void UI_ShipBuilder::PlaceTile(void)
{
	// We can only place a tile if we have one selected and over the ship
	if (!m_tile_being_placed || !m_mouse_is_over_element) return;

	// Perform another, final test of the tile placement to make sure it is valid before placement
	m_tile_placement_issues.clear();
	bool valid = TestTilePlacement(m_tile_being_placed, m_mouse_over_element, m_tile_placement_issues);
	if (!valid || !m_tile_placement_issues.empty()) return;

	// Tile placement is valid, so add it to the ship
	m_ship->AddTile(&m_tile_being_placed);

	// Clear the tile pointer, since this tile is now a member of the ship
	m_tile_being_placed = NULL;
}


// Method that is called when the UI controller is deactivated
void UI_ShipBuilder::Deactivate(void)
{
	// Revert any editor-specific render data
	RevertRenderData();

	// TODO: For now, return to general mode since this mode does not use any fade/transparency effects
	SetEditorMode(UI_ShipBuilder::EditorMode::GeneralMode);

	// Remove any reference to the target ship
	m_ship = NULL;

	// Release the camera to return to normal user control
	Game::Engine->GetCamera()->ReleaseCamera();

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
	Game::Engine->GetCamera()->ZoomToOverheadShipView(m_ship, CameraClass::ZoomToOverheadCompletionAction::FixOverShipAfterOverheadZoom, 
		GetDefaultZoomLevel(), Game::C_DEFAULT_ZOOM_TO_SHIP_SPEED);
	LockCamera(Game::C_DEFAULT_ZOOM_TO_SHIP_SPEED);

	// Set the zoom level based on this ship
	SetZoom(GetDefaultZoomLevel());

	// Update UI components to reflect the new ship
	UITextBox *nameinput = m_render->Components.TextBoxes.GetItem("txt_shipname");
	if (nameinput) nameinput->SetText(m_ship->GetName());

	// Initialise the UI to general mode
	SetEditorMode(UI_ShipBuilder::EditorMode::GeneralMode);
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
	switch (mode)
	{
		case EditorMode::GeneralMode:			DeactivateGeneralMode(next_mode);			break;
		case EditorMode::ShipSectionMode:		/*DeactivateSectionMode(next_mode);*/		break;
		case EditorMode::TileMode:				/*DeactivateTileMode(next_mode);*/			break;
		case EditorMode::ObjectMode:			/*DeactivateObjectMode(next_mode);*/		break;
		case EditorMode::StructuralTestMode:	DeactivateStructuralTestMode(next_mode);	break;
		default:
			return;
	}
}

// Event triggered when an editor mode is activated.  "mode" is the mode being activated
void UI_ShipBuilder::EditorModeActivated(EditorMode mode, EditorMode previous_mode)
{
	// Perform any generic actions common to all editor modes
	ActivateUIModeComponents(mode);

	// Now perform any mode-specific logic
	switch (mode)
	{
		case EditorMode::GeneralMode:			ActivateGeneralMode(previous_mode);			break;
		case EditorMode::ShipSectionMode:		ActivateSectionMode(previous_mode);			break;
		case EditorMode::TileMode:				ActivateTileMode(previous_mode);			break;
		case EditorMode::ObjectMode:			ActivateObjectMode(previous_mode);			break;
		case EditorMode::StructuralTestMode:	ActivateStructuralTestMode(previous_mode);	break;
		default:
			return;
	}
}

// Activate the specified editor mode
void UI_ShipBuilder::ActivateGeneralMode(EditorMode previous_mode)
{
	// Parameter check
	if (!m_ship || !m_ship->GetSpaceEnvironment()) return;

	// Remove any fade effect from the ship or its contents when in the default 'general' mode
	m_ship->FadeToAlpha(UI_ShipBuilder::COMPONENT_FADE_TIME, 1.0f, true);
	m_ship->FadeAllTiles(UI_ShipBuilder::COMPONENT_FADE_TIME, 1.0f, true);
	m_ship->ForceRenderingOfInterior(false);
}

// Activate the specified editor mode
void UI_ShipBuilder::ActivateSectionMode(EditorMode previous_mode)
{
	// Parameter check
	if (!m_ship || !m_ship->GetSpaceEnvironment()) return;

	// Remove any fade effect from the ship or its contents when building entire ship sections
	m_ship->FadeToAlpha(UI_ShipBuilder::COMPONENT_FADE_TIME, 1.0f, true);
	m_ship->FadeAllTiles(UI_ShipBuilder::COMPONENT_FADE_TIME, 1.0f, true);
	m_ship->ForceRenderingOfInterior(false);
}

// Activate the specified editor mode
void UI_ShipBuilder::ActivateTileMode(EditorMode previous_mode)
{
	// Parameter check
	if (!m_ship || !m_ship->GetSpaceEnvironment()) return;

	// Fade out the ship exterior, leaving all tiles at full alpha
	m_ship->FadeToAlpha(UI_ShipBuilder::COMPONENT_FADE_TIME, UI_ShipBuilder::COMPONENT_FADE_OUT_ALPHA, true);
	m_ship->FadeAllTiles(UI_ShipBuilder::COMPONENT_FADE_TIME, 1.0f, true);
	m_ship->ForceRenderingOfInterior(true);
}

// Activate the specified editor mode
void UI_ShipBuilder::ActivateObjectMode(EditorMode previous_mode)
{
	// Parameter check
	if (!m_ship || !m_ship->GetSpaceEnvironment()) return;

	// Fade out the ship exterior and tile exteriors
	m_ship->FadeToAlpha(UI_ShipBuilder::COMPONENT_FADE_TIME, UI_ShipBuilder::COMPONENT_FADE_OUT_ALPHA, true);
	m_ship->FadeAllTiles(UI_ShipBuilder::COMPONENT_FADE_TIME, UI_ShipBuilder::COMPONENT_FADE_OUT_ALPHA, true);
	m_ship->ForceRenderingOfInterior(true);
}


// Activate the specified editor mode
void UI_ShipBuilder::ActivateStructuralTestMode(EditorMode previous_mode)
{
	// Parameter check
	if (!m_ship || !m_ship->GetSpaceEnvironment()) return;

	// Fade out the ship exterior so we can show damage per element
	m_ship->FadeToAlpha(UI_ShipBuilder::COMPONENT_FADE_TIME, UI_ShipBuilder::COMPONENT_FADE_OUT_ALPHA, true);
	m_ship->FadeAllTiles(UI_ShipBuilder::COMPONENT_FADE_TIME, 1.0f, true);
	m_ship->ForceRenderingOfInterior(true);
	
	// Reset the position of the intersection test markers and the test parameters
	ResetStructuralTestParameters();
}

// Deactivate the specified editor mode
void UI_ShipBuilder::DeactivateGeneralMode(EditorMode previous_mode)
{
	
}

// Deactivate the specified editor mode
void UI_ShipBuilder::DeactivateStructuralTestMode(EditorMode previous_mode)
{
	// Remove any of the structural test components that should not be persistent
	if (m_intersect_marker_start) m_intersect_marker_start->SetIsVisible(false);
	if (m_intersect_marker_end) m_intersect_marker_end->SetIsVisible(false);
	if (m_intersect_test_proj()) m_intersect_test_proj()->SetIsVisible(false);
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
	if (keys[DIK_1])			{ SetEditorMode(UI_ShipBuilder::EditorMode::GeneralMode);			keyboard->LockKey(DIK_1); }
	else if (keys[DIK_2])		{ SetEditorMode(UI_ShipBuilder::EditorMode::ShipSectionMode);		keyboard->LockKey(DIK_2); }
	else if (keys[DIK_3])		{ SetEditorMode(UI_ShipBuilder::EditorMode::TileMode);				keyboard->LockKey(DIK_3); }
	else if (keys[DIK_4])		{ SetEditorMode(UI_ShipBuilder::EditorMode::ObjectMode);			keyboard->LockKey(DIK_4); }
	else if (keys[DIK_9])		{ SetEditorMode(UI_ShipBuilder::EditorMode::StructuralTestMode);	keyboard->LockKey(DIK_9); }

	// Adjust which deck of the ship is being modified
	if (keys[DIK_O])			{ MoveUpLevel();													keyboard->LockKey(DIK_O); }
	else if (keys[DIK_L])		{ MoveDownLevel();													keyboard->LockKey(DIK_L); }

	// The user can quit the ship designer by pressing tab again
	if (keys[DIK_TAB])			{ Shutdown();														keyboard->LockKey(DIK_TAB); }

	// TODO: DEBUG
	if (keys[DIK_T])			{ 
		m_tile_being_placed = ComplexShipTile::Create("corridor_ns"); m_tile_being_placed->CompileAndValidateTile();	keyboard->LockKey(DIK_T);
	}
	else if (keys[DIK_Y]) {
		m_tile_being_placed = ComplexShipTile::Create(keys[DIK_LSHIFT] ? "lifesupport_huge_01" : "lifesupport_basic_01"); m_tile_being_placed->CompileAndValidateTile();	keyboard->LockKey(DIK_Y);
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
	INTVECTOR3 old_element = m_mouse_over_element; bool was_over_element = m_mouse_is_over_element;
	m_mouse_is_over_element = m_ship->DetermineElementIntersectedByRay(Game::Mouse.GetWorldSpaceMouseBasicRay(), m_level, m_mouse_over_element);

	// Raise the exit method for the current element if appropriate
	if ((m_mouse_is_over_element && was_over_element && m_mouse_over_element != old_element) ||		// If we have moved from one element to another, OR
		(!m_mouse_is_over_element && was_over_element))												// If we are no longer selecting an element, and were before
	{
		MouseExitingElement(old_element);
	}

	// Raise the enter method for this element where appropriate
	if ((m_mouse_is_over_element && was_over_element && m_mouse_over_element != old_element) ||		// If we have moved from one element to another, OR
		(m_mouse_is_over_element && !was_over_element))												// If we are selecting an element now, and weren't before
	{
		MouseEnteringElement(m_mouse_over_element, was_over_element, old_element);
	}
}

// Event raised when the mouse moves between elements of the ship
void UI_ShipBuilder::MouseExitingElement(const INTVECTOR3 & old_element)
{
	// If we are holding a tile for placement, revert any changes made when the tile was positioned within this element
	RevertPlacementTileUpdates();
}

// Event raised when the mouse moves between elements of the ship
void UI_ShipBuilder::MouseEnteringElement(const INTVECTOR3 & new_element, bool entered_from_another_element, const INTVECTOR3 & entered_from)
{
	// If we are holding a tile for placement, adjust it to fit with its surrounding environment
	UpdatePlacementTile();
}

// If we are holding a tile for placement, adjusts it to fit with its surrounding environment
void UI_ShipBuilder::UpdatePlacementTile(void)
{
	// Make sure we are holding a tile for placement, and have it held over an element
	if (!m_tile_being_placed || !m_mouse_is_over_element) return;

	// Test whether we are attempting to place the tile in a valid location.  
	m_tile_placement_issues.clear();
	m_tile_placement_is_valid = TestTilePlacement(m_tile_being_placed, m_mouse_over_element, m_tile_placement_issues);

	// If the placement is valid, update it and its surroundings to show the effect of placing it here
	if (m_tile_placement_is_valid)
	{
		// We need to directly set the tile location since it is not actually part of the parent ship
		MovePlacementTile(m_mouse_over_element);

		
		/* *** NOTE: DEACTIVATED FOR NOW.  UNCOMMENT BELOW FOR PARTIALLY-WORKING TILE ADJUSTMENT *** */
		// The tile & neighbours will be adjusted to reflect their potential configuration if the tile is 
		// placed here.  This should have been reverted before we reach this point.  However perform a 
		// check here for safety before proceeding
		/*
		if (!m_placement_tile_changes.empty()) RevertPlacementTileUpdates();

		// Now record the new set of adjacent tiles which will be updated, for reverting later
		// We store the LOCATION of the adjacent element within each tile (rather than the tile 
		// itself) since if the tile is updated it will be replaced by a new instance
		bool connects[4]; std::vector<TileAdjacency> adj_tiles;
		m_ship->GetNeighbouringTiles(m_tile_being_placed, connects, adj_tiles);
		std::vector<INTVECTOR3> & adj_locations = m_placement_tile_changes;
		std::for_each(adj_tiles.begin(), adj_tiles.end(),
			[&adj_locations](const TileAdjacency & entry)
		{
			adj_locations.push_back(entry.AdjLocation);
		});

		// Now adjust the tile and its neighbours based on their local connections, if required
		m_ship->UpdateTileConnectionState(&m_tile_being_placed);
		*/
	}
}

// If we are holding a tile for placement, revert any changes made when the tile was positioned within this element
void UI_ShipBuilder::RevertPlacementTileUpdates(void)
{
	// Process each tile that was potentially impacted during the placement operation
	ComplexShipTile *tile;
	std::vector<INTVECTOR3>::iterator it_end = m_placement_tile_changes.end();
	for (std::vector<INTVECTOR3>::iterator it = m_placement_tile_changes.begin(); it != it_end; ++it)
	{
		// Attempt to get the tile at this location, and perform the update if we find one 
		tile = m_ship->FindTileAtLocation((*it));
		if (tile)
		{
			m_ship->UpdateTileConnectionState(&tile);
		}
	}

	// Clear the vector of changes now that they have all been reverted
	m_placement_tile_changes.clear();
}

// Event raised when the LMB is first depressed
void UI_ShipBuilder::ProcessMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component)
{
	// Take different action depending on the editor mode
	if (m_mode == EditorMode::StructuralTestMode)
	{
		HandleStructuralModeMouseFirstDown();
	}
}

// Event raised when the RMB is first depressed
void UI_ShipBuilder::ProcessRightMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component)
{
	// Store the current camera centre point when the RMB is first depressed, for use when panning the camera
	if (m_rmb_down_component == NULL) m_rmb_down_start_centre = m_centre;

}

// Methods to accept generic mouse click events at the specified location
void UI_ShipBuilder::ProcessMouseClickAtLocation(INTVECTOR2 location)
{
	// If we have a tile selected, attempt to place it now.  No need to check the location since this
	// is updated each frame for tile placement.  The PlaceTile() method will validate location before placing
	if (m_tile_being_placed) PlaceTile();
}

// Method to handle the mouse move event
void UI_ShipBuilder::ProcessMouseMoveEvent(INTVECTOR2 location)
{
	// Take different action depending on the active editor mode
	if (m_mode == EditorMode::StructuralTestMode)
	{
		HandleStructuralModeMouseMove();
	}
}

// Method to handle the mouse up event
void UI_ShipBuilder::ProcessMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component)
{
	// Take different action depending on the active editor mode
	if (m_mode == EditorMode::StructuralTestMode)
	{
		HandleStructuralModeMouseUp();
	}
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

// Reset the position of the intersection test markers and the test parameters
void UI_ShipBuilder::ResetStructuralTestParameters(void)
{
	// Parameter check
	if (!m_ship || !m_ship->GetSpaceEnvironment()) return;

	// Make sure the intersection test markers exist; if not, create them
	SimpleShip **markers[2] = { &m_intersect_marker_start, &m_intersect_marker_end };
	for (int i = 0; i < 2; ++i)
	{
		if (*markers[i] == NULL)
		{
			(*markers[i]) = SimpleShip::Create("null_ship");
			(*markers[i])->SetFaction(Game::FactionManager.GetFactionIDByCode("faction_none"));
			(*markers[i])->SetModel(Model::GetModel("unit_cone_model"));
			(*markers[i])->SetCollisionMode(Game::CollisionMode::NoCollision);
			(*markers[i])->MoveIntoSpaceEnvironment(m_ship->GetSpaceEnvironment());
			(*markers[i])->SetPositionAndOrientation(XMVectorScale(ONE_VECTOR, (i * 1000.0f)), ID_QUATERNION);
		}

		(*markers[i])->SetIsVisible(true);
	}

	// Both markers exist; we now want to position them at either side of the ship as a default position
	XMVECTOR offset = XMVector3TransformCoord(
		XMVectorSetX(NULL_VECTOR, m_ship->CollisionOBB.Data().ExtentF.x * 1.25f),		// Position to the side (x) of ship in local space
		m_ship->GetOrientationMatrix());												// Transformed by the orientation of the ship in world space

	// Set the position of each marker accordingly
	SetIntersectionTestMarkerPosition(&m_intersect_marker_start, XMVectorSubtract(m_ship->GetPosition(), offset));
	SetIntersectionTestMarkerPosition(&m_intersect_marker_end, XMVectorAdd(m_ship->GetPosition(), offset));

	// Perform an initial intersection test with the markers at these default positions
	PerformIntersectionTest();
}

// Sets the position of an intersection test marker in world space
void UI_ShipBuilder::SetIntersectionTestMarkerPosition(SimpleShip **ppMarker, const FXMVECTOR position)
{
	// Parameter check
	if (!ppMarker || !(*ppMarker)) return;

	// Adjust the marker position
	(*ppMarker)->SetPosition(position);
}

// Performs the intesection test in 'structural test' mode
void UI_ShipBuilder::PerformIntersectionTest(void)
{
	// Parameter checks
	if (!m_ship || !m_intersect_marker_start || !m_intersect_marker_end) return;

	// Retrieve test parameters from the UI
	UpdateIntersectionTestParameters();

	OutputDebugString(concat("Starting test with mass = ")(m_intersect_proj_mass)(", velocity = ")(m_intersect_proj_vel)(", radius = ")(m_intersect_proj_radius)(", hardness = ")(m_intersect_proj_hardness)("\n").str().c_str());

	// Determine ray direction
	XMVECTOR norm_proj_vec = XMVector3NormalizeEst(XMVectorSubtract(m_intersect_marker_end->GetPosition(), m_intersect_marker_start->GetPosition()));
	XMVECTOR proj_vec = XMVectorScale(norm_proj_vec, m_intersect_proj_vel);

	// Detemrine the ray/OBBB intersection from projectile start location along proj_vec
	Ray proj_ray = Ray(m_intersect_marker_start->GetPosition(), proj_vec);
	bool intersects = Game::PhysicsEngine.DetermineRayVsOBBIntersection(proj_ray, m_ship->CollisionOBB.Data());
	if (intersects == false) return;

	// Determine the corresponding intersection point.  Back up approximately 80% of the projectile bounding sphere radius
	// to ensure we have an approx surface/surface collision.  TODO: in future, make this exact using proj OBB?
	// If projectile has a velocity of v == 2m/s, it will travel its radius r == 6m  in t_rad = r/v = 3s.  We then want to 
	// decrease the intersection time t by t_rad* ~0.8
	float intersect_time = Game::PhysicsEngine.RayIntersectionResult.tmin - ((m_intersect_proj_radius / m_intersect_proj_vel) * 0.8f);
	if (intersect_time < 0.0f) return;
	XMVECTOR intersection = proj_ray.PositionAtTime(intersect_time);

	// Save the state of the ship before this test - any data that could be impacted by the test
	// The environment collisions are simulated so this is primarily the effects of the PhysicsEngine collision itself
	iActiveObject::ObjectPhysicsState phys_state(m_ship->PhysicsState);

	// Create the projectile used for this test if necessary, and set basic properties
	if (m_intersect_test_proj() == NULL)
	{
		m_intersect_test_proj = SimpleShip::Create("intersection_test_proj_ship");
	}
	m_intersect_test_proj()->SetFaction(Game::FactionManager.GetFactionIDByCode("faction_none"));
	m_intersect_test_proj()->SetCollisionMode(Game::CollisionMode::BroadphaseCollisionOnly);
	m_intersect_test_proj()->MoveIntoSpaceEnvironment(m_ship->GetSpaceEnvironment());
	m_intersect_test_proj()->SetPositionAndOrientation(XMVectorSetY(NULL_VECTOR, -1000.0f), ID_QUATERNION);
	m_intersect_test_proj()->SetSimulationState(iObject::ObjectSimulationState::FullSimulation);
	m_intersect_test_proj()->SetIsVisible(false);

	// Move the projectile into intersection position and set other collision properties
	m_intersect_test_proj()->SetPosition(intersection);
	m_intersect_test_proj()->SetCollisionSphereRadius(m_intersect_proj_radius);
	m_intersect_test_proj()->SetMass(m_intersect_proj_mass);
	m_intersect_test_proj()->SetHardness(m_intersect_proj_hardness);
	m_intersect_test_proj()->SetHealthPercentage(1.0f);
	m_intersect_test_proj()->SetWorldMomentum(proj_vec);

	// Process the collision at this intersection point and store the collision results
	bool collision = Game::PhysicsEngine.CheckSingleCollision(m_ship, m_intersect_test_proj());
	if (collision == false) return;
	GamePhysicsEngine::ImpactData impact = Game::PhysicsEngine.ObjectImpact;

	// Restore the physics state of ship and collider following this collisiion
	m_ship->PhysicsState = phys_state;
	m_intersect_test_proj()->SetWorldMomentum(proj_vec);

	// We now want to determine the effect on the environment; enable simulated collision mode for the ship
	iSpaceObjectEnvironment::EnableEnvironmentCollisionSimulationMode(m_ship);

	// Calculate the path of the collision through this environment.  We do not need to explicitly set 
	// this to be an external collision since the start point is fixed & known, with no potential
	// for issues with high-speed objects, and the start point may in fact be placed within the ship
	EnvironmentCollision env_collision;
	m_ship->CalculateCollisionThroughEnvironment(m_intersect_test_proj(), impact, false, env_collision);

	// Make the environment collision data immediately-executable so that it can be simulated immediately 
	// in this method, without waiting for the actual intersection times to occur
	env_collision.MakeImmediatelyExecutable();

	// Process this collision in the environment and retrieve data from the simulation results vector
	m_ship->ProcessEnvironmentCollision(env_collision);
	StoreIntersectionTestResults(iSpaceObjectEnvironment::EnvironmentCollisionSimulationResults);

	// Disable collision simulation now that we have generated all results
	iSpaceObjectEnvironment::DisableEnvironmentCollisionSimulationMode();

	// Move the projectile away from the ship (if it still exists), and restore the ship state
	if (m_intersect_test_proj())
	{
		m_intersect_test_proj()->SetPosition(XMVectorSetX(NULL_VECTOR, -1000.0f));
		m_intersect_test_proj()->SetWorldMomentum(NULL_VECTOR);
	}
	m_ship->PhysicsState = phys_state;

	// Results will be displayed via the per-frame rendering method

	// TODO: for now, also stream out
	OutputDebugString("*** Environment collision begin ***\n");
	OutputDebugString(m_simulated_intersection.ToString().c_str());
	OutputDebugString("*** Environment collision end ***\n\n");
}


// Store the results of an intersection test
void UI_ShipBuilder::StoreIntersectionTestResults(const SimulatedEnvironmentCollision & results)
{
	// Clear any existing results
	ClearIntersectionTestResults();

	// Store the new results
	m_simulated_intersection = results;
}

// Clear any stored intersection test results
void UI_ShipBuilder::ClearIntersectionTestResults(void)
{
	m_simulated_intersection.Reset();
}


// Handles the LMB first-down event in structural test mode
void UI_ShipBuilder::HandleStructuralModeMouseFirstDown(void)
{
	// See if we are clicking on an object in the world
	if (m_lmb_down_component == NULL)
	{
		// Test whether we have selected either of the intersection markers
		const BasicRay & ray = Game::Mouse.GetWorldSpaceMouseBasicRay();
		if (m_intersect_marker_start && 
			Game::PhysicsEngine.TestRaySphereIntersection(ray, m_intersect_marker_start->GetPosition(), XMVectorReplicate(m_intersect_marker_start->GetCollisionSphereRadiusSq())))
		{
			// We have selected the start-marker
			m_selected_intersection_marker = &m_intersect_marker_start;
		}
		else if (m_intersect_marker_end &&
			Game::PhysicsEngine.TestRaySphereIntersection(ray, m_intersect_marker_end->GetPosition(), XMVectorReplicate(m_intersect_marker_end->GetCollisionSphereRadiusSq())))
		{
			// We have selected the end-marker
			m_selected_intersection_marker = &m_intersect_marker_end;
		}
		else
		{
			m_selected_intersection_marker = NULL;
		}
	}
}

// Handles the mouse move event while in structural testing mode
void UI_ShipBuilder::HandleStructuralModeMouseMove(void)
{
	// If we currently have a marker selected, we want to move it
	if (m_selected_intersection_marker && (*m_selected_intersection_marker))
	{
		// Get the marker and mouse positions in screen space
		XMVECTOR marker_sc = Game::Engine->WorldToScreen((*m_selected_intersection_marker)->GetPosition());
		XMVECTOR mouse_sc = VectorFromIntVector2(Game::Mouse.GetCursor());

		// Replace the marker xy coords with the equivalent mouse coords, then unproject back into 
		// world space and set the marker position
		XMVECTOR new_sc = XMVectorSelect(mouse_sc, marker_sc, VCTRL_0011);
		XMVECTOR new_world = Game::Engine->ScreenToWorld(new_sc);
		(*m_selected_intersection_marker)->SetPosition(new_world);
	}
}


// Handles the mouse up event while in structural testing mode
void UI_ShipBuilder::HandleStructuralModeMouseUp(void)
{
	// If we are currently moving one of the projectile markers, perform an intersection 
	// test with the markers at their current positions
	if (m_selected_intersection_marker && (*m_selected_intersection_marker))
	{
		PerformIntersectionTest();
	}

	// Clear the currently-selected marker
	m_selected_intersection_marker = NULL;
}


// Event is triggered whenever a mouse click event occurs on a managed control, e.g. a button
void UI_ShipBuilder::ProcessControlClickEvent(iUIControl *control)
{
	// Change focus to this control, if it is one that can accept focus
	if (control->CanAcceptFocus()) this->SetControlInFocus(control);

	// Pass to more granular downstream methods to handle each type of control as required
	switch (control->GetType())
	{
	case iUIControl::Type::Button:
		ProcessButtonClickEvent((UIButton*)control);
		break;
	}
}

// Process button click events in the UI
void UI_ShipBuilder::ProcessButtonClickEvent(UIButton *button)
{
	if (button == NULL) return;

	if (button->GetCode() == "btn_save") {
		SaveShip();
	}
	else if (button->GetCode() == "btn_load") {
		LoadShip();
	}
	
}

// Methods to accept other managed control events
void UI_ShipBuilder::ProcessTextboxChangedEvent(iUIControl *control)
{
	if (!control) return;

	// If we are in intersection test mode, and this control holds one of the test parameters, 
	// re-execute the collision test with the new parameters
	if (m_mode == UI_ShipBuilder::EditorMode::StructuralTestMode)
	{
		if (control->GetCode() == "txt_projmass" || control->GetCode() == "txt_projvel" || 
			control->GetCode() == "txt_projradius" || control->GetCode() == "txt_projhardness")
		{
			PerformIntersectionTest();
		}
	}
}

// Updates the parameters used in the intersection test based on user interface input
void UI_ShipBuilder::UpdateIntersectionTestParameters(void)
{
	UITextBox *tb = NULL;
	const std::string param_src[4] = { "txt_projmass", "txt_projvel", "txt_projradius", "txt_projhardness" };
	float *param_fields[4] = { &m_intersect_proj_mass, &m_intersect_proj_vel, &m_intersect_proj_radius, &m_intersect_proj_hardness };
	float param_limits[4][2] = { { 1.0f, 1000000000.0f }, { 1.0f, 1000000000.0f }, { 0.0001f, 1000.0f }, { 0.0001f, 1000.0f } };

	for (int i = 0; i < 4; ++i)
	{
		tb = m_render->Components.TextBoxes.GetItem(param_src[i]);
		if (tb)
		{
			// Parse out the value and ensure it is within a valid range.  NOTE: We have to use the SetTextSilent() method below 
			// since this method is called in response to SetText() -> ProcessTextboxChangedEvent() -> UpdateIntersectionTestParameters(), 
			// and we therefore cannot make another call to SetText within the "UpdateIntersectionTestParameters" method
			float val = 0.0f;
			const std::string & text = tb->GetText();
			if (text == NullString)
			{
				// Default to the minimum acceptable value if no text has been entered
				val = param_limits[i][0];
			}
			else
			{
				float orig_val = std::stof(tb->GetText());
				val = clamp(orig_val, param_limits[i][0], param_limits[i][1]);
				if (!FLOAT_EQ(val, orig_val))
				{
					tb->SetTextSilent(std::to_string(val));
				}
			}
			
			// Store the parameter value
			*(param_fields[i]) = val;
		}
	}
}

// Activates the UI component group for the given mode, deactivating all others
void UI_ShipBuilder::ActivateUIModeComponents(EditorMode mode)
{
	for (int i = 0; i < (int)EditorMode::_MODECOUNT; ++i)
	{
		std::string name = concat("mode_group_")(TranslateEditorModeToString((EditorMode)i)).str().c_str();
		StrLowerC(name);

		UIComponentGroup *group = m_render->Components.ComponentGroups.GetItem(name);
		if (group != NULL)
		{
			group->SetRenderActive((EditorMode)i == mode);
		}
	}
}

// Save the current ship design
void UI_ShipBuilder::SaveShip(void)
{
	Result result = PerformSave();
	if (result == ErrorCodes::NoError && m_ship)
	{
		SetStatusMessage("Ship saved successfully");
	}
	else
	{
		SetStatusMessage(concat("Error saving ship design (")(result)(")").str().c_str());
	}
}

// Attempt to save the current ship and return a status code indicating the result 
Result UI_ShipBuilder::PerformSave(void)
{
	// Parameter checks
	if (m_mode != EditorMode::GeneralMode) return ErrorCodes::ShipBuilderMissingMandatorySaveParameters;
	if (!m_ship) return ErrorCodes::ShipBuilderMissingMandatorySaveParameters;
	UITextBox *nameinput = m_render->Components.TextBoxes.GetItem("txt_shipname");
	if (!nameinput) return ErrorCodes::ShipBuilderCannotSaveShipWithInvalidCode;

	// Get and normalise the ship name
	std::string name = nameinput->GetText();
	std::string code = NormaliseString(name);
	if (code == "") return ErrorCodes::ShipBuilderCannotSaveShipWithInvalidCode;

	// Make sure the custom ship directory exists
	string path = concat(D::DATA)(UI_ShipBuilder::CUSTOM_SHIP_DIRECTORY).str();
	if (CreateDirectory(path.c_str(), NULL) == false && GetLastError() != ERROR_ALREADY_EXISTS)
		return ErrorCodes::ShipBuilderCannotGenerateSaveDirectory;

	// Make a copy of the ship in order to save it.  This allows us to update certain parameters without
	// affecting the underlying ship
	ComplexShip *ship = ComplexShip::Create(m_ship);
	if (!ship) return ErrorCodes::ShipBuilderCouldNotInstantiateSaveObject;

	// This ship must be marked as an archetype since it will represent a 'standard' class object
	ship->SetIsStandardObject(true);

	// Make sure the new ship is not visible, though this should not have any practical effect
	ship->SetSimulationState(iObject::ObjectSimulationState::NoSimulation);
	ship->SetIsVisible(false);

	// Set the code and name of the new ship to the specified value
	ship->SetCode(code);
	ship->SetName(name);
	ship->DetermineInstanceCode();

	// Generate xml data for the ship
	TiXmlElement *xml = IO::Data::NewGameDataXMLNode();
	Result result = IO::Data::SaveComplexShip(xml, ship);
	if (result != ErrorCodes::NoError) return result;

	// Save this xml data to file
	std::string filename = concat(path)("/")(code)(".xml").str();
	result = IO::Data::SaveXMLDocument(xml, filename);
	if (result != ErrorCodes::NoError) return result;

	// Dispose of the new ship now that it has been saved
	ship->Shutdown();

	// Return success
	return ErrorCodes::NoError;
}

// Load the specified ship design
void UI_ShipBuilder::LoadShip(void)
{
	// TODO: Implement
}

// Attempt to load the specified ship and return a status code indicating the result 
Result UI_ShipBuilder::PerformLoad(void)
{
	// TODO: Implement
	return ErrorCodes::NoError;
}

// Update the status message text
void UI_ShipBuilder::SetStatusMessage(const std::string & msg)
{
	TextBlock *tb = m_render->Components.TextBlocks.GetItem("txt_status");
	if (!tb) return;

	if (msg.size() < 512)
		tb->SetText(msg.c_str());
	else {
		std::string s = msg.substr(0U, 511);
		tb->SetText(s.c_str());
	}
}



// Translates the given editor mode to a string representation
std::string UI_ShipBuilder::TranslateEditorModeToString(EditorMode mode)
{
	switch (mode)
	{
		case EditorMode::GeneralMode:				return "General";
		case EditorMode::ShipSectionMode:			return "Sections";
		case EditorMode::TileMode:					return "Tiles";
		case EditorMode::ObjectMode:				return "Objects";
		case EditorMode::StructuralTestMode:		return "StructuralTesting";
		default:									return "";
	}
}





