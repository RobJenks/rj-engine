#pragma once

#ifndef __UI_ModelBuilderH__
#define __UI_ModelBuilderH__

#include <vector>
#include "AlignedAllocator.h"
#include "SimpleShip.h"
#include "iUIController.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class UI_ModelBuilder : public ALIGN16<UI_ModelBuilder>, public iUIController
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(UI_ModelBuilder)

	// Collection of terrain objects being added within the modelbuilder
	typedef std::vector<Terrain*, AlignedAllocator<Terrain*, 16U>> TerrainCollection;

	// Enumeration of possible selection types
	enum MVSelectionType { NothingSelected = 0, OBBSelected, TerrainSelected };

	// Static constants
	static const float SELECTION_MOVE_SPEED;		// Rate at which selected objects are moved, in units /second
	static const float SELECTION_RESIZE_SPEED;		// Rate at which selected objects are resized, in units /second
	static const float SELECTION_ROTATE_SPEED;		// Rate at which selected objects are resized, in radians /second
	static const float ZOOM_SPEED;					// Rate at which the camera zooms, as a percentage of the model collision radius
	static const float DEFAULT_ZOOM_LEVEL;			// The default starting zoom level, as a percentage of the model collision radius
	static const float MIN_ZOOM_LEVEL;				// The closest we can zoom into the object

	// Default constructor
	UI_ModelBuilder(void);

	// Initialisation method, called once the UI component has been created
	Result InitialiseController(Render2DGroup *render, UserInterface *ui);

	// Method that is called when the UI controller becomes active
	void Activate(void);

	// Method that is called when the UI controller is deactivated
	void Deactivate(void);

	// Performs a full update of the model collision geometry, including OBBs & terrain objects.  Called whenever objects are 
	// added, removed, moved, resized or rotated
	void RefreshAllCollisionGeometry(void);

	// Refreshes the model builder interface based on current state
	void RefreshInterface(void);

	// Determines the correct orientation for the model based on current mouse events
	void UpdateModelOrientation(const INTVECTOR2 & startlocation, const INTVECTOR2 & currentlocation);

	// Clears all current model data
	void ClearCurrentModel(void);

	// Returns the model to its original orientation
	CMPINLINE void RevertModelOrientation(void) 
	{ 
		m_revertingmodelrotation = true; 
		m_reverttimeremaining = UI_ModelBuilder::ORIENTATION_REVERT_TIME;
	}

	// Revert the currently-selected object to the identity orientation
	void RevertSelectionOrientation();

	// Button event handler methods
	Result LoadModel_ButtonClicked(void);
	Result SaveModel_ButtonClicked(void);
	Result LoadCollData_ButtonClicked(void);
	Result SaveCollData_ButtonClicked(void);

	// Handles the event where the user is clicking on the collision data listing.  Accepts the point which was clicked as input
	void CollisionData_Clicked(INTVECTOR2 location);

	// Handles the event where the user is clicking on the collision data listing.  Accepts the line that has been clicked as input
	void CollisionData_Clicked(int line);

	// Retrieves a pointer to the OBB at the specified index, based on a flattening of the model OBB hierarchy and traversing sequentially
	OrientedBoundingBox *GetOBBByIndex(int index);

	// Retrieves a pointer to the parent of the specified OBB, or NULL if the OBB is a root, based on a flattening 
	// of the model OBB hierarchy and traversing sequentially
	OrientedBoundingBox *FindOBBParent(OrientedBoundingBox *obb);

	// Finds the index of the supplied child within the node child collection.  Returns -1 if it is not a child of the node
	int FindOBBChildIndex(OrientedBoundingBox *obb, OrientedBoundingBox *child);

	// Finds a terrain object in the terrain vector and returns its index.  Returns -1 if the terrain does not exist
	int FindTerrainIndex(Terrain *terrain);

	// Select either an OBB or terrain object in the viewer
	void SelectOBB(OrientedBoundingBox *obb);
	void SelectTerrain(Terrain *terrain);

	// Methods to add NEW objects, or remove existing objects, in the viewer
	void AddNewOBB(void);
	void AddNewTerrain(void);
	void RemoveSelectedObject(void);

	// Adds or removes a terrain object from the model.  Called by AddNewTerrain() and RemoveSelectedObject(), 
	// as well as e.g. the methods to load collision data from file.  Ensures terrain is added/removed correctly
	// from the null environment as well as the model builder data collections
	void AddTerrain(Terrain *terrain);
	void RemoveTerrain(Terrain *terrain);

	// Loads a model into the viewer
	Result LoadModel(const std::string & code);

	// Clear all collision data
	void ClearCollisionData(void);

	// Generates a root OBB to fit the current model, whether or not one already exists
	void GenerateRootOBB(void);

	// Attempts to load collision data from file
	Result LoadCollisionData(const std::string & filename);

	// (Re)Create the ships used as placeholders, e.g. for the player view or as the model placeholder
	void CreateScenarioObjects(void);

	// Methods to modify the selected object
	void MoveSelection(FXMVECTOR mv);
	void ResizeSelection(FXMVECTOR rsz);
	void RotateSelection(FXMVECTOR axis);

	// Terminates the UI layout and disposes of all relevant components
	void Terminate(void);

	// Shut down and deallocate the models/ships used in this scenario
	void ShutdownScenarioObjects(void);

	// Method to process user input into the active UI controller
	void ProcessUserEvents(GameInputDevice *keyboard, GameInputDevice *mouse);
	void ProcessKeyboardInput(GameInputDevice *keyboard);

	// Methods to accept mouse events from the UI manager
	void ProcessMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component) { }

	void ProcessRightMouseDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessRightMouseFirstDownEvent(INTVECTOR2 location, Image2DRenderGroup::InstanceReference component) { }
	void ProcessRightMouseUpEvent(INTVECTOR2 location, INTVECTOR2 startlocation, Image2DRenderGroup::InstanceReference component);

	// Methods to accept generic mouse click events at the specified location
	void ProcessMouseClickAtLocation(INTVECTOR2 location);
	void ProcessRightMouseClickAtLocation(INTVECTOR2 location) { }

	// Methods to accept the processed mouse click events for particular components
	void ProcessMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation) { }
	void ProcessRightMouseClickEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, INTVECTOR2 startlocation) { }

	// Methods to accept the processed mouse click events for managed components, e.g. buttons
	void ProcessControlClickEvent(iUIControl *control);
	void ProcessControlRightClickEvent(iUIControl *control) { }

	// Process button click events in the UI
	void ProcessButtonClickEvent(UIButton *button);

	// Methods to accept other managed control events
	void ProcessTextboxChangedEvent(iUIControl *control) { }

	// Method to accept mouse move events, and also mouse hover events for specific components
	void ProcessMouseMoveEvent(INTVECTOR2 location);
	void ProcessMouseHoverEvent(Image2DRenderGroup::InstanceReference component, INTVECTOR2 location, bool lmb, bool rmb) { }

	// Methods to process specific events raised from individual controls, and routed through the UserInterface
	void ComboBox_SelectedIndexChanged(UIComboBox *control, int selectedindex, int previousindex) { }

	// Zoom the view in or out
	CMPINLINE void ZoomIn(void)								{ PerformZoom(-GetZoomSpeedPerSecond() * Game::PersistentTimeFactor); }
	CMPINLINE void ZoomOut(void)							{ PerformZoom(+GetZoomSpeedPerSecond() * Game::PersistentTimeFactor); }
	CMPINLINE float GetZoomLevel(void) const				{ return m_zoomlevel; }
	CMPINLINE float GetZoomSpeedPerSecond(void) const		{ return (m_object ? (m_object->GetCollisionSphereRadius() * ZOOM_SPEED) : 1.0f); }
	CMPINLINE float GetDefaultZoomLevel(void) const			{ return (m_object ? (m_object->GetCollisionSphereRadius() * DEFAULT_ZOOM_LEVEL) : MIN_ZOOM_LEVEL); }

	// Method to perform rendering of the UI controller components (excluding 2D render objects, which will be handled by the 2D render manager)
	void Render(void);

	// Renders the current user selection
	void RenderCurrentSelection(void);

	// Updates the model each frame
	void UpdateModel(void);

	// Default destructor
	~UI_ModelBuilder(void);

	// Constants for use in this UI component
	static const float						MOUSE_DRAG_DISTANCE_MODIFIER;
	static const float						MOUSE_DRAG_ROTATION_SPEED;

protected:

	// The (null) system that the viewer operates in
	SpaceSystem *							m_system;

	// Placeholder objects for the player view and other editor-specific objects
	SimpleShip *							m_playerviewpoint;
	ComplexShip *							m_nullenv;
	LightSource *							m_lightsource;

	// A custom SimpleShip will be used to display the model
	SimpleShip *							m_object;

	// Vector of terrain objects added to the model
	TerrainCollection						m_terrain;

	// References to key UI controls
	MultiLineTextBlock *					m_collisiondata;
	int										m_colldata_itemcount;
	int										m_colldata_selected;

	// Shortcuts to store user input information for the current frame
	bool									m_key_shift, m_key_ctrl;

	// Parameters that indicate if and how we are rotating the model
	bool									m_rotatingmodel;
	float									m_rotate_yaw, m_rotate_pitch;

	// Fields required to revert the model orientation back to original state
	bool									m_revertingmodelrotation;
	float									m_reverttimeremaining;
	static const float						ORIENTATION_REVERT_TIME;

	// Private recursive method to traverse the OBB hierarchy
	OrientedBoundingBox *					TraverseOBBHierarchy(OrientedBoundingBox *node, int & outIndex);

	// Private recursive method to traverse the OBB hierarchy and locate the parent of the specified OBB, or NULL if the OBB is root
	OrientedBoundingBox *					TraverseHierarchyToFindOBBParent(OrientedBoundingBox *node, OrientedBoundingBox *searchtarget);

	// Performs a recursive traversal of the OBB hierarchy, building a vector of node names as it goes
	void									BuildOBBHierarchyItemList(	OrientedBoundingBox *node, const std::string & parent_name, int child_index,
																		std::vector<std::string> & refItemList);

	// Zooms the view by the specified amount
	float									m_zoomlevel;
	void									PerformZoom(float zoom);

	// Selection parameters
	MVSelectionType							m_selection_type;
	OrientedBoundingBox *					m_selected_obb;
	Terrain *							m_selected_terrain;
	AXMMATRIX								m_selection_transform;

	// Store the prior state of certain key variables, in order to restore them when the model viewer is closed
	bool									m_restore_obbrender;
	bool									m_restore_terrainrender;
	Game::ID_TYPE							m_restore_terrainenvironment;
};


#endif