#pragma once

#ifndef __EnvironmentMapFalloffMethodH__
#define __EnvironmentMapFalloffMethodH__

#include "DefaultValues.h"
#include "FastMath.h"

template <typename T>
class EnvironmentMapFalloffMethod
{
public:

	// Enumeration of possible falloff transmission types
	// 'Square' applies a one-cell falloff to horizontal, vertical and diagonal transitions
	// 'Distance' applies a falloff proportional to transmission distance, i.e. diagonal transitions are sqrt(2)*falloff
	enum FalloffTransmissionType							{ Distance = 0, Square };

	// Constructor.  Initialises a falloff method with default parameters
	EnvironmentMapFalloffMethod(void);

	// Sets an absolute falloff per map cell, regardless of current cell value.  Applied at point of transmission
	EnvironmentMapFalloffMethod &							WithAbsoluteFalloff(T falloff);

	// Sets a relative falloff per map cell, applied at point of transmission.  Applied a float falloff percentage
	// to the cell value
	EnvironmentMapFalloffMethod &							WithRelativeFalloff(float falloff);

	// Specifies how the cell value falls off per transmission.  'Square' applies a one-cell falloff to horizontal, 
	// vertical and diagonal transitions.  'Distance' applies a falloff proportional to transmission distance, i.e.
	// diagonal transitions are sqrt(2)*falloff
	EnvironmentMapFalloffMethod &							WithFalloffTransmissionType(FalloffTransmissionType type);


	// Applies the falloff method to a single-cell transmission of the given source value in the specified direction
	T														ApplyFalloff(T current_value, Direction direction);


protected:

	enum _InternalFalloffType								{ _AbsFalloff = 0, _RelFalloff };
	_InternalFalloffType									m_falloff_type;

	T														m_abs_falloff;
	float													m_rel_falloff;

	T														m_abs_falloff_diag;
	float													m_rel_falloff_diag;

	FalloffTransmissionType									m_transmission_type;

	T														m_abs_falloff_dirs[Direction::_Count];
	float													m_rel_falloff_dirs[Direction::_Count];

#	pragma warning( push )
#	pragma warning( disable : 4244 )
	void													CalculateDerivedFalloffValues(void)
	{
		m_abs_falloff = min(m_abs_falloff, DefaultValues<T>::NullValue());			// Ensure <= 0
		m_rel_falloff = clamp(m_rel_falloff, 0.0f, 1.0f);							// Ensure [0 1]

		m_abs_falloff_diag = (T)((float)m_abs_falloff * ROOT2);						// Calculate diag version
		m_rel_falloff_diag = (1.0f - ((1.0f - m_rel_falloff) * ROOT2));				// Calculate diag version (take from remaining% to reduce% before applying, then revert back)

		m_abs_falloff_diag = min(m_abs_falloff_diag, DefaultValues<T>::NullValue());		// Apply same constraint to diag versions
		m_rel_falloff_diag = clamp(m_rel_falloff_diag, 0.0f, 1.0f);							// Apply same constraint to diag versions

		// Store these values in precalculated per-direction lookup tables
		if (m_transmission_type == FalloffTransmissionType::Distance)
		{
			for (int i = 0; i < (int)Direction::_Count; ++i)
			{
				bool diagonal = IsDiagonalDirection((Direction)i);
				m_abs_falloff_dirs[i] = (diagonal ? m_abs_falloff_diag : m_abs_falloff);
				m_rel_falloff_dirs[i] = (diagonal ? m_rel_falloff_diag : m_rel_falloff);
			}
		}
		else /* if (m_transmission_type == FalloffTransmissionType::Square) */
		{
			for (int i = 0; i < (int)Direction::_Count; ++i)
			{
				m_abs_falloff_dirs[i] = m_abs_falloff;
				m_rel_falloff_dirs[i] = m_rel_falloff;
			}
		}
	}
#	pragma warning( default : 4244 )
#	pragma warning( pop )

};


// Constructor.  Initialises a falloff method with default parameters
template <typename T>
EnvironmentMapFalloffMethod<T>::EnvironmentMapFalloffMethod(void)
{
	// Set default values
	m_falloff_type = _InternalFalloffType::_RelFalloff;
	m_abs_falloff = DefaultValues<T>::OneValue();
	m_rel_falloff = 0.5f;
	m_transmission_type = FalloffTransmissionType::Distance;
	CalculateDerivedFalloffValues();
}

// Sets an absolute falloff per map cell, regardless of current cell value.  Applied at point of transmission
template <typename T>
EnvironmentMapFalloffMethod<T> & EnvironmentMapFalloffMethod<T>::WithAbsoluteFalloff(T falloff)
{
	// Absolute falloff is stored as the -ve value to be ADDED to the cell value
	m_falloff_type = _InternalFalloffType::_AbsFalloff;
	m_abs_falloff = -falloff;
	CalculateDerivedFalloffValues();
	return *this;
}

// Sets a relative falloff per map cell, applied at point of transmission.  Applied a float falloff percentage
// to the cell value
template <typename T>
EnvironmentMapFalloffMethod<T> & EnvironmentMapFalloffMethod<T>::WithRelativeFalloff(float falloff)
{
	// Relative falloff is stored as the % REMAINDER, i.e. a 10% falloff per cell is stored as 0.9f (and constrained (0 1))
	m_falloff_type = _InternalFalloffType::_RelFalloff;
	m_rel_falloff = (1.0f - falloff);
	CalculateDerivedFalloffValues();
	return *this;
}

// Specifies how the cell value falls off per transmission.  'Square' applies a one-cell falloff to horizontal, 
// vertical and diagonal transitions.  'Distance' applies a falloff proportional to transmission distance, i.e.
// diagonal transitions are sqrt(2)*falloff
template <typename T>
EnvironmentMapFalloffMethod<T> & EnvironmentMapFalloffMethod<T>::WithFalloffTransmissionType(FalloffTransmissionType type)
{
	m_transmission_type = type;
	CalculateDerivedFalloffValues();
	return *this;
}


// Applies the falloff method to a single-cell transmission of the given source value in the specified direction
template <typename T>
T EnvironmentMapFalloffMethod<T>::ApplyFalloff(T current_value, Direction direction)
{
	// Apply either absolute or relative falloff, accounting for transmission direction and transmission type (via the lookup table)
	return (m_falloff_type == _InternalFalloffType::_RelFalloff ?
		(T)((float)current_value * m_rel_falloff_dirs[(int)direction]) :
		current_value + m_abs_falloff_dirs[(int)direction]);
}




#endif





