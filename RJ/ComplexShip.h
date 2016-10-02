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
class StaticTerrain;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ComplexShip : public ALIGN16<ComplexShip>, public iSpaceObjectEnvironment
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(ComplexShip)

	// Typedefs for this object
	typedef std::vector<ComplexShipSection*>			ComplexShipSectionCollection;
	typedef std::vector<CapitalShipPerimeterBeacon*>	PerimeterBeaconCollection;

	// Constructor / destructor
	ComplexShip(void);
	~ComplexShip(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void												InitialiseCopiedObject(ComplexShip *source);

	// Methods to add/remove/retrieve the sections that make up this complex ship
	ComplexShipSection *								GetSection(ComplexShip::ComplexShipSectionCollection::size_type index);
	CMPINLINE ComplexShipSectionCollection *			GetSections(void) { return &(m_sections); }
	CMPINLINE ComplexShipSectionCollection::size_type	GetSectionCount(void) { return m_sections.size(); }
	Result												AddShipSection(ComplexShipSection *section);
	void												RemoveShipSection(ComplexShipSection *section);
	void												RemoveShipSection(std::vector<ComplexShip::ComplexShipSectionCollection>::size_type index);

	// Methods triggered based on major events impacting this object
	virtual void										TileAdded(ComplexShipTile *tile);		// When a tile is added.  Virtual inherited from interface.
	virtual void										TileRemoved(ComplexShipTile *tile);		// When a tile is removed.  Virtual inherited from interface.

	// When the layout (e.g. active/walkable state, connectivity) of elements is changed
	virtual void								ElementLayoutChanged(void);					

	// Overrides the iSpaceObject method to ensure that all ship sections are also moved into the environment along with the 'ship' itself
	virtual void								MoveIntoSpaceEnvironment(SpaceSystem *system, const FXMVECTOR location);

	// Builds the complex ship hardpoint collection based on its constituent ship sections
	void										BuildHardpointCollection(void);

	// Fits the element space around this ship, eliminating any extra space allocated outside of the (cuboid) bounds it requires
	Result										FitElementSpaceToShip(void);

	// Returns a reference to the ship section that contains the element at this location
	ComplexShipSection *						GetShipSectionContainingElement(INTVECTOR3 location);

	// Return the minimum & maximum bounds actually occupied by this ship (can be =/= elementsize when sections are being changed)
	INTVECTOR3									GetShipMinimumBounds(void);
	INTVECTOR3									GetShipMaximumBounds(void);

	// Methods to get and set the ship designer offset, determining where the ship will appear within the SD
	INTVECTOR3									GetSDOffset(void) { return m_sdoffset; }
	void										SetSDOffset(INTVECTOR3 offset) { m_sdoffset = offset; }

	// Methods to set and check the flag that determines whether a ship has been directly generated from the SD
	bool										HasBeenDirectlyGeneratedFromSD(void)				{ return m_directlygeneratedfromSD; }
	void										FlagShipAsDirectlyGeneratedFromShipDesigner(bool b)	{ m_directlygeneratedfromSD = b; }
	
	// Fade the ship to the specified alpha level
	void										FadeToAlpha(float time, float alpha, bool ignore_pause);
	CMPINLINE void								FadeToAlpha(float time, float alpha)						{ FadeToAlpha(time, alpha, false); }

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
	void										AttachCapitalShipBeacon(FXMVECTOR position);
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
	CMPINLINE void								CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact) { }
	void										CollisionWithObject(iActiveObject *object, ComplexShipSection *collidingsection, 
																	const GamePhysicsEngine::ImpactData & impact);

	// Removes the 'standard' flag from this ship's definition and it's sections.  Used following a copy from a standard template ship
	void										RemoveStandardComponentFlags(void);

	// Performs a text output of perimeter beacon data for debug purposes
	string										DebugOutputPerimeterBeacons(void);

	// Methods to return the standard path / filename where this ship data should be stored, if it is following the standard convention
	std::string									DetermineXMLDataPath(void);
	std::string									DetermineXMLDataFilename(void);
	std::string									DetermineXMLDataFullFilename(void);

	// Event triggered upon destruction of the object
	void										DestroyObject(void);

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void										ProcessDebugCommand(GameConsoleCommand & command);


protected:

	// The sections that make up this ship
	ComplexShipSectionCollection				m_sections;

	PerimeterBeaconCollection					m_perimeterbeacons;		// Collection of capital ship perimeter beacons for this ship
	int											m_activebeacons;		// The number of active beacons for this ship
	vector<Octree<iObject*>*>					m_activeperimeternodes;	// The nodes currently holding our ship (excluding the main m_treenode)

	bool										m_forcerenderinterior;	// Flag that determines whether the ship interior should always be rendered, regardless of the criteria

	bool										m_suspendupdates;		// Flag that suspends all updates in response to changes, until updates are resumed again

	// Ship designer-related fields 
	INTVECTOR3									m_sdoffset;					// Offset determines where the ship will appear when loaded into the SD
	bool										m_directlygeneratedfromSD;	// Flag that indicates the ship was directly generated from the SD

	// Recalculates any perimeter beacons required to maintain spatial position in the world
	void										UpdatePerimeterBeacons(void);

	// Deactivates all perimeter beacons
	void										DeactivatePerimeterBeacons(void);

	// Deallocates and clears the set of capital ship perimeter beacons attached to this ship
	void										ShutdownPerimeterBeacons(void);


	// Methods to recalculate properties of this ship based on its configuration and current state
	void										CalculateShipSizeData(void);		// Recalculates ship size data by combining component section data
	void										CalculateShipMass();				// Recalculate the total ship mass based on all contributing factors
	void										CalculateVelocityLimits();			// Recalculates velocity limits based on all contributing factors
	void										CalculateBrakeFactor();				// Recalculates ship brake factor based on all factors.  Dependent on velocity limit
	void										CalculateTurnRate();				// Recalculates the overall turn rate based on all contributing factors
	void										CalculateBankRate();				// Recalculates the rate at which the ship will bank on turning
	void										CalculateBankExtents();				// Recalculates the maximum extent in each dimension that the ship can bank
	void										CalculateEngineStatistics(void);	// Recalculates the ship data derived from its engine capabilities

};


#endif