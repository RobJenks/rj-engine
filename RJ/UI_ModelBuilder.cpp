#include "AlignedAllocator.h"
#include "DX11_Core.h"
#include "CoreEngine.h"
#include "OverlayRenderer.h"
#include "RJMain.h"
#include "GameVarsExtern.h"
#include "UserInterface.h"
#include "Render2DGroup.h"
#include "Player.h"
#include "GameUniverse.h"
#include "Model.h"
#include "FileSystem.h"
#include "MultiLineTextBlock.h"
#include "UIButton.h"
#include "UITextBox.h"
#include "SimpleShip.h"
#include "ComplexShip.h"
#include "SpaceSystem.h"
#include "StaticTerrainDefinition.h"
#include "StaticTerrain.h"
#include "XMLGenerator.h"
#include "DataInput.h"
#include "DataOutput.h"

#include "UI_ModelBuilder.h"

// Constant values
const float	UI_ModelBuilder::MOUSE_DRAG_DISTANCE_MODIFIER = 0.25f;
const float	UI_ModelBuilder::MOUSE_DRAG_ROTATION_SPEED = 0.05f;
const float	UI_ModelBuilder::ORIENTATION_REVERT_TIME = 2.0f;
const float UI_ModelBuilder::SELECTION_MOVE_SPEED = 5.0f;
const float UI_ModelBuilder::SELECTION_RESIZE_SPEED = 5.0f;
const float UI_ModelBuilder::SELECTION_ROTATE_SPEED = PI * 0.25f;
const float UI_ModelBuilder::ZOOM_SPEED = 1.0f;
const float UI_ModelBuilder::DEFAULT_ZOOM_LEVEL = 4.0f;
const float UI_ModelBuilder::MIN_ZOOM_LEVEL = 3.0f;

// Default constructor
UI_ModelBuilder::UI_ModelBuilder(void)
{
	// Set default values
	m_system = NULL;
	m_playerviewpoint = NULL;
	m_object = NULL;
	m_nullenv = NULL;
	m_rotatingmodel = false;
	m_rotate_yaw = m_rotate_pitch = 0.0f;
	m_revertingmodelrotation = false;
	m_reverttimeremaining = 0.0f;
	m_zoomlevel = 10.0f;
	m_colldata_itemcount = 0;
	m_colldata_selected = -1;
	m_selection_type = UI_ModelBuilder::MVSelectionType::NothingSelected;
	m_selected_obb = NULL;
	m_selected_terrain = NULL;
	m_selection_transform = ID_MATRIX;
	m_key_shift = m_key_ctrl = false;
	m_restore_obbrender = m_restore_terrainrender = false;
	m_restore_terrainenvironment = 0;
}

// Method that is called when the UI controller becomes active
void UI_ModelBuilder::Activate(void)
{
	// Pause the game while the model builder is active
	Game::Application.Pause();

	// Store a reference to the null system, which this component will use for simulation
	m_system = Game::Universe->GetSystem("NULLSYS");

	// Recreate the player viewpoint and the model 'ship' used to display selected models
	CreateScenarioObjects();

	// Override the player state and force the player into the null system
	Game::CurrentPlayer->OverridePlayerEnvironment(m_system, m_playerviewpoint, NULL_VECTOR, ID_QUATERNION);

	// Store the current state of debug visualisations (will generally be disabled) and then activate them
	m_restore_obbrender = Game::Engine->GetRenderFlag(CoreEngine::RenderFlag::RenderOBBs);
	m_restore_terrainrender = Game::Engine->GetRenderFlag(CoreEngine::RenderFlag::RenderTerrainBoxes);
	m_restore_terrainenvironment = Game::Engine->GetDebugTerrainRenderEnvironment();
	Game::Engine->SetRenderFlag(CoreEngine::RenderFlag::RenderOBBs, true);
	Game::Engine->SetRenderFlag(CoreEngine::RenderFlag::RenderTerrainBoxes, true);
	if (m_nullenv) Game::Engine->SetDebugTerrainRenderEnvironment(m_nullenv->GetID());

	// Initialise per-run values 
	m_rotatingmodel = false;
	m_rotate_yaw = m_rotate_pitch = 0.0f;
}

// Method that is called when the UI controller is deactivated
void UI_ModelBuilder::Deactivate(void)
{
	// Release the override on player state
	Game::CurrentPlayer->ReleasePlayerEnvironmentOverride();

	// Shut down and deallocate the model 'ship' used to display selected models
	ShutdownScenarioObjects();

	// Restore the prior state of debug render variables
	Game::Engine->SetRenderFlag(CoreEngine::RenderFlag::RenderOBBs, m_restore_obbrender);
	Game::Engine->SetRenderFlag(CoreEngine::RenderFlag::RenderTerrainBoxes, m_restore_terrainrender);
	Game::Engine->SetDebugTerrainRenderEnvironment(m_restore_terrainenvironment);

	// Unpause the game once the model builder is deactivated
	Game::Application.Unpause();
}

// Method to perform rendering of the UI controller components (excluding 2D render objects, which will be handled by the 2D render manager)
// We also handle any per-frame update logic here
void UI_ModelBuilder::Render(void)
{
	// Update model orientation etc. if required
	UpdateModel();

	// Display a selection box if an object is currently selected
	RenderCurrentSelection();

	// Force a recalculation of object transforms & OBB every cycle, to ensure all changes are reflected immediately.  We
	// are directly setting position & orientation so need to ensure these transforms are recalculated as normal
	if (m_object)
	{
		m_object->CollisionOBB.Invalidate();
		m_object->RefreshPositionImmediate();

		// Force the null environment into the same pos/orient as the main object, so that terrain is rendered at correct locations
		if (m_nullenv)
		{
			m_nullenv->SetPosition(m_object->GetPosition());
			m_nullenv->SetOrientation(m_object->GetOrientation());
			m_nullenv->RefreshPositionImmediate();
		}
	}
}

// Refreshes the model builder interface based on current state
void UI_ModelBuilder::RefreshInterface(void)
{
	// Build a vector of items to be populated in the collision data control
	std::vector<std::string> items;

	// First, run a recursive traversal of the OBB hierarchy and add all items in flattened hierarchy order
	BuildOBBHierarchyItemList(&(m_object->CollisionOBB), NullString, -1, items);
	std::vector<std::string>::size_type obbcount = items.size();

	// Now add each terrain object to the vector in turn
	TerrainCollection::size_type terraincount = m_terrain.size();
	for (TerrainCollection::size_type i = 0; i < terraincount; ++i)
	{
		items.push_back(concat("Terrain")(i).str());
	}

	// Now clear the collision data control before adding each item in turn
	m_collisiondata->Clear();
	m_colldata_itemcount = (int)items.size();
	int n = min(m_colldata_itemcount, m_collisiondata->GetLineCount());
	for (int i = 0; i < n; ++i)
	{
		m_collisiondata->SetText(i, items[i]);
	}
}

// Performs a recursive traversal of the OBB hierarchy, building a vector of node names as it goes
void UI_ModelBuilder::BuildOBBHierarchyItemList(OrientedBoundingBox *node, const std::string & parent_name, int child_index, std::vector<std::string> & refItemList)
{
	// Parameter check
	if (!node) return;

	// Determine a name for this node
	std::string name;
	if (child_index < 0)					name = "OBB";
	else									name = concat(" ")(parent_name)(".")(child_index).str();

	// Add this node to the item list
	refItemList.push_back(name);

	// Now iterate through each child node in turn and perform the same actions
	for (int i = 0; i < node->ChildCount; ++i)
	{
		BuildOBBHierarchyItemList(&(node->Children[i]), name, i, refItemList);
	}
}

// Renders the current user selection
void UI_ModelBuilder::RenderCurrentSelection(void)
{
	// Take different action depending on the type of object selected
	if (m_selection_type == UI_ModelBuilder::MVSelectionType::OBBSelected && m_selected_obb)
	{
		XMMATRIX world;
		m_selected_obb->GenerateWorldMatrix(world);
		OrientedBoundingBox::CoreOBBData & obb_data = m_selected_obb->Data();
		Game::Engine->GetOverlayRenderer()->RenderCuboid(world, OverlayRenderer::RenderColour::RC_Green, obb_data.ExtentF.x * 2.0f,
			obb_data.ExtentF.y * 2.0f, obb_data.ExtentF.z * 2.0f, 0.5f, m_object->GetPosition());
	}
	else if (m_selection_type == UI_ModelBuilder::MVSelectionType::TerrainSelected && m_selected_terrain && m_nullenv)
	{
		// We want to subtract the terrain centre-translation from its world matrix, since the selection box that we are rendering
		// is already centred about its local (0,0,0) origin
		XMMATRIX removeoffset;
		if (m_selected_terrain->GetDefinition() && m_selected_terrain->GetDefinition()->GetModel())
		{
			removeoffset = XMMatrixTranslationFromVector(XMLoadFloat3(&m_selected_terrain->GetDefinition()->GetModel()->GetModelCentre()));
		}
		else
		{
			removeoffset = ID_MATRIX;
		}
		
		// Construct the world matrix and use it to render the selection box
		XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(m_selected_terrain->GetWorldMatrix(), removeoffset), m_nullenv->GetZeroPointWorldMatrix());
		Game::Engine->GetOverlayRenderer()->RenderCuboid(world, OverlayRenderer::RenderColour::RC_LightBlue, m_selected_terrain->GetExtentF().x * 2.0f,
			m_selected_terrain->GetExtentF().y * 2.0f, m_selected_terrain->GetExtentF().z * 2.0f, 0.5f, m_object->GetPosition());
	}
}

// Updates the model each frame
void UI_ModelBuilder::UpdateModel(void)
{
	// If we are rotating the model then update its orientation here
	if (m_rotatingmodel)
	{
		// Update the model ship orientation by desired pitch & yaw values [ delta quaternion 
		// is defined as (0.5 * orient_change * current_orient) ]
		// m_object->AddDeltaOrientation(0.5f * D3DXQUATERNION(m_rotate_pitch, m_rotate_yaw, 0.0f, 0.0f) * m_object->GetOrientation());
		m_object->AddDeltaOrientation(
			XMVectorScale(
				XMQuaternionMultiply(
					XMVectorSetX(XMVectorSetY(NULL_VECTOR, m_rotate_yaw), m_rotate_pitch), 
				m_object->GetOrientation()),
			0.5f));
	}

	// If the model is reverting to its original orientation then determine the delta change and apply it here
	if (m_revertingmodelrotation)
	{
		// Perform linear interpolation between current and target orientation, then apply the changes immediately
		float revert_pc = 1.0f - (m_reverttimeremaining / UI_ModelBuilder::ORIENTATION_REVERT_TIME);
		XMVECTOR orient = XMQuaternionSlerp(m_object->GetOrientation(), ID_QUATERNION, revert_pc);
		m_object->SetOrientation(orient);

		// We should also revert back to the default zoom level, if we aren't there already, based on the same revert timer
		PerformZoom((GetDefaultZoomLevel() - GetZoomLevel()) * revert_pc);

		// Decrement the revert timer and deactivate it if we have reached the target
		m_reverttimeremaining -= Game::PersistentTimeFactor;
		if (m_reverttimeremaining <= 0.0f)
		{
			m_revertingmodelrotation = false;
			m_reverttimeremaining = 0.0f;
			m_object->SetOrientation(ID_QUATERNION);
			PerformZoom(GetDefaultZoomLevel() - GetZoomLevel());
		}
	}
}

// Initialisation method, called once the UI component has been created
Result UI_ModelBuilder::InitialiseController(Render2DGroup *render, UserInterface *ui)
{
	// Retrieve references to the key UI controls
	if (m_render)
	{
		m_collisiondata = m_render->Components.MultiLineTextBlocks.GetItem("tb_collision_data");
	}

	// Return success
	return ErrorCodes::NoError;
}

// Determines the correct orientation for the model based on current mouse events
void UI_ModelBuilder::UpdateModelOrientation(const INTVECTOR2 & startlocation, const INTVECTOR2 & currentlocation)
{
	// We do not want to rotate the model based on user input if it us currently being moved by the system
	if (m_revertingmodelrotation)
	{
		m_rotatingmodel = false;
		m_rotate_yaw = m_rotate_pitch = 0.0f;
		return;
	}

	// The distance moved in the X axis will correspond to yaw, and distance in the X axis to pitch
	float yaw = (currentlocation.x - startlocation.x) / ((float)Game::ScreenWidth * UI_ModelBuilder::MOUSE_DRAG_DISTANCE_MODIFIER);
	float pitch = (currentlocation.y - startlocation.y) / ((float)Game::ScreenHeight * UI_ModelBuilder::MOUSE_DRAG_DISTANCE_MODIFIER);
	
	// Constrain the yaw and pitch to be within (-1.0 to +1.0)
	yaw = max(min(yaw, 1.0f), -1.0f);
	pitch = max(min(pitch, 1.0f), -1.0f);

	// Multiply the pitch and yaw by the rotation speed constant to give a final rotation value in radians/sec
	m_rotatingmodel = true;
	m_rotate_yaw = yaw * UI_ModelBuilder::MOUSE_DRAG_ROTATION_SPEED;
	m_rotate_pitch = pitch * UI_ModelBuilder::MOUSE_DRAG_ROTATION_SPEED;
}

// Clears all current model data
void UI_ModelBuilder::ClearCurrentModel(void)
{
	// Clear the reference to the current model
	if (m_object) m_object->SetModel(NULL);

	// Refresh the interface to reflect this change in model
	RefreshInterface();
}

// Loads a model into the viewer
Result UI_ModelBuilder::LoadModel(const std::string & code)
{
	// Attempt to retrieve the model with this code, and make sure it is valid
	Model *m = Model::GetModel(code);
	if (!m) return ErrorCodes::CannotLoadModelWithInvalidCodeToViewer;

	// Also make sure the placeholder 'ship' is correctly set up
	if (!m_object) return ErrorCodes::CannotLoadModelWithoutModelViewerEntity;

	// Otherwise, set the placeholder 'ship' to use this model so it is visible in the viewer
	m_object->SetModel(m);

	// Force the placeholder 'ship' to update its size data, which will generate the appropriate root OBB for this model.  Also
	// ensure this root OBB is set to auto-fit by default
	m_object->CollisionOBB.SetAutoFitMode(true);
	m_object->RecalculateShipDataFromCurrentState();

	// Set the base zoom level to be a constant percentage of the model bounding sphere radius, to ensure the model always begins
	// at a sensible distance from the camera.  Subtract current level so we get ((CurrentZoom + Delta) - CurrentZoom) = Delta
	PerformZoom( GetDefaultZoomLevel() - m_zoomlevel );

	// Refresh the interface to reflect the newly-loaded model
	RefreshInterface();

	// Return success
	return ErrorCodes::NoError;
}


// (Re)Create the ships used as placeholders, e.g. for the player view or as the model placeholder
void UI_ModelBuilder::CreateScenarioObjects(void)
{
	// If the objects already exist then we want to remove them first
	ShutdownScenarioObjects();

	// Player viewpoint: player will be situated in a temporary null ship viewpoint
	m_playerviewpoint = SimpleShip::Create("null_modelviewer_ship");
	if (m_playerviewpoint)
	{
		// Player viewpoint is at the origin
		m_playerviewpoint->SetPositionAndOrientation(NULL_VECTOR, ID_QUATERNION);

		// Remove any model from the player ship, and manually remove the OBB data so it is not rendered
		m_playerviewpoint->SetModel(NULL);
		m_playerviewpoint->CollisionOBB.Clear();

		// Add the object to the system
		m_playerviewpoint->MoveIntoSpaceEnvironment(m_system);

		// Set as a simulation hub to ensure the object is always fully-simulated
		m_playerviewpoint->SetAsSimulationHub();

	}

	// Model placeholder: create a new SimpleShip as a model 'wrapper' for this component
	m_object = SimpleShip::Create("null_ship");
	if (m_object)
	{
		// Default the position and orientation of this ship
		m_object->SetPositionAndOrientation(XMVectorSetZ(NULL_VECTOR, 10.0f), ID_QUATERNION);

		// Remove any model reference from the ship, and also the collision data so it is not rendered
		m_object->SetModel(NULL);
		m_object->CollisionOBB.Clear();

		// Add the object to the system
		m_object->MoveIntoSpaceEnvironment(m_system);

		// Set as a simulation hub to ensure the object is always fully-simulated
		m_object->SetAsSimulationHub();


		// Null container: create a new null environment to act as the container for all terrain objects in the viewer
		m_nullenv = ComplexShip::Create("null_environment");
		if (m_nullenv)
		{
			// Set a position and orientation out of the viewer area
			m_nullenv->SetPositionAndOrientation(m_object->GetPosition(), m_object->GetOrientation());

			// Clear the ship collision data so it is not rendered
			m_nullenv->CollisionOBB.Clear();

			// Add the object to the system, and set as a simulation hub so it is always simulated
			m_nullenv->MoveIntoSpaceEnvironment(m_system);
			m_nullenv->SetAsSimulationHub();
		}
	}
}

// Shut down and deallocate the models/ships used in this scenario
void UI_ModelBuilder::ShutdownScenarioObjects(void)
{
	// See if we need to remove the player viewpoint ship
	if (m_playerviewpoint)
	{
		// Remove the object designation as a simulation hub
		m_playerviewpoint->RemoveSimulationHub();

		// Remove from the system itself
		m_system->RemoveObjectFromSystem(m_playerviewpoint);

		// Now shutdown and deallocate the ship
		m_playerviewpoint->Shutdown();
		SafeDelete(m_playerviewpoint);
	}

	// See if we need to remove the placeholder model ship
	if (m_object)
	{
		// Remove the object designation as a simulation hub
		m_object->RemoveSimulationHub();

		// Remove from the system itself
		m_system->RemoveObjectFromSystem(m_object);

		// Now shutdown and deallocate the ship
		m_object->Shutdown();
		SafeDelete(m_object);
	}

	// Also remove the null environment used as a parent for all terrain objects
	if (m_nullenv)
	{
		// Remove the simulation hub designation
		m_nullenv->RemoveSimulationHub();

		// Remove from the system itself
		m_system->RemoveObjectFromSystem(m_nullenv);

		// Shutdown and deallocate the environment
		m_nullenv->Shutdown();
		SafeDelete(m_nullenv);
	}
}

// Method to process user input into the active UI controller
void UI_ModelBuilder::ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse)
{
	// Process keyboard input
	ProcessKeyboardInput(keyboard);
}

// Process keyboard input from the user
void UI_ModelBuilder::ProcessKeyboardInput(GameInputDevice *keyboard)
{
	// Store certain parameters that will be used throughout the UI controller
	m_key_shift = (keyboard->GetKey(DIK_LSHIFT) == TRUE || keyboard->GetKey(DIK_RSHIFT) == TRUE);
	m_key_ctrl = (keyboard->GetKey(DIK_LCONTROL) == TRUE || keyboard->GetKey(DIK_RCONTROL) == TRUE);

	if (keyboard->GetKey(DIK_HOME))
	{
		if (!m_key_ctrl)
		{
			// Revert the model orientation to its original position
			RevertModelOrientation();
		}
		else
		{
			// If the control key is also held, revert the selected object orientation to the identity
			RevertSelectionOrientation();
		}
	}
	else if (keyboard->GetKey(DIK_E))					ZoomIn();
	else if (keyboard->GetKey(DIK_C))					ZoomOut();
	else if (keyboard->GetKey(DIK_R) && m_key_ctrl && m_key_shift)
	{
		if (!m_object || !m_object->GetModel()) return;
		XMVECTOR adjust = XMLoadFloat3(&Float3MultiplyScalar(m_object->GetModel()->GetActualModelSize(), 0.5f));
		for (int i = 0; i < (int)m_terrain.size(); ++i)
			if (m_terrain[i])
				m_terrain[i]->SetPosition(XMVectorSubtract(m_terrain[i]->GetPosition(), adjust));

		RefreshAllCollisionGeometry();
		RefreshInterface();

		keyboard->LockKey(DIK_R);
	}
	else
	{
		// All movement or resize operations can be performed in parallel, as long as other keys are not being pressed
		if (keyboard->GetKey(DIK_W))
		{
			if (m_key_shift)							ResizeSelection(XMVectorSetZ(NULL_VECTOR, 1.0f));
			else if (m_key_ctrl)						RotateSelection(XMVectorSetX(NULL_VECTOR, 1.0f));
			else										MoveSelection(XMVectorSetZ(NULL_VECTOR, 1.0f));
		}
		if (keyboard->GetKey(DIK_S))
		{
			if (m_key_shift)							ResizeSelection(XMVectorSetZ(NULL_VECTOR, -1.0f));
			else if (m_key_ctrl)						RotateSelection(XMVectorSetX(NULL_VECTOR, -1.0f));
			else										MoveSelection(XMVectorSetZ(NULL_VECTOR, -1.0f));
		}
		if (keyboard->GetKey(DIK_A))
		{
			if (m_key_shift)							ResizeSelection(XMVectorSetX(NULL_VECTOR, -1.0f));
			else if (m_key_ctrl)						RotateSelection(XMVectorSetY(NULL_VECTOR, -1.0f));
			else										MoveSelection(XMVectorSetX(NULL_VECTOR, -1.0f));
		}
		if (keyboard->GetKey(DIK_D))
		{
			if (m_key_shift)							ResizeSelection(XMVectorSetX(NULL_VECTOR, 1.0f));
			else if (m_key_ctrl)						RotateSelection(XMVectorSetY(NULL_VECTOR, 1.0f));
			else										MoveSelection(XMVectorSetX(NULL_VECTOR, 1.0f));
		}
		if (keyboard->GetKey(DIK_Q))
		{
			if (m_key_shift)							ResizeSelection(XMVectorSetY(NULL_VECTOR, 1.0f));
			else if (m_key_ctrl)						RotateSelection(XMVectorSetZ(NULL_VECTOR, -1.0f));
			else										MoveSelection(XMVectorSetY(NULL_VECTOR, 1.0f));
		}
		if (keyboard->GetKey(DIK_Z))
		{
			if (m_key_shift)							ResizeSelection(XMVectorSetY(NULL_VECTOR, -1.0f));
			else if (m_key_ctrl)						RotateSelection(XMVectorSetZ(NULL_VECTOR, 1.0f));
			else										MoveSelection(XMVectorSetY(NULL_VECTOR, -1.0f));
		}
	}
}

// Method to handle the mouse move event
void UI_ModelBuilder::ProcessMouseMoveEvent(INTVECTOR2 location)
{
	m_collisiondata->SetText(m_collisiondata->GetLineCount() - 1, concat(location.x)(", ")(location.y)("  | loc=")(m_collisiondata->GetPosition().x)(",")
		(m_collisiondata->GetPosition().y)(" | sz=")(m_collisiondata->GetSize().x)(",")(m_collisiondata->GetSize().y).str());

	// Test whether the RMB is depressed
	if (m_rmb_down)
	{
		// If the RMB is down and the user isn't clicking on a component, we are rotating the model
		if (m_rmb_down_component == NULL)
		{
			// Determine the correct new orientation of the model
			UpdateModelOrientation(m_rmb_down_location, location);
		}
	}
}

// Process the mouse-up event for this component
void UI_ModelBuilder::ProcessRightMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component)
{
	// If we were rotating the model via a right mouse drag then stop the rotation here
	if (m_rotatingmodel)
	{
		m_rotatingmodel = false;
		m_rotate_yaw = m_rotate_pitch = 0.0f;
	}
}

// Event is triggered whenever a mouse click event occurs on a managed control, e.g. a button
void UI_ModelBuilder::ProcessControlClickEvent(iUIControl *control)
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

// Methods to accept generic mouse click events at the specified location
void UI_ModelBuilder::ProcessMouseClickAtLocation(INTVECTOR2 location)
{
	// Test whether the click falls within certain key components
	if (PointWithinBounds(location, m_collisiondata->GetPosition(), m_collisiondata->GetSize())) CollisionData_Clicked(location);
}

// Process button click events in the UI
void UI_ModelBuilder::ProcessButtonClickEvent(UIButton *button)
{
	// Retrieve the code of the button being clicked
	std::string code = button->GetCode();

	// Pass control to the applicable method
	if (code == "btn_loadmodel") 			LoadModel_ButtonClicked();
	else if (code == "btn_savemodel")		SaveModel_ButtonClicked();
	else if (code == "btn_loadcolldata")	LoadCollData_ButtonClicked();
	else if (code == "btn_savecolldata")	SaveCollData_ButtonClicked();
	else if (code == "btn_addobb")			AddNewOBB();
	else if (code == "btn_addterrain")		AddNewTerrain();
	else if (code == "btn_removecolldata")	RemoveSelectedObject();
	else if (code == "btn_clearcolldata")	ClearCollisionData();
	else if (code == "btn_genrootobb")		GenerateRootOBB();
}

// Add a new OBB under the currently-selected node in the OBB hierarchy (if applicable)
void UI_ModelBuilder::AddNewOBB(void)
{
	// Get the currently-selected object and make sure it is an OBB
	if (m_colldata_selected == -1 || m_selection_type != UI_ModelBuilder::MVSelectionType::OBBSelected ||
		m_selected_obb == NULL) return;

	// Otherwise we want to append a new child node to this OBB, by deallocating/reallocating the child data.  Update any
	// fields that should differ from their defaults
	m_selected_obb->AppendNewChildNode();
	m_selected_obb->Children[m_selected_obb->ChildCount - 1].UpdateExtent(XMVectorSetW(XMVectorReplicate(3.0f), 0.0f));

	// Force a refresh of the interface after the new OBB is added
	RefreshInterface();

	// Select this new node after it has been added
	SelectOBB(&(m_selected_obb->Children[m_selected_obb->ChildCount - 1]));
}

// Adds a terrain object to the model.  Called by AddNewTerrain() as well as e.g. the methods 
// to load collision data from file.  Ensures terrain is added correctly to the null 
// environment as well as the model builder data collections
void UI_ModelBuilder::AddTerrain(StaticTerrain *terrain)
{
	// Make sure the terrain object and container environment are valid
	if (!terrain || !m_nullenv) return;

	// Add to the central modelbuilder collection
	m_terrain.push_back(terrain);

	// Also add to the null container environment
	m_nullenv->AddTerrainObject(terrain);

	// Perform a full update of the collision geometry based on this change
	RefreshAllCollisionGeometry();

	// Select this new object after it has been added
	if (m_terrain.size() > 0)
		SelectTerrain(m_terrain.at(m_terrain.size() - 1));
}

// Removes a terrain object from the model.  Called by RemovedSelectedObject() as well as e.g. methods 
// to clear data.  Ensures terrain is removed correctly from the null environment as well as the 
// model builder data collections
void UI_ModelBuilder::RemoveTerrain(StaticTerrain *terrain)
{
	// Make sure the terrain object and container environment are valid
	if (!terrain || !m_nullenv) return;

	// Remove from the environment terrain collection; this will also deallocate the terrain object
	m_nullenv->RemoveTerrainObject(terrain);
	
	// Remove the terrain object it from the central modelbuilder collection; it has already been 
	// deallocated upon removal from the environment
	int index = FindTerrainIndex(terrain);
	if (index >= 0 && index < (int)m_terrain.size() && m_terrain[index] != NULL)
	{
		//SafeDelete(m_terrain[index]);
		m_terrain[index] = NULL;
		m_terrain.erase(m_terrain.begin() + index);
	}

	// Perform a full update of the collision geometry based on this change
	RefreshAllCollisionGeometry();
}

// Performs a full update of the model collision geometry, including OBBs & terrain objects.  Called whenever objects are 
// added, removed, moved, resized or rotated
void UI_ModelBuilder::RefreshAllCollisionGeometry(void)
{
	// Record whether we currently have an OBB/terrain object selected
	int selected = m_colldata_selected;

	// Collision OBB hierarchy - no refresh actions needed at this point

	// Terrain objects - Add all terrain objects to the null environment
	if (m_nullenv)
	{
		// First, remove all current references from the element.  Remove references only; we don't want
		// to actually delete the terrain objects like normal since they are mastered in the modelbuilder
		//m_nullenv->ClearAllTerrainObjects();
		m_nullenv->TerrainObjects.clear();

		// Now add all terrain objects to the environment
		TerrainCollection::size_type n = m_terrain.size();
		for (TerrainCollection::size_type i = 0; i < n; ++i) m_nullenv->AddTerrainObject(m_terrain[i]);
	}

	// Restore the previously-selected item (will handle cases where that item no longer exists)
	CollisionData_Clicked(selected);
}

// Add a new terrain object to the end of the terrain vector, using the unique code specified in the relevant text box
void UI_ModelBuilder::AddNewTerrain(void)
{
	// Terrain will default to use a null model (i.e. pure collision volume) if no code is specified
	std::string code = NullString;

	// Get a reference to the model name text field
	UITextBox *tb = (UITextBox*)m_render->FindUIComponent("txt_terraincode", NullString);
	if (tb) code = tb->GetText();
	
	// Create a new static terrain object with this definition (or NULL) and default parameters
	StaticTerrain *terrain = StaticTerrain::Create(code);

	// Inherit the orientation of the selected terrain if appropriate
	if (m_key_ctrl && m_selection_type == UI_ModelBuilder::MVSelectionType::TerrainSelected && m_selected_terrain != NULL)
		terrain->SetOrientation(m_selected_terrain->GetOrientation());
	
	// If the shift key is held then inherit the extents of the currently selected terrain object
	if (m_key_shift && m_selection_type == UI_ModelBuilder::MVSelectionType::TerrainSelected && m_selected_terrain != NULL)
		terrain->SetExtent(XMLoadFloat3(&m_selected_terrain->GetExtentF()));
	
	// Add this terrain object to the model
	AddTerrain(terrain);

	// Force a refresh of the interface after the new terrain object is added 
	RefreshInterface();
}

// Remove the currently-selected object 
void UI_ModelBuilder::RemoveSelectedObject(void)
{
	// Get a reference to the currently-selected object; if nothing is selected, quit immediately
	if (m_colldata_selected == -1 || m_selection_type == UI_ModelBuilder::MVSelectionType::NothingSelected ||
		(m_selection_type == UI_ModelBuilder::MVSelectionType::OBBSelected && !m_selected_obb) ||
		(m_selection_type == UI_ModelBuilder::MVSelectionType::TerrainSelected && !m_selected_terrain)) return;

	// Take different action depending on the type of object
	if (m_selection_type == UI_ModelBuilder::MVSelectionType::OBBSelected)
	{
		// Delete the selected OBB; this and all recursive deallocation is handled by the OBB object
		OrientedBoundingBox *obb = GetOBBByIndex(m_colldata_selected); if (!obb) return;
		OrientedBoundingBox *parent = FindOBBParent(obb);
		if (parent == NULL)
		{
			// If the parent is NULL then this is the root node, so clear the entire collision OBB
			m_object->CollisionOBB.Clear();
		}
		else
		{
			// Get the index of this child, then remove from the parent's child collection
			int childindex = FindOBBChildIndex(parent, obb);
			if (childindex != -1)
			{
				parent->RemoveChildNode(childindex);
			}
		}

		// Refresh the interface and viewer to reflect this change
		RefreshInterface();
	}
	else if (m_selection_type == UI_ModelBuilder::MVSelectionType::TerrainSelected)
	{
		// Remove this terrain object, then refresh the viewer and interface to reflect the change
		RemoveTerrain(m_selected_terrain);
		RefreshInterface();
	}

	// Deselect everything once the object has been removed
	SelectOBB(NULL);
}

// Handles the event where the user is clicking on the collision data listing.  Accepts the point which was clicked as input
void UI_ModelBuilder::CollisionData_Clicked(INTVECTOR2 location)
{
	// Get the line (if any) within this control that was clicked.  Returns -1 if no line was clicked
	INTVECTOR2 local = (location - m_collisiondata->GetPosition());
	int line = m_collisiondata->GetLineAtLocation(local);

	// Pass to the overloaded method with this line index
	CollisionData_Clicked(line);
}

// Handles the event where the user is clicking on the collision data listing.  Accepts the line that has been clicked as input
void UI_ModelBuilder::CollisionData_Clicked(int line)
{
	// Parameter check
	if (line < 0 || line >= m_collisiondata->GetLineCount()) line = -1;

	// Store the currently-selected line
	m_colldata_selected = line;

	// If the user has not clicked on any line then deselect all objects
	if (line == -1) SelectOBB(NULL);
	const std::string & text = m_collisiondata->GetText(line);
	if (text == "") SelectOBB(NULL);

	// Determine the item that was clicked; OBB lines appear first, in hierarchy order, followed by terrain objects in vector order
	// Use the cached vector of references that was recorded during RefreshInterface() to select the correct item
	int terraincount = (int)m_terrain.size();
	int obbcount = (m_colldata_itemcount - terraincount);
	if (line < obbcount)
	{
		// This is an OBB line; determine which item by its position in the hierarchy
		int obbindex = line;
		if (obbindex < 0 || obbindex >= obbcount) return;						// Sanity check; should never happen

		// Attempt to retrieve the obb at this index
		OrientedBoundingBox *obb = GetOBBByIndex(obbindex);
		if (!obb) return;														// Sanity check; should never happen
	
		// Set this OBB to be the selected object
		SelectOBB(obb);
	}
	else
	{
		// This is a terrain line; determine which item based on index
		int terrainindex = (line - obbcount);
		if (terrainindex < 0 || terrainindex >= terraincount) return;			// Sanity check; should never happen

		// Highlight this terrain object in the viewer
		StaticTerrain *terrain = m_terrain[terrainindex];
		if (!terrain) return;

		// Set this terrain object to be the selected item
		SelectTerrain(terrain);
	}

}

// Select an OBB object in the viewer
void UI_ModelBuilder::SelectOBB(OrientedBoundingBox *obb)
{
	// OBB can be NULL in order to deselect everything
	if (!obb)
	{
		m_selection_type = UI_ModelBuilder::MVSelectionType::NothingSelected;
		m_selected_obb = NULL;
		m_selected_terrain = NULL;
		m_selection_transform = ID_MATRIX;
		return;
	}

	// Otherwise, set the selection parameters
	m_selection_type = UI_ModelBuilder::MVSelectionType::OBBSelected;
	m_selected_obb = obb;
	m_selected_terrain = NULL;
}

// Select a terrain object in the viewer
void UI_ModelBuilder::SelectTerrain(StaticTerrain *terrain)
{
	// Terrain object can be NULL in order to deselect everything
	if (!terrain)
	{
		m_selection_type = UI_ModelBuilder::MVSelectionType::NothingSelected;
		m_selected_obb = NULL;
		m_selected_terrain = NULL;
		return;
	}

	// Otherwise, set the selection parameters
	m_selection_type = UI_ModelBuilder::MVSelectionType::TerrainSelected;
	m_selected_obb = NULL;
	m_selected_terrain = terrain;
}

// Move the selected object
void UI_ModelBuilder::MoveSelection(FXMVECTOR mv)
{
	// Determine the amount of movement required, based upon the base movement speed and current time delta
	XMVECTOR move = XMVectorScale(mv, UI_ModelBuilder::SELECTION_MOVE_SPEED * Game::PersistentTimeFactor);

	// Move the currently-selected object
	if (m_selection_type == UI_ModelBuilder::MVSelectionType::OBBSelected && m_selected_obb)
	{
		// Generate a delta translation matrix and apply it to the current object offset matrix
		XMMATRIX offset = XMMatrixMultiply(XMMatrixTranslationFromVector(move), m_selected_obb->Offset);

		// Assign this new offset matrix and update all the way down the OBB hierarchy
		m_selected_obb->UpdateOffset(offset);
	}
	else if (m_selection_type == UI_ModelBuilder::MVSelectionType::TerrainSelected && m_selected_terrain)
	{
		// Update the terrain object relative position by this delta, which will recalculate all derived fields automatically
		m_selected_terrain->SetPosition(XMVectorAdd(m_selected_terrain->GetPosition(), move));
	}

	// Perform a full update of the collision geometry based on this change
	//RefreshAllCollisionGeometry();
}

// Resize the selected object
void UI_ModelBuilder::ResizeSelection(FXMVECTOR rsz)
{
	// Determine the amount of resizing required, based upon the base resize speed and current time delta
	XMVECTOR resize = XMVectorScale(rsz, UI_ModelBuilder::SELECTION_RESIZE_SPEED * Game::PersistentTimeFactor);

	// Resize the currently-selected object
	if (m_selection_type == UI_ModelBuilder::MVSelectionType::OBBSelected && m_selected_obb)
	{
		// Update the extent of this OBB, including bounds checking
		XMVECTOR extent = XMVectorMax(XMVectorAdd(XMLoadFloat3(&m_selected_obb->Data().ExtentF), resize), Game::C_EPSILON_V);
		m_selected_obb->UpdateExtent(extent);
	}
	else if (m_selection_type == UI_ModelBuilder::MVSelectionType::TerrainSelected && m_selected_terrain)
	{
		// Update the extent of this terrain object, including bounds checking
		XMVECTOR extent = XMVectorMax(XMVectorAdd(XMLoadFloat3(&m_selected_terrain->GetExtentF()), resize), Game::C_EPSILON_V);
		m_selected_terrain->SetExtent(extent);
	}

	// Perform a full update of the collision geometry based on this change
	RefreshAllCollisionGeometry();
}

// Rotate the selected object about the specified axis
void UI_ModelBuilder::RotateSelection(FXMVECTOR axis)
{
	// Take different action depending on the type of object that is selected
	if (m_selection_type == UI_ModelBuilder::MVSelectionType::OBBSelected && m_selected_obb)
	{
		// Generate a delta rotation matrix about the specified axis, plus centre- and inverse-centre translation matrices
		XMMATRIX delta = XMMatrixRotationAxis(axis, UI_ModelBuilder::SELECTION_ROTATE_SPEED * Game::PersistentTimeFactor);

		// Assign this new offset matrix and update all the way down the OBB hierarchy
		XMMATRIX offset = XMMatrixMultiply(delta, m_selected_obb->Offset);
		m_selected_obb->UpdateOffset(offset);
	}
	else if (m_selection_type == UI_ModelBuilder::MVSelectionType::TerrainSelected && m_selected_terrain)
	{
		// Generate a quaternion for this delta rotation about the specified axis
		XMVECTOR delta = XMQuaternionRotationAxis(axis, UI_ModelBuilder::SELECTION_ROTATE_SPEED * Game::PersistentTimeFactor);

		// Apply this delta rotation to the terrain orientation and apply it
		XMVECTOR orient = XMQuaternionMultiply(m_selected_terrain->GetOrientation(), delta);
		m_selected_terrain->SetOrientation(orient);
	}
}

// Revert the currently-selected object to the identity orientation
void UI_ModelBuilder::RevertSelectionOrientation()
{
	if (m_selection_type == UI_ModelBuilder::MVSelectionType::OBBSelected && m_selected_obb)
	{
		// If this is an OBB object, replace its offset matrix with a direct translation to its current 
		// position (thereby removing any rotation component)

		// Transform both the object & OBB positions back into the same local space, then determine the translation between them
		XMVECTOR transformedpos = XMVector3TransformCoord(m_object->GetPosition(), m_object->GetInverseWorldMatrix());
		XMVECTOR transformedcentre = XMVector3TransformCoord(m_selected_obb->Data().Centre, m_object->GetInverseWorldMatrix());
		XMVECTOR trans = XMVectorSubtract(transformedcentre, transformedpos);

		// Replace the OBB offset matrix with this direct transform, effectively removing any rotation components
		m_selected_obb->UpdateOffset(XMMatrixTranslationFromVector(trans));
	}
	else if (m_selection_type == UI_ModelBuilder::MVSelectionType::TerrainSelected && m_selected_terrain)
	{
		// If this is a terrain object, simply reset it's orientation component and allow it to recalculate everything itself
		m_selected_terrain->SetOrientation(ID_QUATERNION);
	}
}

// Retrieves a pointer to the OBB at the specified index, based on a flattening of the model OBB hierarchy and traversing sequentially
OrientedBoundingBox *UI_ModelBuilder::GetOBBByIndex(int index) 
{
	// Parameter check
	if (index < 0 || !m_object) return NULL;

	// Traverse the OBB hierarchy and return the node at this index, or NULL if the index is not valid
	// Always start at index+1, since the recursive function will --index as the first step
	return TraverseOBBHierarchy(&(m_object->CollisionOBB), ++index);
}

// Retrieves a pointer to the parent of the specified OBB, or NULL if the OBB is a root, based on a flattening 
// of the model OBB hierarchy and traversing sequentially
OrientedBoundingBox *UI_ModelBuilder::FindOBBParent(OrientedBoundingBox *obb)
{
	// Parameter check
	if (!obb || !m_object) return NULL;

	// Traverse the OBB hierarchy and return the node at this index, or NULL if the index is not valid
	return TraverseHierarchyToFindOBBParent(&(m_object->CollisionOBB), obb);
}

// Finds the index of the supplied child within the node child collection.  Returns -1 if it is not a child of the node
int UI_ModelBuilder::FindOBBChildIndex(OrientedBoundingBox *obb, OrientedBoundingBox *child)
{
	// Parameter check
	if (!obb || !child) return -1;

	// Simply check each child in turn to see whether it is the target child
	for (int i = 0; i < obb->ChildCount; ++i)
	{
		if (&(obb->Children[i]) == child) return i;
	}

	// Child was not found below this node
	return -1;
}

// Finds a terrain object in the terrain vector and returns its index.  Returns -1 if the terrain does not exist
int UI_ModelBuilder::FindTerrainIndex(StaticTerrain *terrain)
{
	if (!terrain) return -1;
	return FindIndexInVector<TerrainCollection, StaticTerrain*>(m_terrain, terrain);
}

// Private recursive method to traverse the OBB hierarchy
OrientedBoundingBox * UI_ModelBuilder::TraverseOBBHierarchy(OrientedBoundingBox *node, int & outIndex)
{
	// If the index has reached zero (decremented at each OBB) then we want to return this node
	if (--outIndex == 0) return node;

	// Make sure the node and index are valid (and if not, return NULL as an error)
	if (!node || outIndex < 0) return NULL;

	// Now move into each child node in turn, and return once the index reaches zero
	OrientedBoundingBox *obb;
	for (int i = 0; i < node->ChildCount; ++i)
	{
		obb = TraverseOBBHierarchy(&(node->Children[i]), outIndex);
		if (outIndex == 0) return obb;
	}

	// Return null since we did not find this node (it must have an index greater than the total count of all nodes)
	return NULL;
}

// Private recursive method to traverse the OBB hierarchy and locate the parent of the specified OBB, or NULL if the OBB is root
OrientedBoundingBox * UI_ModelBuilder::TraverseHierarchyToFindOBBParent(OrientedBoundingBox *node, OrientedBoundingBox *searchtarget)
{
	// Parameter check; we are only interested in child nodes
	if (!searchtarget || !node || !node->Children) return NULL;

	// Check each child node in turn; if it is the one we are searching for, return this node as the parent.  Otherwise
	// move recursively into its children
	for (int i = 0; i < node->ChildCount; ++i)
	{
		// Check whether this is the target
		if (&(node->Children[i]) == searchtarget) return node;

		// If not, and if the node has children, move recursively into it now and continue searching
		if (node->ChildCount > 0)
		{
			OrientedBoundingBox *result = TraverseHierarchyToFindOBBParent(&(node->Children[i]), searchtarget);
			if (result) return result;
		}
	}

	// We did not find the target so return NULL
	return NULL;
}

// Zooms the view by the specified amount
void UI_ModelBuilder::PerformZoom(float zoom)
{
	// The player viewpoint is fixed at (0,0,0) and the model is offset along the positive z-axis.  Generate
	// a vector that will translate the model closer/further from the camera, accounting for zoom limits
	m_zoomlevel = max(UI_ModelBuilder::MIN_ZOOM_LEVEL, m_zoomlevel + zoom);

	// Simply set the model z-axis position to this zoom level
	m_object->SetPosition(XMVectorSetZ(NULL_VECTOR, m_zoomlevel));
}

// Processes the user request to load a new model
Result UI_ModelBuilder::LoadModel_ButtonClicked(void)
{
	// Get a reference to the model name text field
	UITextBox *tb = (UITextBox*)m_render->FindUIComponent("txt_modelcode", NullString);
	if (!tb) return ErrorCodes::CannotGetHandleToModelViewerUIComponent;

	// Retrieve the model code that we are trying to load and make sure it is valid
	std::string code = tb->GetText();
	if (code == NullString || Model::GetModel(code) == NULL) return ErrorCodes::CannotLoadModelWithInvalidCodeToViewer;

	// Remove all previous model data before attempting to load the new model
	ClearCurrentModel();

	// Also remove all collision data for this new model
	ClearCollisionData();

	// Attempt to load this new model in the viewer and return the result
	return LoadModel(code);
}

// Processes the user request to save the current model
Result UI_ModelBuilder::SaveModel_ButtonClicked(void)
{
	MultiLineTextBlock *tb = (MultiLineTextBlock*)m_render->FindUIComponent("mltb_test", NullString);
	if (!tb) return 1;

	if (tb->GetOperationMode() == MultiLineTextBlock::OperationMode::IndividualLines)
	{
		for (int i = 0; i < 10; ++i)
			tb->SetText(i, concat("This is line ")(i).str());
	}
	else if (tb->GetOperationMode() == MultiLineTextBlock::OperationMode::WordWrap)
	{
		tb->SetText("This is a test block of text that should exceed the 32x5 character limit that is imposed.  Note that the component does not wrap text on word boundaries at the moment, rather at whichever character is at the line limit.  Should 'walk back' from that limit to the nearest word separator to correctly wrap text");
	}

	return 1;
}


// Button event handler to load collision data
Result UI_ModelBuilder::LoadCollData_ButtonClicked(void)
{
	// Get a reference to the collision data text field
	UITextBox *tb = (UITextBox*)m_render->FindUIComponent("txt_colldata", NullString);
	if (!tb) return ErrorCodes::CannotGetHandleToModelViewerUIComponent;

	// Retrieve the filename that we are trying to load and make sure it is valid
	std::string name = tb->GetText();
	if (name == NullString) return ErrorCodes::CannotLoadNullCollisionDataFile;

	// Check whether this file exists
	std::string filename = concat(D::DATA)("\\ModelBuilder\\")(name)(".xml").str();
	if (!FileSystem::FileExists(filename.c_str())) return ErrorCodes::CollisionDataFileDoesNotExist;

	// Attempt to load this collision data
	return LoadCollisionData(filename);
}

// Button event handler to save collision data
Result UI_ModelBuilder::SaveCollData_ButtonClicked(void)
{
	Result result;

	// Get a reference to the collision data text field
	UITextBox *tb = (UITextBox*)m_render->FindUIComponent("txt_colldata", NullString);
	if (!tb) return ErrorCodes::CannotGetHandleToModelViewerUIComponent;

	// Retrieve the filename that we are trying to save and make sure it is valid
	std::string name = tb->GetText();
	if (name == NullString) return ErrorCodes::CannotSaveNullCollisionDataFile;
	std::string filename = concat(D::DATA)("\\ModelBuilder\\")(name)(".xml").str();

	// Create a new root data node for this collision data
	TiXmlElement *root = IO::Data::NewGameDataXMLNode();

	// First, generate XML for the OBB hierarchy and add it below this root node
	if (m_object)
	{
		result = IO::Data::SaveCollisionOBB(root, &(m_object->CollisionOBB));
		if (result != ErrorCodes::NoError) return result;
	}

	// Now generate XML entries for each terrain object in turn
	TerrainCollection::size_type n = m_terrain.size();
	for (TerrainCollection::size_type i = 0; i < n; ++i)
	{
		if (m_terrain[i])
		{
			result = IO::Data::SaveStaticTerrain(root, m_terrain[i]);
			if (result != ErrorCodes::NoError) return result;
		}
	}

	// Save the root node with all its contents to a local file with the specified filename
	return IO::Data::SaveXMLDocument(root, filename);
}

// Clear all collision data
void UI_ModelBuilder::ClearCollisionData(void)
{
	// Remove all collision OBB data for the model
	if (m_object) m_object->CollisionOBB.Clear();

	// Clear any terrain objects stored in the null placeholder environment, before we later remove from the
	// central UI controller collection and deallocate them.  Remove references only; we don't want to 
	// delete the objects here since they are mastered by the modelbuilder
	//m_nullenv->ClearAllTerrainObjects();
	m_nullenv->TerrainObjects.clear();

	// Delete any terrain objects that have been created, then clear the vector
	TerrainCollection::size_type n = m_terrain.size();
	for (TerrainCollection::size_type i = 0; i < n; ++i) if (m_terrain[i]) SafeDelete(m_terrain[i]);
	m_terrain.clear();

	// Clear any reference to currently selected objects, since these are no longer valid
	m_selection_type = UI_ModelBuilder::MVSelectionType::NothingSelected;
	m_selected_obb = NULL;
	m_selected_terrain = NULL;

	// Refresh the object list once all data has been cleared
	RefreshInterface();

	// Perform a full update of the collision geometry based on this change
	RefreshAllCollisionGeometry();
}

// Generates a root OBB to fit the current model, whether or not one already exists
void UI_ModelBuilder::GenerateRootOBB(void)
{
	// Force a recalculation of the model size data, which will generate a new root OBB
	if (m_object)
	{
		m_object->CollisionOBB.SetAutoFitMode(true);
		m_object->RecalculateShipDataFromCurrentState();
	}

	// Perform a full update of the collision geometry based on this change
	RefreshAllCollisionGeometry();
}

// Attempts to load collision data from file
Result UI_ModelBuilder::LoadCollisionData(const std::string & filename)
{
	// We want to clear all current data before loading a new set
	ClearCollisionData();

	// Call the data input method to load a raw XML document
	TiXmlDocument *doc = IO::Data::LoadXMLDocument(filename);
	if (!doc) return ErrorCodes::CouldNotLoadCollisionDataFromFile;

	// Get a reference to the root node
	TiXmlElement *root = doc->FirstChildElement();
	if (root == NULL) { delete doc; return ErrorCodes::CannotFindXMLRoot; }

	// Parse each element within the node in turn
	std::string name; HashVal hash;
	TiXmlElement *child = root->FirstChildElement();
	for (child; child; child = child->NextSiblingElement())
	{
		// Test the type of this node
		name = child->Value(); StrLowerC(name);
		hash = HashString(name);

		if (hash == HashedStrings::H_CollisionOBB) {
			IO::Data::LoadCollisionOBB(m_object, child, m_object->CollisionOBB, true);
		}
		else if (hash == HashedStrings::H_StaticTerrain) {
			StaticTerrain *t = IO::Data::LoadStaticTerrain(child);
			if (!t) continue;

			// Add the terrain object to the null environment container
			m_nullenv->AddTerrainObject(t);

			// Add to the vector of terrain objects
			m_terrain.push_back(t);
		}
	}

	// Update the placeholder ship data, so that if the root OBB is set to auto-calculate it will be updated based on model size
	m_object->RecalculateShipDataFromCurrentState();

	// Refresh the interface once all objects have been loaded
	RefreshInterface();

	// Perform a full update of the collision geometry based on this change
	RefreshAllCollisionGeometry();

	// Dispose of the document and return success
	if (doc) delete doc;
	return ErrorCodes::NoError;
}

// Terminates the UI layout and disposes of all relevant components
void UI_ModelBuilder::Terminate(void)
{
	// Deactivate the UI layout, if it hasn't been deactivated already
	Deactivate();
}



// Default destructor
UI_ModelBuilder::~UI_ModelBuilder(void)
{

}