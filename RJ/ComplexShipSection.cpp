#include "Utility.h"
#include "ComplexShip.h"
#include "Hardpoint.h"
#include "Hardpoints.h"
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
	m_suspendupdates = false;
	m_sectionupdated = false;
	m_elementlocation = NULL_INTVECTOR3;
	m_elementsize = NULL_INTVECTOR3;
	m_rotation = Rotation90Degree::Rotate0;
	m_relativepos = NULL_VECTOR;
	m_relativeorient = ID_QUATERNION;
	m_velocitylimit = 1.0f;
	m_angularvelocitylimit = 1.0f;
	m_turnrate = 0.01f;
	m_turnangle = 0.01f;
	m_bankrate = 0.0f;
	m_bankextent = NULL_VECTOR;
	m_brakefactor = m_brakeamount = 1.0f;
	m_forcerenderinterior = false;
	m_previewimage = NULL;

	// This class of space object will perform full collision detection by default (iSpaceObject default = no collision)
	this->SetCollisionMode(Game::CollisionMode::FullCollision);
}

ComplexShipSection *ComplexShipSection::Create(const string & code)
{
	// Attempt to get the ship template matching this code; if it doesn't exist then return NULL
	ComplexShipSection *template_sec = D::ComplexShipSections.Get(code);
	if (template_sec == NULL) return NULL;

	// Invoke the spawn function using these template details & return the result
	return (ComplexShipSection::Create(template_sec));
}

ComplexShipSection *ComplexShipSection::Create(ComplexShipSection *template_sec)
{
	// If we are passed an invalid class pointer then return NULL immediately
	if (template_sec == NULL) return NULL;

	// Create a new instance of the ship from this template; class-specific initialisation will all be performed
	// automatically up the inheritance hierarchy as part of this operation.  If any part of the operation fails, 
	// the returned value will be NULL (which we then pass on as the return value from this function)
	ComplexShipSection *sec = CopyObject<ComplexShipSection>(template_sec);

	// Return a pointer to the newly-created ship section
	return sec;
}


// Method to initialise fields back to defaults on a copied object.  Called by all classes in the object hierarchy, from
// lowest subclass up to the iObject root level.  Objects are only responsible for initialising fields specifically within
// their level of the implementation
void ComplexShipSection::InitialiseCopiedObject(ComplexShipSection *source)
{
	// Pass control to all base classes
	iSpaceObject::InitialiseCopiedObject(source);


	/* Now perform ComplexShipSection-specific initialisation logic for new objects */

	// Remove reference to the template section's parent
	SetParent(NULL);

	// The hardpoints vector will have been shallow-copied from the template.  Perform a deep-copy instead
	m_hardpoints.clear();
	std::vector<Hardpoint*>::size_type n = source->GetHardpoints().size();
	m_hardpoints.reserve(n);

	// Copy each hardpoint in turn
	Hardpoint *hp;
	const std::vector<Hardpoint*> & hps = source->GetHardpoints();
	for (std::vector<Hardpoint*>::size_type i = 0; i < n; ++i)
	{
		// All hardpoints should be non-null, but perform this check for safety
		hp = hps[i]; if (!hp) continue;

		// Clone and store the hardpoint
		m_hardpoints.push_back(hp->Clone());
	}
}

// Sets the section position relative to its parent ship, recalculating required data at the same time
void ComplexShipSection::SetRelativePosition(const FXMVECTOR relativepos)
{
	// Store the new relative position
	m_relativepos = relativepos;
}

// Sets the section rotation, relative to its parent ship
void ComplexShipSection::SetRotation(Rotation90Degree rot)
{
	// Store the new rotation value
	m_rotation = rot;

	// Recalculate the orientation offset for this section
	m_relativeorient = GetRotationQuaternion(rot);
}

// Update the section based on a change to the parent ship's position or orientation
void ComplexShipSection::UpdatePositionFromParent(void)
{
	// Make sure we have a parent object
	if (!m_parent) return;

	// Transform our relative position and orientation into parent world space, and thereby determine our actual section position and orient
	// D3DXVec3TransformCoord(&pos, &m_relativepos, m_parent->GetOrientationMatrix()) // SetPosition(m_parent->GetPosition() + pos);
	SetPositionAndOrientation(
		XMVectorAdd(m_parent->GetPosition(), XMVector3TransformCoord(m_relativepos, m_parent->GetOrientationMatrix())),
		XMQuaternionMultiply(m_parent->GetOrientation(), m_relativeorient)
	);
			
	// Update position of the ship in the spatial partitioning tree
	if (m_treenode) m_treenode->ItemMoved(this, m_position);
}

void ComplexShipSection::RecalculateShipDataFromCurrentState(void)
{
	// Recalculate ship properties based on all our component sections, loadout and any modifiers
	CalculateShipSizeData();
	CalculateShipMass();
	CalculateVelocityLimits();
	//CalculateBrakeFactor();		// Dependent on velocity limit, so is called directly from within that function instead
	CalculateTurnRate();
	CalculateBankRate();
	CalculateBankExtents();
}

// Add a hardpoint to the section.  Updates will be triggered if they are not suspended
void ComplexShipSection::AddHardpoint(Hardpoint *hp)
{
	if (!hp) return;
	m_hardpoints.push_back(hp);
	
	if (!m_suspendupdates) SetSectionUpdateFlag();
}

// Remove a hardpoint from the section.  Updates will be triggered if they are not suspended
void ComplexShipSection::RemoveHardpoint(Hardpoint *hp)
{
	if (!hp) return;

	std::vector<Hardpoint*>::size_type n = m_hardpoints.size();
	for (std::vector<Hardpoint*>::size_type i = 0; i < n; ++i)
	{
		if (m_hardpoints[i] == hp)
		{
			m_hardpoints.erase(m_hardpoints.begin() + i);
			break;
		}
	}

	if (!m_suspendupdates) SetSectionUpdateFlag();
}

// Clear all hardpoints.  Flag indicates whether the hardpoints should be deallocated, or simply removed from the collection
void ComplexShipSection::ClearAllHardpoints(bool deallocate)
{
	// If we want to deallocate the hardpoints then we need to iterate through each one in turn
	if (deallocate)
	{
		std::vector<Hardpoint*>::size_type n = m_hardpoints.size();
		for (std::vector<Hardpoint*>::size_type i = 0; i < n; ++i)
		{
			if (m_hardpoints[i]) SafeDelete(m_hardpoints[i]);
		}
	}

	// We can now clear the collection
	m_hardpoints.clear();

	// Flag the change in status if updates are active
	if (!m_suspendupdates) SetSectionUpdateFlag();
}

void ComplexShipSection::CalculateShipSizeData(void)
{
	// Section is sized to encompass its element space
	if (m_elementsize.x <= 0 || m_elementsize.y <= 0 || m_elementsize.z <= 0) 
	{
		// If we don't have any elements allocated for this section then default the section size to 1x1x1
		SetSize(Game::C_CS_ELEMENT_SCALE_V);
	}
	else
	{
		// Otherwise determine the ship section size from its underlying model
		this->SetSize(Game::ElementLocationToPhysicalPosition(m_elementsize));
	}
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
	// Deallocate all hardpoint data
	ClearAllHardpoints(true);

	// Pass control back to the base class
	iSpaceObject::Shutdown();
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

// Event triggered upon destruction of the object
void ComplexShipSection::DestroyObject(void)
{
	OutputDebugString("Destruction of ComplexShipSection\n");
}

// Sets the preview image texture associated with this ship
void ComplexShipSection::SetPreviewImage(const std::string & filename)
{
	// Texture manager logic will deal with loading an external texture if required, or returning a pointer to an existing resource if not
	m_previewimage = new Texture(concat(D::IMAGE_DATA)("\\")(filename).str());
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


// Overrides the iSpaceObject virtual method
void ComplexShipSection::MoveIntoSpaceEnvironment(SpaceSystem *system, const FXMVECTOR location)
{
	// No section-specific logic; simply pass back to the base class
	iSpaceObject::MoveIntoSpaceEnvironment(system, location);
}