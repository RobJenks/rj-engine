#pragma once

#ifndef __UI_ShipDesignerH__
#define __UI_ShipDesignerH__

#include <vector>
#include "iUIController.h"
#include "Image2DRenderGroup.h"
#include "ComplexShipElement.h"
#include "ComplexShipTile.h"
class Render2DGroup;
class iUIControl;
class UIButton;
class CoreEngine;
class Image2D;
class UserInterface;
class SDGridItem;
class ComplexShip;
class ComplexShipSection;
class ComplexShipTileDefinition;
class CSCorridorTile;

// This class has no special alignment requirements
class UI_ShipDesigner : public iUIController
{
public: 

	// The different views available in the ship designer
	enum SDViewMode { General = 0, Construction = 1, Corridor = 2, Tile = 3, SDTab4 = 4, SDTab5 = 5, SDTab6 = 6, SDTab7 = 7 };

	// The set of possible environment states that the ship designer can be in
	enum SDEnvState { DefaultEnvironment = 0 };

	// The set of possible mouse input states that the ship designer can be in
	enum SDMouseState { 
		Default = 0, 
		ScrollingSDGrid,
		DraggingShipSection,
		DraggingOutCorridorSection,
		DraggingOutTileSection
	};

	// Enumeration of all issues that can be identified with placement of a ship section
	enum SectionPlacementIssueType { 
		None = 0, 
		ConflictWithExistingSection,
		IncompatibleEdgeAttachment
	};

	// Holds the data pertaining to one issue with ship section placement
	struct SectionPlacementIssue {
		SectionPlacementIssueType		Type;
		INTVECTOR3						ShipLocation;
		INTVECTOR2						GridLocation;

		SectionPlacementIssue() : Type(SectionPlacementIssueType::None), ShipLocation(INTVECTOR3(-1, -1, -1)), GridLocation(INTVECTOR2(-1, -1)) { }
		SectionPlacementIssue(SectionPlacementIssueType type, INTVECTOR3 shiplocation, INTVECTOR2 gridlocation) 
				{ Type = type; ShipLocation = shiplocation; GridLocation = gridlocation; }
	};

	// Stores the consolidated result following evaluation of ship section placemnet
	struct SectionPlacementResult {
		std::vector<SectionPlacementIssue>::size_type	IssueCount;
		std::vector<SectionPlacementIssue>				Issues;
	};

	// Struct holding the mapping between ship sections and the images rendered to screen in the SD
	struct SDShipSection { 
		ComplexShipSection *Section; 
		Image2DRenderGroup::InstanceReference RenderInstance; 

		SDShipSection() : Section(NULL) { RenderInstance = Image2DRenderGroup::InstanceReference(); }
		SDShipSection(ComplexShipSection *sec, Image2DRenderGroup::InstanceReference ref) : Section(sec), RenderInstance(ref) { }
	};
	typedef std::vector<SDShipSection> SDShipSectionCollection;

	// Enumeration of possible corridor view modes
	enum CorridorViewMode {
		Unknown = 0,
		SingleCorridor
	};

	// Constructor / destructor
	UI_ShipDesigner(void);
	~UI_ShipDesigner(void);


	// Method that is called when the UI controller becomes active
	void Activate(void);

	// Method that is called when the UI controller is deactivated
	void Deactivate(void);


	// Initialise the ship designer
	Result								InitialiseController(Render2DGroup *render, UserInterface *ui);
	Result								InitialiseRenderGroups(Render2DGroup *render, UserInterface *ui);
	Result								Initialise2DRenderingGroup(Image2DRenderGroup **group, std::string key, std::string itemkey, Render2DGroup *render);
	Result								InitialiseRenderingConstants(void);
	
	// Methods to initialise each key view in turn
	Result								InitialiseShipSectionView(void);
	Result								InitialiseCorridorView(void);
	Result								InitialiseTileView(void);

	// Updates the SD grid layout on screen, to show all visible grid cells and also ensure linkages between them
	void								UpdateSDGridLayout(void);

	// Create new ship with default starting size
	Result								CreateNewShip(void);

	// Shuts down & deallocates the ship currently under construction in the SD
	void								ShutdownSDShip(void);

	// Performs final preparation on the ship to make it ready for use in the game
	Result								PrepareShipForOperation(ComplexShip *s, std::string code);

	// Retrieves the name of the blueprint we are working on
	std::string							GetBlueprintName(void);

	// Methods to load and save ships in the ship designer
	Result								LoadShip(std::string code);
	Result								SaveShip(std::string code);

	// Scrolls the grid by the specified amount
	void								ScrollGrid(float x, float y);

	// Zooms the grid in and out, with the main function zooming to a specific zoom (grid size) level
	int									DetermineZoomLevelIncrement(void);
	void								ZoomGridIn(void);
	void								ZoomGridOut(void);
	void								ZoomGridToLevel(int gridsizelevel);

	// Builds a collection of complex ship section preview images, for use in building & rendering ships under construction
	void								BuildShipSectionImageMap(void);

	// Renders the ship designer view.  Render methods specific to each view mode follow
	void								Render(void);

	// Performs rendering of the ship designer grid that is common to all views, e.g. the images of the ship sections 
	void								PerformCommonViewRendering(void);
	void								RenderMouseCursor(void);

	// General view rendering
	void								RenderGeneralView(void);

	// Construction view rendering
	void								RenderConstructionView(void);
	void								RenderShipSectionPreview(void);
	void								RenderShipSectionPreviewOnSDGrid(INTVECTOR2 gridsquare);

	// Method which is invoked whenever the ship blueprint is modified.  Updates the underlying data and render groups
	void								ShipBlueprintModified(void);

	// Performs a full update of the SD grid.  Main update method to be called when non-trivial changes are made
	void								PerformSDGridUpdate(void);

	// Methods to update the elements of the SD grid, and update all rendering components in the grid
	void								UpdateSDGridElementRendering(void);
	void								UpdateSDGridRendering(void);

	// Sets rendering of all SD grid component groups to the default; called before selectively making changes based on a new view mode
	void								ResetSDGridRenderingConfiguration(void);

	// Activates the render group configuration specific to a view mode
	void								ActivateViewModeRenderConfiguration(SDViewMode mode);

	// Recreates the mapping from ship sections to their render instances
	void								RefreshSDShipSectionInstances(void);

	// Method to process user input through this UI controller
	void								ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse);

	// Methods to handle mouse events
	void								ProcessMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component);
	void								ProcessMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component);
	void								ProcessMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component);
	void								ProcessRightMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component);
	void								ProcessRightMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component);
	void								ProcessRightMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component);

	// Methods to handle mouse click events on specific components
	void								ProcessMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation);
	void								ProcessRightMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation);

	// Methods to accept mouse move events, and mouse hover events relating to specific components
	void								ProcessMouseMoveEvent(INTVECTOR2 location);
	void								ProcessMouseHoverEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, bool lmb, bool rmb);

	// Methods to accept generic mouse click events at the specified location
	void								ProcessMouseClickAtLocation(INTVECTOR2 location) { }
	void								ProcessRightMouseClickAtLocation(INTVECTOR2 location) { }

	// Methods to process mouse clicks on managed components, e.g. buttons
	void								ProcessControlClickEvent(iUIControl *control);
	void								ProcessControlRightClickEvent(iUIControl *control);

	// Methods to process user events relating to each class of managed control
	void								ProcessButtonClickEvent(UIButton *button);

	// Methods to accept other managed control events
	void								ProcessTextboxChangedEvent(iUIControl *control) { }

	// Called when a control panel tab is clicked; changes the SD view accordingly
	void								ControlPanelTabClicked(UIButton *button);

	// Event raised whenever the key SD selectors are used by the user & the index is changed
	void								ShipSectionSelector_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex);
	void								CorridorSelector_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex);
	void								TileSelector_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex);

	// Called whenever the user selects a ship section in the construction view
	void								ShipSectionSelectedInConstructionView(std::string code);

	// Changes the corridor view mode based on the supplied parameter
	void								SetCorridorViewMode(CorridorViewMode mode);

	// Recalculates the size and positioning of the ship section preview within the preview window
	void								RecalculateShipSectionPreviewPositioning(void);

	// Evaluates the placement of a ship section for conflicts, and in general whether it can be attached to the ship in this place
	CMPINLINE SectionPlacementResult *  EvaluateSectionPlacement(void) { return EvaluateSectionPlacement(m_ship, m_consview_selected, m_consview_selected_gridpos, m_gridzpos); }
	SectionPlacementResult *			EvaluateSectionPlacement(ComplexShip *ship, ComplexShipSection *section, 
																 INTVECTOR2 gridpos, int gridzpos);

	// Evaluates the edge criteria around a ship section placement
	void								EvaluateSectionEdgePlacement(ComplexShipElement *sectionelement, Direction sectionedge,
																	 ComplexShipElement *neighbourelement, SectionPlacementResult *pPlacementResult,
																	 INTVECTOR3 shippos, INTVECTOR2 gridpos);

	// Renders all ship tiles onto the SD grid
	void								RenderShipTilesToSDGrid(void);

	// Renders all base tile data (stored within the base class & common to all tile classes) for the specified tile
	void								RenderBaseTileData(ComplexShipTile *tile, INTVECTOR3 tilepos, INTVECTOR3 tilesize, 
														   INTVECTOR3 gridstart, INTVECTOR3 gridend);

	// Renders a corridor tile to the SD grid
	void								RenderCorridorTile(CSCorridorTile *tile, INTVECTOR3 tilepos);

	// Renders the section attach points for a complex ship element on the SD grid
	void								RenderElementAttachPoints(ComplexShipElement *el, int shipxpos, int shipypos, INTVECTOR2 gridpos);

	// Rotates the ship section currently being dragged
	void								RotateShipSection(void);

	// Attempt to place the currently-selected ship section at the point it is currently hovering over
	void								PlaceShipSection(void);

	// Methods to apply and remove the SD offset, for translating between the in-game ship object and its SD representation
	void								ApplySDOffset(ComplexShip *ship);
	void								RemoveSDOffset(ComplexShip *ship);

	// Methods to check and change the current view mode
	CMPINLINE SDViewMode				GetCurrentViewMode(void) { return m_viewmode; }
	void								ChangeViewMode(SDViewMode mode);

	// Methods to process specific events raised from individual controls, and routed through the UserInterface
	void ComboBox_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex);

	// Tests whether a tile of the specified type could be placed at the selected location.  Note this is based on
	// the tile class only; a complete analysis is performed when we actually try to place the tile
	bool								TestTilePlacement(INTVECTOR3 location, D::TileClass tileclass);

	// Overloaded method to test tile placement based on a 2D grid square, rather than the element location in 3D space
	bool								TestTilePlacement(INTVECTOR2 location, D::TileClass tileclass);

	// Tests a set of tile placement options to determine whether they are valid; optionally also shows visually on the SD grid
	bool								TestTileSetPlacement(std::vector<INTVECTOR2> gridsquares, int zpos, D::TileClass tileclass, bool showpreview);
	bool								TestTileSetPlacement(std::vector<INTVECTOR2> gridsquares, D::TileClass tileclass, bool showpreview);

	// Updates the preview generated on the SD grid while we are dragging out a corridor section
	void								UpdateCorridorDragPreview(INTVECTOR2 location);

	// Deploys one or more tiles in the region defined by the specified start and end grid squares.  Will deploy any tiles
	// that can be placed, e.g. if we drag 4 tiles and 1 has a conflict, the remaining 3 will still be placed
	void								PlaceTiles(INTVECTOR2 start, INTVECTOR2 end);

	// Attempts to place one instance of a tile at the specified location on the SD grid
	void								PlaceTile(ComplexShipTileDefinition *tile, INTVECTOR2 start, INTVECTOR2 end);

	// Examines the elements at the specified mouse location and displays the context dependent tool for changing connections if appropriate
	void								ShowConnectionChangeTool(INTVECTOR2 location);
	
	// Forces a refresh of the connection change tool, by invoking the method with an invalid location so that it overrides the cached previous mouse location
	void								ForceRefreshOfConnectionChangeTool(void);

	// Applies the connection change tool to the currently selected elements
	void								ApplyConnectionChangeTool(INTVECTOR2 mouselocation);

	// Deploys a corridor tile at the specified location.  Also recalculates connections & tile types of the neighbouring tiles
	void								DeployCorridorTile(INTVECTOR2 location);

	// Deploys a set of corridor tiles from the start to end grid squares.  Will refuse to deploy the full set
	// if any tile fails placement validation
	void								DeployCorridorTiles(INTVECTOR2 start, INTVECTOR2 end);

	// Returns a new corridor tile depending on the environment in which it is about to be placed.  Params indicate whether
	// we have corridor tiles bordering on the specified sides
	CSCorridorTile *					CreateCorridorTile(bool left, bool up, bool right, bool down);

	// Returns a set of flags determining the corridor environment surrounding this element
	void								AnalyseCorridorEnvironment(ComplexShipElement *el, bool *pOutLeft, bool *pOutUp, bool *pOutRight, bool *pOutDown);

	// Updates a corridor tile and (optionally) its neighbours at the specified location, to make sure all connections are made correctly
	void								UpdateTile(ComplexShipTile *tile);
	void								UpdateTileAndNeighbours(ComplexShipTile *tile);

	// Updates a single tile at the specified location.  Does not consider neighbouring tiles
	void								UpdateCorridorTileAtElement(ComplexShipElement *el);

	// Determines the actual range of elements that would be covered by a tile placement, taking into account things
	// like fixed tile sizes.  Accepts a proposed start & end grid square as input, and returns a new value for the 
	// end grid square based on the size restrictions.  If no change is required it will return the proposed end grid square
	INTVECTOR2							GetExactTilePlacementExtent(INTVECTOR2 start, INTVECTOR2 end);

	// Methods to update the previews displayed while placing or dragging out a tile section
	void								UpdateTileDragPreview(INTVECTOR2 location);
	void								UpdateTileFixedSizePreview(INTVECTOR2 location);

	// Returns a reference to the render group that should be used for rendering the specified tile class
	Image2DRenderGroup *				GetTileRenderGroup(D::TileClass tileclass);
	
	// Static methods to translate between corridor view modes and their string tag representation
	static CorridorViewMode				TranslateCorridorViewModeFromString(std::string code);
	static std::string					TranslateCorridorViewModeToString(CorridorViewMode mode);

	// Returns the SD grid square at the specified location
	CMPINLINE INTVECTOR2				GetGridElementAtLocation(INTVECTOR2 location)
	{
		if (location.x < m_gridx || location.x > m_gridr || location.y < m_gridy || location.y > m_gridb) return INTVECTOR2(-1, -1);
		return INTVECTOR2(  (float)(location.x - m_gridx) / (float)m_gridsize, 
							(float)(location.y - m_gridy) / (float)m_gridsize );
	}

	// Returns the screen coordinates of a specified grid element
	CMPINLINE INTVECTOR2				GetGridElementLocation(INTVECTOR2 element)
										{ return INTVECTOR2(m_gridx + (element.x * m_gridsize), m_gridy + (element.y * m_gridsize)); }

	// Returns the ship element at the specified grid square
	CMPINLINE INTVECTOR3				GetElementAtGridSquare(INTVECTOR2 gridsquare)
										{ return INTVECTOR3(m_gridstart.x + gridsquare.x, m_gridstart.y + gridsquare.y, m_gridzpos); }

	// Calculates the set of elements covered by a path from a start to an end grid square
	std::vector<INTVECTOR2>				CalculateGridPath(INTVECTOR2 start, INTVECTOR2 end);

	// Returns the set of elements covered by a rectangular extent from a start to an end grid square
	std::vector<INTVECTOR2>				CalculateGridExtent(INTVECTOR2 start, INTVECTOR2 end);

	// Termination method to deallocate all resources
	void								Terminate(void);

private:

	// Applies a specific SD offset to a ship object
	void								ApplySDOffsetExplicit(ComplexShip *ship, INTVECTOR3 offset);

private:
	// Constant values
	static const int			GRID_SCROLL_THRESHOLD = 8;
	static const float			GRID_SCROLL_SPEED;
	static const float			CORRIDOR_TILE_WIDTH;
	static const float			CORRIDOR_TILE_OFFSET;
	static const float			ELEMENT_EDGE_THRESHOLD;

	// The SD grid
	SDGridItem **				m_grid;				// The SD grid
	int							m_gridx, m_gridy;	// Grid coordinates
	int							m_gridw, m_gridh;	// Grid size
	int							m_gridr, m_gridb;	// Right & bottom bounds of the SD grid
	int							m_numgx, m_numgy;	// Number of grid elements in each direction
	INTVECTOR2					m_gridscreenlocation;//Screen position of the grid (equivalent to m_gridx & m_gridy)
	INTVECTOR2					m_gridscreensize;	// Size of the grid on the screen (screenloc + screensize = {gridr, gridb})
	
	int							m_gridsize;			// Current size of each grid element
	int							m_gridsizemin;		// Minimum size each grid element can be displayed at
	int							m_gridsizemax;		// Maximum size each grid element can be displayed at

	int							m_gridxmax;			// The maximum number of grid cells that will be rendered in the x direction
	int							m_gridymax;			// The maximum number of grid cells that will be rendered in the y direction

	XMFLOAT2					m_gridpos;			// Exact position within the grid that is currently at the top-left corner
	INTVECTOR2					m_gridstart;		// Grid square that is currently in the top-left of the SD grid
	int							m_gridzpos;			// The z-level that we are currently viewing in the ship designer

	// The ship being constructed
	ComplexShip *				m_ship;				// The ship being constructed

	// The view mode we are currently in
	UI_ShipDesigner::SDViewMode		m_viewmode;			// Current view mode

	// State variables for the ship designer
	UI_ShipDesigner::SDEnvState		m_envstate;				// Current SD environment state
	UI_ShipDesigner::SDMouseState	m_mousestate;			// Current SD mouse state
	INTVECTOR2						m_griddragstart;		// SD grid square in which the current drag event began
	INTVECTOR2						m_lastgriddragsquare;	// The grid square that we were dragging within last frame (for efficiency)

	// Collection of pointers to the control panel tab buttons, for easier processing at runtime
	static const int			CONTROL_PANEL_TAB_COUNT = 8;
	UIButton *					m_panel_tabs[8];	

	// Set of image data collections for rendering each type of complex ship section; maps from the ship image to the I2DRG that covers it
	typedef std::unordered_map<const Texture*, Image2DRenderGroup*> CSSectionImageCollection;
	CSSectionImageCollection	m_css_images;
	
	// Set of references that link ship sections to image instances for rendering in the SD
	SDShipSectionCollection		m_sdshipsections;

	// The ship section currently selected in the construction view
	std::string								m_consview_selected_code;
	ComplexShipSection *					m_consview_selected;
	Image2DRenderGroup::InstanceReference 	m_consview_selected_image;

	// The position & size of the whole ship section preview window
	INTVECTOR2								m_consview_preview_pos, m_consview_preview_size;

	// The position & size of the ship section preview, within the overall section preview window.  Differs depending on
	// the ship section preview dimensions & overall size.  Also calculate the actual section pos & size on the SD grid.
	// We also store the ship section rotation, which affects only the physical ship model and not the element space
	INTVECTOR2								m_consview_selected_pos, m_consview_selected_size;
	INTVECTOR2								m_consview_selected_gridpos, m_consview_selected_gridsize;
	Rotation90Degree						m_consview_selected_rotation;

	SectionPlacementResult *				m_consview_section_placement_result;

	// Rendering constants relating to ship element attach points
	INTVECTOR2								m_apoint_pos_offset, m_apoint_size;		// Pos & size of a left-side attach point
	int										m_apoint_pos_faroffset;					// Offset for rendering the opposite side point

	// The render groups used for rendering items on the ship designer grid
	Image2DRenderGroup *		m_rg_inactivegrid;	// Rendering of inactive SD grid cells
	Image2DRenderGroup *		m_rg_activegrid;	// Rendering of active SD grid cells
	Image2DRenderGroup *		m_rg_buildablegrid;	// Rendering of buildable SD grid cells

	Image2DRenderGroup *		m_rg_blueprint;				// Blueprint preview squares on the SD grid
	Image2DRenderGroup *		m_rg_placementconflict;		// Squares that conflict during item placement
	Image2DRenderGroup *		m_rg_apoint_standard;		// Element attach points - standard type
	Image2DRenderGroup *		m_rg_apoint_turret;			// Element attach points - turret type
	Image2DRenderGroup *		m_rg_walkable_connect;		// Walkable connection points
	Image2DRenderGroup *		m_rg_corridor;				// Corridor tiles
	
	Image2DRenderGroup *		m_rg_tile_border;			// Border used to delimit tiles
	Image2DRenderGroup *		m_rg_tile_civilian;			// Civilian-type tiles, e.g. crew quarters, gyms, canteens
	Image2DRenderGroup *		m_rg_tile_engineer;			// Engineering-type tiles, e.g. engines, maintenance, reactors
	Image2DRenderGroup *		m_rg_tile_military;			// Military-type tiles, e.g. turrets, armories, hangars

	// Render components for other parts of the ship designer
	Image2DRenderGroup *		m_rg_connectionchangetool;	// The tool used for changing connections to an element

	// Rendering order constants loaded in from the data file at initialisation
	float						z_shipsections;
	float						z_shippreview;

	// Reference to the mouse and keyboard input controllers
	GameInputDevice *			m_mouse;
	GameInputDevice *			m_keyboard;
	BOOL *						m_keys;
	INTVECTOR2					m_mousepos;

	// Key controls in the ship designer
	UIComboBox *				m_shipsectionselector;
	UIComboBox *				m_corridorselector;
	UIComboBox *				m_tileselector;

	// Fields maintaining the state of the corridor view
	CorridorViewMode			m_corridormode;
	INTVECTOR2					m_corridorselectsize;

	// Fields maintaining the state of the tile view
	ComplexShipTileDefinition * m_selectedtiletype;
	INTVECTOR3					m_connectionchangetool_element1;
	INTVECTOR3					m_connectionchangetool_element2;

};

class SDGridItem
{
public:
	INTVECTOR2					Position;
	INTVECTOR2					Size;

	int							GridRenderItemInstance;

	SDGridItem *				Left;
	SDGridItem *				Right;
	SDGridItem *				Up;
	SDGridItem *				Down;

	SDGridItem(void)			
	{ 
		GridRenderItemInstance = -1; 
		Position = INTVECTOR2(0, 0); Size = INTVECTOR2(0, 0);
		Left = NULL; Right = NULL; Up = NULL; Down = NULL; 
	}
};

#endif