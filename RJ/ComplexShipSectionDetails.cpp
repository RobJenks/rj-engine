#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "Utility.h"
#include "GameDataExtern.h"
#include "Texture.h"
#include "ComplexShipDetails.h"
#include "ComplexShipElement.h"
#include "Hardpoints.h"

#include "ComplexShipSectionDetails.h"

// Static collection of preview images relating to each type of complex ship section
unordered_map<string, Texture*> ComplexShipSectionDetails::__previewimages;


// Constructor
ComplexShipSectionDetails::ComplexShipSectionDetails(void)
{
	// Initialise all key variables to null
	m_code = ""; 
	m_name = "";
	m_standardsection = false;
	m_model = NULL;
	m_parent = NULL;
	m_visibilitytestingmode = VisibilityTestingModeType::UseBoundingSphere;
	
	m_mass = 1.0f;
	m_velocitylimit = 1.0f;
	m_angularvelocitylimit = 1.0f;
	m_turnrate = 0.01f;
	m_turnangle = 0.01f;
	m_bankrate = 0.0f;
	m_bankextent = NULL_VECTOR;
	m_brakefactor = 1.0f;

	m_hardpoints = new Hardpoints();
	m_elements = NULL;
	m_tilecount = 0;
	m_hastiles = m_multitile = false;

	m_previewimagepath = "";
	m_previewimagesize = INTVECTOR2();
}


// Method to handle the addition of a ship tile to this object
void ComplexShipSectionDetails::ShipTileAdded(ComplexShipTile *tile)
{
	// TODO: To be implemented
}

// Method to handle the removal of a ship tile from this object
void ComplexShipSectionDetails::ShipTileRemoved(ComplexShipTile *tile)
{
	// TODO: To be implemented
}

// Allocates and initialises storage for elements based on the element size properties.  
// Will dispose of any existing data if it exists.
Result ComplexShipSectionDetails::InitialiseAllElements(void)
{
	// Generate the new element space for this ship section
	return ComplexShipElement::CreateElementSpace(this, &m_elements, m_elementsize);
}

// Rotates a ship section by the specified amount, generating a new element space and mapping data to new positions
Result ComplexShipSectionDetails::RotateShipSection(Rotation90Degree rot)
{
	Result result;

	// Rotate the ship section element space
	result = ComplexShipElement::RotateElementSpace(&m_elements, &m_elementsize, rot);
	if (result != ErrorCodes::NoError) return result;

	// Rotate the ship preview image dimensions
	if (rot == Rotation90Degree::Rotate90 || rot == Rotation90Degree::Rotate270)
		m_previewimagesize = INTVECTOR2(m_previewimagesize.y, m_previewimagesize.x);

	// Return success
	return ErrorCodes::NoError;
}

// Static method that attempts to load the set of details specified by the string code parameter
ComplexShipSectionDetails *ComplexShipSectionDetails::Get(string code)
{
	// If the code is invalid or does not exist then return a NULL pointer
	if (code == NullString || D::ComplexShipSections.count(code) == 0) return NULL;

	// Otherwise return the relevant ship section details now
	return D::ComplexShipSections[code];
}

// Static method to copy an instance of ship section details
ComplexShipSectionDetails *ComplexShipSectionDetails::Copy(ComplexShipSectionDetails *details)
{
	// Make a copy of the details via copy constructor
	ComplexShipSectionDetails *d = new ComplexShipSectionDetails(*details);

	// Copy the set of external hardpoints assigned to this ship section
	d->SetHardpoints(details->GetHardpoints()->Copy());

	// Copy the size of the elements collection
	INTVECTOR3 esize = details->GetElementSize();
	d->SetElementSize(esize);

	// Allocate element storage in the new ship section
	d->SetElements(NULL);
	d->InitialiseAllElements();

	// Now copy element data across from the source ship section
	for (int x = 0; x < esize.x; x++) {
		for (int y = 0; y < esize.y; y++) {
			for (int z = 0; z < esize.z; z++)
			{
				// Get a reference to the target element in the new ship section
				ComplexShipElement *e = d->GetElementDirect(x, y, z);

				// Copy element data across from the source element
				ComplexShipElement::CopyData(details->GetElementDirect(x, y, z), e);
			}
		}
	}

	// Clone the ship section preview data, used for rendering the ship designer and schematic views
	if (details->GetPreviewImage())
		d->SetPreviewImageParametersDirect(details->GetPreviewImage()->Clone(), details->GetPreviewImageSize(), 
										   details->GetPreviewImageRelPath(), details->GetPreviewImagePath());

	// Remove the "standard" flag from this section after it has been copied
	d->SetIsStandardSection(false);

	// Return a pointer to the cloned section details
	return d;
}

// Recalculates the ship section details based on components that are loaded.  Called after loading from file
void ComplexShipSectionDetails::RecalculateShipSectionDetails(void)
{
	// Recalculate data for all hardpoints attached to this section
	m_hardpoints->RecalculateHardpoints();
}

// Loads the preview image associated with this ship section
Result ComplexShipSectionDetails::LoadPreviewImage(string path)
{
	Result result;

	// Determine the full path and then store these parameters
	string filename = BuildStrFilename(D::DATA, path);
	m_previewimagepath = filename;
	m_previewimagerelpath = path;

	// Test whether we already have an image loaded for the specified path
	if (ComplexShipSectionDetails::HaveSectionPreviewImage(filename))
	{
		// If we do, simply set the pointer to reference this existing texture
		m_previewimage = ComplexShipSectionDetails::GetSectionPreviewImage(filename);
		return ErrorCodes::NoError;
	}

	// The preview image does not exist so attempt to create it now
	m_previewimage = new Texture();
	result = m_previewimage->Initialise(filename.c_str());

	// If the image could not be initialised then reset fields and return the error
	if (result != ErrorCodes::NoError) {
		m_previewimage->Shutdown();
		m_previewimage = NULL;
		m_previewimagepath = "";
		m_previewimagerelpath = "";
		return result;
	}

	// Otherwise, add this image to the static collection (so we don't need to re-load it in future) and return success
	ComplexShipSectionDetails::AddSectionPreviewImage(filename, m_previewimage);
	return ErrorCodes::NoError;
}

void ComplexShipSectionDetails::SetPreviewImageParametersDirect(Texture *texture, INTVECTOR2 previewsize, string previewrelpath, string previewpath)
{
	// Store the parameters directly, bypassing costly operations around loading from disk if we already have the required resources loaded
	m_previewimage = texture;
	m_previewimagesize = previewsize;
	m_previewimagerelpath = previewrelpath;
	m_previewimagepath = previewpath;
}


// Returns the file path where XML data relating to this ship section should be stored.  This is either within the directory
// holding the rest of its parent ship's data, if it has a parent ship, or the "Sections" directory if not.  All paths are
// relative to the game data path
string ComplexShipSectionDetails::DetermineXMLDataPath(void)
{
	if (m_parent)
		return concat("\\Ships\\")(m_parent->GetCode()).str();
	else
		return "\\Ships\\Sections";
}
// Returns the name of the XML file to be generated for this ship section
string ComplexShipSectionDetails::DetermineXMLDataFilename(void)
{
	return concat(m_code)(".xml").str();
}
// Returns the full expected filename for the XML data relating to this ship section
string ComplexShipSectionDetails::DetermineXMLDataFullFilename(void)
{
	return concat(DetermineXMLDataPath())("\\")(DetermineXMLDataFilename()).str();
}

ComplexShipSectionDetails::~ComplexShipSectionDetails(void)
{
}

// Shuts down and deallocates the ship section details
void ComplexShipSectionDetails::Shutdown(void)
{
	// Deallocate the element space within this section
	if (m_elements)
	{
		ComplexShipElement::DeallocateElementStorage(&m_elements, m_elementsize);
		m_elements = NULL;
	}

	// Deallocate the hardpoints collection assigned to this section
	if (m_hardpoints)
	{
		delete m_hardpoints; m_hardpoints = NULL;
	}
}

// Terminates all the shared preview image data for complex ship sections
void ComplexShipSectionDetails::TerminateSectionPreviewData(void)
{
	__PreviewImageCollection::iterator it_end = __previewimages.end();
	for (__PreviewImageCollection::iterator it = __previewimages.begin(); it != it_end; ++it)
	{
		if (it->second)
		{
			it->second->Shutdown();
			delete (it->second);
			it->second = NULL;
		}
	}
}