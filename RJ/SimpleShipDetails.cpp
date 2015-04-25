#include "iSpaceObject.h"
#include "SimpleShipDetails.h"

SimpleShipDetails *SimpleShipDetails::Copy(void)
{
	// Generate a copy of this object via default copy constructor
	SimpleShipDetails *ss = new SimpleShipDetails(*this);

	// Assign pointer to the standard, template model for this class.  We only generate a new version of
	// the model if it changes in the future
	ss->Model = this->Model;
	
	// Recursively copy the ship hardpoint data.  This creates a new Hardpoints object and so replaces any
	// attempt the default copy constructor makes.
	ss->HP = this->HP->Copy();

	return ss;
}

SimpleShipDetails::SimpleShipDetails(void) : ShipDetails()
{
	// Set default values (delta, on top of those to be set by the base class)
	this->CameraPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	this->CameraPositionMatrix = NULL_MATRIX;
}

// Destructor
SimpleShipDetails::~SimpleShipDetails(void)
{
}

// Shutdown method to deallocate all simple ship data
void SimpleShipDetails::Shutdown(void)
{
	// Pass control back to the base class for shutdown of common ship data
	ShipDetails::Shutdown();
}

void SimpleShipDetails::RecalculateShipDetails()
{
	// Camera position & orientation; generate a translation & rotation matrix for the camera
	D3DXMATRIX cam, rot;
	D3DXMatrixTranslation(&cam, this->CameraPosition.x, this->CameraPosition.y, this->CameraPosition.z);
	D3DXMatrixRotationYawPitchRoll(&rot, this->CameraRotation.y, this->CameraRotation.x, this->CameraRotation.z);

	// Combine the transformations and store the camera matrix
	D3DXMatrixMultiply(&cam, &cam, &rot);
	this->CameraPositionMatrix = cam;

	// Reallocate hardpoints to relevant groups to allow fast subsequent indexing
	this->HP->RecalculateHardpoints();

	// Calculate a default bounding box for the ship if it hasn't already been allocated one
}

SimpleShipDetails *SimpleShipDetails::Get(const string &code)
{
	// Attempt to locate in the ship register based on string code
	ShipDetails *ship = D::Ships[code];

	// Return either a pointer to the ship or null, depending on whether anything was found and if
	// the returned object is the correct class
	if (ship && ship->Category == Ships::Class::Simple)
		return (SimpleShipDetails*)ship;
	else
		return NULL;
}



