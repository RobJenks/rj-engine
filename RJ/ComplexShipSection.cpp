#include "Utility.h"
#include "CoreEngine.h"
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
	INTVECTOR3 *addr = &(m_elementsize);
	DefaultElementState = ElementStateDefinition(ElementStateFilters::SECTION_PROPERTIES);
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

	// Ship sections will no longer perform collision detection - the ship itself will do this
	this->SetCollisionMode(Game::CollisionMode::NoCollision);
}

ComplexShipSection *ComplexShipSection::Create(const std::string & code)
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

void ComplexShipSection::SetElementLocation(const INTVECTOR3 & loc) 
{
	// Store the new relative location
	m_elementlocation = loc; 

	// Also calculate the relative position in world coordinates
	m_relativepos = Game::ElementLocationToPhysicalPosition(m_elementlocation);
}


// Update the size of this section, measured in elements.  Will allocate/deallocate memory accordingly
void ComplexShipSection::ResizeSection(const INTVECTOR3 & size)
{
	// Store the new element size
	m_elementsize = size;
	m_elementcount = (size.x * size.y * size.z);

	// Set the base object size to match our element size.  This will also resize the model geometry to fit
	SetSize(Game::ElementLocationToPhysicalPosition(m_elementsize));

	// Reallocate the element state collection to match this new size
	DefaultElementState.Initialise(m_elementsize);
}

// Sets the section rotation, relative to its parent ship
void ComplexShipSection::RotateSection(Rotation90Degree new_rotation)
{
	// Determine the change in rotation from our current to new value
	Rotation90Degree delta = Rotation90BetweenValues(m_rotation, new_rotation);
	if (delta == Rotation90Degree::Rotate0) return;

	// This may also change the section dimensions
	INTVECTOR3 newsize = ((delta == Rotation90Degree::Rotate90 || delta == Rotation90Degree::Rotate270) ?
		INTVECTOR3(m_elementsize.y, m_elementsize.x, m_elementsize.z) : m_elementsize);

	// Store the new size and rotation values
	m_rotation = new_rotation;
	m_elementsize = newsize;

	// Recalculate the orientation offset for this section
	m_relativeorient = GetRotationQuaternion(new_rotation);
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
void ComplexShipSection::CollisionWithObject(iActiveObject *object, const GamePhysicsEngine::ImpactData & impact)
{
	// Pass to the base class method
	iActiveObject::CollisionWithObject(object, impact);

	// Pass the collision message to the parent ship, assuming we have a valid pointer back from this section
	throw "SECTIONS SHOULD NOT COLLIDE ANY MORE";
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
void ComplexShipSection::SetPreviewImage(const std::string & name)
{
	// Texture manager logic will deal with loading an external texture if required, or returning a pointer to an existing resource if not
	m_previewimage = Game::Engine->GetAssets().GetTexture(name);
	if (!m_previewimage)
	{
		Game::Log << LOG_WARN << "Cannot load preview image \"" << name << "\" for ship section \"" << m_code << "\"\n";
	}
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
void ComplexShipSection::MoveIntoSpaceEnvironment(SpaceSystem *system)
{
	// No section-specific logic; simply pass back to the base class
	iSpaceObject::MoveIntoSpaceEnvironment(system);
}

// Custom debug string function
std::string	ComplexShipSection::DebugString(void) const
{
	return iObject::DebugString(concat
		("Parent=")(m_parent ? m_parent->GetInstanceCode() : "(NULL)").str());
}


// Process a debug command from the console.  Passed down the hierarchy to this base class when invoked in a subclass
// Updates the command with its result if the command can be processed at this level
void ComplexShipSection::ProcessDebugCommand(GameConsoleCommand & command)
{
	// Debug functions are largely handled via macros above for convenience
	INIT_DEBUG_FN_TESTING(command)

	// Attempt to execute the function.  Relies on data and code added by the init function, so maintain this format for all methods
	// Parameter(0) is the already-matched object ID, and Parameter(1) is the function name, so we pass Parameter(2) onwards

	// Accessor methods
	REGISTER_DEBUG_ACCESSOR_FN(GetParent)
	REGISTER_DEBUG_ACCESSOR_FN(SectionIsUpdated)
	REGISTER_DEBUG_ACCESSOR_FN(GetRelativePosition)
	REGISTER_DEBUG_ACCESSOR_FN(GetElementLocation)
	REGISTER_DEBUG_ACCESSOR_FN(GetElementSize)
	REGISTER_DEBUG_ACCESSOR_FN(GetRotation)
	REGISTER_DEBUG_ACCESSOR_FN(InteriorShouldAlwaysBeRendered)	

	// Mutator methods
	REGISTER_DEBUG_FN(ClearAllHardpoints, command.ParameterAsBool(2))
	REGISTER_DEBUG_FN(SuspendUpdates)
	REGISTER_DEBUG_FN(ResumeUpdates)
	REGISTER_DEBUG_FN(SetSectionUpdateFlag)
	REGISTER_DEBUG_FN(ClearSectionUpdateFlag)
	REGISTER_DEBUG_FN(UpdatePositionFromParent)
	REGISTER_DEBUG_FN(RefreshPositionImmediate)
	REGISTER_DEBUG_FN(SetElementLocation, INTVECTOR3(command.ParameterAsInt(2), command.ParameterAsInt(3), command.ParameterAsInt(4)))
	REGISTER_DEBUG_FN(ResizeSection, INTVECTOR3(command.ParameterAsInt(2), command.ParameterAsInt(3), command.ParameterAsInt(4)))
	REGISTER_DEBUG_FN(RotateSection, (Rotation90Degree)command.ParameterAsInt(2))
	REGISTER_DEBUG_FN(RecalculateShipDataFromCurrentState)
	REGISTER_DEBUG_FN(SimulateObject)
	REGISTER_DEBUG_FN(ForceRenderingOfInterior, command.ParameterAsBool(2))


	// Pass processing back to any base classes, if applicable, if we could not execute the function
	if (command.OutputStatus == GameConsoleCommand::CommandResult::NotExecuted)		iSpaceObject::ProcessDebugCommand(command);

}
