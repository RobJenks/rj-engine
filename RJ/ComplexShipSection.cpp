#include "Utility.h"
#include "ComplexShip.h"
#include "ComplexShipDetails.h"
#include "Hardpoint.h"
#include "Hardpoints.h"
#include "FadeEffect.h"
#include "Texture.h"
#include "Model.h"
#include "CopyObject.h"
#include "iSpaceObject.h"

#include "ComplexShipSection.h"


ComplexShipSection::ComplexShipSection(void)
{
	// Set the object type
	SetObjectType(iObject::ObjectType::ComplexShipSectionObject);

	// Initialise ship properties to default upon object creation
	m_parent = NULL;
	m_model = NULL;
	m_standardobject = false;
	m_sectionupdated = false;
	m_elementlocation = NULL_VECTOR;
	m_elementsize = NULL_VECTOR;
	m_rotation = Rotation90Degree::Rotate0;
	m_relativepos = NULL_VECTOR;
	m_sectionoffsetmatrix = ID_MATRIX;
	m_velocitylimit = 1.0f;
	m_angularvelocitylimit = 1.0f;
	m_turnrate = 0.01f;
	m_turnangle = 0.01f;
	m_bankrate = 0.0f;
	m_bankextent = NULL_VECTOR;
	m_brakefactor = m_brakeamount = 1.0f;
	m_forcerenderinterior = false;
	m_previewimage = NULL;

	// Create a new hardpoints collection for the ship section
	m_hardpoints = new Hardpoints();
	m_hardpoints->SetParent(this, this);

	// This class of space object will perform full collision detection by default (iSpaceObject default = no collision)
	this->SetCollisionMode(Game::CollisionMode::FullCollision);
}

ComplexShipSection *ComplexShipSection::Create(const string & code)
{
	// Attempt to get the ship template matching this code; if it doesn't exist then return NULL
	ComplexShipSection *template_sec = D::GetComplexShipSection(code);
	if (template_sec == NULL) return NULL;

	// Invoke the spawn function using these template details & return the result
	return (ComplexShipSection::Create(template_sec));
}

ComplexShipSection *ComplexShipSection::Create(ComplexShipSection *template_sec)
{
	// If we are passed an invalid class pointer then return NULL immediately
	if (template_sec == NULL) return NULL;

	// Create a new instance of the ship from this template
	ComplexShipSection *sec = CopyObject<ComplexShipSection>(template_sec);
	if (!sec) return NULL;

	// Remove reference to the template section's parent
	sec->SetParent(NULL);

	// Initialise the ship element space with new data, since currently the pointer is copied from the source ship
	// Important: Set to NULL first, otherwise copy method will deallocate the original element space before replacing it
	// MORE important: if we remove the element base from sections, remove these lines
	//sec->SetElements(NULL);

	// Remove all references to ship tiles and generate copies for the new ship
	// Important: remove if we no longer have tiles included in sections...
	sec->RemoveAllShipTiles();
	//sec->CopyTileDataFromObject(template_ship);
	
	// Copy the hardpoint collection and assign to the new section
	sec->SetHardpoints(template_sec->GetHardpoints()->Copy());
	sec->GetHardpoints()->SetParent(sec, sec);

	
	// Return a pointer to the newly-created ship section
	return sec;
}


// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void ComplexShipSection::InitialiseCopiedObject(ComplexShipSection *source)
{
	// Pass control to all base classes
	iSpaceObject::InitialiseCopiedObject((iSpaceObject*)source);
}


// Updates the object before it is rendered.  Called only when the object enters the render queue (i.e. not when it is out of view)
void ComplexShipSection::PerformRenderUpdate(void)
{
	// Update any render effects that may be active on the object
	Fade.Update();
}

void ComplexShipSection::SetHardpoints(Hardpoints *hp)
{
	m_hardpoints = hp;
	if (m_hardpoints) m_hardpoints->SetParent(this, this);
}

// Sets the section position relative to its parent ship, recalculating required data at the same time
void ComplexShipSection::SetRelativePosition(const D3DXVECTOR3 & relativepos)
{
	// Store the new relative position
	m_relativepos = relativepos;

	// Recalculate the section offset matrix based upon this change
	DeriveNewSectionOffsetMatrix();
}

// Sets the section rotation, relative to its parent ship
void ComplexShipSection::SetRotation(Rotation90Degree rot)
{
	// Store the new rotation value
	m_rotation = rot;

	// Recalculate the section offset matrix based upon this change
	DeriveNewSectionOffsetMatrix();
}

// Update the section based on a change to the parent ship's position or orientation
void ComplexShipSection::UpdatePositionFromParent(void)
{
	// Make sure we have a parent object
	if (!m_parent) return;

	// Transform our relative position into parent world space, and thereby determine our actual section position
	D3DXVECTOR3 pos;
	D3DXVec3TransformCoord(&pos, &m_relativepos, m_parent->GetOrientationMatrix());
	SetPosition(m_parent->GetPosition() + pos);

	// Derive a new world matrix for this section by adding the translation in local space (WM = Child * Parent)
	//D3DXMatrixMultiply(&m_worldmatrix, &m_sectionoffsetmatrix, m_parent->GetWorldMatrix());
	m_worldmatrix = (m_sectionoffsetmatrix * (*m_parent->GetWorldMatrix()));
	
	// Update position of the ship in the spatial partitioning tree
	if (m_treenode) m_treenode->ItemMoved(this, m_position);
}

void ComplexShipSection::RecalculateShipDataFromCurrentState(void)
{
	// Perform an update of the ship based on all hardpoints
	PerformHardpointChangeRefresh(NULL);

	// Recalculate ship properties based on all our component sections, loadout and any modifiers
	CalculateShipSizeData();
	CalculateShipMass();
	CalculateVelocityLimits();
	//CalculateBrakeFactor();		// Dependent on velocity limit, so is called directly from within that function instead
	CalculateTurnRate();
	CalculateBankRate();
	CalculateBankExtents();
}

// Derives a new offset matrix for the section, based on its ship-related position and rotation
void ComplexShipSection::DeriveNewSectionOffsetMatrix(void)
{
	D3DXMATRIX centre, rot, trans;

	// Determine the centre offset matrix to use based upon the section model size / centre point
	if (m_model && m_model->IsGeometryLoaded())
	{
		const D3DXVECTOR3 centrepoint = m_size * 0.5f; //m_model->GetModelCentre();
		D3DXMatrixTranslation(&centre, -centrepoint.x, -centrepoint.y, -centrepoint.z);
	}
	else
	{
		centre = ID_MATRIX;
	}

	// Construct a rotation matrix based upon our rotation value
	switch (m_rotation)
	{
		case Rotation90Degree::Rotate90: D3DXMatrixRotationY(&rot, PIOVER2); break;
		case Rotation90Degree::Rotate180: D3DXMatrixRotationY(&rot, PI); break;
		case Rotation90Degree::Rotate270: D3DXMatrixRotationY(&rot, PI + PIOVER2); break;
		default: rot = ID_MATRIX; break;
	}

	// Translation matrix is simply constructed from the section position offset
	D3DXMatrixTranslation(&trans, m_relativepos.x, m_relativepos.y, m_relativepos.z);

	// Construct the section offset matrix based on (WM_offset = centretrans * rot * trans)
	m_sectionoffsetmatrix = (centre * rot * trans);
}

// Makes updates to this object based on a change to the specified hardpoint hp.  Alternatively if a NULL
// pointer is passed then all potential refreshes are performed on the parent 
void ComplexShipSection::PerformHardpointChangeRefresh(Hardpoint *hp)
{
	// Get the type of hardpoint being considered, or assign 'unknown' if NULL is provided and we want to make all potential updates
	Equip::Class hptype = Equip::Class::Unknown;
	if (hp) hptype = hp->GetType();

	// Make any updates to the section itself

	// Set the section update flag.  Parent ship will then be updated from this section in the next simulation cycle
	SetSectionUpdateFlag();
}

void ComplexShipSection::CalculateShipSizeData(void)
{
	// Section size is based upon its underlying model
	if (!m_model) 
	{
		// If we don't have a model for this section then default the section size
		SetSize(D3DXVECTOR3(1.0f, 1.0f, 1.0f)); 
	}
	else
	{
		// Otherwise determine the ship section size from its underlying model
		this->SetSize(m_model->GetModelSize());
	}

	// Recalculate the section offset matrix based upon this change
	DeriveNewSectionOffsetMatrix();
}

void ComplexShipSection::CalculateShipMass()
{
	// Nothing to do here; this attribute is stored directly within the ship section
}

void ComplexShipSection::CalculateVelocityLimits()
{
	// Nothing to do here; this attribute is stored directly within the ship section

	// Trigger a recalculation of the brake factors, since these are dependent on the velocity limit
	CalculateBrakeFactor();
}

void ComplexShipSection::CalculateBrakeFactor()
{
	// No calculation required for brake factor; this attribute is stored directly within the ship section

	// Recalculate the absolute brake amount for efficiency, based on max velocity & the new brake factor
	m_brakeamount = (m_brakefactor * m_velocitylimit);
}

void ComplexShipSection::CalculateTurnRate()
{
	// Nothing to do here; this attribute is stored directly within the ship section
}

void ComplexShipSection::CalculateBankRate()
{
	// Nothing to do here; this attribute is stored directly within the ship section
}

void ComplexShipSection::CalculateBankExtents()
{
	// Nothing to do here; this attribute is stored directly within the ship section
}

ComplexShipSection::~ComplexShipSection(void)
{
}

// Shuts down and deallocates the ship section
void ComplexShipSection::Shutdown(void)
{

}

// Method called when this object collides with another.  Virtual inheritance from iSpaceObject
void ComplexShipSection::CollisionWithObject(iObject *object, const GamePhysicsEngine::ImpactData & impact)
{
	// Pass the collision message to the parent ship, assuming we have a valid pointer back from this section
	if (m_parent) m_parent->CollisionWithObject(object, this, impact);
}

// Method to handle the addition of a ship tile to this object
void ComplexShipSection::ShipTileAdded(ComplexShipTile *tile)
{
	// TODO: To be implemented
}

// Method to handle the removal of a ship tile from this object
void ComplexShipSection::ShipTileRemoved(ComplexShipTile *tile)
{
	// TODO: To be implemented
}

// Sets the preview image texture associated with this ship
void ComplexShipSection::SetPreviewImage(const std::string & filename)
{
	// Texture manager logic will deal with loading an external texture if required, or returning a pointer to an existing resource if not
	m_previewimage = new Texture(filename);
}

// Returns the file path where XML data relating to this ship section should be stored.  This is either within the "Sections"
// directory if this is a standard section, or within its parent ship's directory if not.  All paths relative to the game data path
std::string ComplexShipSection::DetermineXMLDataPath(void)
{
	if (m_standardobject)
						return "\\Ships\\Sections";
	else
	{
		if (m_parent)	return concat("\\Ships\\")(m_parent->GetCode()).str();
		else			return "Ships\\UnlinkedCustomSections";
	}
}
// Returns the name of the XML file to be generated for this ship section
std::string ComplexShipSection::DetermineXMLDataFilename(void)
{
	return concat(m_code)(".xml").str();
}
// Returns the full expected filename for the XML data relating to this ship section
std::string ComplexShipSection::DetermineXMLDataFullFilename(void)
{
	return concat(DetermineXMLDataPath())("\\")(DetermineXMLDataFilename()).str();
}