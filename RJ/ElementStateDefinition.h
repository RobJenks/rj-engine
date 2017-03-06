#pragma once

#ifndef __ElementStateDefinition__
#define __ElementStateDefinition__

#include <vector>
#include "CompilerSettings.h"
#include "Utility.h"
#include "FastMath.h"
#include "ElementStateFilters.h"

class ElementStateDefinition
{
public:

	// Struct holding data on the state of a particular element within the tile upon creation
	struct ElementState
	{
		bitstring Properties;

		ElementState() { Properties = 0U; }
		ElementState(bitstring properties) { Properties = properties; }
	};


	// Default constructor	
	CMPINLINE ElementStateDefinition()
	{ 
		m_size = m_size_transposed = NULL_INTVECTOR3;
		m_count = 0;
		m_filter = ElementStateFilters::ALL_PROPERTIES;
	}

	// Constructor with specific state type specified
	CMPINLINE ElementStateDefinition(ElementStateFilters::ElementStateFilter state_filter)
	{
		m_size = m_size_transposed = NULL_INTVECTOR3;
		m_count = 0;
		m_filter = state_filter;
	}

	// Initialise the element state for a specific area size
	CMPINLINE void Initialise(const INTVECTOR3 & elementsize) 
	{
		m_size = elementsize;
		m_size_transposed = INTVECTOR3(m_size.y, m_size.x, m_size.z);
		m_count = (m_size.x * m_size.y * m_size.z);
		for (int i = 0; i < 4; ++i)
		{
			m_state[i] = std::vector<ElementState>(m_count);
		}
	}

	// Store a default element state, and apply it to all elements in the area
	void ApplyDefaultElementState(ElementState default_state);
	
	// Retuns the default state of an element at the specified location, given the specified tile orientation
	CMPINLINE ElementState			GetElementState(const INTVECTOR3 & location, Rotation90Degree tile_rotation) const;

	// Retuns the default state of an element at the specified location, in the default unrotated tile orientation
	CMPINLINE ElementState			GetElementState(const INTVECTOR3 & location) const 
	{ 
		return GetElementState(location, Rotation90Degree::Rotate0); 
	}

	// Set the default state of an element within the tile.  Properties are replicated to each copy of 
	// the ElementState set (once per orientation)
	void							SetElementState(ElementState element_state, const INTVECTOR3 & location, Rotation90Degree rotation);

	// Set the default state of an element within the tile, assuming the tile is unrotated.  Properties are 
	// replicated to each copy of the ElementState set (once per orientation)
	void							SetElementState(ElementState state, const INTVECTOR3 & location)
	{
		SetElementState(state, location, Rotation90Degree::Rotate0);
	}

	// Retrieve any state filter currently applied to this definition (default: 111...111)
	CMPINLINE ElementStateFilters::ElementStateFilter GetCurrentStateFilter(void) const { return m_filter; }

	// Change the state filter in use by this definition.  Will re-evaluate all current state data to ensure 
	// it complies with the new filter
	void							ChangeStateFilter(ElementStateFilters::ElementStateFilter filter);

	// Returns a debug string output representing the set of element states, given the specified tile orientation
	// Shown as a 2D x/y representation, with z values represented within an array at each element
	CMPINLINE std::string			DebugStringOutput(Rotation90Degree rotation)
	{
		return DebugStringOutput_Internal(rotation, rotation);
	}

	// Returns a debug string output representing the set of element states at all rotations.
	// Shown as a 2D x/y representation, with z values represented within an array at each element
	CMPINLINE std::string DebugStringOutput(void) 
	{ 
		return DebugStringOutput_Internal(Rotation90Degree::Rotate0, Rotation90Degree::Rotate270); 
	}


protected:

	std::vector<ElementState>		m_state[4];					// The element state in each of the four possible area orientations
	INTVECTOR3						m_size;						// Size in Rotate0 and Rotate180 orientations
	INTVECTOR3						m_size_transposed;			// Size in Rotate90 and Rotate270 orientations
	int								m_count;					// Total count of elements in the area
	ElementState					m_defaultstate;				// Default state for any element which is not explicitly set
	ElementStateFilters::ElementStateFilter m_filter;			// Filter which can be applied to limit the definition to only certain properties

	// Returns the relevant area dimensions for a particular tile rotation
	CMPINLINE INTVECTOR3			GetSize(Rotation90Degree rotation) const
	{ 
		return ((rotation == Rotation90Degree::Rotate90 || rotation == Rotation90Degree::Rotate270) ? m_size_transposed : m_size);
	}

	// Applies the state filter to all stored element state data
	void							ApplyStateFilter(ElementStateFilters::ElementStateFilter filter);
	CMPINLINE void					ApplyStateFilter(void) { ApplyStateFilter(m_filter); }


	// Internal method which returns a debug string output representing the set of element states, given 
	// the specified tile orientation(s) to be returned. Shown as a 2D x/y representation, with z 
	// values represented within an array at each element
	std::string						DebugStringOutput_Internal(Rotation90Degree start_rotation, Rotation90Degree end_rotation);


};

#endif