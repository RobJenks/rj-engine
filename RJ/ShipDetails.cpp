#include "Model.h"
#include "ShipDetails.h"


ShipDetails::ShipDetails(void)
{
	// Assign each ship type a unique ID on declaration
	static int _ShipDetails_internalIDGenerator = 1;
	ID = _ShipDetails_internalIDGenerator++;

	// Set default values
	this->Category = Ships::Class::Simple;
	this->Code = "";
	this->Name = "";
	this->m_parent = NULL;
	this->DefaultLoadout = "";
	this->HullMass = 1.0f;
	this->HullVelocityLimit = 1.0f;
	this->HullAngularVelocityLimit = 1.0f;
	this->HullTurnRate = 1.0f;
	this->Model = NULL;

	// Create a new Hardpoints collection and link back to this object.  Note this is always where the hardpoints
	// collection is created.  We therefore assign the Hardpoints->Parent pointer upon assigning the ShipDetails object,
	// since it will always contain the hardpoints collection at this point.
	this->HP = new Hardpoints();
}

ShipDetails::~ShipDetails(void)
{
}

void ShipDetails::Shutdown(void)
{
	// Release all memory associated with the ship model, assuming it is owned by us individually
	// and not a standard model used across all non-deformed entities of this class.  If this is
	// a standard model we simply undirect the pointer to prevent the model being automatically
	// deallocated by the default C++ destructors
	if (this->Model)				
	{
		if ((this->Model)->IsStandardModel())
			this->Model = NULL;					// Remove the pointer to the standard model
		else
		{
			delete this->Model;					// Otherwise, if this is our own model, deallocate it now
			this->Model = NULL;
		}
	}

	// Release all memory associated with the ship hardpoint collection
	if (this->HP)			{ delete this->HP; this->HP = NULL; }
}

void ShipDetails::SetHullMass(float m)
{
	// Perform bounds checking and never allow a ~0 or negative mass value.  Saves a lot of costly validation later
	// if we can (near enough) guarantee those situations will not come up
	if (m < Game::C_EPSILON) return;

	// Otherwise set the new mass value
	this->HullMass = m;

	// If applicable, force the parent object to recalculate its mass
	if (this->m_parent) this->m_parent->CalculateShipMass();
}

void ShipDetails::SetHullVelocityLimit(float v)
{
	// Set the new velocity limit value
	this->HullVelocityLimit = v;

	// If applicable, force the parent object to recalculate its overall velocity limit
	if (this->m_parent) this->m_parent->CalculateVelocityLimits();
}

void ShipDetails::SetHullAngularVelocityLimit(float v)
{
	// Set the new velocity limit value
	this->HullAngularVelocityLimit = v;

	// If applicable, force the parent object to recalculate its overall velocity limit
	if (this->m_parent) this->m_parent->CalculateVelocityLimits();
}

void ShipDetails::SetHullBrakeFactor(float f)
{
	// Set the new brake factor
	this->HullBrakeFactor = f;

	// If applicable, force the parent object to recalculate its overall brake factor
	if (this->m_parent) this->m_parent->CalculateBrakeFactor();	
}	

void ShipDetails::SetHullTurnRate(float r)
{
	// Set the new velocity limit value
	this->HullTurnRate = r;

	// If applicable, force the parent object to recalculate its overall velocity limit
	if (this->m_parent) this->m_parent->CalculateTurnRate();
}

void ShipDetails::SetHullTurnAngle(float a)
{
	// Set the new max turn rate 
	this->HullTurnAngle = a;

	// If applicable, force the parent object to recalculate its overall velocity limit
	if (this->m_parent) this->m_parent->CalculateTurnRate();
}


void ShipDetails::SetParent(Ship *p) 
{	
	// Set pointer back to the parent ship
	this->m_parent = p; 
}
