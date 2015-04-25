#pragma once

#ifndef __CSLifeSupportTileDefinitionH__
#define __CSLifeSupportTileDefinitionH__

#include "ComplexShipTileDefinition.h"
#include "AdjustableParameter.h"
#include "Utility.h"

class CSLifeSupportTileDefinition : public ComplexShipTileDefinition
{

public:

	// Default constructor
	CSLifeSupportTileDefinition(void);

	// Virtual method implemented by definition subclasses, if required, to apply subclass-specific properties to a tile
	void						ApplyClassSpecificDefinition(ComplexShipTile *tile);

	// Virtual method to read class-specific XML data for the tile
	void						ReadClassSpecificXMLData(TiXmlElement *node);


	// Store the starting values for key properties maintained by the life support system
	AdjustableParameter<float>	InitialGravity;
	AdjustableParameter<int>	InitialOxygenLevel;
	AdjustableParameter<int>	InitialOxygenRange;

	// Returns the standard gravity strength expected at the specified distance
	CMPINLINE float				GetGravityStrength(int distance) const				
	{ 
		return (distance <= m_gravity_range ? m_gravity_strength[distance] : 0.0f);
	}

	// Set or retrieve the gravity falloff & exponent values
	CMPINLINE int				GetGravityRange(void) const							{ return m_gravity_range; }
	CMPINLINE void				SetGravityRange(int r)								{ m_gravity_range = r; }
	CMPINLINE float				GetGravityFalloffDelay(void) const					{ return m_gravity_falloffdelay; }
	CMPINLINE void				SetGravityFalloffDelay(float f)						{ m_gravity_falloffdelay = f; }
	CMPINLINE int				GetGravityExponent(void) const						{ return m_gravity_exponent; }
	CMPINLINE void				SetGravityExponent(int e)							{ m_gravity_exponent = e; }

	// Recalculates all derived values in the definition based on the current properties
	void						RecalculateDefinitionProperties(void);

	// Default destructor
	~CSLifeSupportTileDefinition(void);

protected:

	// Gravity falloff and exponent determine the gravity area of effect that is created by this life support system. 
	int							m_gravity_range;
	float						m_gravity_falloffdelay;
	int							m_gravity_exponent;

	// Table of gravity strength values; will only ever have max size of 100 due to upper bound on gravity range
	float *						m_gravity_strength;

	// Private methods to recalculate part of the tile properties
	void						RecalculateGravityData(void);
	void						RecalculateOxygenData(void);


};



#endif









