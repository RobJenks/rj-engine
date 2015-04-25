#pragma once

#ifndef __ComplexShipSectionH__
#define __ComplexShipSectionH__

#include "DX11_Core.h"

#include "Utility.h"
#include "iSpaceObject.h"
#include "iContainsHardpoints.h"
#include "iContainsComplexShipTiles.h"
#include "FadeEffect.h"
#include "HighlightEffect.h"
class ComplexShipSectionDetails;
class ComplexShipElement;
class ComplexShipTile;
class ComplexShip;
class Hardpoint;
class Hardpoints;
class Texture;


class ComplexShipSection : public iSpaceObject, public iContainsHardpoints, public iContainsComplexShipTiles
{
public:
	
	// Default constructor / destructor
	ComplexShipSection(void);
	~ComplexShipSection(void);

	// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
	// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
	// their level of the implementation
	void								InitialiseCopiedObject(ComplexShipSection *source);

	// Each section belongs to a parent ComplexShip object
	CMPINLINE ComplexShip *						GetParent(void)						{ return m_parent; }
	CMPINLINE void								SetParent(ComplexShip *parent)		{ m_parent = parent; }

	// Retrieve or set the section hardpoints collection
	CMPINLINE Hardpoints *						GetHardpoints(void) { return m_hardpoints; }
	void										SetHardpoints(Hardpoints *hp);

	// Updates the object before it is rendered.  Called only when the object is processed in the render queue (i.e. not when it is out of view)
	void										PerformRenderUpdate(void);

	// Flag indicating whether the section has been updated, and signalling to the parent ship that it therefore needs to update itself
	CMPINLINE bool								SectionIsUpdated(void) const	{ return m_sectionupdated; }
	CMPINLINE void								SetSectionUpdateFlag(void)		{ m_sectionupdated = true; }
	CMPINLINE void								ClearSectionUpdateFlag(void)	{ m_sectionupdated = false; }

	// Methods to retrieve and change the section position relative to its parent
	CMPINLINE D3DXVECTOR3						GetRelativePosition(void) const	{ return m_relativepos; }
	void										SetRelativePosition(const D3DXVECTOR3 & relativepos);

	// Update the section based on a change to the parent ship's position or orientation
	void										UpdatePositionFromParent(void);

	// Virtual inheritance from iObject.  There are no intra-frame activities we can take to refresh our data, since 
	// sections are moved by their parent ship objects and cannot change their own state
	CMPINLINE void								RefreshPositionImmediate(void) { }

	CMPINLINE D3DXMATRIX *						GetSectionOffsetMatrix(void) { return &m_sectionoffsetmatrix; }
	CMPINLINE void								SetSectionOffsetMatrix(D3DXMATRIX m) { m_sectionoffsetmatrix = m; } 

	CMPINLINE INTVECTOR3 						GetElementLocation(void)			{ return m_elementlocation; }
	CMPINLINE void								SetElementLocation(INTVECTOR3 loc)	{ m_elementlocation = loc; } 

	CMPINLINE INTVECTOR3 						GetElementSize(void)				{ return m_elementsize; }
	CMPINLINE int								GetElementSizeX(void) const			{ return m_elementsize.x; }
	CMPINLINE int								GetElementSizeY(void) const			{ return m_elementsize.y; }
	CMPINLINE int								GetElementSizeZ(void) const			{ return m_elementsize.z; }
	CMPINLINE void								SetElementSize(INTVECTOR3 size)		{ m_elementsize = size; }

	CMPINLINE Rotation90Degree					GetRotation(void)					{ return m_rotation; }
	void										SetRotation(Rotation90Degree rot);

	// Recalculates all section statistics based upon the loadout and base statistics
	void										RecalculateShipDataFromCurrentState(void);

	// Makes updates to this object based on a change to the specified hardpoint hp.  Alternatively if a NULL
	// pointer is passed then all potential refreshes are performed on the parent 
	void										PerformHardpointChangeRefresh(Hardpoint *hp);

	// Implemented to satisfy iSpaceObject interface.  In reality all section positions are determined by the parent ship
	CMPINLINE void								SimulateObject(bool PermitMovement) { }

	// Perform the post-simulation update.  Pure virtual inherited from iObject base class
	CMPINLINE void								PerformPostSimulationUpdate(void) { }

	// Static methods to create a new instance of the specified complex ship section
	static ComplexShipSection *					Create(const string & code);
	static ComplexShipSection *					Create(ComplexShipSection *template_sec);


	// Methods to recalculate properties of this section based on its configuration and current state
	void				CalculateShipSizeData();		// Recalculate the section size based on our model bounds
	void				CalculateShipMass();			// Recalculate the total ship mass based on all contributing factors
	void				CalculateVelocityLimits();		// Recalculates velocity limit based on all contributing factors
	void				CalculateBrakeFactor();			// Recalculates brake factor for the section; dependent on velocity limit
	void				CalculateTurnRate();			// Recalculates the overall turn rate based on all contributing factors
	void				CalculateBankRate();			// Recalculates the overall turn rate based on all contributing factors
	void				CalculateBankExtents();			// Recalculates the overall turn rate based on all contributing factors

	// Methods to return properties of this section once they have been calculated
	CMPINLINE float								GetVelocityLimit(void)			{ return m_velocitylimit; }
	CMPINLINE float								GetAngularVelocityLimit(void)	{ return m_angularvelocitylimit; }
	CMPINLINE float								GetBrakeFactor(void)			{ return m_brakefactor; }
	CMPINLINE float								GetBrakeAmount(void)			{ return m_brakeamount; }
	CMPINLINE float								GetTurnRate(void)				{ return m_turnrate; }
	CMPINLINE float								GetTurnAngle(void)				{ return m_turnangle; }
	CMPINLINE float								GetBankRate(void)				{ return m_bankrate; }
	CMPINLINE D3DXVECTOR3						GetBankExtents(void)			{ return m_bankextent; }

	// Methods to set properties directly - those which are not derived by the calculation methods above
	CMPINLINE void								SetVelocityLimit(float v)		{ m_velocitylimit = v; }
	CMPINLINE void								SetAngularVelocityLimit(float v){ m_angularvelocitylimit = v; }
	CMPINLINE void								SetBrakeFactor(float b)			{ m_brakefactor = b; }
	CMPINLINE void								SetTurnRate(float r)			{ m_turnrate = r; }
	CMPINLINE void								SetTurnAngle(float a)			{ m_turnangle = a; }
	CMPINLINE void								SetBankRate(float b)			{ m_bankrate = b; }
	CMPINLINE void								SetBankExtents(D3DXVECTOR3 &e)	{ m_bankextent = e; }

	// Derives a new offset matrix for the section, based on its ship-related position and rotation
	void										DeriveNewSectionOffsetMatrix(void);

	// Effects that can be activated on this object
	FadeEffect									Fade;					// Allows the object to be faded in and out
	HighlightEffect								Highlight;

	// Methods to force rendering of a section interior even when it does not otherwise meet any criteria for doing so
	bool										InteriorShouldAlwaysBeRendered(void) const	{ return m_forcerenderinterior; }
	void										ForceRenderingOfInterior(bool render)		{ m_forcerenderinterior = render; }

	// Interface method from iContainsHardpoints; returns a reference back to this object's base iSpaceObject object
	iSpaceObject *								GetSpaceObjectReference(void)	{ return (iSpaceObject*)this; }

	// Method called when this object collides with another.  Virtual inheritance from iObject
	void										CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact);
	
	// Methods triggered based on major events impacting this object
	void										ShipTileAdded(ComplexShipTile *tile);		// When a tile is added.  Virtual inherited from interface.
	void										ShipTileRemoved(ComplexShipTile *tile);		// When a tile is removed.  Virtual inherited from interface.

	// Methods to get and set the preview image for this section
	CMPINLINE Texture *							GetPreviewImage(void) { return m_previewimage; }
	void										SetPreviewImage(const std::string & filename);

	// Shutdown method to terminate the ship section and any resources
	void										Shutdown(void);

	// Static method to create a copy of the supplied ship section
	//static ComplexShipSection *					Copy(ComplexShipSection *sec);

	// Methods to return the standard path / filename where this ship section data should be stored, if it is following the standard convention
	std::string									DetermineXMLDataPath(void);
	std::string									DetermineXMLDataFilename(void);
	std::string									DetermineXMLDataFullFilename(void);


private:
	ComplexShip *					m_parent;					// The ship that this section belongs to
	Hardpoints *					m_hardpoints;				// The hardpoint collection for this ship section
	INTVECTOR3						m_elementlocation;			// x,y,z location of the top-top-left element, in element space
	INTVECTOR3						m_elementsize;				// The size in elements, taking into account rotation etc
	Rotation90Degree				m_rotation;					// Rotation of this section about the Y axis
	D3DXVECTOR3						m_relativepos;				// x,y,z position relative to parent ship object, in world space

	D3DXMATRIX						m_sectionoffsetmatrix;		// Translation offset for the ship section, in local space

	bool							m_sectionupdated;			// Indicates to the parent ship object that it should refresh based on its component sections next cycle

	float							m_velocitylimit;			// Velocity limit of the ship section, incorporating base "details" value and modifiers
	float							m_angularvelocitylimit;		// Angular velocity limit of the ship section, incorporating base "details" value and modifiers
	float							m_turnrate;					// Turn rate of the ship section, incorporating base "details" value and modifiers
	float							m_turnangle;				// Max turn angle of the ship section, incorporating base "details" value and modifiers
	float							m_bankrate;					// Bank rate of the ship section, incorporating base "details" value and modifiers
	D3DXVECTOR3						m_bankextent;				// Bank extents for the ship section, incorporating base "details" value and modifiers
	float							m_brakefactor;				// Percentage of total velocity limit we can brake per second
	float							m_brakeamount;				// Absolute amount of velocity we can brake per second

	bool							m_forcerenderinterior;		// Flag that indicates whether the section interior should ALWAYS be rendered, even if criteria are not met

	Texture *						m_previewimage;				// Preview texture for this section; pointer into a central static collection

};


#endif