#pragma once

#ifndef __ComplexShipH__
#define __ComplexShipH__

#include <vector>
#include "CompilerSettings.h"
#include "Octree.h"
#include "iSpaceObjectEnvironment.h"
#include "iContainsComplexShipTiles.h"
#include "FastMath.h"
#include "Ship.h"
#include "Utility.h"
class iSpaceObject;
class ComplexShipSection;
class CapitalShipPerimeterBeacon;
class NavNetwork;
class StaticTerrain;

class ComplexShip : public iSpaceObjectEnvironment
{
public:
	
	// Typedefs for this object
	typedef std::vector<ComplexShipSection*>			ComplexShipSectionCollection;
	typedef std::vector<CapitalShipPerimeterBeacon*>	PerimeterBeaconCollection;

	// Constructor / destructor
	ComplexShip(void);
	~ComplexShip(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void										InitialiseCopiedObject(ComplexShip *source);

	// Methods to add/remove/retrieve the sections that make up this complex ship
	ComplexShipSection *						GetSection(int index);
	CMPINLINE ComplexShipSectionCollection *	GetSections(void) { return &(m_sections); }
	CMPINLINE int								GetSectionCount(void) { return m_sections.size(); }
	Result										AddShipSection(ComplexShipSection *section);
	void										RemoveShipSection(ComplexShipSection *section);
	void										RemoveShipSection(int index);

	// Methods triggered based on major events impacting this object
	void										ShipTileAdded(ComplexShipTile *tile);		// When a tile is added.  Virtual inherited from interface.
	void										ShipTileRemoved(ComplexShipTile *tile);		// When a tile is removed.  Virtual inherited from interface.
	void										ElementLayoutChanged(void);					// When the layout (e.g. active/walkable state, connectivity) of elements is changed

	// Overrides the iSpaceObject method to ensure that all ship sections are also moved into the environment along with the 'ship' itself
	void										MoveIntoSpaceEnvironment(SpaceSystem *system, const D3DXVECTOR3 & location);

	// Builds the complex ship hardpoint collection based on its constituent ship sections
	void										BuildHardpointCollection(void);

	// Copies all tiles from another object and adds the copies to this object
	Result										CopyTileDataFromObject(iContainsComplexShipTiles *src);

	// Fits the element space around this ship, eliminating any extra space allocated outside of the (cuboid) bounds it requires
	Result										FitElementSpaceToShip(void);

	// Returns a reference to the ship section that contains the element at this location
	ComplexShipSection *						GetShipSectionContainingElement(INTVECTOR3 location);

	// Return the minimum & maximum bounds actually occupied by this ship (can be =/= elementsize when sections are being changed)
	INTVECTOR3									GetShipMinimumBounds(void);
	INTVECTOR3									GetShipMaximumBounds(void);

	// Get a reference to the navigation network assigned to this ship
	CMPINLINE NavNetwork *						GetNavNetwork(void)				{ return m_navnetwork; }

	// Delete or simply remove the nav network. Removing the link will leave the network itself intact, just unliked - for when we 
	// copy objects and want to retain original ship's network
	void										ShutdownNavNetwork(void);
	CMPINLINE void								RemoveNavNetworkLink(void)		{ m_navnetwork = NULL; }

	// Updates the ship navigation network based on the set of elements and their properties
	void										UpdateNavigationNetwork(void);

	// Methods to update life support-related properties of the ship; we set flags that force an update next cycle
	CMPINLINE void								UpdateGravity(void)			{ m_gravityupdaterequired = true; }
	CMPINLINE void								UpdateOxygenLevels(void)	{ m_oxygenupdaterequired = true; }

	// Methods to get and set the ship designer offset, determining where the ship will appear within the SD
	INTVECTOR3									GetSDOffset(void) { return m_sdoffset; }
	void										SetSDOffset(INTVECTOR3 offset) { m_sdoffset = offset; }

	// Methods to set and check the flag that determines whether a ship has been directly generated from the SD
	bool							HasBeenDirectlyGeneratedFromSD(void)				{ return m_directlygeneratedfromSD; }
	void							FlagShipAsDirectlyGeneratedFromShipDesigner(bool b)	{ m_directlygeneratedfromSD = b; }

	// Suspend or resume updates based on changes to the ship.  This relates to structural changes, e.g. the addition
	// or removal of ship sections.  Likely only required during first-time initialisation of the object
	CMPINLINE void								SuspendUpdates(void)		{ m_suspendupdates = true; }
	CMPINLINE void								ResumeUpdates(void)			
	{ 
		// Clear the flag
		m_suspendupdates = false; 

		// Perform a full rebuild of structural objects, e.g. hardpoints
		BuildHardpointCollection();

		// Perform an update of ship statistics based on the current configuration
		RecalculateAllShipData();
	}

	// Generats the set of capital ship perimeter beacons used for navigation and collision avoidance
	void										GenerateCapitalShipPerimeterBeacons(void);

	// Method to attach a capital ship perimeter beacon to the ship at the specified location
	void										AttachCapitalShipBeacon(D3DXVECTOR3 position);
	CMPINLINE const PerimeterBeaconCollection *	GetPerimeterBeaconCollection(void) { return &m_perimeterbeacons; }

	// Recalculates the ship position.  Extends on the method of the base Ship class
	void							SimulateObject(void);

	// Virtual method implementation from iObject to handle a change in simulation state.  We are guaranteed that prevstate != newstate
	// Further derived classes (e.g. ships) can implement this method and then call ComplexShip::SimulationStateChanged() to maintain the chain
	void				SimulationStateChanged(ObjectSimulationState prevstate, ObjectSimulationState newstate);


	// Recalculates all properties, hardpoints & statistics of the ship.  Called once the ship has been created
	void				RecalculateAllShipData(void);

	// Recalculates the ship statistics based on its current state & loadout.  Called when the ship state changes during operation
	void				RecalculateShipDataFromCurrentState(void);


	// Methods to recalculate properties of this ship based on its configuration and current state
	void				CalculateShipSizeData(void);		// Recalculates ship size data by combining component section data
	void				CalculateShipMass();				// Recalculate the total ship mass based on all contributing factors
	void				CalculateVelocityLimits();			// Recalculates velocity limits based on all contributing factors
	void				CalculateBrakeFactor();				// Recalculates ship brake factor based on all factors.  Dependent on velocity limit
	void				CalculateTurnRate();				// Recalculates the overall turn rate based on all contributing factors
	void				CalculateBankRate();				// Recalculates the rate at which the ship will bank on turning
	void				CalculateBankExtents();				// Recalculates the maximum extent in each dimension that the ship can bank
	void				CalculateEngineStatistics(void);	// Recalculates the ship data derived from its engine capabilities

	// Methods to force rendering of a ship interior even when it does not otherwise meet any criteria for doing so
	bool										InteriorShouldAlwaysBeRendered(void) const	{ return m_forcerenderinterior; }
	void										ForceRenderingOfInterior(bool render);

	// Terminates the ship object and deallocates all associated storage
	void										Shutdown(void);
	void										Shutdown(	bool IncludeStandardObjects, bool ShutdownSections,
															bool UnlinkTiles, bool ShutdownBeacons);

	// Static methods to create a complex ship from the supplied ship code
	static ComplexShip *						Create(const string & code);
	static ComplexShip *						Create(ComplexShip *template_ship);

	// Methods called when this object collides with another.  Virtual inheritance from iSpaceObject.  Overriden method providing a section
	// reference is the one that will be used for CS, since only the sections themselves can collide with anything
	CMPINLINE void								CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact) { }
	void										CollisionWithObject(iObject *object, ComplexShipSection *collidingsection, 
																	const GamePhysicsEngine::ImpactData & impact);

	// Removes the 'standard' flag from this ship's definition and it's sections.  Used following a copy from a standard template ship
	void										RemoveStandardComponentFlags(void);

	// Performs a text output of perimeter beacon data for debug purposes
	string										DebugOutputPerimeterBeacons(void);

	// Methods to return the standard path / filename where this ship data should be stored, if it is following the standard convention
	std::string									DetermineXMLDataPath(void);
	std::string									DetermineXMLDataFilename(void);
	std::string									DetermineXMLDataFullFilename(void);


protected:

	// The sections that make up this ship
	ComplexShipSectionCollection				m_sections;

	PerimeterBeaconCollection					m_perimeterbeacons;		// Collection of capital ship perimeter beacons for this ship
	int											m_activebeacons;		// The number of active beacons for this ship
	vector<Octree<iSpaceObject*>*>				m_activeperimeternodes;	// The nodes currently holding our ship (excluding the main m_treenode)

	bool										m_forcerenderinterior;	// Flag that determines whether the ship interior should always be rendered, regardless of the criteria

	bool										m_suspendupdates;		// Flag that suspends all updates in response to changes, until updates are resumed again

	// The navigation network that actors will use to move around this ship
	NavNetwork *								m_navnetwork;

	// Flags used to indicate whether certain ship properties need to be recalculated
	bool										m_gravityupdaterequired;
	bool										m_oxygenupdaterequired;

	// Ship designer-related fields 
	INTVECTOR3									m_sdoffset;					// Offset determines where the ship will appear when loaded into the SD
	bool										m_directlygeneratedfromSD;	// Flag that indicates the ship was directly generated from the SD



	// Private methods used to update key ship properties
	void										PerformShipGravityUpdate(void);
	void										PerformShipOxygenUpdate(void);

	// Recalculates any perimeter beacons required to maintain spatial position in the world
	void										UpdatePerimeterBeacons(void);

	// Deactivates all perimeter beacons
	void										DeactivatePerimeterBeacons(void);

	// Deallocates and clears the set of capital ship perimeter beacons attached to this ship
	void										ShutdownPerimeterBeacons(void);


};


#endif