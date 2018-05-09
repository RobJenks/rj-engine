#pragma once

#ifndef __ComplexShipSectionH__
#define __ComplexShipSectionH__

#include "DX11_Core.h"

#include "Utility.h"
#include "iSpaceObject.h"
#include "iContainsComplexShipTiles.h"
#include "FadeEffect.h"
#include "HighlightEffect.h"
#include "ElementStateDefinition.h"
class ComplexShipSectionDetails;
class ComplexShipElement;
class ComplexShipTile;
class ComplexShip;
class Hardpoint;
class Hardpoints;
class TextureDX11;

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class ComplexShipSection : public ALIGN16<ComplexShipSection>, public iSpaceObject
{
public:

	// Force the use of aligned allocators to distinguish between ambiguous allocation/deallocation functions in multiple base classes
	USE_ALIGN16_ALLOCATORS(ComplexShipSection)
	
	// Default constructor / destructor
	ComplexShipSection(void);
	~ComplexShipSection(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void										InitialiseCopiedObject(ComplexShipSection *source);

	// Each section belongs to a parent ComplexShip object
	CMPINLINE ComplexShip *						GetParent(void)						{ return m_parent; }
	CMPINLINE void								SetParent(ComplexShip *parent)		{ m_parent = parent; }

	// Retrieve a reference to the section hardpoints collection
	CMPINLINE const std::vector<Hardpoint*> & 	GetHardpoints(void) const			{ return m_hardpoints; }

	// Add or remove hardpoints to the section.  Updates will be triggered if they are not suspended
	void										AddHardpoint(Hardpoint *hp);
	void										RemoveHardpoint(Hardpoint *hp);

	// Clear all hardpoints.  Flag indicates whether the hardpoints should be deallocated, or simply removed from the collection
	void										ClearAllHardpoints(bool deallocate);

	// Suspend or resume updates based on changes to the section.  Resuming will trigger an immediate update
	CMPINLINE void								SuspendUpdates(void)				{ m_suspendupdates = true; }
	CMPINLINE void								ResumeUpdates(void)
	{
		m_suspendupdates = false;
		SetSectionUpdateFlag();
	}

	// Flag indicating whether the section has been updated, and signalling to the parent ship that it therefore needs to update itself
	CMPINLINE bool								SectionIsUpdated(void) const	{ return m_sectionupdated; }
	CMPINLINE void								SetSectionUpdateFlag(void)		{ m_sectionupdated = true; }
	CMPINLINE void								ClearSectionUpdateFlag(void)	{ m_sectionupdated = false; }

	// Returns the position of this section relative to its parent, in world coordinates
	CMPINLINE XMVECTOR							GetRelativePosition(void) const	{ return m_relativepos; }

	// Update the section based on a change to the parent ship's position or orientation
	void										UpdatePositionFromParent(void);

	// Virtual inheritance from iObject.  There are no intra-frame activities we can take to refresh our data, since 
	// sections are moved by their parent ship objects and cannot change their own state
	CMPINLINE void								RefreshPositionImmediate(void) { }

	CMPINLINE INTVECTOR3 						GetElementLocation(void) const				{ return m_elementlocation; }
	void										SetElementLocation(const INTVECTOR3 & loc);
	CMPINLINE INTVECTOR3 						GetElementSize(void) const					{ return m_elementsize; }
	CMPINLINE Rotation90Degree					GetRotation(void) const						{ return m_rotation; }

	// Updating the section size or rotation will trigger a recalculation of its underlying properties
	void										ResizeSection(const INTVECTOR3 & size);
	void										RotateSection(Rotation90Degree rot);

	// Default property set applied to all elements of the tile; element-specific changes are then made when compiling the tile
	ElementStateDefinition						DefaultElementState;

	// Recalculates all section statistics based upon the loadout and base statistics
	void										RecalculateShipDataFromCurrentState(void);

	// Overrides the iSpaceObject virtual method
	virtual void								MoveIntoSpaceEnvironment(SpaceSystem *system);

	// Implemented to satisfy iSpaceObject interface.  In reality all section positions are determined by the parent ship
	CMPINLINE void								SimulateObject(void) { }

	// Static methods to create a new instance of the specified complex ship section
	static ComplexShipSection *					Create(const std::string & code);
	static ComplexShipSection *					Create(ComplexShipSection *template_sec);


	// Methods to recalculate properties of this section based on its configuration and current state
	void				CalculateShipSizeData();		// Recalculate the section size based on our model bounds
	void				CalculateShipMass();			// Recalculate the total ship mass based on all contributing factors
	void				CalculateVelocityLimits();		// Recalculates velocity limit based on all contributing factors
	void				CalculateBrakeFactor();			// Recalculates brake factor for the section; dependent on velocity limit
	void				CalculateTurnRate();			// Recalculates the overall turn rate based on all contributing factors
	void				CalculateBankRate();			// Recalculates the overall bank rate based on all contributing factors
	void				CalculateBankExtents();			// Recalculates the overall bank extents based on all contributing factors

	// Methods to return properties of this section once they have been calculated
	CMPINLINE float								GetVelocityLimit(void)			{ return m_velocitylimit; }
	CMPINLINE float								GetAngularVelocityLimit(void)	{ return m_angularvelocitylimit; }
	CMPINLINE float								GetBrakeFactor(void)			{ return m_brakefactor; }
	CMPINLINE float								GetBrakeAmount(void)			{ return m_brakeamount; }
	CMPINLINE float								GetTurnRate(void)				{ return m_turnrate; }
	CMPINLINE float								GetTurnAngle(void)				{ return m_turnangle; }
	CMPINLINE float								GetBankRate(void)				{ return m_bankrate; }
	CMPINLINE XMVECTOR							GetBankExtents(void)			{ return m_bankextent; }

	// Methods to set properties directly - those which are not derived by the calculation methods above
	CMPINLINE void								SetVelocityLimit(float v)		{ m_velocitylimit = v; }
	CMPINLINE void								SetAngularVelocityLimit(float v){ m_angularvelocitylimit = v; }
	CMPINLINE void								SetBrakeFactor(float b)			{ m_brakefactor = b; }
	CMPINLINE void								SetTurnRate(float r)			{ m_turnrate = r; }
	CMPINLINE void								SetTurnAngle(float a)			{ m_turnangle = a; }
	CMPINLINE void								SetBankRate(float b)			{ m_bankrate = b; }
	CMPINLINE void								SetBankExtents(FXMVECTOR e)		{ m_bankextent = e; }

	// Methods to force rendering of a section interior even when it does not otherwise meet any criteria for doing so
	bool										InteriorShouldAlwaysBeRendered(void) const	{ return m_forcerenderinterior; }
	void										ForceRenderingOfInterior(bool render)		{ m_forcerenderinterior = render; }

	// Method called when this object collides with another.  Virtual inheritance from iObject
	void										CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact);
	
	// Methods triggered based on major events impacting this object
	void										ShipTileAdded(ComplexShipTile *tile);		// When a tile is added.  Virtual inherited from interface.
	void										ShipTileRemoved(ComplexShipTile *tile);		// When a tile is removed.  Virtual inherited from interface.

	// Methods to get and set the preview image for this section
	CMPINLINE TextureDX11 *						GetPreviewImage(void) { return m_previewimage; }
	void										SetPreviewImage(const std::string & name);

	// Event triggered upon destruction of the object
	void										DestroyObject(void);

	// Shutdown method to terminate the ship section and any resources
	void										Shutdown(void);

	// Static method to create a copy of the supplied ship section
	//static ComplexShipSection *					Copy(ComplexShipSection *sec);

	// Custom debug string function
	std::string									DebugString(void) const;

	// Methods to return the standard path / filename where this ship section data should be stored, if it is following the standard convention
	std::string									DetermineXMLDataPath(void);
	std::string									DetermineXMLDataFilename(void);
	std::string									DetermineXMLDataFullFilename(void);

	// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
	// Updates the command with its result if the command can be processed at this level
	void										ProcessDebugCommand(GameConsoleCommand & command);


private:

	ComplexShip *					m_parent;					// The ship that this section belongs to
	INTVECTOR3						m_elementlocation;			// x,y,z location of the top-top-left element, in element space
	INTVECTOR3						m_elementsize;				// The size in elements, taking into account rotation etc
	int								m_elementcount;				// The number of elements that this section covers
	Rotation90Degree				m_rotation;					// Rotation of this section about the Y axis
	AXMVECTOR						m_relativepos;				// x,y,z position relative to parent ship object, in world space
	AXMVECTOR						m_relativeorient;			// Orientation relative to the parent ship object, in world space

	std::vector<Hardpoint*>			m_hardpoints;				// The hardpoint collection for this ship section; simple vector HPs, which are 
																// copied to the parent ship when the section is added

	bool							m_sectionupdated;			// Indicates to the parent ship object that it should refresh based on its component sections next cycle
	bool							m_suspendupdates;			// Flag that indicates all updates (via section update flag) should be suspended until updates resume

	float							m_velocitylimit;			// Velocity limit of the ship section, incorporating base "details" value and modifiers
	float							m_angularvelocitylimit;		// Angular velocity limit of the ship section, incorporating base "details" value and modifiers
	float							m_turnrate;					// Turn rate of the ship section, incorporating base "details" value and modifiers
	float							m_turnangle;				// Max turn angle of the ship section, incorporating base "details" value and modifiers
	float							m_bankrate;					// Bank rate of the ship section, incorporating base "details" value and modifiers
	AXMVECTOR						m_bankextent;				// Bank extents for the ship section, incorporating base "details" value and modifiers
	float							m_brakefactor;				// Percentage of total velocity limit we can brake per second
	float							m_brakeamount;				// Absolute amount of velocity we can brake per second

	bool							m_forcerenderinterior;		// Flag that indicates whether the section interior should ALWAYS be rendered, even if criteria are not met

	TextureDX11 *					m_previewimage;				// Preview texture for this section; pointer into a central static collection

};


#endif