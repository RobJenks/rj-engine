#pragma once

#ifndef __EnvironmentMapH__
#define __EnvironmentMapH__

#include "DefaultValues.h"
#include "Utility.h"
#include "EnvironmentMapBlendMode.h"
#include "EnvironmentMapFalloffMethod.h"
#include "ComplexShipElement.h"

// Templated with <T, EnvironmentMapBlendMode::'BlendMode'<T>>
template <typename T, template<typename> class TBlendMode>
class EnvironmentMap
{
public:

	// Basic structure holding the value of a particular cell
	struct MapCell
	{
		int Index; T Value;
		MapCell(void) : Index(0), Value(DefaultValues<T>::NullValue) { }
		MapCell(int index, T value) : Index(index), Value(value) { }
	};

	// Map data
	std::vector<T>									Data;

	// Constructor; accepts dimensions of the area to be represented
	EnvironmentMap(const INTVECTOR3 & element_size);

	// Begins an update of the environment map
	EnvironmentMap &								BeginUpdate(void);

	// Adds one or several source cells to the update
	EnvironmentMap &								WithSourceCell(MapCell source);
	EnvironmentMap &								WithSourceCells(const std::vector<MapCell> & sources);

	// Specifies that the current map data will be retained and used as the basis for this update
	CMPINLINE EnvironmentMap &						WithUpdateAppendingToExistingData(void) { m_append_to_existing_data = true; }

	// Sets the initial value to be applied to all cells before updating the map.  Overridden by "UpdateAppendingToExistingData" if set
	EnvironmentMap &								WithInitialValues(T initial_value);

	// Sets the possible range of initial values for each map cell.  ONLY compatible with blend modes that 
	// have "USES_INITIAL_VALUE" == false
	EnvironmentMap &								WithInitialValueRange(T min_initial_value, T max_initial_value);

	// Executes an update of the map.  Accepts a reference to the underlying element collection as a mandatory parameter
	Result											Execute(const ComplexShipElement *elements);


	// Set the properties that allow transmission of value through the map
	void											SetTransmissionProperties(bitstring properties);

	// Set the properties that prevent transmission of value through the map
	void											SetBlockingProperties(bitstring properties);

	// Set the falloff method as value is transmitted through the map
	void											SetFalloffMethod(const EnvironmentMapFalloffMethod<T> & falloff_method);

	// Sets the 'zero threshold', below which we consider the value to have fallen to insignificance and cease processing it
	void											SetZeroThreshold(T zero_threshold);


	// Default values of each transmission-relevant property
	static const bitstring							DEFAULT_TRANSMISSION_PROPERTIES;
	static const bitstring							DEFAULT_BLOCKING_PROPERTIES;	

	

protected:

	// Updates the size of this environment and recalculates dependent data
	void											SetElementSize(const INTVECTOR3 & element_size);

	// Initialises all update-relevant fields to their default starting values, before they are potentially
	// updated via update construction calls "With*"
	void											InitialiseUpdateParameters(void);

	// Triggered when any property-related fields are updated
	void											PropertyParametersUpdated(void);

	// Blend method for applying multiple results on top of each other
	TBlendMode<T>									m_blend;

	// Falloff method for attenuating values as they pass between cells
	EnvironmentMapFalloffMethod<T>					m_falloff;

	// Dimensions and count of elements in the map
	INTVECTOR3										m_elementsize;
	typename std::vector<T>::size_type				m_elementcount;

	// Index deltas for a move in each direction on the map
	static const int								NEIGHBOUR_COUNT = (int)Direction::_Count;
	int												m_movedelta[NEIGHBOUR_COUNT];

	// Array of source cells
	std::vector<MapCell>							m_sources;

	// Flag indicating whether existing cell values should be kept for this update (rather than 
	// starting from the supplied initial value set)
	bool											m_append_to_existing_data;

	// Initial value(s) to be applied to the map cells before any update takes place
	bool											m_fixed_initial_value;		// True (default) = use m_initial_value, False = use min/max
	T												m_initial_value;
	T												m_initial_value_min, m_initial_value_max;
	T												m_initial_value_range;

	// Property bitstrings that transmit or block value passing between cells
	bitstring										m_transmission_properties;
	bitstring										m_blocking_properties;
	bool											m_property_dependent_transmission;

	// Any values which fall below the 'zero threshold' will be considered insignificant and cease being processed
	T												m_zero_threshold;

};


// Constructor; accepts dimensions of the area to to represented
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode>::EnvironmentMap(const INTVECTOR3 & element_size)
{
	// Determine element space size and dimenstions
	SetElementSize(element_size);
	
	// Set default per-map values
	SetZeroThreshold(DefaultValues<T>::NullValue());
	SetTransmissionProperties(DEFAULT_TRANSMISSION_PROPERTIES);
	SetBlockingProperties(DEFAULT_BLOCKING_PROPERTIES);

	// Set default per-update values (though these will be reset on each BeginUpdate() call)
	InitialiseUpdateParameters();	

}

template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::SetElementSize(const INTVECTOR3 & element_size)
{
	// Store the new element size and count
	m_elementsize = IntVector3Clamp(element_size, ONE_INTVECTOR3, INTVECTOR3(10000, 10000, 10000));
	m_elementcount = (std::vector<T>::size_type)(m_elementsize.x * m_elementsize.y * m_elementsize.z);

	// Recalculate the move deltas for each direction in the map, based on this new element space
	for (int i = 0; i < NEIGHBOUR_COUNT; ++i)
	{
		INTVECTOR3 offset = DirectionUnitOffset((Direction)i);
		m_movedelta[i] = ELEMENT_INDEX_EX(offset.x, offset.y, offset.z, m_elementsize);
	}

	// Initialise the data array for this map
	Data = std::vector<T>(m_elementcount);
}

// Initialises all update-relevant fields to their default starting values, before they are potentially
// updated via update construction calls "With*"
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::InitialiseUpdateParameters(void)
{
	// Initialise all update-relevant fields to their default starting values
	m_append_to_existing_data = false;

	m_fixed_initial_value = true;
	m_initial_value = DefaultValues<T>::NullValue();
	m_initial_value_min = DefaultValues<T>::NullValue();
	m_initial_value_max = DefaultValues<T>::NullValue();
	m_initial_value_range = DefaultValues<T>::NullValue();

}

// Begins an update of the environment map
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode> & EnvironmentMap<T, TBlendMode>::BeginUpdate(void)
{
	// Clear the array of source cells
	m_sources.clear();

	// Set default values that may be supplied by other methods in the update construction
	InitialiseUpdateParameters();

	return *this;
}


// Adds one source cell to the update
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode> & EnvironmentMap<T, TBlendMode>::WithSourceCell(MapCell source)
{
	m_sources.push_back(source);
	return *this;
}

// Adds multiple source cells to the update
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode> & EnvironmentMap<T, TBlendMode>::WithSourceCells(const std::vector<MapCell> & sources)
{
	m_sources.insert(m_sources.end(), sources.begin(), sources.end());
	return *this;
}

// Sets the initial value to be applied to all cells before updating the map.  Overridden by "UpdateAppendingToExistingData" if set
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode> & EnvironmentMap<T, TBlendMode>::WithInitialValues(T initial_value)
{
	m_fixed_initial_value = true;
	m_initial_value = initial_value;
	return *this;
}

// Sets the possible range of initial values for each map cell.  ONLY compatible with blend modes that 
// have "USES_INITIAL_VALUE" == false
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode> & EnvironmentMap<T, TBlendMode>::WithInitialValueRange(T min_initial_value, T max_initial_value)
{
	if (m_blend.USES_INITIAL_VALUE)
	{
		// This is not permitted.  Default to a fixed/null initial value and return
		return WithInitialValues(DefaultValues<T>::NullValue());
	}

	m_fixed_initial_value = false;
	m_initial_value_min = min_initial_value;
	m_initial_value_max = max(max_initial_value, min_initial_value + DefaultValues<T>::EpsilonValue());		// Ensure max>min
	m_initial_value_range = (m_initial_value_max - m_initial_value_min);
	m_initial_value = DefaultValues<T>::NullValue();
	return this;
}

// Initialise transmission-relevant property constants
template <typename T, template<typename> class TBlendMode> 
const bitstring EnvironmentMap<T, TBlendMode>::DEFAULT_TRANSMISSION_PROPERTIES = ComplexShipElement::ALL_PROPERTIES;
template <typename T, template<typename> class TBlendMode>
const bitstring EnvironmentMap<T, TBlendMode>::DEFAULT_BLOCKING_PROPERTIES = ComplexShipElement::NULL_PROPERTIES;

// Set the properties that allow transmission of value through the map
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::SetTransmissionProperties(bitstring properties)
{
	m_transmission_properties = properties;
	PropertyParametersUpdated();
}

// Set the properties that prevent transmission of value through the map
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::SetBlockingProperties(bitstring properties)
{
	m_blocking_properties = properties;
	PropertyParametersUpdated();
}

// Triggered when any property-related fields are updated
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::PropertyParametersUpdated(void)
{
	m_property_dependent_transmission = (
		(m_transmission_properties != DEFAULT_TRANSMISSION_PROPERTIES) ||
		(m_blocking_properties != DEFAULT_BLOCKING_PROPERTIES)
	);
}


// Set the falloff method as value is transmitted through the map
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::SetFalloffMethod(const EnvironmentMapFalloffMethod<T> & falloff_method)
{
	m_falloff = falloff_method;
}

// Sets the 'zero threshold', below which we consider the value to have fallen to insignificance and cease processing it
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::SetZeroThreshold(T zero_threshold)
{
	m_zero_threshold = zero_threshold;
}




// Executes an update of the map.  Accepts a reference to the underlying element collection as a mandatory parameter
template <typename T, template<typename> class TBlendMode>
Result EnvironmentMap<T, TBlendMode>::Execute(const ComplexShipElement *elements)
{
	// Set initial value for all cells, UNLESS we have chosen to retain the existing data
	if (!m_append_to_existing_data)
	{
		if (m_fixed_initial_value)
		{
			for (std::vector<T>::size_type i = 0; i < m_elementcount; ++i)
				Data[i] = m_initial_value;
		}
		else
		{
			for (std::vector<T>::size_type i = 0; i < m_elementcount; ++i)
				Data[i] = (m_initial_value_min + (T)(frand() * m_initial_value_range));
		}
	}

	// Initialise the blend mode with our initial value.  This can only be a fixed initial value for blend modes
	// which actually make use of it (those which have 'USES_INITIAL_VALUE = true' set)
	m_blend.SetInitialValue(m_initial_value);

	// We must have a reference to the underlying elements to do any computation.  Exit here if we do not 
	// have that reference, in which case all elements will simply be set to their initial values
	if (elements == NULL) return ErrorCodes::CannotEvaluateEnvironmentMapWithoutElementRef;

	// We maintain a fixed array of values to indicate when a value has been updated.  Use int rather than bool
	// so we can quickly revert via memset on each cycle
	int *updated = new int[m_elementcount];
	size_t update_size = (sizeof(int) * m_elementcount);

	// Process each source cell in turn
	std::vector<std::vector<T>::size_type> queue;
	std::vector<std::vector<T>::size_type>::size_type queue_index;
	std::vector<EnvironmentMap<T, TBlendMode>::MapCell>::const_iterator it_end = m_sources.end();
	for (std::vector<EnvironmentMap<T, TBlendMode>::MapCell>::const_iterator it = m_sources.begin(); it != it_end; ++it)
	{
		// Initialise the update tracking array for this cycle
		memset(updated, 0, update_size);

		// Initialise the value of the source cell
		// TODO: this may result in source cells having a value lower than their neighbours, if they are in a range
		// of cells which were updated via another much stronger source first.  Use max() ?
		std::vector<T>::size_type index = (*it).Index;
		Data[index] = (*it).Value;

		// Initialise the processing queue with this source cell
		queue.clear(); 
		queue.push_back(index);
		queue_index = 0U;
		updated[index] = 1;

		// Now calculate the recursive transmission to all neighbouring cells via non-recursive vector traversal
		while (queue_index < queue.size())
		{
			// Get the next index to be processed
			index = queue[queue_index];
			const ComplexShipElement & el = elements[index];
			T current_value = Data[index];

			// Increment the queue index
			++queue_index;

			// We want to check all neighbours of this cell that have not yet been processed
			for (int i = 0; i < NEIGHBOUR_COUNT; ++i)
			{
				// Get the neighbour and perform basic checks of whether it should be processed
				int neighbour = el.GetNeighbour((Direction)i);
				if (neighbour == -1) continue;									// Must be within the environment map area; -1 signifies not a valid element
				if (updated[neighbour] != 0) continue;							// Ignore any elements we have already updated
				updated[neighbour] = 1;											// Mark this neighbour as processed

				// Check against both transmission and blocking element properties
				bitstring neighbour_properties = elements[neighbour].GetProperties();
				if ((CheckBit_All_NotSet(neighbour_properties, m_transmission_properties)) ||
					(CheckBit_Any(neighbour_properties, m_blocking_properties))) continue;

				// We want to transmit to this cell.  First calculate the transmitted value.  If it has fallen
				// below the zero threshold then we can stop propogating it here
				T transmitted = m_falloff.ApplyFalloff(current_value, (Direction)i);
				if (transmitted < m_zero_threshold) continue;

				// Blend this value into the target cell
				Data[neighbour] = m_blend.Apply(Data[neighbour], transmitted);

				// We want to move on and process all this cell's neighbours in a future cycle
				queue.push_back(neighbour);
			}

		}

	}

	// Delete any temporarily-allocate memory
	delete[] updated;

	// Return success
	return ErrorCodes::NoError;
}



#endif




