#include <math.h>
#include <windows.h>
#include <vector>
#include "time.h"
#include "DX11_Core.h"

#include "CoreEngine.h"
#include "UserInterface.h"
#include "Render2DGroup.h"
#include "RenderMouseEvent.h"
#include "Image2D.h"
#include "Image2DRenderGroup.h"
#include "UIComponentGroup.h"
#include "UIButton.h"
#include "UITextBox.h"
#include "UIComboBox.h"
#include "XMLGenerator.h"
#include "DataInput.h"
#include "DataOutput.h"
#include "GameDataExtern.h"
#include "GameVarsExtern.h"
#include "Utility.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "ComplexShipElement.h"
#include "iContainsComplexShipTiles.h"
#include "ComplexShipTileDefinition.h"
#include "ComplexShipTile.h"
#include "CSCorridorTile.h"

#include "UI_ShipDesigner.h"

// Constant values
const float UI_ShipDesigner::GRID_SCROLL_SPEED = 0.01f;
const float UI_ShipDesigner::CORRIDOR_TILE_WIDTH = 0.7f;
const float UI_ShipDesigner::CORRIDOR_TILE_OFFSET = ((1.0f - CORRIDOR_TILE_WIDTH) / 2.0f);
const float UI_ShipDesigner::ELEMENT_EDGE_THRESHOLD = 0.4f;

// Constructor
UI_ShipDesigner::UI_ShipDesigner(void)
{
	m_grid = NULL;
	m_gridpos = NULL_FLOAT2;
	m_gridstart.x = 0; m_gridstart.y = 0; m_gridzpos = 0;
	
	m_mouse = m_keyboard = NULL;
	m_keys = NULL;
	
	m_ship = NULL;

	m_consview_selected = NULL;
	m_consview_preview_pos = m_consview_selected_pos = INTVECTOR2(0, 0);
	m_consview_preview_size = m_consview_selected_size = INTVECTOR2(0, 0);
	m_consview_selected_gridpos = m_consview_selected_gridsize = INTVECTOR2(0, 0);
	m_consview_selected_rotation = Rotation90Degree::Rotate0;
	m_consview_section_placement_result = NULL;
	
	z_shipsections = 0.0f;
	z_shippreview = 0.0f;
	
	m_shipsectionselector = NULL;
	m_corridorselector = NULL;
	m_tileselector = NULL;

	// Set the default view mode and environment states 
	m_viewmode = UI_ShipDesigner::SDViewMode::General;
	m_envstate = UI_ShipDesigner::SDEnvState::DefaultEnvironment;
	m_mousestate = UI_ShipDesigner::SDMouseState::Default;
	m_griddragstart = INTVECTOR2(-1, -1);

	// Corridor mode variables
	m_corridorselectsize = INTVECTOR2(1, 1);
	m_lastgriddragsquare = INTVECTOR2(-1, -1);

	// Tile mode variables
	m_selectedtiletype = NULL;
	m_connectionchangetool_element1 = m_connectionchangetool_element2 = INTVECTOR3(-1, -1, -1);
}


// Method that is called when the UI controller becomes active
void UI_ShipDesigner::Activate(void)
{

}

// Method that is called when the UI controller is deactivated
void UI_ShipDesigner::Deactivate(void)
{

}

Result UI_ShipDesigner::InitialiseController(Render2DGroup *render, UserInterface *ui)
{
	Result result;
	int gx, gy, gw, gh, gsz;
	std::string str;
	std::string gtexture, stexfile, shlfile;
		
	// Build an image instance collection for each ship section, for use in building & rendering ships under construction
	BuildShipSectionImageMap();

	// Initialise all render groups used for rendering the ship designer
	result = InitialiseRenderGroups(render, ui);
	if (result != ErrorCodes::NoError) return result;

	// First initialise the ship designer grid; make sure the required constants have all been provided
	if ( (render->HaveConstant("sd_grid.x") && render->HaveConstant("sd_grid.y") && render->HaveConstant("sd_grid.w") &&
		  render->HaveConstant("sd_grid.h") && render->HaveConstant("sd_grid.gridsize.current") && 
		  render->HaveConstant("sd_grid.gridsize.min") && render->HaveConstant("sd_grid.gridsize.max") ) == false)
				return ErrorCodes::MissingDataRequiredToInitialiseShipDesignerUI;

	// Pull the data required to construct the SD grid
	gx =  atoi(render->GetConstant("sd_grid.x").c_str());
	gy =  atoi(render->GetConstant("sd_grid.y").c_str());
	gw =  atoi(render->GetConstant("sd_grid.w").c_str());
	gh =  atoi(render->GetConstant("sd_grid.h").c_str());

	// Grid size data is pulled as min/current/max.  We will store all and then use the minimum size when initialising
	m_gridsize = atoi(render->GetConstant("sd_grid.gridsize.current").c_str());
	m_gridsizemin = atoi(render->GetConstant("sd_grid.gridsize.min").c_str());
	m_gridsizemax = atoi(render->GetConstant("sd_grid.gridsize.max").c_str());

	// Also store these constants for future reference
	m_gridx = gx; m_gridy = gy;
	m_gridw = gw; m_gridh = gh;

	// Based on minimum grid cell size (i.e. maximum number of cells), calculate the 
	// number of elements in each direction and allocate storage
	gsz = m_gridsizemin;
	m_numgx = (int)ceilf((float)gw / (float)gsz);
	m_numgy = (int)ceilf((float)gh / (float)gsz);

	// Also store these values as the maximum size of the SD grid, i.e. when its cells are displayed at their lowest size
	m_gridxmax = m_numgx; m_gridymax = m_numgy;

	// Store grid location and size in INTVECTORs as well for each of use later
	m_gridscreenlocation = INTVECTOR2(m_gridx, m_gridy);
	m_gridscreensize = INTVECTOR2((m_numgx * gsz), (m_numgy * gsz));

	// Determine the far bounds of the SD grid
	m_gridr = m_gridx + m_gridscreensize.x;
	m_gridb = m_gridy + m_gridscreensize.y;

	// Allocate storage for the SD grid
	m_grid = new SDGridItem*[m_gridxmax];
	for (int i=0; i<m_gridxmax; i++)
		m_grid[i] = new SDGridItem[m_gridymax];

	// Initialise each element in the SD grid 
	int instancecount = 0;
	for (int x=0; x<m_gridxmax; x++)
	{
		for (int y=0; y<m_gridymax; y++)
		{	
			// Create the a new render instance for this grid item and add to the render group
			m_grid[x][y].Position = INTVECTOR2(gx + x*gsz, gy + y*gsz);
			m_grid[x][y].Size = INTVECTOR2(gsz, gsz);

			// Create a new inactive AND active grid instance for rendering.  We will hide selectively as required
			m_rg_inactivegrid->AddInstance(m_grid[x][y].Position, m_rg_inactivegrid->GetZOrder(), m_grid[x][y].Size, true, Rotation90Degree::Rotate0);
			m_rg_activegrid->AddInstance(m_grid[x][y].Position, m_rg_activegrid->GetZOrder(), m_grid[x][y].Size, false, Rotation90Degree::Rotate0);
			m_rg_buildablegrid->AddInstance(m_grid[x][y].Position, m_rg_buildablegrid->GetZOrder(), m_grid[x][y].Size, false, Rotation90Degree::Rotate0);

			// Also record the index into each of these render collections, in case we need to directly access later
			m_grid[x][y].GridRenderItemInstance = instancecount;
			instancecount++;

			// Set up this grid element, including links through to all neighbouring items
			if (x > 0)				m_grid[x][y].Left = &m_grid[x-1][y];	else		m_grid[x][y].Left = NULL;
			if (x < (m_numgx-1))	m_grid[x][y].Right = &m_grid[x+1][y];	else		m_grid[x][y].Right = NULL;
			if (y > 0)				m_grid[x][y].Up = &m_grid[x][y-1];		else		m_grid[x][y].Up = NULL;
			if (y < (m_numgy-1))	m_grid[x][y].Down = &m_grid[x][y+1];	else		m_grid[x][y].Down = NULL;
		}
	}

	// Also build an array of the control panel buttons, so we can process them more easily at runtime
	for (int i=0; i<CONTROL_PANEL_TAB_COUNT; i++)
		m_panel_tabs[i] = m_render->Components.Buttons.GetItem(concat("panel_tab_")(i).str());

	// Read the ship section preview window data from render constants
	if (m_render->HaveConstant("shipsecprev.x") && m_render->HaveConstant("shipsecprev.y") && 
		m_render->HaveConstant("shipsecprev.w") && m_render->HaveConstant("shipsecprev.h")) {
			std::string cx = m_render->GetConstant("shipsecprev.x");
			std::string cy = m_render->GetConstant("shipsecprev.y");
			std::string cw = m_render->GetConstant("shipsecprev.w");
			std::string ch = m_render->GetConstant("shipsecprev.h");
			m_consview_preview_pos = INTVECTOR2(atoi(cx.c_str()), atoi(cy.c_str()));
			m_consview_preview_size = INTVECTOR2(atoi(cw.c_str()), atoi(ch.c_str()));
			if (m_consview_preview_size.x == 0 || m_consview_preview_size.y == 0) { m_consview_preview_size = INTVECTOR2(128, 128); }
	} else {
		m_consview_preview_pos = INTVECTOR2(0, 0); m_consview_preview_size = INTVECTOR2(128, 128);
	}

	// Load rendering order constants
	if (m_render->HaveConstant("shipsections.z")) { str = m_render->GetConstant("shipsections.z"); z_shipsections = (float)atof(str.c_str()); }
	if (m_render->HaveConstant("shippreview.z")) { str = m_render->GetConstant("shippreview.z"); z_shippreview = (float)atof(str.c_str()); }

m_gridsize=gsz;

	// Initialise rendering constants
	InitialiseRenderingConstants();

	// Initialise each SD view in turn
	HandleErrors( InitialiseShipSectionView()			, result);
	HandleErrors( InitialiseCorridorView()				, result);
	HandleErrors( InitialiseTileView()					, result);

	// Initialise the ship designer to the first tab, by simulating a click of the tab button
	UIButton *tab = m_render->Components.Buttons.GetItem("panel_tab_0");
	if (tab) ControlPanelTabClicked(tab);

	// Create a new empty ship to begin with
	CreateNewShip();

	// Return success
	return ErrorCodes::NoError;
}

Result UI_ShipDesigner::InitialiseShipSectionView(void)
{
	// Retrieve pointers to the key components for this view
	m_shipsectionselector = m_render->Components.ComboBoxes.GetItem("shipsec_selector");

	// Make sure we have all key components available
	if (!m_shipsectionselector) return ErrorCodes::CouldNotInitialiseSDSectionView;

	// Populate the ship section selector
	DataRegister<ComplexShipSection>::RegisterType::const_iterator it_end = D::ComplexShipSections.Data.end();
	for (DataRegister<ComplexShipSection>::RegisterType::const_iterator it = D::ComplexShipSections.Data.begin(); it != it_end; ++it)
		if (it->second && it->second->IsStandardObject() && it->second->GetName() != NullString)
			m_shipsectionselector->AddItem(it->second->GetName(), it->second->GetCode());

	// Select the first item in the list to initialise the ship section display.  Set manually since initialisation
	// takes place before the controller has been set by the UI
	if (m_shipsectionselector->GetItemCount() > 0) 
		ShipSectionSelector_SelectedIndexChanged(m_shipsectionselector, 0, -1);

	// Return success
	return ErrorCodes::NoError;
}

Result UI_ShipDesigner::InitialiseCorridorView(void)
{
	// Retrieve pointers to the key components for this view
	m_corridorselector = m_render->Components.ComboBoxes.GetItem("corridor_selector");

	// Make sure we have all key components available
	if (!m_corridorselector) return ErrorCodes::CouldNotInitialiseSDCorridorView;

	// Manually populate (at least for now) the available corridor tiles
	m_corridorselector->AddItem("Single Corridor", TranslateCorridorViewModeToString(CorridorViewMode::SingleCorridor));

	// Select the first item in the list to initialise the SD corridor view.  Also manually trigger the event since 
	// the controller is not linked up until post-initialisation
	m_corridorselector->SelectItem(0);
	CorridorSelector_SelectedIndexChanged(m_corridorselector, 0, -1);	

	// Return success
	return ErrorCodes::NoError;
}

Result UI_ShipDesigner::InitialiseTileView(void)
{
	// Retrieve pointers to the key components for this view
	m_tileselector = m_render->Components.ComboBoxes.GetItem("tile_selector");

	// Make sure we have all key components available
	if (!m_tileselector) return ErrorCodes::CouldNotInitialiseSDTileView;

	// Manually populate (at least for now) the available tiles
	m_tileselector->AddItem("Crew quarters", "QUARTERS_DORM_01");

	// Select the first item in the list to initialise the SD tile view.  Also manually trigger the event since 
	// the controller is not linked up until post-initialisation
	m_tileselector->SelectItem(0);
	TileSelector_SelectedIndexChanged(m_tileselector, 0, -1);

	// Return success
	return ErrorCodes::NoError;
}


// Updates the SD grid layout on screen, to show all visible grid cells and also ensure linkages between them
void UI_ShipDesigner::UpdateSDGridLayout(void)
{
	Image2DRenderGroup::Instance *inst = NULL;

	// Recalculate the number of visible grid cells in each direction
	m_numgx = (int)ceilf((float)m_gridw / (float)m_gridsize);
	m_numgy = (int)ceilf((float)m_gridh / (float)m_gridsize);

	// Determine the far bounds of the visible SD grid
	m_gridr = m_gridx + (m_numgx * m_gridsize);
	m_gridb = m_gridy + (m_numgy * m_gridsize);

	// Loop over each element in the full SD grid (i.e. both visible and not)
	for (int x=0; x<m_gridxmax; x++)
	{
		for (int y=0; y<m_gridymax; y++)
		{	
			// Determine the instance number of this grid cell
			int instance = m_grid[x][y].GridRenderItemInstance;

			// Test whether this cell falls within the visible grid bounds
			if (x >= m_numgx || y >= m_numgy)
			{
				// This cell is not currently visible, so hide it now
				m_rg_inactivegrid->GetInstance(instance)->SetRenderActive(false);
				m_rg_activegrid->GetInstance(instance)->SetRenderActive(false);
				m_rg_buildablegrid->GetInstance(instance)->SetRenderActive(false);
			}
			else
			{
				// This cell is currently visible; first, calculate its size and position
				INTVECTOR2 pos = m_grid[x][y].Position = INTVECTOR2(m_gridx + x*m_gridsize, m_gridy + y*m_gridsize);
				INTVECTOR2 size = m_grid[x][y].Size = INTVECTOR2(m_gridsize, m_gridsize);

				// Set the size and position of each SD grid render group.  NOTE we don't need to set render=1/0; this is done by the update methods
				inst = m_rg_inactivegrid->GetInstance(instance);
				if (inst) { inst->position = pos; inst->size = size; }
				inst = m_rg_activegrid->GetInstance(instance);
				if (inst) { inst->position = pos; inst->size = size; }
				inst = m_rg_buildablegrid->GetInstance(instance);
				if (inst) { inst->position = pos; inst->size = size; }
				
				// Also set neighbour pointers between grid cells
				if (x > 0)				m_grid[x][y].Left = &m_grid[x-1][y];	else		m_grid[x][y].Left = NULL;
				if (x < (m_numgx-1))	m_grid[x][y].Right = &m_grid[x+1][y];	else		m_grid[x][y].Right = NULL;
				if (y > 0)				m_grid[x][y].Up = &m_grid[x][y-1];		else		m_grid[x][y].Up = NULL;
				if (y < (m_numgy-1))	m_grid[x][y].Down = &m_grid[x][y+1];	else		m_grid[x][y].Down = NULL;
			}
		}
	}

	// Also recalculate the size of any ship section being dragged onto the SD grid
	m_consview_selected_gridsize = INTVECTOR2(	m_consview_selected->GetElementSize().x * m_gridsize, 
												m_consview_selected->GetElementSize().y * m_gridsize );

}

// Initialises all render groups
Result UI_ShipDesigner::InitialiseRenderGroups(Render2DGroup *render, UserInterface *ui)
{
	Result result;

	// Base grid cells
	HandleErrors(Initialise2DRenderingGroup(&m_rg_inactivegrid, "SDGridInactive", "sd_grid.inactive", render), result);
	HandleErrors(Initialise2DRenderingGroup(&m_rg_activegrid, "SDGridActive", "sd_grid.active", render), result);
	HandleErrors(Initialise2DRenderingGroup(&m_rg_buildablegrid, "SDGridBuildable", "sd_grid.buildable", render), result);

	// Placement blueprint preview & conflicts
	HandleErrors(Initialise2DRenderingGroup(&m_rg_blueprint, "SDGridBlueprint", "sd_grid.blueprint", render), result);
	HandleErrors(Initialise2DRenderingGroup(&m_rg_placementconflict, "SDGridPlacementConflict", "sd_grid.placementconflict", render), result);

	// Element attach points
	HandleErrors(Initialise2DRenderingGroup(&m_rg_apoint_standard, "SDGridAPointStandard", "sd_grid.apoint_standard", render), result);
	HandleErrors(Initialise2DRenderingGroup(&m_rg_apoint_turret, "SDGridAPointTurret", "sd_grid.apoint_turret", render), result);

	// Tile render groups
	HandleErrors(Initialise2DRenderingGroup(&m_rg_walkable_connect, "SDGridWalkableConnect", "sd_grid.walkable_connect", render), result);
	HandleErrors(Initialise2DRenderingGroup(&m_rg_corridor, "SDGridCorridor", "sd_grid.corridor", render), result);
	HandleErrors(Initialise2DRenderingGroup(&m_rg_tile_civilian, "SDGridTileCivilian", "sd_grid.tile_civilian", render), result);
	HandleErrors(Initialise2DRenderingGroup(&m_rg_tile_engineer, "SDGridTileEngineering", "sd_grid.tile_engineering", render), result);
	HandleErrors(Initialise2DRenderingGroup(&m_rg_tile_military, "SDGridTileMilitary", "sd_grid.tile_military", render), result);
	HandleErrors(Initialise2DRenderingGroup(&m_rg_tile_border, "SDGridTileBorder", "sd_grid.tile_border", render), result);

	// SD tools and one-off icons
	HandleErrors(Initialise2DRenderingGroup(&m_rg_connectionchangetool, "SDConnectionChangeTool", "sd_connectionchangetool", render), result);

	// Set initial render states
	ResetSDGridRenderingConfiguration();

	// Return success
	return ErrorCodes::NoError;
}

Result UI_ShipDesigner::Initialise2DRenderingGroup(Image2DRenderGroup **group, std::string key, std::string itemkey, Render2DGroup *render)
{
	Result result;
	std::string texname, texfname;
	const char *filename;
	float z;

	// Derive property keys from this item key
	std::string texturekey = concat(itemkey)(".texture").str();
	std::string zkey = concat(itemkey)(".z").str();

	// Make sure we have a constant matching this texture key
	if (!render || !render->HaveConstant(texturekey) || !render->HaveConstant(zkey))
	{
		//filename = "";
		//z = 0.0f;
		return ErrorCodes::CannotLocateShipDesignerRenderGroupData;
	}
	else
	{
		// Construct the texture filename
		texname = render->GetConstant(texturekey);
		texfname = BuildStrFilename(D::IMAGE_DATA, texname);
		filename = texfname.c_str();	

		// Retrieve the component z order
		std::string zstring = render->GetConstant(zkey);
		const char *zcs = zstring.c_str();
		if (zcs) z = (float)atof(zcs); else z = 0.0f;
	}

	// Create a new render group
	(*group) = new Image2DRenderGroup();
	result = (*group)->Initialize(	Game::ScreenWidth, Game::ScreenHeight, filename, false );
	if (result != ErrorCodes::NoError) return result;
	
	// Set the rendering order
	(*group)->SetZOrder(z);

	// Add this render group to the 2D render group so that it is rendered each frame
	(*group)->SetCode(key);
	render->Components.Image2DGroup.AddItem(key, (*group));
	render->RegisterRenderableComponent((*group));

	// Return success
	return ErrorCodes::NoError;
}

// Sets rendering of all SD grid component groups to the default; called before selectively making changes based on a new view mode
void UI_ShipDesigner::ResetSDGridRenderingConfiguration(void)
{
	// The base grid rendering is always performed
	m_rg_inactivegrid->SetRenderActive(true);
	m_rg_activegrid->SetRenderActive(true);

	// We always enable rendering of grid previews & errors, since the instances are only created when required anyway
	m_rg_blueprint->SetRenderActive(true);
	m_rg_placementconflict->SetRenderActive(true);

	// The remaining render groups are all disabled by default
	m_rg_buildablegrid->SetRenderActive(false);
	m_rg_apoint_standard->SetRenderActive(false);
	m_rg_apoint_turret->SetRenderActive(false);
	m_rg_walkable_connect->SetRenderActive(false);
	m_rg_corridor->SetRenderActive(false);
	m_rg_tile_border->SetRenderActive(false);
	m_rg_tile_civilian->SetRenderActive(false);
	m_rg_tile_engineer->SetRenderActive(false);
	m_rg_tile_military->SetRenderActive(false);
	m_rg_connectionchangetool->SetRenderActive(false);
}

// Activates the render group configuration specific to a view mode
void UI_ShipDesigner::ActivateViewModeRenderConfiguration(UI_ShipDesigner::SDViewMode mode)
{
	// Reset all render groups to the default
	ResetSDGridRenderingConfiguration();

	// Now make any delta changes to the render groups based on the active view mode
	switch (mode)
	{
		case UI_ShipDesigner::SDViewMode::General:					/* General view */
			break;

		case UI_ShipDesigner::SDViewMode::Construction:				/* Construction view */
			m_rg_buildablegrid->SetRenderActive(true);				// We also want to show the builable tile grid
			m_rg_apoint_standard->SetRenderActive(true);			// All attachment points are shown in construction view
			m_rg_apoint_turret->SetRenderActive(true);				// All attachment points are shown in construction view
			break;

		case UI_ShipDesigner::SDViewMode::Corridor:
			m_rg_buildablegrid->SetRenderActive(true);
			m_rg_walkable_connect->SetRenderActive(true);
			m_rg_corridor->SetRenderActive(true);
			m_rg_tile_border->SetRenderActive(true);
			m_rg_tile_civilian->SetRenderActive(true);
			m_rg_tile_engineer->SetRenderActive(true);
			m_rg_tile_military->SetRenderActive(true);
			break;

		case UI_ShipDesigner::SDViewMode::Tile:
			m_rg_buildablegrid->SetRenderActive(true);
			m_rg_walkable_connect->SetRenderActive(true);
			m_rg_corridor->SetRenderActive(true);
			m_rg_tile_border->SetRenderActive(true);
			m_rg_tile_civilian->SetRenderActive(true);
			m_rg_tile_engineer->SetRenderActive(true);
			m_rg_tile_military->SetRenderActive(true);

		/* Add more view modes as required */
	}
}

// Initialises all remaining rendering constants at startup, including those precalculated from other constants
Result UI_ShipDesigner::InitialiseRenderingConstants(void)
{
	// Constants relating to element attach point rendering; all relate to a left-side attach point & are rotated accordingly
	m_apoint_size = INTVECTOR2((int)ceilf((float)m_gridsize / 4.0f), (int)ceilf((float)m_gridsize * 0.75f));
	m_apoint_pos_offset = INTVECTOR2(-(int)ceilf((float)m_apoint_size.x / 2.0f), (int)ceilf((float)(m_gridsize - m_apoint_size.y) / 2.0f));							
	m_apoint_pos_faroffset = ((m_gridsize - m_apoint_size.x) - m_apoint_pos_offset.x);

	// Return success
	return ErrorCodes::NoError;
}

// Changes the current view mode, modifying render groups as appropriate ready for the main rendering functions
void UI_ShipDesigner::ChangeViewMode(SDViewMode mode)
{
	// Record the new view mode
	m_viewmode = mode;

	// Adjust the active SD grid render groups to change the SD view
	ActivateViewModeRenderConfiguration(mode);

	// Loop through each component group in turn to change the control panel view
	for (int i=0; i<CONTROL_PANEL_TAB_COUNT; i++)
	{
		// Test each index to see if it corresponds to the view mode in question
		if (i == (int)mode)
		{
			// If this is the view mode being activated, then enable rendering of the corresponding render group
			UIComponentGroup *group = m_render->Components.ComponentGroups.GetItem(concat("panel_tab_controlgroup_")(i).str());
			if (group) group->EnableGroupRendering(false);
		}
		else
		{
			// If this is not the active view mode then disable rendering of the corresponding components
			UIComponentGroup *group = m_render->Components.ComponentGroups.GetItem(concat("panel_tab_controlgroup_")(i).str());
			if (group) group->DisableGroupRendering();
		}
	}

}

// Renders the ship designer view based on the section of ship being viewed
void UI_ShipDesigner::Render(void)
{
	// Make sure we have something to render
	if (!m_ship) return;

	// Perform any rendering that is common to all view modes
	PerformCommonViewRendering();

	// Now call a rendering function based on the current view
	switch (m_viewmode)
	{
		case UI_ShipDesigner::SDViewMode::General:
			RenderGeneralView();								break;
		case UI_ShipDesigner::SDViewMode::Construction:
			RenderConstructionView();							break;
		
		/* Add additional methods for each new view mode */
	}
}

// Performs rendering of the ship designer grid that is common to all views, e.g. the images of the ship sections 
void UI_ShipDesigner::PerformCommonViewRendering(void)
{
	// Render the mouse cursor
	RenderMouseCursor();
}

void UI_ShipDesigner::RenderGeneralView(void)
{

}

void UI_ShipDesigner::RenderConstructionView(void)
{
	// Render the ship section preview, either in its preview window or within the SD
	RenderShipSectionPreview();
}

// Render the mouse cursor and anything tied to it, for example anything we are dragging
void UI_ShipDesigner::RenderMouseCursor(void)
{

}

// Builds a collection of complex ship section preview images, for use in building & rendering ships under construction
void UI_ShipDesigner::BuildShipSectionImageMap(void)
{
	Result result;
	int numloaded = 0;

	// Get a device context reference for use in cloning the required texture resources
	auto devicecontext = Game::Engine->GetDeviceContext();
	if (!devicecontext) return;

	// Iterate through the set of all complex ship sections
	DataRegister<ComplexShipSection>::RegisterType::const_iterator it_end = D::ComplexShipSections.Data.end();
	for (DataRegister<ComplexShipSection>::RegisterType::const_iterator it = D::ComplexShipSections.Data.begin(); it != it_end; ++it)
	{
		// Make sure this instance is valid
		if (!it->second || !it->second->GetPreviewImage()) continue;

		// Get a reference to the image associated with this ship section
		TextureDX11 *sectiontexture = it->second->GetPreviewImage();
		if (!sectiontexture) continue;

		// We only need to create a new render group per preview image, not per section, since many sections may 
		// share the same image.  We can therefore skip this section if its preview image has already been loaded
		if (m_css_images.count(sectiontexture) > 0) continue;

		// Create a new image render group for this section, without loading a texture from file at this point
		Image2DRenderGroup *rg = new Image2DRenderGroup();
		result = rg->Initialize(Game::ScreenWidth, Game::ScreenHeight, NULL, false);
		if (result != ErrorCodes::NoError) { delete rg; continue; }

		// Assign the section preview image directly to this render group
		rg->SetTexture(sectiontexture);

		// Set the rendering order for ship section images
		rg->SetZOrder(z_shipsections);

		// Now add this new render group to the collection, indexed by ship section code
		rg->SetCode(concat("CSSPreviewImage_")(++numloaded).str());
		m_css_images[sectiontexture] = rg;

		// Also add this new render group to the queue for rendering
		m_render->Components.Image2DGroup.AddItem(rg->GetCode(), rg);
		m_render->RegisterRenderableComponent(rg);
	}
}

// Create a new blank ship
Result UI_ShipDesigner::CreateNewShip(void)
{
	// Default size for new ships is just 1x1x1, since we will dynamically reallocate as the blueprint grows
	INTVECTOR3 initialsize = INTVECTOR3(1, 1, 1);

	// If we already have an active ship then deallocate the data before creating a new one
	if (m_ship) ShutdownSDShip();

	// Create a new ship details object
	m_ship = ComplexShip::Create("null_environment");
	m_ship->InitialiseElements(initialsize);
	
	// Update the view to show this new (blank) ship
	ShipBlueprintModified();

	// Return success
	return ErrorCodes::NoError;
}

// Shuts down the ship currently under construction in the SD
void UI_ShipDesigner::ShutdownSDShip(void)
{
	if (m_ship)
	{
		m_ship->Shutdown();
		delete m_ship;
		m_ship = NULL;
	}
}

// Prepares a new ship for saving to the global collection, by updating e.g. unique codes to avoid conflict
Result UI_ShipDesigner::PrepareShipForOperation(ComplexShip *s, std::string code)
{
	// Set the unique code of the main ship to the code provided
	s->SetCode(code);

	// Also generate a unique code for each of the ship sections
	size_t ns = s->GetSectionCount();
	for (size_t i = 0; i < ns; ++i)
	{
		s->GetSection(i)->SetCode(concat(code)("_section_")(i).str());
	}

	// Re-evaluate the ship tile data to ensure that everything has been precalculated correctly before saving
	int nt = s->GetTileCount();
	for (int i=0; i<nt; ++i)
	{
		// Get a reference to the tile
		ComplexShipTile *tile = s->GetTile(i);
		if (!tile) continue;

		// Update the element containing this tile (in future, generalise to update of *any* tile type on the element, or just process all elements)
		UpdateTile(tile);
	}

	// Return success
	return ErrorCodes::NoError;
}

// Load a ship into the ship designer, creating a copy that can be worked upon
Result UI_ShipDesigner::LoadShip(std::string code)
{
	// Validate the ship code exists
	StrLowerC(code);
	if (code == NullString || D::ComplexShips.Exists(code) == false) return ErrorCodes::ShipDesignerCannotLoadInvalidShipCode;

	// If we already have a ship loaded in the designer then deallocate it first, to avoid memory leaks
	if (m_ship) ShutdownSDShip();
	
	// Load this ship and make a copy for editing in the ship designer
	m_ship = ComplexShip::Create(code);
	if (!m_ship) return ErrorCodes::ShipDesignerCouldNotLoadAndCopyShipDetails;

	// Apply the SD offset to this ship so it appears in the desired location in the SD
	ApplySDOffset(m_ship);

	// Update the ship designer rendering data and force a refresh
	ShipBlueprintModified();

	// Return success
	return ErrorCodes::NoError;
}

// Save this ship to the global collection, and also stream to file.  For now, we ignore things like 
// confirmation to overwrite existing ship details
Result UI_ShipDesigner::SaveShip(std::string code)
{
	Result result;

	// Validate the ship code
	if (code == NullString) return ErrorCodes::ShipDesignerCannotSaveInvalidShipCode;

	// Prepare the ship for saving to the global collection
	PrepareShipForOperation(m_ship, code);

	// Determine the SD offset based on blueprint bounds and store the value within the ship details for saving
	//m_ship->SetSDOffset(m_ship->GetShipMinimumBounds());

	// Make a copy of this ship to be saved
	ComplexShip *ship = ComplexShip::Create(m_ship);

	// Remove the SD offset from this ship before saving, so that all in-game ships correctly have (0,0,0) as their origi
	RemoveSDOffset(ship);

	// Set the temporary offset to be applied to ship size upon saving.  This is a temporary change only required for the SD, 
	// since the SD will expand the ship size to cover everything from 0,0 in the SD grid.  Removing the SD offset will adjust 
	// the location value of elements/tiles/sections, however element size cannot also be offset since that would prevent the 
	// ship saving method from iterating through the full range of elements
	//ship->FlagShipAsDirectlyGeneratedFromShipDesigner(true);

	// Stream the new ship data to file; for now, generate a temporary path based on the code
	std::string filename = concat(D::DATA)(ship->DetermineXMLDataPath()).str();
	if (CreateDirectory(filename.c_str(), NULL) == false && GetLastError() != ERROR_ALREADY_EXISTS)
		return ErrorCodes::ShipDesignerCannotCreateShipDataDirectory;

	// Generate xml files at this new location; first, for the ship itself
	TiXmlElement *shipxml = IO::Data::NewGameDataXMLNode();
	result = IO::Data::SaveComplexShip(shipxml, ship);
	if (result != ErrorCodes::NoError) return result;

	// Save this xml data to file
	result = IO::Data::SaveXMLDocument(shipxml, concat(D::DATA)(ship->DetermineXMLDataFullFilename()).str());
	if (result != ErrorCodes::NoError) return result;

	// Generate XML data files for each ship section
	int n = (int)ship->GetSections()->size();
	for (int i=0; i<n; i++)
	{
		// Get a reference to the ship section
		ComplexShipSection *sec = ship->GetSection(i);

		// If this is a standard ship section then we don't need to generate any XML for it; it will always be loaded
		// from the central collection.  (Non-standard sections are saved directly since they are changed from the original)
		if (sec->IsStandardObject()) continue;

		// Now generate xml for the ship section
		TiXmlElement *sectionxml = IO::Data::NewGameDataXMLNode();
		result = IO::Data::SaveComplexShipSection(sectionxml, sec);
		if (result != ErrorCodes::NoError) return result;

		// Make sure the target directory exists
		std::string sectionfilename = concat(D::DATA)(sec->DetermineXMLDataPath()).str();
		if (CreateDirectory(sectionfilename.c_str(), NULL) == false && GetLastError() != ERROR_ALREADY_EXISTS)
		return ErrorCodes::ShipDesignerCannotCreateShipDataDirectory;

		// Save this ship section data to file
		result = IO::Data::SaveXMLDocument(sectionxml, concat(D::DATA)(sec->DetermineXMLDataFullFilename()).str());
		if (result != ErrorCodes::NoError) return result;
	}

	/* The data has all been saved to disk.  Now, we want to directly load the xml file that was generated for 
	   the ship.  This will independently load each of the sections from disk as part of the process */

	// Determine the filename of the ship to be loaded
	std::string loadfile = ship->DetermineXMLDataFullFilename();

	// Make sure the ship doesn't already exist, and remove it if it does
	if (D::ComplexShips.Exists(code))
		if (D::ComplexShips.Get(code) != NULL)
			delete(D::ComplexShips.Get(code));

	// Also shut down the temporary ship copy now that we have saved it, and are about to load it properly from disk
	ship->Shutdown(); delete ship; ship = NULL;

	// Now load the ship from the file that was generated.  This will also separately load each non-standard section in turn
	result = IO::Data::LoadGameDataFile(loadfile);
	if (result != ErrorCodes::NoError) return result;

	// Finally update the complex ship register file to reflect the change in register data (i.e. the addition of one new ship)
	result = IO::Data::GenerateComplexShipRegisterXMLFile();
	if (result != ErrorCodes::NoError) return ErrorCodes::CouldNotUpdateComplexShipRegisterFile;

	/* We have now saved the ship & sections to disk, and also loaded them into the global collection, so return success */
	return ErrorCodes::NoError;
}

// Applies the SD offset parameter to a ship object, for correct viewing in the SD
void UI_ShipDesigner::ApplySDOffset(ComplexShip *ship)
{
	// Parameter check
	if (!ship) return;

	// By default we apply the SD offset stored in the ship details object
	//ApplySDOffsetExplicit(ship, ship->GetSDOffset());
}

// Removes the SD offset parameter from a ship object, for setting the origin to (0,0,0) before publishing to the global collection
void UI_ShipDesigner::RemoveSDOffset(ComplexShip *ship)
{
	// Parameter check
	if (!ship) return;

	// Simply apply a negation to remove the SD offset effect
	//ApplySDOffsetExplicit(ship, (ship->GetSDOffset() * -1));
}


// Applies the SD offset parameter to a ship object, for correct viewing in the SD
void UI_ShipDesigner::ApplySDOffsetExplicit(ComplexShip *ship, INTVECTOR3 offset)
{
	// Parameter check
	if (!ship) return;

	// First loop over the set of ship sections and offset their position
	ComplexShip::ComplexShipSectionCollection::iterator it_end = ship->GetSections()->end();
	for (ComplexShip::ComplexShipSectionCollection::iterator it = ship->GetSections()->begin(); it != it_end; ++it)
	{
		// Get a reference to this section
		ComplexShipSection *sec = (*it);
		if (!sec) continue;

		// Offset the section location and physical position by the specified amount
		sec->SetElementLocation(sec->GetElementLocation() + offset);
		sec->SetPosition(Game::ElementLocationToPhysicalPosition(sec->GetElementLocation()));
	}	

	// Next loop over the set of ship tiles and apply the same offset to their position
	iContainsComplexShipTiles::ConstTileIterator it2_end = ship->GetTiles().end();
	for (iContainsComplexShipTiles::ConstTileIterator it2 = ship->GetTiles().begin(); it2 != it2_end; ++it2)
	{
		// Get a reference to this tile
		ComplexShipTile *tile = (*it2).value;
		if (!tile) continue;

		// Offset the tile location by the specified amount
		tile->SetElementLocation(tile->GetElementLocation() + offset);
	}

	// Now loop over the entire ship element space and update the location of each by the offset value
	ComplexShipElement *elements = ship->GetElements();
	int n = ship->GetElementCount();
	for (int i = 0; i < n; ++i)
	{
		elements[i].SetLocation(elements[i].GetLocation() + offset);
	} 
}

void UI_ShipDesigner::ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse)
{
	// Store a reference to the parameters for use in other methods
	m_keyboard = keyboard;
	m_mouse = mouse;
	
	// Store the current mouse position for reference in other methods
	m_mousepos = m_mouse->GetCursor();

	// Process keyboard input
	m_keys = keyboard->GetKeys();
	if (m_keys[DIK_R] && m_mousestate == UI_ShipDesigner::SDMouseState::DraggingShipSection)
	{
		RotateShipSection();	// If we are trying to rotate the ship section that is currently being dragged
		keyboard->LockKey(DIK_R);
	}
	if (m_keys[DIK_PGDN])
	{
		ZoomGridIn();
		keyboard->LockKey(DIK_PGDN);
	}
	else if (m_keys[DIK_PGUP])
	{
		ZoomGridOut();
		keyboard->LockKey(DIK_PGUP);
	}
	else if (m_keys[DIK_C])
	{
		ProcessMouseMoveEvent(m_mouse->GetCursor());
	}

	// Certain tools are activated based upon a key state - update these here
	m_rg_connectionchangetool->SetRenderActive(m_keys[DIK_C] == TRUE);	

}

// Scroll the grid by the specified amount.  Note this currently only scrolls by full grid squares; 
// possible improvements will be to actually move grid squares by [0.0 - 1.0] and simulate smooth scrolling
void UI_ShipDesigner::ScrollGrid(float x, float y)
{
	// Adjust the position of the top-left corner of the SD grid
	m_gridpos.x = max(m_gridpos.x + x, 0.0f); 
	m_gridpos.y = max(m_gridpos.y + y, 0.0f);

	// Set the top-left element to the one closest to this position
	m_gridstart.x = (int)floor(m_gridpos.x);
	m_gridstart.y = (int)floor(m_gridpos.y);

	// Perform a full update of the SD grid
	PerformSDGridUpdate();
}

// Determines the incremental zoom in or out to be applied, based on the current zoom level and available zoom range
int UI_ShipDesigner::DetermineZoomLevelIncrement(void)
{
	// Determine the available zoom range, and our current position within it
	//int range = (m_gridsizemax - m_gridsizemin);
	//float pczoom = (float)(m_gridsize - m_gridsizemin) / (float)range;

	// Derive an increment based on the current zoom level
	int increment = (int)ceil((float)m_gridsize / 10.0f);

	// Clamp the result to some reasonable bounds
	increment = min(increment, 12);
	increment = max(increment, 2);

	// Return the increment to be applied
	return increment;
}

// Zooms the SD grid in, i.e. increasing the grid size
void UI_ShipDesigner::ZoomGridIn(void)
{
	// Increase the grid size by the appropriate increment for this zoom level
	int increment = DetermineZoomLevelIncrement();
	ZoomGridToLevel(m_gridsize + increment);
}

// Zooms the SD grid out, i.e. decreasing the grid size
void UI_ShipDesigner::ZoomGridOut(void)
{
	// Decrease the grid size by the appropriate increment for this zoom level
	int increment = DetermineZoomLevelIncrement();
	ZoomGridToLevel(m_gridsize - increment);
}

// Zoom the SD grid to a specific grid-size level
void UI_ShipDesigner::ZoomGridToLevel(int gridsizelevel)
{
	// Perform a bounds check to limit the grid zoom level to the available range
	gridsizelevel = min(max(gridsizelevel, m_gridsizemin), m_gridsizemax);

	// Update the SD grid size
	m_gridsize = gridsizelevel;

	// Now recalculate rendering constants for this new zoom level
	InitialiseRenderingConstants();

	// Update the number of cells current visible in the SD grid, and also ensure the linkages are all set correctly
	UpdateSDGridLayout();

	// Refresh the rendering of components on the SD grid to reflect the new zoom level
	ShipBlueprintModified();
	RecalculateShipSectionPreviewPositioning();
	RenderShipSectionPreview();
}


// Rotates the ship section currently being dragged
void UI_ShipDesigner::RotateShipSection(void)
{
	// Make sure we are in the right state
	if (m_mousestate != UI_ShipDesigner::SDMouseState::DraggingShipSection || !m_consview_selected || !m_consview_selected_image.instance) 
		return;

	// Rotate the ship section, which will remap and recalculate the whole section element space
	//m_consview_selected->RotateShipSection(Rotation90Degree::Rotate90);	// NOT REQUIRED?  IF NO LONGER CONTAINING ELEMENTS

	// Also rotate and resize the preview image in the ship designer
	m_consview_selected_image.instance->rotation = Compose90DegreeRotations(m_consview_selected_image.instance->rotation, Rotation90Degree::Rotate90);
	
	// Calculate the new dimensions of this ship section as a preview image and on the SD grid
	m_consview_selected_size = INTVECTOR2(m_consview_selected_size.y, m_consview_selected_size.x);
	m_consview_selected_gridsize = INTVECTOR2(	m_consview_selected_size.x * m_gridsize, 
												m_consview_selected_size.y * m_gridsize );

	// Also store the new rotation, which will apply to the physical instance of the ship section in space
	m_consview_selected_rotation = Compose90DegreeRotations(m_consview_selected_rotation, Rotation90Degree::Rotate90);

	// Alter the currently selected grid square to force a render refresh next cycle
	m_consview_selected_gridpos = INTVECTOR2(-1, -1);

	// Update the SD view to reflect this change
	RecalculateShipSectionPreviewPositioning();
	RenderShipSectionPreview();
}

// Event is triggered whenever a mouse click event occurs on a managed control, e.g. a button
void UI_ShipDesigner::ProcessControlClickEvent(iUIControl *control)
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

// Processes the events raised when a combobox selection is changed
void UI_ShipDesigner::ComboBox_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex)
{
	// Action will depend on the control in question
	if (control == m_shipsectionselector)
		ShipSectionSelector_SelectedIndexChanged(control, selectedindex, previousindex);
	else if (control == m_corridorselector)
		CorridorSelector_SelectedIndexChanged(control, selectedindex, previousindex);
	else if (control == m_tileselector)
		TileSelector_SelectedIndexChanged(control, selectedindex, previousindex);
}

// Event is triggered whenever a right mouse click event occurs on a managed control, e.g. a button
void UI_ShipDesigner::ProcessControlRightClickEvent(iUIControl *control)
{

}

// Process button click events in the UI
void UI_ShipDesigner::ProcessButtonClickEvent(UIButton *button)
{
	// Retrieve the code of the button being clicked
	std::string code = button->GetCode();

	// Pass control to the applicable method
	if (button->GetUpComponent()->rendergroup->GetCode() == "panel_tab_inactive") {		// If this is a control panel tab button
		ControlPanelTabClicked(button);
	}
	else if (code == "btn_newbp") {										// New blueprint button
		CreateNewShip();
	}
	else if (code == "btn_savebp") {									// Save blueprint button
		SaveShip(GetBlueprintName());
	}
	else if (code == "btn_loadbp") {									// Load blueprint button
		LoadShip(GetBlueprintName());			
	}

	// TODO: Process the return value and display a message somehow if the methods do not succeed, e.g. if the specific blueprint does not exist
}

// Returns the name of the blueprint we are currently working on, as specified in the blueprint name textbox
std::string UI_ShipDesigner::GetBlueprintName(void)
{
	UITextBox *tb = m_render->Components.TextBoxes.GetItem("txt_bpname");
	if (tb) 
		return tb->GetText();
	else
		return "";
}

// Method called when one of the control pabel tabs is clicked; changes the SD view accordingly
void UI_ShipDesigner::ControlPanelTabClicked(UIButton *button)
{
	// Take action depending on the tab that has been clicked; derive the index that we are trying to activate
	std::string code = button->GetCode();
	int index = (code.at(code.size()-1)) - ('0');
	if (index < 0 || index >= CONTROL_PANEL_TAB_COUNT) return;
	
	// Change the view mode to this new index
	ChangeViewMode((SDViewMode)index);

	// We now handle the UI appearance changes; get a reference to the 'active' tab render group and stop here if we don't have it
	Image2DRenderGroup *activetabs = m_render->Components.Image2DGroup.GetItem("panel_tab_active");
	if (!activetabs) return;

	// Now set the render status of each panel tab based on the one that has been selected
	for (int i=0; i<CONTROL_PANEL_TAB_COUNT; i++) 
	{
		// Look up the 'active' tab component for this index along the tab control
		Image2DRenderGroup::Instance *active = activetabs->GetInstanceByCode(concat("")(i).str());

		if (m_panel_tabs[i] == button) 
		{
			// This is the ACTIVE tab, so hide the tab button and replace with the active tab component
			button->SetRenderActive(false);
			if (active) active->render = true;
		}
		else if (m_panel_tabs[i])
		{
			// This is not the active tab (and we have confirmed it exists) so make sure it is inactive now
			m_panel_tabs[i]->SetRenderActive(true);
			if (active) active->render = false;
		}
	}
}

// Event raised whenever the ship section selector is used by the user & the index is changed
void UI_ShipDesigner::ShipSectionSelector_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex)
{
	// Make sure we have valid parameters
	if (!control) return;

	// Get the code (combobox tag) of the ship section from the combo box
	std::string code = control->GetSelectedItemTag();
	if (code == NullString) return;

	// Attempt to select the ship section with this unique string code
	ShipSectionSelectedInConstructionView(code);
}

// Called whenever the user selects a ship section in the construction view
void UI_ShipDesigner::ShipSectionSelectedInConstructionView(std::string code)
{
	// Validate that the parameter corresponds to a valid ship section
	if (code == NullString || D::ComplexShipSections.Exists(code) == false)
	{
		m_consview_selected_code = "";
		m_consview_selected = NULL;
		m_consview_selected_gridsize = NULL_INTVECTOR2;
		m_consview_selected_rotation = Rotation90Degree::Rotate0;
		return;
	}

	// If we have re-selected the same ship section then quit here since we don't need to change anything
	if (m_consview_selected && code == m_consview_selected->GetCode()) return;

	// Store the new ship section code 
	m_consview_selected_code = code;

	// Get and store a reference to this complex ship section in the central collection
	m_consview_selected = D::ComplexShipSections.Get(code);

	// Calculate the size of this ship section on the SD grid
	m_consview_selected_gridsize = INTVECTOR2(	m_consview_selected->GetElementSize().x * m_gridsize, 
												m_consview_selected->GetElementSize().y * m_gridsize );

	// Reset the rotation of the selected ship section back to normal
	m_consview_selected_rotation = Rotation90Degree::Rotate0;

	// Change the image in the preview window; if one already exists, remove and dispose of it first
	if (m_consview_selected_image.instance) {
		int index = m_consview_selected_image.rendergroup->GetInstanceReferenceByCode("sd_selected_section").index;
		if (index != -1) m_consview_selected_image.rendergroup->RemoveInstance(index);
	}

	// Remove any existing entry in the tab render group
	UIComponentGroup *rendercg = m_render->Components.ComponentGroups.GetItem("panel_tab_controlgroup_1");
	if (rendercg) {
		int cgindex = rendercg->FindItem("sd_selected_section");
		if (cgindex != -1) rendercg->RemoveItem(cgindex);
	}

	// The ship preview texture object is used as a key into the image instance collection
	const TextureDX11 *imagetex = m_consview_selected->GetPreviewImage();

	// Now create a new instance for the newly-selected ship section
	if (imagetex && m_css_images.count(imagetex) > 0)
	{
		// Recalculate the ship preview sizing & positioning
		RecalculateShipSectionPreviewPositioning();

		// Create an add a new instance for the new preview image
		Image2DRenderGroup::Instance *instance = 
			m_css_images[imagetex]->AddInstance(m_consview_selected_pos, 0.05f, 
												 m_consview_selected_size, true, Rotation90Degree::Rotate0);
		instance->SetCode("sd_selected_section");
	
		// Store the reference to this new instance
		m_consview_selected_image = m_css_images[imagetex]->GetInstanceReferenceByCode("sd_selected_section");

		// Also add a reference to the tab render group so it continues behaving in the UI as normal
		rendercg->AddItem(instance);
	}
}

void UI_ShipDesigner::RecalculateShipSectionPreviewPositioning(void)
{
	// Calculate the size that the preview image should have, based on its dimensions
	m_consview_selected_size = m_consview_selected->GetPreviewImage()->Get2DSize();
	if (m_consview_selected_size.x == 0 || m_consview_selected_size.y == 0) m_consview_selected_size = INTVECTOR2(1, 1);
	float scalefactor = min( ((float)m_consview_preview_size.x / (float)m_consview_selected_size.x), 
							 ((float)m_consview_preview_size.y)/ (float)m_consview_selected_size.y );
	if (scalefactor < 1.0f) { m_consview_selected_size *= scalefactor; }

	// Calculate the centred position of this image in the preview window
	m_consview_selected_pos = m_consview_preview_pos;
	m_consview_selected_pos.x += (int)floor((m_consview_preview_size.x - m_consview_selected_size.x) / 2.0f);
	m_consview_selected_pos.y += (int)floor((m_consview_preview_size.y - m_consview_selected_size.y) / 2.0f);
}

// Renders the ship section preview, depending on state the SD is currently in
void UI_ShipDesigner::RenderShipSectionPreview(void)
{
	// Make sure we have a preview image to be rendered
	if (!m_consview_selected_image.instance) return;

	// Render position depends on the current environment state
	if (m_mousestate == UI_ShipDesigner::SDMouseState::DraggingShipSection)
	{
		// We are currently dragging the ship section preview
		if (m_mousepos.x > m_gridx && m_mousepos.y > m_gridy &&
			m_mousepos.x < (m_gridr - m_consview_selected_gridsize.x + m_gridsize) && 
			m_mousepos.y < (m_gridb - m_consview_selected_gridsize.y + m_gridsize) )
		{
			// We have dragged the section to a valid position within the SD grid.  Show in full layout on the grid
			// First, identify the grid square that the mouse is currently over - this will become the top-left element of the section
			INTVECTOR2 pos = GetGridElementAtLocation(m_mousepos);
			if (pos != m_consview_selected_gridpos)
			{
				// Update the rendering of this ship section on the SD grid
				RenderShipSectionPreviewOnSDGrid(pos);
			}

			D::UI->WriteDebugOutput(3, concat("Grid square: ")(pos.x)(", ")(pos.y).str());
		}
		else
		{
			// We are dragging the ship section, but it is not within the valid bounds of the SD grid.  Show as an attached preview image
			m_consview_selected_image.instance->position = INTVECTOR2(m_mousepos.x - 8, m_mousepos.y - 8);
			m_consview_selected_image.instance->size = m_consview_selected_size;

			// Clear any placement images displayed on the SD grid
			m_rg_placementconflict->ClearAllInstances();
		}
	}
	else
	{
		// We are not dragging the preview image, so make sure it is sitting within the preview window where it should be
		m_consview_selected_image.instance->position = m_consview_selected_pos;
		m_consview_selected_image.instance->size = m_consview_selected_size;
	}
}

// Event raised whenever the corridor selector is used by the user & the index is changed
void UI_ShipDesigner::CorridorSelector_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex)
{
	// Make sure we have valid parameters
	if (!control) return;

	// Get the tag of the selected item
	std::string tag = control->GetSelectedItemTag();
	if (tag == NullString) return;

	// Determine the state of the corridor view based on this value
	CorridorViewMode mode = TranslateCorridorViewModeFromString(tag);
	if (mode == CorridorViewMode::Unknown) return;

	// Change the state of the corridor view depending on the selected item
	SetCorridorViewMode(mode);	
}

// Changes the corridor view mode based on the supplied parameter
void UI_ShipDesigner::SetCorridorViewMode(CorridorViewMode mode)
{
	// Store the new view mode
	if (mode == CorridorViewMode::Unknown) return;
	m_corridormode = mode;

	// Take different action depending on the view mode
	if (mode == CorridorViewMode::SingleCorridor)
	{
		// Single corridor only affects a 1x1 area on the SD grid
		m_corridorselectsize = INTVECTOR2(1, 1);
	}
}


// Event raised whenever the tile selector is used by the user & the index is changed
void UI_ShipDesigner::TileSelector_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex)
{
	// Attempt to retrieve a pointer to the tile definition that was selected
	std::string code = control->GetSelectedItemTag(); StrLowerC(code);
	m_selectedtiletype = D::ComplexShipTiles.Get(code);
}


// Method which is invoked whenever the ship blueprint is modified.  Updates the underlying data and render groups
void UI_ShipDesigner::ShipBlueprintModified(void)
{
	// Refresh the SD ship section mapping and recreate all render instances
	RefreshSDShipSectionInstances();

	// Force an update of the SD grid rendering
	PerformSDGridUpdate();
}

// Performs a full update of the SD grid.  Main update method to be called when non-trivial changes are made
void UI_ShipDesigner::PerformSDGridUpdate(void)
{
	// Update all elements in the SD grid
	UpdateSDGridElementRendering();
	
	// Also update all components being rendered on the SD grid
	UpdateSDGridRendering();
}

// Updates the elements of the SD grid to reflect the blueprint loaded within it
void UI_ShipDesigner::UpdateSDGridElementRendering(void)
{
	int index;
	ComplexShipElement *el;
	Image2DRenderGroup::Instance *iInactive, *iActive, *iBuildable;

	// Process each cell in turn
	for (int x=0; x<m_numgx; x++) {
		for (int y=0; y<m_numgy; y++)
		{
			// Get a reference to the relevant instances
			index = m_grid[x][y].GridRenderItemInstance;
			iInactive = m_rg_inactivegrid->GetInstance(index);
			iActive = m_rg_activegrid->GetInstance(index);
			iBuildable = m_rg_buildablegrid->GetInstance(index);

			// Get the element at this grid location, taking into account any scrolling of the grid
			el = m_ship->GetElement(x + m_gridstart.x, y + m_gridstart.y, m_gridzpos);

			// If this element is inactive, or doesn't exist because it's outside the ship bounds, then show as inactive and quit straight away
			if (!el || !el->IsActive()) 
			{
				iInactive->render = true;
				iActive->render = iBuildable->render = false;
			}
			else
			{
				// Render as active
				iInactive->render = false;
				iActive->render = true;
				
				// Is this cell also buildable?
				iBuildable->render = (el->IsBuildable());
			}
		}
	}
}

// Update all rendering components in the SD grid following an update to its state
void UI_ShipDesigner::UpdateSDGridRendering(void)
{
	ComplexShipElement *el;
	INTVECTOR2 startpos, endpos, gridpos, index;

	// Clear any previous rendering data before we begin recreating it
	m_rg_apoint_standard->ClearAllInstances();
	m_rg_apoint_turret->ClearAllInstances();
	m_rg_corridor->ClearAllInstances();
	m_rg_walkable_connect->ClearAllInstances();
	m_rg_tile_border->ClearAllInstances();
	m_rg_tile_civilian->ClearAllInstances();
	m_rg_tile_engineer->ClearAllInstances();
	m_rg_tile_military->ClearAllInstances();
	m_rg_connectionchangetool->ClearAllInstances();

	// Retrieve key data up front before processing the ship sections
	ComplexShip::ComplexShipSectionCollection *sections = m_ship->GetSections();
	ComplexShip::ComplexShipSectionCollection::size_type numsections = sections->size();
	INTVECTOR2 gridend = INTVECTOR2(m_gridstart.x + m_numgx, m_gridstart.y + m_numgy);

	// Process each ship section in turn
	for (ComplexShip::ComplexShipSectionCollection::size_type i = 0; i < numsections; ++i)
	{
		// Retrieve data on this ship section
		ComplexShipSection *section = sections->at(i);
		INTVECTOR3 sectionpos = section->GetElementLocation();

		// Calculate the range of elements (if any) that are visible in the section
		startpos = INTVECTOR2(max(m_gridstart.x, sectionpos.x), max(m_gridstart.y, sectionpos.y));
		endpos = INTVECTOR2(min(gridend.x, sectionpos.x + section->GetElementSize().x), 
							min(gridend.y, sectionpos.y + section->GetElementSize().y));

		// Determine whether the section is actually visible; if the endpos < startpos in either x or y direction
		// then no part of the section is visible and we can skip the remainder of rendering for the section
		if (endpos.x < startpos.x || endpos.y < startpos.y) 
		{
			// If there is a section image available then stop it being rendered
			if (m_sdshipsections[i].RenderInstance.instance) 
				m_sdshipsections[i].RenderInstance.instance->render = false;

			// Continue to the next section
			continue;
		}

		// Set the position of this ship section's render image to correspond to its relative location on the SD grid
		// The section is visible so long as its endpos >= startpos, i.e. it is possible to render some of the elements
		INTVECTOR2 screenstartpos = GetGridElementLocation(startpos);
		if (m_sdshipsections[i].RenderInstance.instance) {
			m_sdshipsections[i].RenderInstance.instance->position = screenstartpos;
			m_sdshipsections[i].RenderInstance.instance->render = true;
		}

		// Now loop across this range of section elements and perform per-element rendering
		for (int x = startpos.x; x < endpos.x; x++) {
			for (int y = startpos.y; y < endpos.y; y++)
			{
				// Translate ship coords [x,y] into grid coords [gridpos] and section element indices [index]
				gridpos = INTVECTOR2(x - m_gridstart.x, y - m_gridstart.y);
				index = INTVECTOR2(x - sectionpos.x, y - sectionpos.y);

				// Try to retrieve the element at this index
				el = NULL; // section->GetElement(index.x, index.y, m_gridzpos);	// *********************************************************************************
																					// *********************************************************************************
																					// *********** TODO: THIS WAS REMOVED, S.D. LIKELY NEEDS TO BE FIXED ***************
																					// *********************************************************************************
																					// *********************************************************************************
				if (!el) continue;
	
				// Calculate the position of this element in screen coordinates
				INTVECTOR2 screenpos = GetGridElementLocation(gridpos);	

				// Now render anything relating to this element; first, render any attachment points
				RenderElementAttachPoints(el, x, y, screenpos);
			}
		}
	}

	// Now render all tiles onto the SD grid
	RenderShipTilesToSDGrid();
}

// Renders all ship tile to the SD grid
void UI_ShipDesigner::RenderShipTilesToSDGrid(void)
{
	// Determine the start & end bounds within which elements should be rendered to the SD grid
	INTVECTOR3 gridstart = INTVECTOR3(m_gridstart.x, m_gridstart.y, m_gridzpos);
	INTVECTOR3 gridend = INTVECTOR3(gridstart.x + m_numgx, gridstart.y + m_numgy, gridstart.z);

	// Iterate over the ship tile collection and process each in turn
	iContainsComplexShipTiles::ConstTileIterator it_end = m_ship->GetTiles().end();
	for (iContainsComplexShipTiles::ConstTileIterator it = m_ship->GetTiles().begin(); it != it_end; ++it)
	{
		// Make sure the tile is valid
		ComplexShipTile *tile = (*it).value;
		if (!tile) continue;

		// Retrieve position & size of this tile
		INTVECTOR3 pos = tile->GetElementLocation();
		INTVECTOR3 size = tile->GetElementSize();
		
		// We only need to consider this tile for rendering if some part of it is within the rendering bounds
		if ( (pos.x + size.x - 1 < gridstart.x) || (pos.x > gridend.x) ||
			 (pos.y + size.y - 1 < gridstart.y) || (pos.y > gridend.y) || 
			 (pos.z + size.z - 1 < gridstart.z) || (pos.z > gridend.z) ) continue;

		// Perform base tile rendering, i.e. of any data in the base class that is common to all tile classes
		RenderBaseTileData(tile, pos, size, gridstart, gridend);

		// Now perform more specialised rendering dependent on the tile class
		D::TileClass tclass = tile->GetClass();
		switch (tclass)
		{
			case D::TileClass::Corridor:
				RenderCorridorTile((CSCorridorTile*)tile, pos);
				break;
		}
	}
}

// Renders all tile data (stored within the base class & common to all tiles) for the specified tile.  Still makes some
// limited changes based on the subclass, so will use the virtual GetClass method to determine that type
void UI_ShipDesigner::RenderBaseTileData(ComplexShipTile *tile, INTVECTOR3 tilepos, INTVECTOR3 tilesize, INTVECTOR3 gridstart, INTVECTOR3 gridend)
{
	// Parameter check, and also retrieve type of tile subclass for reference
	if (!tile) return;
	D::TileClass tileclass = tile->GetClass();

	// Get a reference to the render group for this tile type, based on the class
	Image2DRenderGroup *tilerendergroup = GetTileRenderGroup(tileclass);
	if (!tilerendergroup || !m_rg_tile_border) return;

	/* Render the tile interior & borders */

	// Determine the starting grid square for rendering this tile
	INTVECTOR2 start = INTVECTOR2(tilepos.x - m_gridstart.x, tilepos.y - m_gridstart.y);
	if (start.x < 0 || start.y < 0) return;
	INTVECTOR2 end = INTVECTOR2(min(start.x + tilesize.x - 1, m_numgx - 1), 
								min(start.y + tilesize.y - 1, m_numgy - 1) );
	
	// Now add an instance to render each element of the tile in turn
	INTVECTOR2 pos;
	float zinterior = tilerendergroup->GetZOrder();
	float zborder = m_rg_tile_border->GetZOrder();
	for (int x = start.x; x <= end.x; x++)
	{
		for (int y = start.y; y <= end.y; y++)
		{
			// Add the tile interior
			pos = GetGridElementLocation(INTVECTOR2(x, y));
			tilerendergroup->AddInstance(pos, zinterior, m_gridsize, true, Rotation90Degree::Rotate0);

			// Add borders depending on whether this is an edge element in the tile
			if (false)
			{
				if (x == start.x)	m_rg_tile_border->AddInstance(pos, zborder, m_gridsize, true, Rotation90Degree::Rotate0);
				if (y == start.y)	m_rg_tile_border->AddInstance(pos, zborder, m_gridsize, true, Rotation90Degree::Rotate90);
				if (x == end.x)		m_rg_tile_border->AddInstance(pos, zborder, m_gridsize, true, Rotation90Degree::Rotate180);
				if (y == end.y)		m_rg_tile_border->AddInstance(pos, zborder, m_gridsize, true, Rotation90Degree::Rotate270);
			}
		}
	}
	

	/* Now render connection points to this tile */

	// Determine the rendering size for a (left) connection point
	int corridoroffset = (int)ceil(CORRIDOR_TILE_OFFSET * m_gridsize);
	int corridorsize = (int)ceil(CORRIDOR_TILE_WIDTH * m_gridsize);
	
	// Special case: if this is a corridor tile then we don't render any connectors, since the corridor subclass will handle this itself
	if (tileclass != D::TileClass::Corridor)
	{
		// Render each tile connection point in turn
/*		ElementConnectionSet::iterator it_end = tile->GetConnectionIteratorEnd();
		for (ElementConnectionSet::iterator it = tile->GetConnectionIteratorStart(); it != it_end; ++it)
		{
			// Get the absolute position of this connection in the ship element space.  Only render if it is within the bounds
			INTVECTOR3 connpos = tilepos + (*it).Location;
			if (connpos.x < gridstart.x || connpos.x > gridend.x || connpos.y < gridstart.y || connpos.y > gridend.y ||
				connpos.z > gridstart.z || connpos.z > gridend.z) continue;

			// We want to render this connection point, so get the grid square and then screen position where it should be rendered
			INTVECTOR2 gridsquare = INTVECTOR2(connpos.x - m_gridstart.x, connpos.y - m_gridstart.y);
			INTVECTOR2 screenpos = GetGridElementLocation(gridsquare);

			// Add an instance depending on the direction of connection
			switch ((*it).Connection)
			{
				case Direction::Left:
					m_rg_walkable_connect->AddInstance(INTVECTOR2(screenpos.x - corridoroffset, screenpos.y + corridoroffset), m_rg_walkable_connect->GetZOrder(),
												INTVECTOR2(corridoroffset * 2, corridorsize), true, Rotation90Degree::Rotate0);
					break;
				case Direction::Right:
					m_rg_walkable_connect->AddInstance(INTVECTOR2(screenpos.x + m_gridsize - corridoroffset, screenpos.y + corridoroffset),
					   							m_rg_walkable_connect->GetZOrder(), INTVECTOR2(corridoroffset * 2, corridorsize), true, Rotation90Degree::Rotate0);
					break;
				case Direction::Up:
					m_rg_walkable_connect->AddInstance(INTVECTOR2(screenpos.x + corridoroffset, screenpos.y - corridoroffset), m_rg_walkable_connect->GetZOrder(),
												INTVECTOR2(corridorsize, corridoroffset * 2), true, Rotation90Degree::Rotate0);
					break;
				case Direction::Down:
					m_rg_walkable_connect->AddInstance(INTVECTOR2(screenpos.x + corridoroffset, screenpos.y + m_gridsize - corridoroffset),
												m_rg_walkable_connect->GetZOrder(), INTVECTOR2(corridorsize, corridoroffset * 2), true, Rotation90Degree::Rotate0);
					break;
				default:
					break;		// Add no render instance unless the connection is in one of the four 2D-plane directions
			}
		}*/
	}
}

// Renders a corridor tile in the SD grid
void UI_ShipDesigner::RenderCorridorTile(CSCorridorTile *tile, INTVECTOR3 tilepos)
{
	// Get the position of this tile in grid squares and then screen coordinates
	INTVECTOR2 gridpos = INTVECTOR2(tilepos.x - m_gridstart.x, tilepos.y - m_gridstart.y);
	INTVECTOR2 screenpos = GetGridElementLocation(gridpos);

	// Calculate the position & size of the 'centre' of the corridor, then add an instance to render it
	int centreoffset = (int)ceil(CORRIDOR_TILE_OFFSET * m_gridsize);
	int centresize = (int)ceil(CORRIDOR_TILE_WIDTH * m_gridsize);
	m_rg_corridor->AddInstance(INTVECTOR2(screenpos.x + centreoffset, screenpos.y + centreoffset), 
							   m_rg_corridor->GetZOrder(), INTVECTOR2(centresize), true, Rotation90Degree::Rotate0);

	// Attempt to link back to the parent element
	INTVECTOR3 loc = tile->GetElementLocation();
	ComplexShipElement *el = m_ship->GetElement(loc);
	if (!el) return;
	
	// Get the surrounding environment of the tile to see where we need to render connectors
	bool left = false, up = false, right = false, down = false;
	AnalyseCorridorEnvironment(el, &left, &up, &right, &down);

	// Determine the rendering size for a (left) connection point
	int corridoroffset = (int)ceil(CORRIDOR_TILE_OFFSET * m_gridsize);
	int corridorsize = (int)ceil(CORRIDOR_TILE_WIDTH * m_gridsize);

	// Now render connectors wherever we are connected
	if (left) m_rg_corridor->AddInstance(INTVECTOR2(screenpos.x, screenpos.y + corridoroffset), m_rg_corridor->GetZOrder(),
				   					     INTVECTOR2(corridoroffset, corridorsize), true, Rotation90Degree::Rotate0);
		
	if (right) m_rg_corridor->AddInstance(INTVECTOR2(screenpos.x + m_gridsize - corridoroffset, screenpos.y + corridoroffset),
		   								  m_rg_corridor->GetZOrder(), INTVECTOR2(corridoroffset, corridorsize), true, Rotation90Degree::Rotate0);
		
	if (up) m_rg_corridor->AddInstance(INTVECTOR2(screenpos.x + corridoroffset, screenpos.y), m_rg_corridor->GetZOrder(),
									   INTVECTOR2(corridorsize, corridoroffset), true, Rotation90Degree::Rotate0);
		
	if (down) m_rg_corridor->AddInstance(INTVECTOR2(screenpos.x + corridoroffset, screenpos.y + m_gridsize - corridoroffset),
										 m_rg_corridor->GetZOrder(), INTVECTOR2(corridorsize, corridoroffset), true, Rotation90Degree::Rotate0);				
}

// Recreates the mapping from ship sections to their render instances
void UI_ShipDesigner::RefreshSDShipSectionInstances(void)
{
	Image2DRenderGroup *rg;
	Image2DRenderGroup::Instance *instance;

	// Loop through each section in turn and remove existing render instances
	int nsds = (int)m_sdshipsections.size();
	for (int i=0; i<nsds; i++)
		if (m_sdshipsections.at(i).RenderInstance.instance)
			m_sdshipsections.at(i).RenderInstance.rendergroup->RemoveInstance(
			m_sdshipsections.at(i).RenderInstance.instance);

	// Clear the instance mapping now that it contains no useful data
	m_sdshipsections.clear();

	// Now recreate a new mapping for each ship section
	ComplexShip::ComplexShipSectionCollection::size_type n = m_ship->GetSectionCount();
	for (ComplexShip::ComplexShipSectionCollection::size_type i = 0; i < n; ++i)
	{
		// Default values before starting
		rg = NULL; instance = NULL;

		// Retrieve details on this section
		ComplexShipSection *sec = m_ship->GetSection(i);

		// Attempt to locate the render group holding images for this ship section
		if (sec->GetPreviewImage() && m_css_images.count(sec->GetPreviewImage()) > 0)
			rg = m_css_images[sec->GetPreviewImage()];

		// Generate a key for this section instance
		std::string key = concat("sd_section_")(i).str();

		// Add a new instance to the render group and store a mapping to it.  Position is not important
		// at this point since it will be set during the next update method
		if (rg) { 
			instance = rg->AddInstance(	INTVECTOR2(0, 0), z_shipsections, 
				INTVECTOR2(sec->GetElementSize().x * m_gridsize, sec->GetElementSize().y * m_gridsize),
				true, Rotation90Degree::Rotate0);
			if (instance) instance->code = key;
		}

		// Create a new section mapping, link it up and add it; if we couldn't create an image instance the R.I. will simply be set to NULL
		UI_ShipDesigner::SDShipSection map;
		map.Section = sec;
		map.RenderInstance = (rg ? rg->GetInstanceReferenceByCode(key) : Image2DRenderGroup::InstanceReference());
		m_sdshipsections.push_back(map);
	}
}

// Renders the section attach points for a complex ship element on the SD grid
void UI_ShipDesigner::RenderElementAttachPoints(ComplexShipElement *el, int shipxpos, int shipypos, INTVECTOR2 gridpos)
{
	// Static constant data used during rendering of attach points
	static Image2DRenderGroup *rendergroups[2] = { m_rg_apoint_standard, m_rg_apoint_turret };
	static const Direction directions[4] = { Direction::Left, Direction::Up, Direction::Right, Direction::Down };
	static const DirectionBS bsdir[4] = { DirectionBS::Left_BS, DirectionBS::Up_BS, DirectionBS::Right_BS, DirectionBS::Down_BS }; 
	static const INTVECTOR2 pos_offset[4] =
		{ INTVECTOR2(m_apoint_pos_offset.x, m_apoint_pos_offset.y), INTVECTOR2(m_apoint_pos_offset.y, m_apoint_pos_offset.x), 
		  INTVECTOR2(m_apoint_pos_faroffset, m_apoint_pos_offset.y), INTVECTOR2(m_apoint_pos_offset.y, m_apoint_pos_faroffset) };
	static const INTVECTOR2 rendersize[4] = { m_apoint_size, INTVECTOR2(m_apoint_size.y, m_apoint_size.x),
											  m_apoint_size, INTVECTOR2(m_apoint_size.y, m_apoint_size.x) };

	// Make sure we have an element to be rendered
	if (!el) return;

	// Process each attach type in turn
	for (int i = 0; i < (int)ComplexShipElement::AttachType::_AttachTypeCount; ++i)
	{
		// If the element has no attach points of this type then skip all other checks immediately
		if (!el->HasAnyAttachPoints((ComplexShipElement::AttachType)i)) continue;

		// Check each relevant direction and render an attach point wherever necessary
		for (int d = 0; d < 4; ++d)
		{
			// Check whether an attach point exists in this direction
			if (el->HasAttachPoints((ComplexShipElement::AttachType)i, bsdir[d]))
			{
				// Attach points are not relevant at the outer edge of the environment
				int neighbour = el->GetNeighbour(directions[d]);
				if (neighbour != ComplexShipElement::NO_ELEMENT)
				{
					// Only render an attach point if there is not already an element attached to it
					if (!m_ship->GetElements()[neighbour].IsActive())
					{
						// Add an instance for rendering
						rendergroups[i]->AddInstance(INTVECTOR2(shipxpos + pos_offset[d].x, shipypos + pos_offset[d].y), 0.0f,
													 rendersize[d], true, Rotation90Degree::Rotate0);
					}
				}
			}
		}
	}
}

// Evaluates the placement of a ship section for conflicts, and in general whether it can be attached to the ship in this place
UI_ShipDesigner::SectionPlacementResult *UI_ShipDesigner::EvaluateSectionPlacement(	ComplexShip *ship, 
																					ComplexShipSection *section, 
																					INTVECTOR2 gridpos, int gridzpos)
{	
	ComplexShipElement *eship;
	INTVECTOR3 shippos;
	INTVECTOR2 gridelement;
	int z;

	// Create a new placement result object to hold the result of this evauation
	UI_ShipDesigner::SectionPlacementResult *result = new UI_ShipDesigner::SectionPlacementResult();
	if (!ship) return result;

	// Calculate the element bounds for this ship section
	INTVECTOR3 sectionsize = section->GetElementSize();

	// TODO: temporary?  Simply store the z position for now, on the assumption that we only process this z-level
	z = gridzpos;		

	// Loop over the elements covered by this ship section
	int numconflicts = 0;
	for (int x=0; x<sectionsize.x; x++)
	{
		for (int y=0; y<sectionsize.y; y++)
		{
			// Translate local section coordinates into global ship blueprint coordinates
			shippos = INTVECTOR3(x + gridpos.x + m_gridstart.x, y + gridpos.y + m_gridstart.y, gridzpos + m_gridzpos);
			gridelement = INTVECTOR2(gridpos.x + x, gridpos.y + y);

			// Get the element at this position in the ship section, and also in the ship itself
			//esec = section->GetElement(x, y, z);
			eship = ship->GetElement(shippos.x, shippos.y, shippos.z);

			// For safety, although the section elements within its bounds should never be null
			//if (!esec) continue;

			// If the element is already flagged as active in the ship blueprint then it conflicts with this new ship section
			if (eship && eship->IsActive())
			{
				// Add a new issue.  Also continue through to the next loop cycle; this is a "hard" error with the element
				result->Issues.push_back(SectionPlacementIssue(	UI_ShipDesigner::SectionPlacementIssueType::ConflictWithExistingSection,
																shippos, gridelement));
				continue;
			}

			// Test whether this is an edge element, and if so whether it meets any attachment criteria on either the ship or section edge
			if (x == 0) 
				EvaluateSectionEdgePlacement(/*esec*/NULL, Direction::Left, 
											 ship->GetElement(shippos.x-1, shippos.y, shippos.z), result, shippos, gridelement);
			if (x == sectionsize.x-1) 
				EvaluateSectionEdgePlacement(/*esec*/NULL, Direction::Right,
											 ship->GetElement(shippos.x+1, shippos.y, shippos.z), result, shippos, gridelement);						
			if (y == 0) 
				EvaluateSectionEdgePlacement(/*esec*/NULL, Direction::Up,
											 ship->GetElement(shippos.x, shippos.y-1, shippos.z), result, shippos, gridelement);
			if (y == sectionsize.y-1) 
				EvaluateSectionEdgePlacement(/*esec*/NULL, Direction::Down,
											 ship->GetElement(shippos.x, shippos.y+1, shippos.z), result, shippos, gridelement);

		}
	}

	// Update the placement result object with final precalculated data and return it
	result->IssueCount = result->Issues.size();
	return result;
}

// Evaluates the edge conditions around a ship section placement
void UI_ShipDesigner::EvaluateSectionEdgePlacement(ComplexShipElement *sectionelement, Direction sectionedge,
												   ComplexShipElement *neighbourelement, SectionPlacementResult *pPlacementResult,
												   INTVECTOR3 shippos, INTVECTOR2 gridpos)
{
	// If there is no neighbouring element then we automatically pass any conflict tests
	if (!neighbourelement) return;

	// Test whether the attachment criteria on each element are compatible
	if (!ComplexShipElement::AttachmentIsCompatible(sectionelement, DirectionToBS(sectionedge), neighbourelement))
	{
		// Raise a new placement issue if this attachment is not permitted
		pPlacementResult->Issues.push_back(SectionPlacementIssue(SectionPlacementIssueType::IncompatibleEdgeAttachment,
																 shippos, gridpos));																	 
	}
}

// Renders the ship section preview on the SD grid, along with any issues identified while the placement is being evaluated
void UI_ShipDesigner::RenderShipSectionPreviewOnSDGrid(INTVECTOR2 gridsquare)
{
	// First, position and resize the section to cover these grid squares
	m_consview_selected_gridpos = gridsquare;
	m_consview_selected_image.instance->position = GetGridElementLocation(m_consview_selected_gridpos);
	m_consview_selected_image.instance->size = m_consview_selected_gridsize;

	// Evaluate the placement of this section and store the result; delete any previous result first
	if (m_consview_section_placement_result) { delete m_consview_section_placement_result; m_consview_section_placement_result = NULL; }
	m_consview_section_placement_result = EvaluateSectionPlacement();

	// Clear all placement conflict image groups before processing the placement result
	m_rg_placementconflict->ClearAllInstances();

	// If we have no issues then show the placement as being possible
	if (m_consview_section_placement_result->IssueCount == 0) 
	{
		// Show that this ship placement is acceptable
	}
	else 
	{
		// Otherwise visually show each placement conflict that we identified
		for (std::vector<SectionPlacementIssue>::size_type i = 0; i < m_consview_section_placement_result->IssueCount; ++i)
		{
			UI_ShipDesigner::SectionPlacementIssueType type = m_consview_section_placement_result->Issues[i].Type;
			INTVECTOR2 gridloc = m_consview_section_placement_result->Issues[i].GridLocation;

			// Process based on issue type
			if (type == UI_ShipDesigner::SectionPlacementIssueType::ConflictWithExistingSection)
			{
				// Conflict with existing ship element; show a red placement conflict on this grid element
				m_rg_placementconflict->AddInstance(GetGridElementLocation(gridloc), 0.0f, 
													INTVECTOR2(m_gridsize, m_gridsize), true, Rotation90Degree::Rotate0);
			}
			else if (type == UI_ShipDesigner::SectionPlacementIssueType::IncompatibleEdgeAttachment)
			{
				// Conflict with attachment points on neighbouring element; show a red placement conflict at the relevant attachment point

			}
		}
	}

}

void UI_ShipDesigner::PlaceShipSection(void)
{
	// Make sure that we currently have the mouse in a valid position on the SD grid
	if (! (m_mousepos.x > m_gridx && m_mousepos.y > m_gridy &&
		   m_mousepos.x < (m_gridr - m_consview_selected_gridsize.x + m_gridsize) && 
		   m_mousepos.y < (m_gridb - m_consview_selected_gridsize.y + m_gridsize)) ) return;
	
	// Perform a final evaluation of this placement to confirm it is valid; any errors, quit now
	UI_ShipDesigner::SectionPlacementResult *result = EvaluateSectionPlacement();
	if (!result || result->IssueCount > 0) return;

	// The placement is valid, so create a new ship section now.  Set the details to the currently selected item (which will create a copy)
	ComplexShipSection *section = ComplexShipSection::Create(m_consview_selected);

	// Determine actual element location of this ship section
	INTVECTOR3 location = INTVECTOR3(	m_gridstart.x + m_consview_selected_gridpos.x, 
										m_gridstart.y + m_consview_selected_gridpos.y, m_gridzpos);

	// Set properties of the new ship section.  
	section->SetElementLocation(location);

	// We will set the physical position based on a constant scaling factor, at least for now
	section->SetPosition(Game::ElementLocationToPhysicalPosition(location));

	// Set the ship section rotation.  This impacts the physical orientation, but elements are rotation-independent and calculated by the SD
	section->RotateSection(m_consview_selected_rotation);

	// Before attaching to the ship, we need to expand the ship element space if necessary to fully include the new section
	INTVECTOR3 extent = (location + section->GetElementSize());
	m_ship->EnsureShipElementSpaceIncorporatesLocation(extent);

	// Add this section to the ship
	m_ship->AddShipSection(section);

	// Reset the SD state back to normal, by simulating another selection of the same ship section
	std::string scode = m_consview_selected_code; m_consview_selected_code = "";
	ShipSectionSelectedInConstructionView(scode);

	// Perform a full refresh since the ship blueprint has been modified
	ShipBlueprintModified();
}

void UI_ShipDesigner::ProcessMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component)
{
	
}

void UI_ShipDesigner::ProcessMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component)
{
	// Mouse events, beginning with the SD in default mouse state
	if (m_mousestate == UI_ShipDesigner::SDMouseState::Default)
	{
		// Process events depending on the current SD view mode 
		if (m_viewmode == SDViewMode::Construction) 
		{
			// If we have the mouse within the ship section preview control, begin a drag of the section onto the SD grid
			if (PointWithinBounds(location, m_consview_selected_pos, m_consview_selected_size))
			{
				m_mousestate = UI_ShipDesigner::SDMouseState::DraggingShipSection;
			}
		}
		else if (m_viewmode == SDViewMode::Corridor)
		{
			// Also take different action depending on the mode currently selected within corridor view
			if (m_corridormode == CorridorViewMode::SingleCorridor)
			{
				// If we have the mouse within the SD grid bounds, and we are not using a tool, then we are beginning to drag out a corridor section
				if (PointWithinBounds(location, m_gridscreenlocation, m_gridscreensize) && !m_keys[DIK_C])
				{
					m_mousestate = UI_ShipDesigner::SDMouseState::DraggingOutCorridorSection;
					m_griddragstart = GetGridElementAtLocation(location);
				}
			}
		}
		else if (m_viewmode == SDViewMode::Tile)
		{
			// If we have the mouse within the SD grid bounds, and are not using a tool, we are beginning to drag out a tile section
			if (PointWithinBounds(location, m_gridscreenlocation, m_gridscreensize) && !m_keys[DIK_C])
			{
				m_mousestate = UI_ShipDesigner::SDMouseState::DraggingOutTileSection;
				m_griddragstart = GetGridElementAtLocation(location);
			}
		}
	}
}

void UI_ShipDesigner::ProcessMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component)
{
	// Test whether we are in any mouse state where a mouse-up event could be significant
	if (m_viewmode == UI_ShipDesigner::SDViewMode::Construction)
	{
		if (m_mousestate == UI_ShipDesigner::SDMouseState::DraggingShipSection)
		{	
			// Test whether the section we are dragging can be placed here
			PlaceShipSection();

			// We are currently dragging a ship section; clear any images being displayed on the SD grid for placement conflict
			m_rg_placementconflict->ClearAllInstances();
		}
	}
	else if (m_viewmode == UI_ShipDesigner::SDViewMode::Corridor)
	{
		if (m_mousestate == UI_ShipDesigner::SDMouseState::DraggingOutCorridorSection)
		{
			// Attempt construction of this corridor stretch
			DeployCorridorTiles(m_griddragstart, GetGridElementAtLocation(location));

			// Clear all the preview images generated while we were dragging out the corridor section
			m_rg_blueprint->ClearAllInstances();
			m_rg_placementconflict->ClearAllInstances();

			// Update the SD grid
			PerformSDGridUpdate();
		}
		else
		{
			if (m_keys[DIK_C])
			{
				// Make sure we are clicking within the same grid element
				INTVECTOR2 start = GetGridElementAtLocation(startlocation);
				if (start == GetGridElementAtLocation(location) && start.x >= 0 && start.y >= 0)
				{
					// Apply the connection change tool to the currently selected elements
					ApplyConnectionChangeTool(location);
				}
			}
		}
	}
	else if (m_viewmode == UI_ShipDesigner::SDViewMode::Tile)
	{
		if (m_mousestate == UI_ShipDesigner::SDMouseState::DraggingOutTileSection)
		{
			// Attempt to create the tiles at this location
			PlaceTiles(m_griddragstart, GetGridElementAtLocation(location));

			// Clear the preview images generated while dragging out the tile sections
			m_rg_blueprint->ClearAllInstances();
			m_rg_placementconflict->ClearAllInstances();

			// Update the SD grid
			PerformSDGridUpdate();
		}
		else
		{
			if (m_keys[DIK_C])
			{
				// Make sure we are clicking within the same grid element
				INTVECTOR2 start = GetGridElementAtLocation(startlocation);
				if (start == GetGridElementAtLocation(location) && start.x >= 0 && start.y >= 0)
				{
					// Apply the connection change tool to the currently selected elements
					ApplyConnectionChangeTool(location);
				}
			}
		}
	}
	

	// Reset the mouse state to normal, and clear any recorded 'start' position for this mouse action
	m_mousestate = UI_ShipDesigner::SDMouseState::Default;
	m_griddragstart = INTVECTOR2(-1, -1);
	m_lastgriddragsquare = INTVECTOR2(-1, -1);
}

void UI_ShipDesigner::ProcessMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation)
{
	
}
void UI_ShipDesigner::ProcessRightMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation)
{
	
}

void UI_ShipDesigner::ProcessMouseMoveEvent(INTVECTOR2 location)
{
	// DEBUG: Show the mouse location on the screen for debug purposes
	INTVECTOR2 e = this->GetGridElementAtLocation(location);
	D::UI->WriteDebugOutput(3, concat("Mouse location: ")(location.x)(", ")(location.y)(" | Element: ")(e.x)(", ")(e.y)(" | Gridsize: ")(m_gridsize).str());

	// Take different action depending on current view & mouse states

	/* Corridor mode */
	if (m_viewmode == SDViewMode::Corridor)
	{
		if (m_mousestate == SDMouseState::DraggingOutCorridorSection)
		{
			// We are dragging out a length of corridor tiles; update the corridor preview according to new mouse position
			UpdateCorridorDragPreview(location);
		}
		else
		{
			if (m_keys[DIK_C])							// If we are holding the 'C' key, operate in connection tool mode
			{
				ShowConnectionChangeTool(location);
			}
		}
	}

	/* Tile mode */
	else if (m_viewmode == SDViewMode::Tile)
	{
		if (m_mousestate == SDMouseState::DraggingOutTileSection)
		{
			// Update the preview image displayed while dragging out a tile section
			UpdateTileDragPreview(location);
		}
		else
		{
			if (m_keys[DIK_C])							// If we are holding the 'C' key, operate in connection tool mode
			{
				ShowConnectionChangeTool(location);
			}
			else										// Otherwise, if no keys are being held
			{
				// Show a preview of the currently-selected tile on the grid, only if it is a fixed-size tile
				UpdateTileFixedSizePreview(location);
			}
		}
	}
}

void UI_ShipDesigner::ProcessMouseHoverEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, bool lmb, bool rmb)
{

}


void UI_ShipDesigner::ProcessRightMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component)
{
	INTVECTOR2 rstart, rdelta;

	// We will scroll the grid if the mouse movement originated within it
	rstart = m_mouse->GetRMBStartPosition();
	if (rstart.x > m_gridx && rstart.y > m_gridy && rstart.x < (m_gridx + m_gridw) && rstart.y < (m_gridy + m_gridh))
	{
		// Calculate the mouse movement delta to detemine whether we should start scrolling
		rdelta.x = (location.x - rstart.x);
		rdelta.y = (location.y - rstart.y);

		if ( (abs(rdelta.x) + abs(rdelta.y)) > GRID_SCROLL_THRESHOLD )
		{
			// We want to move the grid by an amount relative to the mouse movement delta, 
			// and also the time delta to ensure smooth scrolling
			float factor = UI_ShipDesigner::GRID_SCROLL_SPEED * Game::TimeFactor;
			ScrollGrid(rdelta.x * factor, rdelta.y * factor);
		}
	}
}

void UI_ShipDesigner::ProcessRightMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component)
{

}

void UI_ShipDesigner::ProcessRightMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component)
{

}




// Terminates the UI layout and disposes of all relevant components
void UI_ShipDesigner::Terminate(void)
{
	// Delete the ship designer ship, which is not part of the global ship collection so must be terminated separately
	//ShutdownSDShip();
	m_ship = NULL;

	// Deallocate all memory assigned for the ship designer grid
	for (int i=0; i<m_gridxmax; i++) delete[] m_grid[i];
	delete[] m_grid;

	// Deallocate all memory for the ship section render mapping
	m_sdshipsections.clear();

	// Remove any remaining memory allocated to construction view placement results
	if (m_consview_section_placement_result) { delete m_consview_section_placement_result; m_consview_section_placement_result = NULL; }
}

// Calculates the set of elements covered by a path from a start to an end grid square
std::vector<INTVECTOR2> UI_ShipDesigner::CalculateGridPath(INTVECTOR2 start, INTVECTOR2 end)
{
	int incr;
	std::vector<INTVECTOR2> path;

	// If start or end location are not within valid bounds then stop here
	if (start.x < 0 || start.y < 0 || end.x < 0 || end.y < 0 || start.x >= m_numgx || start.y >= m_numgy || end.x >= m_numgx || end.y >= m_numgy)
		return path;

	// First, process the horizontal direction .  Determine the increment depending on whether end is left or right of the start
	if (end.x < start.x) incr = -1; else incr = +1;

	// Iterate in the x direction from the start to the end and add vectors for each grid element
	for (int x=start.x; x != (end.x + incr) ; x += incr)
		path.push_back(INTVECTOR2(x, start.y));

	// Now process the vertical direction; determine the increment to be applied based on relative start/end positions
	if (end.y < start.y) incr = -1; else incr = +1;

	// Iterate in the y direction from the start level to the end; note we skip the first element since we just covered it going horizontally
	for (int y=(start.y + incr); y != (end.y + incr) ; y += incr)
		path.push_back(INTVECTOR2(end.x, y));
	
	// Return the path of grid elements
	return path;
}

// Returns the set of elements covered by a rectangular extent from a start to an end grid square
std::vector<INTVECTOR2> UI_ShipDesigner::CalculateGridExtent(INTVECTOR2 start, INTVECTOR2 end)
{
	std::vector<INTVECTOR2> extent;

	// If start or end location are not within valid bounds then stop here
	if (start.x < 0 || start.y < 0 || end.x < 0 || end.y < 0 || start.x >= m_numgx || start.y >= m_numgy || end.x >= m_numgx || end.y >= m_numgy)
		return extent;

	// Determine the increment (+/-ve) in each of the x and y dimensions
	int incr_x = (end.x < start.x ? -1 : +1);
	int incr_y = (end.y < start.y ? -1 : +1);

	// Loop through both x and y dimensions
	for (int y = start.y; y != (end.y+incr_y); y += incr_y)
	{
		for (int x = start.x; x != (end.x+incr_x); x += incr_x)
		{
			// Add this element to the extent
			extent.push_back(INTVECTOR2(x, y));
		}
	}

	// Return the extent of cells between this start and end point
	return extent;
}

// Tests a set of tile placement options to determine whether they are valid.  Overloaded version that assumes z = the current SD z position 
bool UI_ShipDesigner::TestTileSetPlacement(std::vector<INTVECTOR2> gridsquares, D::TileClass tileclass, bool showpreview)
{
	return TestTileSetPlacement(gridsquares, m_gridzpos, tileclass, showpreview);
}

// Tests a set of tile placement options to determine whether they are valid; optionally also shows visually on the SD grid
bool UI_ShipDesigner::TestTileSetPlacement(std::vector<INTVECTOR2> gridsquares, int zpos, D::TileClass tileclass, bool showpreview)
{
	bool valid = true;
	float zbp = 0.0f, zconflict = 0.0f;

	// If we have been passed an empty vector then this signals an issue, so return false immediately
	if (gridsquares.size() == 0) return false;

	// If we will be displaying a preview then clear the previous placement image vectors first, and precalc any required values
	if (showpreview) {
		m_rg_blueprint->ClearAllInstances();
		m_rg_placementconflict->ClearAllInstances();

		zbp = m_rg_blueprint->GetZOrder();
		zconflict = m_rg_placementconflict->GetZOrder();
	}

	// Iterate over the set of potential tile placements
	std::vector<INTVECTOR2>::const_iterator it_end = gridsquares.end();
	for (std::vector<INTVECTOR2>::const_iterator it = gridsquares.begin(); it != it_end; ++it)
	{
		// Calculate the position of the element 
		INTVECTOR3 elpos = GetElementAtGridSquare(*it);
		if (TestTilePlacement(elpos, tileclass))
		{
			// Tile can be placed here.  Generate a blueprint image on the grid if required
			if (showpreview)
				m_rg_blueprint->AddInstance(GetGridElementLocation(*it), zbp, m_gridsize, true, Rotation90Degree::Rotate0);
		}
		else 
		{
			// Tile cannot be placed here
			if (showpreview) 
			{
				// If we are displaying the preview then record the failure & create a placement conflict image here
				valid = false;
				m_rg_placementconflict->AddInstance(GetGridElementLocation(*it), zconflict, m_gridsize, true, Rotation90Degree::Rotate0);
			} 
			else
			{
				// If we are not displaying the preview then we can actually just quit here since we have an instance that is not valid
				return false;
			}
		}
	}

	// Return the result of the placement tests
	return valid;
}

// Deploys one or more tiles in the region defined by the specified start and end grid squares.  Will deploy any tiles
// that can be placed, e.g. if we drag 4 tiles and 1 has a conflict, the remaining 3 will still be placed
void UI_ShipDesigner::PlaceTiles(INTVECTOR2 start, INTVECTOR2 end)
{
	// Parameter check
	if (!m_selectedtiletype || start.x < 0 || start.y < 0 || end.x < 0 || end.y < 0 || 
		 start.x >= m_numgx || start.y >= m_numgy || end.x >= m_numgx || end.y >= m_numgy) return;

	// Determine the actual end point for this range; this can be different to the grid square under the mouse cursor
	// due to e.g. size restrictions 
	end = GetExactTilePlacementExtent(start, end);
	if (end.x < 0 || end.y < 0) return;

	// Take different action depending on whether this is variable- or fixed-size tile type
	if (!m_selectedtiletype->HasFixedSize())
	{
		// This is a variable-size tile, so simply attempt to deploy over the full range of cells
		PlaceTile(m_selectedtiletype, start, end);
	}
	else
	{
		// This is a fixed-size tile, so attempt to place each tile within the range in turn

		// Switch the x & y dimensions if required to ensure that start <= end in both x and y dimensions
		if (end.x < start.x) { int tmp = start.x; start.x = end.x; end.x = tmp; }
		if (end.y < start.y) { int tmp = start.y; start.y = end.y; end.y = tmp; }

		// Loop from start to end, incrementing by the tile size in each direction
		INTVECTOR2 tilesize = INTVECTOR2(m_selectedtiletype->GetElementSize().x, m_selectedtiletype->GetElementSize().y);
		for (int y = start.y; y <= end.y; y += tilesize.y)
		{
			for (int x = start.x; x <= end.x; x += tilesize.x)
			{
				// Attempt to place an instance of the tile at this location
				PlaceTile(m_selectedtiletype, INTVECTOR2(x, y), INTVECTOR2(x+tilesize.x-1, y+tilesize.y-1));
			}
		}
	}
}

// Attempts to place one instance of a tile at the specified location on the SD grid
void UI_ShipDesigner::PlaceTile(ComplexShipTileDefinition *tile, INTVECTOR2 start, INTVECTOR2 end)
{
	// Parameter check
	if (!tile || start.x < 0 || start.y < 0 || end.x < 0 || end.y < 0 || 
		 start.x >= m_numgx || start.y >= m_numgy || end.x >= m_numgx || end.y >= m_numgy) return;

	// Switch the x & y dimensions if required to ensure that start <= end in both x and y dimensions
	if (end.x < start.x) { int tmp = start.x; start.x = end.x; end.x = tmp; }
	if (end.y < start.y) { int tmp = start.y; start.y = end.y; end.y = tmp; }

	// Determine the element location corresponding to this point on the grid
	INTVECTOR3 elstart = GetElementAtGridSquare(start);
	INTVECTOR3 elend = INTVECTOR3(m_gridstart.x + end.x, m_gridstart.y + end.y, 
								  (tile->GetElementSize().z <= 0 ? m_gridzpos : m_gridzpos + tile->GetElementSize().z - 1));

	// Test whether the tile can be placed at this location.  Check each floor (z value) in turn
	for (int z = elstart.z; z <= elend.z; z++)
	{
		// Get the region of tiles in this range
		std::vector<INTVECTOR2> extent = CalculateGridExtent(INTVECTOR2(elstart.x, elstart.y), INTVECTOR2(elend.x, elend.y));

		// Test the placement of this tile across that range
		bool valid = TestTileSetPlacement(extent, z, tile->GetClass(), false);
		if (!valid) return;
	}

	// We have passed all the checks, so create a new tile over this range
	ComplexShipTile *t = tile->CreateTile();
	t->SetElementLocation(elstart);
	t->SetElementSize(INTVECTOR3(elend.x-elstart.x+1, elend.y-elstart.y+1, elend.z-elstart.z+1));
	t->SetRotation(Rotation90Degree::Rotate0);		// TODO: Allow rotation while in tile mode (using e.g. 'R')
	
	// Attempt to compile and validate the tile; if it fails, remove the tile and create nothing
	Result result = t->CompileAndValidateTile();
	if (result != ErrorCodes::NoError)
	{
		delete t; t = NULL; return;
	}
	
	// Otherwise, add this new tile to the ship under construction
	m_ship->AddTile(&t);

	// Refresh the surrounding environment to determine e.g. connection points to neighbours
	UpdateTileAndNeighbours(t);
			
	// Update the SD grid.  This could be wasteful to peform for every tile, but not a time-critical operation
	PerformSDGridUpdate();
}

// Examines the elements at the specified mouse location and displays the context dependent tool for changing connections if appropriate
void UI_ShipDesigner::ShowConnectionChangeTool(INTVECTOR2 location)
{
	// For efficiency, only re-evaluate the tool state if the mouse location has changed
	static INTVECTOR2 previouslocation = INTVECTOR2(0, 0);
	if (location == previouslocation) return;
	previouslocation = location;

	// Set the connection change tool to be invisible by default, unless we discover a valid scenario below
	m_rg_connectionchangetool->SetRenderActive(false);
	m_connectionchangetool_element1 = m_connectionchangetool_element2 = INTVECTOR3(-1, -1, -1);

	// Attempt to get the grid square and then element at this location
	INTVECTOR2 gridsquare = GetGridElementAtLocation(location);
	if (gridsquare.x < 0 || gridsquare.y < 0) return;
	INTVECTOR3 elpos = GetElementAtGridSquare(gridsquare);
	ComplexShipElement *el = m_ship->GetElement(elpos);

	// If no element exists, or it is not walkable, then we can quit now
	if (!el || !el->GetProperty(ComplexShipElement::PROPERTY::PROP_WALKABLE)) return;

	// Get the primary tile linked to this element; we must have a primary tile for this element to be connectable
	ComplexShipTile *elptile = el->GetTile();
	if (!elptile) return;

	// Attempt to get the neighbouring element closest to the edge of the element that we are mousing over, as a first priority
	INTVECTOR2 gridloc = GetGridElementLocation(gridsquare);
	float xdiff = location.x - (gridloc.x + (float)m_gridsize * 0.5f);
	float ydiff = location.y - (gridloc.y + (float)m_gridsize * 0.5f);
	float edgethreshold = ((m_gridsize * 0.5f) * UI_ShipDesigner::ELEMENT_EDGE_THRESHOLD);
	ComplexShipElement *nbrs[4];
	ComplexShipElement *nbr = NULL;

	// Get each neighbouring element in turn
	nbrs[0] = m_ship->GetElement(elpos.x - 1, elpos.y, elpos.z);
	nbrs[1] = m_ship->GetElement(elpos.x, elpos.y - 1, elpos.z);
	nbrs[2] = m_ship->GetElement(elpos.x + 1, elpos.y, elpos.z);
	nbrs[3] = m_ship->GetElement(elpos.x, elpos.y + 1, elpos.z);
	
	// See if a neighbour is available in our preferred direction, as a priority
	if		(xdiff < -edgethreshold && nbrs[0] && nbrs[0]->GetProperty(ComplexShipElement::PROPERTY::PROP_WALKABLE) 
				&& nbrs[0]->GetTile() != elptile)																	nbr = nbrs[0];
	else if (ydiff < -edgethreshold && nbrs[1] && nbrs[1]->GetProperty(ComplexShipElement::PROPERTY::PROP_WALKABLE)
				&& nbrs[1]->GetTile() != elptile)																	nbr = nbrs[1];
	else if (xdiff >  edgethreshold && nbrs[2] && nbrs[2]->GetProperty(ComplexShipElement::PROPERTY::PROP_WALKABLE)
				&& nbrs[2]->GetTile() != elptile)																	nbr = nbrs[2];
	else if (ydiff >  edgethreshold && nbrs[3] && nbrs[3]->GetProperty(ComplexShipElement::PROPERTY::PROP_WALKABLE)
				&& nbrs[3]->GetTile() != elptile)																	nbr = nbrs[3];
	
	// If we don't have a neighbour in the preferred direction, look for any other that is eligible
	if (!nbr)
		for (int i=0; i<4; i++)
			if (nbrs[i] && nbrs[i]->GetProperty(ComplexShipElement::PROPERTY::PROP_WALKABLE) && nbrs[i]->GetTile() != elptile)
				{ nbr = nbrs[i]; break; }

	// If we still don't have any eligible neighbour then quit now
	if (!nbr) return;

	// Determine the rendering size for the tool at this location
	int renderoffset = (int)ceil(CORRIDOR_TILE_OFFSET * m_gridsize);
	int rendersize = (int)ceil(CORRIDOR_TILE_WIDTH * m_gridsize);
	float renderz = m_rg_connectionchangetool->GetZOrder();

	// Show the connection tool at this location, depending on the neighbour that we have selected
	INTVECTOR3 nbrloc = nbr->GetLocation();
	if (nbrloc.x < elpos.x)		
		m_rg_connectionchangetool->AddInstance(INTVECTOR2(gridloc.x - renderoffset, gridloc.y + renderoffset), renderz, INTVECTOR2(renderoffset * 2, rendersize), true, Rotation90Degree::Rotate0);
	else if (nbrloc.y < elpos.y)
		m_rg_connectionchangetool->AddInstance(INTVECTOR2(gridloc.x + renderoffset, gridloc.y - renderoffset), renderz, INTVECTOR2(rendersize, renderoffset * 2), true, Rotation90Degree::Rotate90);
	else if (nbrloc.x > elpos.x)
		m_rg_connectionchangetool->AddInstance(INTVECTOR2(gridloc.x + m_gridsize - renderoffset, gridloc.y + renderoffset), renderz, INTVECTOR2(renderoffset * 2, rendersize), true, Rotation90Degree::Rotate180);
	else if (nbrloc.y > elpos.y)
		m_rg_connectionchangetool->AddInstance(INTVECTOR2(gridloc.x + renderoffset, gridloc.y + m_gridsize - renderoffset), renderz, INTVECTOR2(rendersize, renderoffset * 2), true, Rotation90Degree::Rotate270);
	else
		return;

	// Store the location of the two elements that will potentially be connected/disconnected
	m_connectionchangetool_element1 = elpos;
	m_connectionchangetool_element2 = nbrloc;

	// Make sure the tool is set to be rendered
	m_rg_connectionchangetool->SetRenderActive(true);
}

// Forces a refresh of the connection change tool, by invoking the method with an invalid location so that it overrides the cached previous mouse location
void UI_ShipDesigner::ForceRefreshOfConnectionChangeTool(void)
{
	ShowConnectionChangeTool(INTVECTOR2(-1, -1));
}

// Applies the connection change tool to the currently selected elements
void UI_ShipDesigner::ApplyConnectionChangeTool(INTVECTOR2 mouselocation)
{
	// Do validation to make sure that our parameters are still correct
	INTVECTOR2 selectedgridsquare = GetGridElementAtLocation(mouselocation);
	if (!m_rg_connectionchangetool->GetRenderActive() || selectedgridsquare.x < 0 || selectedgridsquare.y < 0 || 
		 m_connectionchangetool_element1 == m_connectionchangetool_element2 ) return;

	// Make sure that the selected grid square still corresponds to one of the elements being manipulated
	INTVECTOR3 selectedelement = GetElementAtGridSquare(selectedgridsquare);
	if (m_connectionchangetool_element1 != selectedelement && m_connectionchangetool_element2 != selectedelement) return;

	// Get a reference to each element being manipulated
	ComplexShipElement *e1 = m_ship->GetElement(m_connectionchangetool_element1);
	ComplexShipElement *e2 = m_ship->GetElement(m_connectionchangetool_element2);
	if (!e1 || !e2 || e1 == e2) return;

	// Get a reference to the primary tile linked to each element; if this is the same tile, we do not want to connect them
	ComplexShipTile *t1 = e1->GetTile();
	ComplexShipTile *t2 = e2->GetTile();
	if (!t1 || !t2 || t1 == t2) return;

	// Determine the direction of this connection from e1 > e2
	Direction dir = Direction::_Count;
	if		(m_connectionchangetool_element2.x < m_connectionchangetool_element1.x)		dir = Direction::Left;
	else if (m_connectionchangetool_element2.y < m_connectionchangetool_element1.y)		dir = Direction::Up;
	else if (m_connectionchangetool_element2.x > m_connectionchangetool_element1.x)		dir = Direction::Right;
	else if (m_connectionchangetool_element2.y > m_connectionchangetool_element1.y)		dir = Direction::Down;
	else																				return;
	/*
	// See whether connections already exist between these two tiles, at this location
	bool c1 = t1->HasConnection(m_connectionchangetool_element1 - t1->GetElementLocation(), dir);
	bool c2 = t2->HasConnection(m_connectionchangetool_element2 - t2->GetElementLocation(), GetOppositeDirection(dir));

	// Take different action depending on the connections that already exist
	if (c1 || c2)
	{
		// If we have a connection from e1>e2 OR e2>e1 then we treat the connection as already existing, and remove both sides of it now
		t1->RemoveConnection(m_connectionchangetool_element1 - t1->GetElementLocation(), dir);
		t2->RemoveConnection(m_connectionchangetool_element2 - t2->GetElementLocation(), GetOppositeDirection(dir));
	}
	else
	{
		// Otherwise, if we do not have a connection in either connection, we can create it now
		t1->AddConnection(m_connectionchangetool_element1 - t1->GetElementLocation(), dir);
		t2->AddConnection(m_connectionchangetool_element2 - t2->GetElementLocation(), GetOppositeDirection(dir));
	}
	*/
	// Refresh the SD grid to show the change in connections
	ForceRefreshOfConnectionChangeTool();
	PerformSDGridUpdate();
}


// Deploys a set of corridor tiles from the start to end grid squares.  Will refuse to deploy the full set
// if any tile fails placement validation
void UI_ShipDesigner::DeployCorridorTiles(INTVECTOR2 start, INTVECTOR2 end)
{
	// Parameter check - make sure we have been passed valid start & end grid squares
	if (start.x < 0 || start.y < 0 || end.x < 0 || end.y < 0 || start.x >= m_numgx || start.y >= m_numgy || end.x >= m_numgx || end.y >= m_numgy)
		return;

	// Calculate the path of grid squares to be traversed by this stretch of corridor
	std::vector<INTVECTOR2> path = CalculateGridPath(start, end);

	// Now test the placement of this path on the SD grid, to make sure they can all be deployed successfully
	bool valid = TestTileSetPlacement(path, D::TileClass::Corridor, false);
	if (!valid) return;

	// The path is valid so create a tile for each item in the stretch of corridor
	std::vector<INTVECTOR2>::iterator it_end = path.end();
	for (std::vector<INTVECTOR2>::iterator it = path.begin(); it != it_end; ++it)
	{
		DeployCorridorTile(*it);
	}
}

// Deploys a corridor tile at the specified location.  Also recalculates connections & tile types of the neighbouring tiles
// Note: no need to compile/validate the tile in this method.  This is performed in the UpdateCorridor... method due to dependency on neighbours
void UI_ShipDesigner::DeployCorridorTile(INTVECTOR2 location)
{
	// Make sure the parameter is valid
	if (location.x < 0 || location.y < 0 || location.x >= m_numgx || location.y >= m_numgy) return;

	// Calculate the element location that corresponds to this grid square, and test to make sure we can deploy a corridor tile here
	INTVECTOR3 elpos = GetElementAtGridSquare(location);
	if (!TestTilePlacement(elpos, D::TileClass::Corridor)) return;

	// Get the element at this location
	ComplexShipElement *el = m_ship->GetElement(elpos);
	if (!el) return;

	// Create a new corridor tile at this location.  Start with all possible connections active and trim based on surroundings
	CSCorridorTile *tile = CreateCorridorTile(true, true, true, true);
	if (!tile) return;

	// Set the element location within the ship
	tile->SetElementLocation(elpos);

	// Link this tile to the relevant parent objects
	ComplexShipTile *pbase = static_cast<ComplexShipTile*>(tile);
	m_ship->AddTile(&pbase);

	// Now we want to perform an update of this tile, plus its neighbours, to see if the connections will change
	UpdateTileAndNeighbours(tile);
}

// Updates a single tile and all underlying elements
void UI_ShipDesigner::UpdateTile(ComplexShipTile *tile)
{
	// TODO: To be implemented using relevant logic from UpdateCorridorTilesAtElement()
}

// Updates a corridor tile and all its neighbours at the specified location, to make sure all connections are made correctly
void UI_ShipDesigner::UpdateTileAndNeighbours(ComplexShipTile *tile)
{
	// Parameter checks
	if (!tile) return;
	
	// Check this tile first
	UpdateTile(tile);

	// Now also update any neighbouring tiles, if this is a corridor; DEBUG: TEMP: NOTE: this assumes corridor tiles are 1x1x1
	if (tile->GetTileClass() == D::TileClass::Corridor)
	{
		ComplexShipElement *elements = m_ship->GetElements();
		ComplexShipElement *el = m_ship->GetElement(tile->GetElementLocation());
		if (elements && el)
		{
			int index;
			index = el->GetNeighbour(Direction::Left); if (index != ComplexShipElement::NO_ELEMENT) UpdateCorridorTileAtElement(&elements[index]);
			index = el->GetNeighbour(Direction::Up); if (index != ComplexShipElement::NO_ELEMENT) UpdateCorridorTileAtElement(&elements[index]);
			index = el->GetNeighbour(Direction::Right); if (index != ComplexShipElement::NO_ELEMENT) UpdateCorridorTileAtElement(&elements[index]);
			index = el->GetNeighbour(Direction::Down); if (index != ComplexShipElement::NO_ELEMENT) UpdateCorridorTileAtElement(&elements[index]);
		}
	}
}

// Updates a single tile at the specified location.  Does not consider neighbouring tiles
void UI_ShipDesigner::UpdateCorridorTileAtElement(ComplexShipElement *el)
{
	return;
	/*
	bool left = false, up = false, right = false, down = false;

	// Parameter check
	if (!el) return;

	// Make sure we have a corridor tile in this element, otherwise there is no point in continuing
	CSCorridorTile *tile = (CSCorridorTile*)el->GetFirstTileOfType(D::TileClass::Corridor);
	if (!tile) return;

	// Analyse the tiles surrounding this element
	AnalyseCorridorEnvironment(el, &left, &up, &right, &down);

	// Based on this environment, create a new tile for the location 
	CSCorridorTile *newtile = CreateCorridorTile(left, up, right, down);
	if (!newtile) return;

	// Test whether the new tile matches the current tile
	if (tile->GetCode() == newtile->GetCode() && tile->GetRotation() == newtile->GetRotation()) 
	{
		// If it does, copy any newer data from the updated tile before we delete it
		tile->SetConnections(*(newtile->GetConnections()));

		// Delete the tile that we no longer need
		delete newtile; 

		// Before leaving this method, compile and validate the current tile to make sure it is still valid
		tile->CompileAndValidateTile();
		return;
	}

	// Tile didn't match, so we should replace the current one with this tile; first remove the existing one
	tile->UnlinkFromParent();
	delete tile;

	// Now add the new one in its place
	newtile->SetElementLocation(el->GetLocation());
	newtile->LinkToParent(m_ship);

	// Compile and validate the corridor tile
	newtile->CompileAndValidateTile();*/
}

// Returns a set of flags determining the corridor environment surrounding this element
void UI_ShipDesigner::AnalyseCorridorEnvironment(ComplexShipElement *el, bool *pOutLeft, bool *pOutUp, bool *pOutRight, bool *pOutDown)
{
	//int index;
	ComplexShipElement *en = NULL;
	ComplexShipTile *nbr = NULL;
	if (!el) return;

	// Get the tile at this element 
	ComplexShipTile *tile = el->GetTile();
	if (!tile || tile->GetClass() != D::TileClass::Corridor) return;
	/*
	// Check left neighbour
	index = el->GetNeighbour(Direction::Left);
	if (index != ComplexShipElement::NO_ELEMENT)
	{
		en = m_ship->GetElement(index);
		if (en) {
			nbr = en->GetTile();
			if (nbr && nbr->GetClass() == D::TileClass::Corridor)
				if (tile->HasConnection(NULL_INTVECTOR3, Direction::Left) || nbr->HasConnection(NULL_INTVECTOR3, Direction::Right))
					(*pOutLeft) = true;
		}
	}

	// Check up neighbour
	index = el->GetNeighbour(Direction::Up);
	if (index != ComplexShipElement::NO_ELEMENT)
	{
		en = m_ship->GetElement(index);
		if (en) {
			nbr = en->GetTile();
			if (nbr && nbr->GetClass() == D::TileClass::Corridor)
				if (tile->HasConnection(NULL_INTVECTOR3, Direction::Up) || nbr->HasConnection(NULL_INTVECTOR3, Direction::Down))
					(*pOutUp) = true;
		}
	}

	// Check right neighbour
	index = el->GetNeighbour(Direction::Right);
	if (index != ComplexShipElement::NO_ELEMENT)
	{
		en = m_ship->GetElement(index);
		if (en) {
			nbr = en->GetTile();
			if (nbr && nbr->GetClass() == D::TileClass::Corridor)
				if (tile->HasConnection(NULL_INTVECTOR3, Direction::Right) || nbr->HasConnection(NULL_INTVECTOR3, Direction::Left))
					(*pOutRight) = true;
		}
	}

	// Check down neighbour
	index = el->GetNeighbour(Direction::Down);
	if (index != ComplexShipElement::NO_ELEMENT)
	{
		en = m_ship->GetElement(index);
		if (en) {
			nbr = en->GetTile();
			if (nbr && nbr->GetClass() == D::TileClass::Corridor)
				if (tile->HasConnection(NULL_INTVECTOR3, Direction::Down) || nbr->HasConnection(NULL_INTVECTOR3, Direction::Up))
					(*pOutDown) = true;
		}
	}
	*/
}

// Returns a new corridor tile depending on the environment in which it is about to be placed.  Params indicate whether
// we have corridor tiles bordering on the specified sides
CSCorridorTile *UI_ShipDesigner::CreateCorridorTile(bool left, bool up, bool right, bool down)
{
	CSCorridorTile *tile = NULL;

	// Count the number of surrounding connections
	int numconn = ( (left ? 1 : 0) + (up ? 1 : 0) + (right ? 1 : 0) + (down ? 1 : 0) );

	// Determine the tile type based on number of connections
	if (numconn < 2)
	{
		// Fewer than two connections, so just create a standard straight corridor
		tile = (CSCorridorTile*)ComplexShipTile::Create("corridor_ns");
		if (!tile) return NULL;

		// We default to a NS tile, however if we have either E or W connections then rotate this tile
		if (left || right) tile->SetRotation(Rotation90Degree::Rotate90);
	}
	else if (numconn == 2)
	{
		// Two connections; either a straight or a corner
		if ((up && down) || (left && right))
		{
			// Straight tile
			tile = (CSCorridorTile*)ComplexShipTile::Create("corridor_ns");
			if (!tile) return NULL;

			// Rotate the tile if this is actually a left-to-right tile, then return it
			if (left) tile->SetRotation(Rotation90Degree::Rotate90);
		}
		else
		{
			// Corner tile
			tile = (CSCorridorTile*)ComplexShipTile::Create("corridor_se");
			if (!tile) return NULL;

			// Rotate based on which type of corner this is
			if		(up && right)			tile->SetRotation(Rotation90Degree::Rotate0);
			else if (right && down)			tile->SetRotation(Rotation90Degree::Rotate270);		// Rotates anti-clockwise... 
			else if (down && left)			tile->SetRotation(Rotation90Degree::Rotate180);
			else if (left && up)			tile->SetRotation(Rotation90Degree::Rotate90);		// Rotates anti-clockwise...
			else							return NULL;
		}
	}
	else if (numconn == 3)
	{
		// T-junction tile
		tile = (CSCorridorTile*)ComplexShipTile::Create("corridor_nse");
		if (!tile) return NULL;

		// Rotate based on which type of t-junction this is
		if		(!left)						tile->SetRotation(Rotation90Degree::Rotate0);
		else if (!up)						tile->SetRotation(Rotation90Degree::Rotate270);			// Rotates anti-clockwise...
		else if (!right)					tile->SetRotation(Rotation90Degree::Rotate180);
		else if (!down)						tile->SetRotation(Rotation90Degree::Rotate90);			// Rotates anti-clockwise...
		else								return NULL;
	}
	else if (numconn == 4)
	{
		// Four-way tile; no need to rotate this tile so simply return it
		tile = (CSCorridorTile*)ComplexShipTile::Create("corridor_nsew");
		if (!tile) return NULL;
	}
	else 
	{
		// Unexpected number of connections
		return NULL;
	}

	// Final check to make sure we successfully created the tile
	if (!tile) return NULL;
	/*
	// Now set tile-to-tile connections depennding on the surrounding environment.  Note that we only need to
	// set the connection from this tile outwards; the reciprocal connection will be established when updating
	// the surrounding corridor environment after this
	if (left)	tile->AddConnection(NULL_INTVECTOR3, Direction::Left);
	if (up)		tile->AddConnection(NULL_INTVECTOR3, Direction::Up);
	if (right)	tile->AddConnection(NULL_INTVECTOR3, Direction::Right);
	if (down)	tile->AddConnection(NULL_INTVECTOR3, Direction::Down);
	*/
	// Return the new tile
	return tile;
}

// Updates the preview generated on the SD grid while we are dragging out a corridor section
void UI_ShipDesigner::UpdateCorridorDragPreview(INTVECTOR2 location)
{
	// Determine which grid square the mouse is currently within
	INTVECTOR2 dragpos = GetGridElementAtLocation(location);

	// If the mouse is still within the same grid square, or not even within the grid, then no update is necessary
	if (dragpos == m_lastgriddragsquare) return;
	if (dragpos.x == -1 || dragpos.y == -1) return;

	// Record the new grid square we are dragging into, so that we only recalc the preview for this stretch once
	m_lastgriddragsquare = dragpos;

	// Calculate the path of grid squares to be traversed by this stretch of corridor
	std::vector<INTVECTOR2> path = CalculateGridPath(m_griddragstart, dragpos);

	// Now test the placement of this path on the SD grid, showing the preview at the same time
	TestTileSetPlacement(path, D::TileClass::Corridor, true);
}

// Update the preview displayed while dragging out a tile section
void UI_ShipDesigner::UpdateTileDragPreview(INTVECTOR2 location)
{
	// Determine which grid square the mouse is currently within
	INTVECTOR2 dragpos = GetGridElementAtLocation(location);

	// If the mouse is still within the same grid square, or not even within the grid, then no update is 
	// necessary.  Also make sure a tile is selected
	if (!m_selectedtiletype) return;
	if (dragpos == m_lastgriddragsquare) return;
	if (dragpos.x == -1 || dragpos.y == -1) return;

	// Record the new grid square we are dragging into, so that we only recalc the preview for this stretch once
	m_lastgriddragsquare = dragpos;

	// Retrieve the actual end point for this range; this can be different to the grid square under the mouse cursor
	// due to e.g. size restrictions 
	dragpos = GetExactTilePlacementExtent(m_griddragstart, dragpos);

	// Retrieve the extend of cells between these points
	std::vector<INTVECTOR2> extent = CalculateGridExtent(m_griddragstart, dragpos);
	if (extent.size() == 0) return;

	// Test the tile placement over this extent, including display of a preview of any conflicts on the SD grid
	TestTileSetPlacement(extent, m_selectedtiletype->GetClass(), true);
}

// Determines the actual range of elements that would be covered by a tile placement, taking into account things
// like fixed tile sizes.  Accepts a proposed start & end grid square as input, and returns a new value for the 
// end grid square based on the size restrictions.  If no change is required it will return the proposed end grid square
INTVECTOR2 UI_ShipDesigner::GetExactTilePlacementExtent(INTVECTOR2 start, INTVECTOR2 end)
{
	// Parameter check
	if (start.x < 0 || start.y < 0 || end.x < 0 || end.y < 0) return INTVECTOR2(-1, -1);

	// If this is a variable-size tile then (aside from class min/max size requirements) we don't need to do any further modification
	if (!m_selectedtiletype->HasFixedSize())
	{
		// Simply return the proposed end grid square
		return end;
	}
	else
	{
		// This is a fixed-size tile type, so we may need to adjust the range of elements that are tested
		INTVECTOR2 tilesize = INTVECTOR2(	m_selectedtiletype->GetElementSize().x, 
											m_selectedtiletype->GetElementSize().y  );			// Drop the z dimension for now
		INTVECTOR2 diff = INTVECTOR2(end.x - start.x, end.y - start.y);

		// Extend by one in each dimension to count both start and end tiles in the range
		diff.x += (diff.x < 0 ? -1 : +1);
		diff.y += (diff.y < 0 ? -1 : +1);
		
		// Restrict the extent of this tile to be a multiple of the tile size
		INTVECTOR2 tilecount = INTVECTOR2((int)floor((float)diff.x / (float)tilesize.x), (int)floor((float)diff.y / (float)tilesize.y));
		if (tilecount.x == 0 || tilecount.y == 0) return INTVECTOR2(-1, -1);

		// Determine the desired end point based on the number of tiles in each direction
		end = INTVECTOR2(	start.x + (tilecount.x * tilesize.x), 
							start.y + (tilecount.y * tilesize.y));

		// Remove one from each dimension, to reverse the step above.  We are going from a count of tiles to the index of the end tile
		end.x -= (diff.x < 0 ? -1 : +1);
		end.y -= (diff.y < 0 ? -1 : +1);

		// Return the correct end point for this range
		return end;
	}
}

// Update the preview image displayed while a tile is selected and the mouse is over the SD grid
void UI_ShipDesigner::UpdateTileFixedSizePreview(INTVECTOR2 location)
{

}

// Tests tile placement at the specified grid square.  Simply calls the main method after translating from grid
// square coords into ship element coordinates
bool UI_ShipDesigner::TestTilePlacement(INTVECTOR2 location, D::TileClass tileclass)
{
	return TestTilePlacement(GetElementAtGridSquare(location), tileclass);
}

// Tests whether a tile of the specified type could be placed at the selected location.  Note this is based on
// the tile class only; a complete analysis is performed when we actually try to place the tile
bool UI_ShipDesigner::TestTilePlacement(INTVECTOR3 location, D::TileClass tileclass)
{
	// Get the element at this location; if there isn't one, return false immediately since we couldn't place anything here
	ComplexShipElement *el = m_ship->GetElement(location);
	if (!el) return false;

	// We cannot build anything on the element if it is not flagged as buildable
	if (!el->IsBuildable()) return false;

	// If the element is buildable, and there are no tiles currently linked to it, then we can return true immediately
	ComplexShipTile *tile = el->GetTile();
	if (!tile) return true;

	// Test the tile class provided
	if (tileclass == D::TileClass::Unknown) return false;
	bool infrastructure = false; // ComplexShipTile::IsInfrastructureTile(tileclass);

	// Get the class of the tile currently in this place
	D::TileClass existingclass = tile->GetClass();
	bool existinginfra = true; // ComplexShipTile::IsInfrastructureTile(existingclass);

	// If we are trying to deploy a 'normal' tile, then we cannot allow any other in this place aside from an infrastructure tile
	// TODO: This will no longer work since we link max one tile per element
	if (!infrastructure && !existinginfra) return false;

	// If we are trying to deploy an infrastructure tile, then we can always deploy unless a tile of the same class is already here
	if (infrastructure && (tileclass == existingclass)) return false;

	// We have passed all checks so return true to signal that the tile can be placed here
	return true;
}

// Returns a reference to the render group that should be used for rendering the specified tile class
Image2DRenderGroup * UI_ShipDesigner::GetTileRenderGroup(D::TileClass tileclass)
{
	// Return a reference to a render group based on the class
	switch (tileclass)
	{
		// Corridor tiles are rendered separately and do not need to use thes render groups
		case D::TileClass::Corridor:
			return NULL;

		// Military-type tiles
//		case D::TileClass::(XYZ):
//			return m_rg_tile_military;

		// Engineering-type tiles
		//case D::TileClass::Power:
		//case D::TileClass::Coolant:
			//return m_rg_tile_engineer;

		// Civilian-type tiles (default render group if not one of the above)
		default:
			return m_rg_tile_civilian;
	}
}

UI_ShipDesigner::~UI_ShipDesigner(void)
{
}

// Static method to translate from a corridor view mode's string representation to the mode itself
UI_ShipDesigner::CorridorViewMode UI_ShipDesigner::TranslateCorridorViewModeFromString(std::string code)
{
	if (code == "SINGLE_CORRIDOR")
		return UI_ShipDesigner::CorridorViewMode::SingleCorridor;

	else
		return UI_ShipDesigner::CorridorViewMode::Unknown;
}

// Static method to translate a corridor view mode into its string code representation
std::string UI_ShipDesigner::TranslateCorridorViewModeToString(CorridorViewMode mode)
{
	switch (mode) 
	{
		case UI_ShipDesigner::CorridorViewMode::SingleCorridor:
			return "SINGLE_CORRIDOR";								break;

		default:
			return "";
	}
}
