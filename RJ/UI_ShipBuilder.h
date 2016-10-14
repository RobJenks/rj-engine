#pragma once

#ifndef __UI_ShipBuilder__
#define __UI_ShipBuilder__

#include <vector>
#include "AlignedAllocator.h"
#include "iUIController.h"
#include "VolumetricLine.h"
#include "TileAdjacency.h"
#include "SimulatedEnvironmentCollision.h"
class ComplexShip;
class UIButton;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class UI_ShipBuilder : public ALIGN16<UI_ShipBuilder>, public iUIController
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(UI_ShipBuilder)

	// Enumeration of possible editor modes
	enum EditorMode { ShipSectionMode = 0, TileMode, ObjectMode, StructuralTestMode };

	// Enumeration of possible camera states
	enum SBCameraState { Normal = 0, Rotating, Locked };

	// Static constants
	static const float								ZOOM_SPEED;						// Rate at which the camera zooms, as a percentage of the model collision radius
	static const float								ZOOM_INCREMENT_SCALE;			// Speed at which camera zoom increments are applied
	static const float								ZOOM_INCREMENT_TIME;			// The time (secs) over which a zoom increment is applied
	static const float								DEFAULT_ZOOM_LEVEL;				// The default starting zoom level, as a percentage of the model collision radius
	static const AXMVECTOR							DEFAULT_CAMERA_ROTATION;		// Default rotation state for the camera
	static const float								MIN_ZOOM_LEVEL;					// The closest we can zoom into the object
	static const float								MOUSE_DRAG_DISTANCE_MODIFIER;	// Multiplier on mouse drag distance when converting to camera movement
	static const float								MOUSE_DRAG_ROTATION_SPEED;		// Rotation speed for the camera about the current origin point
	static const float								MOUSE_DRAG_PAN_SPEED;			// Pan speed for the camera based on user input
	static const float								CAMERA_REVERT_TIME;				// Time for the camera to revert to origin and ID orientation
	static const float								COMPONENT_FADE_TIME;			// Time (secs) for components to fade in or out 
	static const float								COMPONENT_FADE_OUT_ALPHA;		// Alpha value for components that have been faded out
	static const XMFLOAT3							TILE_PLACEMENT_COLOUR_VALID;	// Selection outline for valid placement
	static const XMFLOAT3							TILE_PLACEMENT_COLOUR_INVALID;	// Selection outline for invalid placement
	static const XMFLOAT3							TILE_PLACEMENT_COLOUR_PLACEMENTERROR;	// For specific elements in error

	// Default constructor
	UI_ShipBuilder(void);

	// Initialisation method, called once the UI component has been created
	Result												InitialiseController(Render2DGroup *render, UserInterface *ui);

	// Method that is called when the UI controller becomes active
	void												Activate(void);

	// Method to perform per-frame logic and perform rendering of the UI controller (excluding 2D render objects, which will be handled by the 2D render manager)
	void												Render(void);

	// Updates any editor-specific render data for the current frame
	void												PerformRenderUpdate(void);

	// Sets the ship under construction and focuses the view on it
	void												SetShip(ComplexShip *ship);

	// Sets the currently-active editor mode
	void												SetEditorMode(EditorMode mode);

	// Method that is called when the UI controller is deactivated
	void												Deactivate(void);

	// Returns a pointer to the ship currently under construction
	CMPINLINE ComplexShip *								GetShip(void)						{ return m_ship; }

	// Returns the current editor mode
	CMPINLINE EditorMode								GetEditorMode(void) const			{ return m_mode; }

	// Initialise any editor-specific render states and data
	void												InitialiseRenderData(void);

	// Revert any editor-specific render states and data
	void												RevertRenderData(void);

	// Zoom the view in or out
	CMPINLINE void										ZoomIn(void)						{ PerformZoom(-GetZoomSpeedPerSecond() * Game::PersistentTimeFactor); }
	CMPINLINE void										ZoomOut(void)						{ PerformZoom(+GetZoomSpeedPerSecond() * Game::PersistentTimeFactor); }
	CMPINLINE void										ZoomInIncrement(void)				{ PerformZoomIncrement(-GetZoomIncrement()); }
	CMPINLINE void										ZoomOutIncrement(void)				{ PerformZoomIncrement(+GetZoomIncrement()); }
	CMPINLINE float										GetZoomLevel(void) const			{ return m_zoomlevel; }
	float												GetZoomSpeedPerSecond(void) const;
	float												GetDefaultZoomLevel(void) const;
	float												GetMinZoomLevel(void) const;
	float												GetZoomIncrement(void) const;

	// Revert the camera back to base position/orientation/zoom
	void												RevertCamera(void);

	// Adjust the level of the environment currently being modified
	CMPINLINE void										MoveUpLevel(void)					{ MoveToLevel(m_level + 1); }
	CMPINLINE void										MoveDownLevel(void)					{ MoveToLevel(m_level - 1); }
	void												MoveToLevel(int level);

	// Events raised when the mouse moves between elements of the ship
	void												MouseExitingElement(const INTVECTOR3 & old_element);
	void												MouseEnteringElement(	const INTVECTOR3 & new_element, bool entered_from_another_element, 
																				const INTVECTOR3 & entered_from);

	// If we are holding a tile for placement, adjusts it to fit with its surrounding environment
	void												UpdatePlacementTile(void);

	// If we are holding a tile for placement, revert any changes made when the tile was positioned within this element
	void												RevertPlacementTileUpdates(void);

	// Method to process user input into the active UI controller
	void ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse);
	void ProcessKeyboardInput(GameInputDevice *keyboard);
	void ProcessMouseInput(GameInputDevice *mouse, GameInputDevice *keyboard);

	// Methods to accept mouse events from the UI manager
	void ProcessMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component);
	void ProcessMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component);

	void ProcessRightMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessRightMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component);
	void ProcessRightMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component) { }

	// Methods to accept generic mouse click events at the specified location
	void ProcessMouseClickAtLocation(INTVECTOR2 location);
	void ProcessRightMouseClickAtLocation(INTVECTOR2 location) { }

	// Methods to accept the processed mouse click events for particular components
	void ProcessMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation) { }
	void ProcessRightMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation) { }

	// Methods to accept the processed mouse click events for managed components, e.g. buttons
	void ProcessControlClickEvent(iUIControl *control) { }
	void ProcessControlRightClickEvent(iUIControl *control) { }

	// Process button click events in the UI
	void ProcessButtonClickEvent(UIButton *button) { }

	// Method to accept mouse move events, and also mouse hover events for specific components
	void ProcessMouseMoveEvent(INTVECTOR2 location);
	void ProcessMouseHoverEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, bool lmb, bool rmb) { }

	// Methods to process specific events raised from individual controls, and routed through the UserInterface
	void ComboBox_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex) { }


protected:

	// Enumeration of possible tile placement issue types
	enum TilePlacementIssueType { OutOfEnvironmentBounds = 0, ElementAlreadyOccupied };

	// Struct holding data on a tile placement issue
	struct TilePlacementIssue
	{
		TilePlacementIssueType					Type;						// The type of issue.  Mandatory
		INTVECTOR3								Element;					// Specific element with the error (if applicable based on type)

		TilePlacementIssue(void) { }
		TilePlacementIssue(TilePlacementIssueType type) : Type(type) { }
		TilePlacementIssue(TilePlacementIssueType type, const INTVECTOR3 & element) : Type(type), Element(element) { }
	};

	// Initialise the UI and focus on the target ship
	void										InitialiseForShip(ComplexShip *ship);

	// Event triggered when an editor mode is deactivated.  "mode" is the mode being deactivated
	void										EditorModeDeactivated(EditorMode mode, EditorMode next_mode);

	// Event triggered when an editor mode is activated.  "mode" is the mode being activated
	void										EditorModeActivated(EditorMode mode, EditorMode previous_mode);

	// Activate the specified editor mode
	void										ActivateSectionMode(EditorMode previous_mode);
	void										ActivateTileMode(EditorMode previous_mode);
	void										ActivateObjectMode(EditorMode previous_mode);
	void										ActivateStructuralTestMode(EditorMode previous_mode);

	// Locks the camera for the specified period of time, after which it will be released again to the user
	void										LockCamera(unsigned int time_ms);
	void										LockCamera(float time_secs);

	// Releases the camera back to normal user control again
	void										ReleaseCamera(void);

	// Perform any updates of the camera required since the previous frame
	void										PerformCameraUpdate(void);

	// Zooms the view by the specified amount
	void										PerformZoom(float zoom);
	void										PerformZoomIncrement(float increment);

	// Indicates whether a zoom increment is currently in progress
	CMPINLINE bool								ZoomIncrementInProgress(void) const			{ return (m_zoom_increment_end >= Game::PersistentClockTime); }

	// Sets the camera zoom level directly
	void										SetZoom(float zoom);

	// Rotates the camera based on user mouse input
	void										RotateCameraBasedOnUserInput(const INTVECTOR2 & startlocation, const INTVECTOR2 & currentlocation);

	// Pans the camera based on user mouse input
	void										PanCameraBasedOnUserInput(const INTVECTOR2 & startlocation, const INTVECTOR2 & currentlocation);

	// Render the editor grid, depending on editor mode
	void										RenderEditorGrid(void);

	// Perform editor-mode-specific rendering
	void										PerformTileModeRendering(void);
	void										PerformStructuralTestModeRendering(void);

	// Moves the 'temporary' tile being placed to a new location in the environment, recalculating data as required
	void										MovePlacementTile(const INTVECTOR3 & location);

	// If a tile is currently being placed, renders the tile and performs any other associated rendering
	void										RenderTilePlacement(void);

	// Place the selected tile at its current location, assuming the placement is valid
	void										PlaceTile(void);

	// Tests whether the proposed tile placement is valid.  Returns a flag indicating validity.  Also outputs
	// a list of errors to the supplied output vector, if any exist
	bool										TestTilePlacement(	ComplexShipTile *tile, const INTVECTOR3 & location, 
																	std::vector<TilePlacementIssue> & outPlacementIssues);

	// Reset the position of the intersection test markers and the test parameters
	void										ResetStructuralTestParameters(void);

	// Sets the position of an intersection test marker in world space
	void										SetIntersectionTestMarkerPosition(SimpleShip **ppMarker, const FXMVECTOR position);

	// Performs the intesection test in 'structural test' mode
	void										PerformIntersectionTest(void);

	// Store the results of an intersection test
	void										StoreIntersectionTestResults(const SimulatedEnvironmentCollision & results);

	// Clear any stored intersection test results
	void										ClearIntersectionTestResults(void);

	// Handles the LMB first-down event in structural test mode
	void										HandleStructuralModeMouseFirstDown(void);

	// Handles the mouse move event while in structural testing mode
	void										HandleStructuralModeMouseMove(void);

	// Handles the mouse up event while in structural testing mode
	void										HandleStructuralModeMouseUp(void);

	// Internal methods to get the current position/orientation of the game camera
	XMVECTOR									GetCameraPosition(void) const;
	XMVECTOR									GetCameraOrientation(void) const;




	// Member variables
	ComplexShip *								m_ship;						// The ship currently being worked on
	EditorMode									m_mode;						// Current editor mode
	AXMVECTOR									m_centre;					// The centre point (relative to ship centre) that the camera is focused on
	VolumetricLine								m_gridline;					// Volumetric line used to render the editor grid

	int											m_level;					// The level of the environment that we are currently editing.  0-based

	SBCameraState								m_camerastate;				// Indicates the current state of the editor camera
	AXMVECTOR									m_camera_rotate;			// Current camera rotation about centre
	unsigned int								m_camera_release;			// The clock ms at which the camera is released again, following e.g. a camera path transition
	float										m_zoomlevel;				// Current camera zoom level
	bool										m_reverting_camera;			// Flag that indicates the camera is currently reverting to base position/orientation/zoom
	float										m_reverttimeremaining;		// Time (secs) left for the camera to be fully-reverted
	AXMVECTOR									m_reverting_from;			// Orientation quaternion that we are reverting from 
	AXMVECTOR									m_revert_centre_from;		// Centre point that we are reverting from
	float										m_revert_zoom_from;			// Zoom level that we are reverting from
	AXMVECTOR									m_rmb_down_start_centre;	// Centre point of the camera when the RMB was first depressed; used for panning
	float										m_zoom_increment_amount;	// Zoom increment that is currently being applied
	float										m_zoom_increment_end;		// Time at which the current zoom increment will end

	// Data relating to editor-specific lighting
	unsigned int								m_revert_dir_light_index;			// Index of the directional light that is being used for the editor
	bool										m_revert_dir_light_is_overriding;	// Flag indicating whether the directional light is overriding another (that needs to be reverted afterwards)
	LightData									m_revert_dir_light;					// Data on the directional light that was temporarily overridden

	// Fields related to selection of ship elements
	bool										m_mouse_is_over_element;			// Indicates whether the mouse is currently over a ship element
	INTVECTOR3									m_mouse_over_element;				// Ship element that the mouse is currently over, if m_mouse_is_over_element == true

	// Fields relating to the selection and placing of ship tiles
	ComplexShipTile *							m_tile_being_placed;				// The ship tile that follows the mouse and can be placed in the environment
	bool										m_placing_generic_corridors;		// Flag indicating whether the corridor tile we are placing is to be adapted to meet 
																					// the connections of tiles around it
	bool										m_tile_placement_is_valid;			// Based on the last tile placement test
	std::vector<TilePlacementIssue>				m_tile_placement_issues;			// Vector populated with each tile placement error that is encountered
	std::vector<INTVECTOR3>						m_placement_tile_changes;			// Vector populated with the location of all tiles that were temporarily modified during 
																					// tile placement, and which need to be reverted once tile placement ends or moves to another element
public:
	// Fields relating to the structural testing functionality
	SimpleShip *								m_intersect_marker_start;			// Marker at the start of the projectile intersection test path
	SimpleShip *								m_intersect_marker_end;				// Marker at the end of the projectile intersection test path
	SimpleShip **								m_selected_intersection_marker;		// The marker that is currently being manipulated (if applicable)
	VolumetricLine								m_intersect_test_trajectory;		// Trajectory of the collider in interection test mode
	ObjectReference<SimpleShip>					m_intersect_test_proj;				// The projectile used to perform the intersection test
	SimulatedEnvironmentCollision				m_simulated_intersection;			// The sequence of intersection events that occured during the last intersection event

};




#endif