#pragma once

#ifndef __ResourceH__
#define __ResourceH__

#include <string>
#include "CompilerSettings.h"
#include "ErrorCodes.h"
class ProductionCost;

// This class has no special alignment requirements
class Resource
{
public:

	// Default constructor
	Resource(void);

	// Get/set methods for key fields
	CMPINLINE const std::string &			GetCode(void) const							{ return m_code; }
	CMPINLINE void							SetCode(const std::string & code)			{ m_code = code; }

	CMPINLINE const std::string &			GetName(void) const							{ return m_name; }
	CMPINLINE void							SetName(const std::string & name)			{ m_name = name; }

	CMPINLINE float							GetValue(void) const						{ return m_value; }
	CMPINLINE void							SetValue(float value)						
	{ 
		if (value > 0.0f)	m_value = value; 
		else				m_value = 1.0f;
	}

	CMPINLINE bool							IsAsteroidResource(void) const				{ return m_asteroidresource; }
	CMPINLINE void							SetIsAsteroidResource(bool b)				{ m_asteroidresource = b; }

	CMPINLINE bool							IsPlanetResource(void) const				{ return m_planetresource; }
	CMPINLINE void							SetIsPlanetResource(bool b)					{ m_planetresource = b; }

	CMPINLINE ProductionCost *				GetProductionCost(void)						{ return m_productioncost; }
	CMPINLINE ProductionCost *				GetProductionCostConst(void) const			{ return m_productioncost; }
	void									SetProductionCost(ProductionCost *pcost);
	
	CMPINLINE float							GetCompoundValue(void) const				{ return m_compoundvalue; }
	CMPINLINE void							SetCompoundValue(float value)				{ m_compoundvalue = value; }

	// Recursive method which will determine the compound value of the resource based on its dependencies.  Handles stack/infinite loop issues
	Result									DetermineCompoundValue(void);

	// Shutdown method - not required for this class
	CMPINLINE void Shutdown(void) { throw "Shutdown method not implemented for this class"; }

	// Default destructor
	~Resource(void);





protected:
	std::string				m_code;						// String code for this resource
	std::string				m_name;						// Readable string name of this resource

	float					m_value;					// Indicates how valuable the resource is in general.  Used to derive prices, and also for
														// population of the resource in the universe.  Driven by things like rarity, trends, ...

	bool					m_asteroidresource;			// Flag indicating whether this resource can be mined from asteroids
	bool					m_planetresource;			// Flag indicating whether this resource can be mined from planets
	
	ProductionCost *		m_productioncost;			// Requirements to construct this resource from base resources.  Will have *zero* resource 
														// requirements and a time requirement if this is a base resource that can be mined

	float					m_compoundvalue;			// Value derived from all resources.  Calculated on game initialisation by traversing the tree
														// of resources and summing up the value (rarity) of all component resources.

};




#endif