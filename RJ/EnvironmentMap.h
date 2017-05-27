#pragma once

#ifndef __EnvironmentMapH__
#define __EnvironmentMapH__

#include "DefaultValues.h"
#include "Utility.h"
#include "EnvironmentMapBlendMode.h"
#include "EnvironmentMapFalloffMethod.h"
#include "ComplexShipElement.h"
#include "PrecalculatedRandomSequence.h"

// Compiler flag which can be set to enable output of map-related debug information during each update
//#define ENV_MAP_DEBUG_OUTPUT

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

	// Enumerations used to store internal state and map configuration
	enum ExistingDataUpdate { NoUpdate = 0, AdditiveUpdate, MultiplicativeUpdate };
	enum MapValueConstraints { NoConstraints = 0, ValueFloor, ValueCeiling, ValueClamped };
	enum EmissionBehaviour { EmissionRemainsInSource = 0, EmissionRemovedFromSource };

	// Map data
	std::vector<T>									Data;

	// Constructor; accepts dimensions of the area to be represented
	EnvironmentMap(const INTVECTOR3 & element_size);

	// Returns the dimensions of the map
	CMPINLINE INTVECTOR3							GetMapSize(void) const { return m_elementsize; }

	// Begins an update of the environment map
	EnvironmentMap &								BeginUpdate(void);

	// Adds one or several source cells to the update
	EnvironmentMap &								WithSourceCell(MapCell source);
	EnvironmentMap &								WithSourceCells(const std::vector<MapCell> & sources);

	// Specifies that the current map data will be retained and used as the basis for this update
	EnvironmentMap &								WithPreserveExistingData(void);
	EnvironmentMap &								WithAdditiveModifierToExistingData(T modifier); 
	EnvironmentMap &								WithMultiplicativeModifierToExistingData(float modifier); 

	// Sets the initial value to be applied to all cells before updating the map.  Overridden by "UpdateAppendingToExistingData" if set
	EnvironmentMap &								WithInitialValues(T initial_value);

	// Sets the possible range of initial values for each map cell.  ONLY compatible with blend modes that 
	// have "USES_INITIAL_VALUE" == false
	EnvironmentMap &								WithInitialValueRange(T min_initial_value, T max_initial_value);

	// Sets the maximum amount of value that can be transferred in the current cycle
	EnvironmentMap &								WithTransferLimit(T transfer_limit);

	// Executes an update of the map.  Accepts a reference to the underlying element collection as a mandatory parameter
	Result											Execute(const ComplexShipElement *elements);

	// Returns the value of a specific map cell
	CMPINLINE T										GetCellValue(int index) const { return Data[index]; }

	// Sets the value of a specific map cell
	CMPINLINE void									SetCellValue(int index, T value) { Data[index] = value; }

	// Returns the properties that allow transmission of value through the map
	CMPINLINE bitstring								GetTransmissionProperties(void) const { return m_transmission_properties; }

	// Set the properties that allow transmission of value through the map
	void											SetTransmissionProperties(bitstring properties);

	// Returns the properties that block transmission of values through the map
	CMPINLINE bitstring								GetBlockingProperties(void) const { return m_blocking_properties; }

	// Set the properties that prevent transmission of value through the map
	void											SetBlockingProperties(bitstring properties);

	// Set the falloff method as value is transmitted through the map
	void											SetFalloffMethod(const EnvironmentMapFalloffMethod<T> & falloff_method);

	// Sets the 'zero threshold', below which we consider the value to have fallen to insignificance and cease processing it
	void											SetZeroThreshold(T zero_threshold);

	// Sets a floor or ceiling for values within the map, applied as a final post-processing step after all updates
	void			 								SetValueFloor(T floor);
	void			 								SetValueCeiling(T ceiling);
	void			 								SetValueConstraints(T floor, T ceiling);
	void			 								RemoveValueConstraints(void);

	// Specifies whether value emitted by the source cell is actually removed from that cell, or remains in the source
	// while being propogated (less any falloff) to the destination cell
	void											SetEmissionBehaviour(EmissionBehaviour behaviour);

	// Immediately set the value of every cell to the specified value
	CMPINLINE void									InitialiseCellValues(T value)
	{
		for (std::vector<T>::size_type i = 0; i < m_elementcount; ++i) Data[i] = value;
	}

	// Immediately set the value of every cell to a value within the specified range
	CMPINLINE void									InitialiseCellValues(T min_value, T max_value)
	{
		float value_range = (float)(max_value - min_value);		// Store as float to avoid recasting in each operation
		for (std::vector<T>::size_type i = 0; i < m_elementcount; ++i) Data[i] = (min_value + (T)(frand() * value_range));
	}

	// Generates a debug string output of the environment map contents
	CMPINLINE std::string							DebugStringOutput(void) { return DebugStringOutput(0, m_elementsize.z - 1, NULL); }
	CMPINLINE std::string							DebugStringOutput(const char *format) { return DebugStringOutput(0, m_elementsize.z - 1, format); }
	CMPINLINE std::string							DebugStringOutput(int z_level) { return DebugStringOutput(z_level, z_level, NULL); }
	CMPINLINE std::string							DebugStringOutput(int z_level, const char *format) { return DebugStringOutput(z_level, z_level, format); }
	CMPINLINE std::string							DebugStringOutput(int z_start, int z_end) { return DebugStringOutput(z_start, z_end, NULL); }
	std::string										DebugStringOutput(int z_start, int z_end, const char *format);

	// Default values of each transmission-relevant property
	static const bitstring							DEFAULT_TRANSMISSION_PROPERTIES;
	static const bitstring							DEFAULT_BLOCKING_PROPERTIES;	

	

protected:

	// Debug logging for environment map updates, if enabled via relevant flag
#	ifdef ENV_MAP_DEBUG_OUTPUT
#		define ENV_MAP_DEBUG_LOG(cstr)				OutputDebugString(cstr)
#	else
#		define ENV_MAP_DEBUG_LOG(cstr)				
#	endif

	// Static set of randomised direction values, for use in randomising direction of value spread
	static const PrecalculatedRandomSequence<int>	DirectionSequenceData;

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
	int												m_movedelta[Direction::_Count];

	// Array of source cells
	std::vector<MapCell>							m_sources;

	// Flag indicating whether existing cell values should be kept for this update (rather than 
	// starting from the supplied initial value set)
	bool											m_preserve_existing_data;

	// Parameters determining whether a pre-processing update is applied to all map cells before applying new source data
	ExistingDataUpdate								m_update_existing_data;
	T												m_existing_data_additive_update;
	float											m_existing_data_multiplicative_update;

	// Parameters determining whether a post-processing step is applied to keep all values within a defined set of bounds
	MapValueConstraints								m_value_constraints;
	T												m_value_floor, m_value_ceiling;

	// Sets the maximum amount of value that can be transferred in the current cycle
	T												m_transfer_limit; 

	// Initial value(s) to be applied to the map cells before any update takes place
	bool											m_fixed_initial_value;		// True (default) = use m_initial_value, False = use min/max
	T												m_initial_value;
	T												m_initial_value_min, m_initial_value_max;

	// Property bitstrings that transmit or block value passing between cells
	bitstring										m_transmission_properties;
	bitstring										m_blocking_properties;
	bool											m_property_dependent_transmission;

	// Any values which fall below the 'zero threshold' will be considered insignificant and cease being processed
	T												m_zero_threshold;

	// Multiplier applied to emission values when subtracting them from the source cell.  Determines whether the emitted
	// value is removed from the source cell when transmitte to the destination, or whether it remains after propogation
	T												m_source_emission_multiplier;

	// Store a local copy of T-zero for efficiency / cache coherency during tight loop updates
	T												m_zero;

};

// Initialise static set of randomised direction values, for use in randomising direction of value spread
template <typename T, template<typename> class TBlendMode>
const PrecalculatedRandomSequence<int> EnvironmentMap<T, TBlendMode>::DirectionSequenceData
	= PrecalculatedRandomSequence<int>(0, (int)Direction::_Count, (int)Direction::_Count, 100, true);

// Constructor; accepts dimensions of the area to to represented
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode>::EnvironmentMap(const INTVECTOR3 & element_size)
	: 
	m_zero(DefaultValues<T>::NullValue())
{
	// Determine element space size and dimensions
	SetElementSize(element_size);
	
	// Set default per-map values
	SetZeroThreshold(DefaultValues<T>::NullValue());
	SetTransmissionProperties(DEFAULT_TRANSMISSION_PROPERTIES);
	SetBlockingProperties(DEFAULT_BLOCKING_PROPERTIES);
	SetEmissionBehaviour(EmissionBehaviour::EmissionRemainsInSource);
	RemoveValueConstraints();

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
	for (int i = 0; i < Direction::_Count; ++i)
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
	m_preserve_existing_data = false;
	m_update_existing_data = ExistingDataUpdate::NoUpdate;

	m_fixed_initial_value = true;
	m_initial_value = DefaultValues<T>::NullValue();
	m_initial_value_min = DefaultValues<T>::NullValue();
	m_initial_value_max = DefaultValues<T>::NullValue();

	m_transfer_limit = DefaultValues<T>::MaxValue();
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
	m_initial_value = DefaultValues<T>::NullValue();
	return this;
}

// Specifies that the current map data will be retained into the current update
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode> & EnvironmentMap<T, TBlendMode>::WithPreserveExistingData(void)
{
	m_preserve_existing_data = true;
	return *this;
}

// Sets an additive modifier to be applied to all existing data before new sources are applied
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode> & EnvironmentMap<T, TBlendMode>::WithAdditiveModifierToExistingData(T modifier)
{
	m_update_existing_data = ExistingDataUpdate::AdditiveUpdate;
	m_existing_data_additive_update = modifier;
	return *this;
}

// Sets a multiplicative modifier to be applied to all existing data before new sources are applied
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode> & EnvironmentMap<T, TBlendMode>::WithMultiplicativeModifierToExistingData(float modifier)
{
	m_update_existing_data = ExistingDataUpdate::MultiplicativeUpdate;
	m_existing_data_multiplicative_update = modifier;
	return *this;
}

// Sets the maximum amount of value that can be transferred in the current cycle
template <typename T, template<typename> class TBlendMode>
EnvironmentMap<T, TBlendMode> & EnvironmentMap<T, TBlendMode>::WithTransferLimit(T transfer_limit)
{
	m_transfer_limit = transfer_limit;
	return *this;
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

// Sets a floor for values within the map, applied as a final post-processing step after all updates
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::SetValueFloor(T floor)
{
	m_value_constraints = (m_value_constraints == MapValueConstraints::NoConstraints ? MapValueConstraints::ValueFloor : MapValueConstraints::ValueClamped);
	m_value_floor = floor;
}

// Sets a ceiling for values within the map, applied as a final post-processing step after all updates
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::SetValueCeiling(T ceiling)
{
	m_value_constraints = (m_value_constraints == MapValueConstraints::NoConstraints ? MapValueConstraints::ValueCeiling : MapValueConstraints::ValueClamped);
	m_value_ceiling = ceiling;
}

// Sets a floor and ceiling for values within the map, applied as a final post-processing step after all updates
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::SetValueConstraints(T floor, T ceiling)
{
	m_value_constraints = MapValueConstraints::ValueClamped;
	m_value_floor = floor;
	m_value_ceiling = ceiling;
}


// Removes any floor and ceiling for values within the map, disabling the final post-processing step after all updates
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::RemoveValueConstraints(void)
{
	m_value_constraints = MapValueConstraints::NoConstraints;
}



// Specifies whether value emitted by the source cell is actually removed from that cell, or remains in the source
// while being propogated (less any falloff) to the destination cell
template <typename T, template<typename> class TBlendMode>
void EnvironmentMap<T, TBlendMode>::SetEmissionBehaviour(EmissionBehaviour behaviour)
{
	if (behaviour == EmissionBehaviour::EmissionRemovedFromSource)	m_source_emission_multiplier = DefaultValues<T>::OneValue();
	else															m_source_emission_multiplier = DefaultValues<T>::NullValue();
}


// Executes an update of the map.  Accepts a reference to the underlying element collection as a mandatory parameter
template <typename T, template<typename> class TBlendMode>
Result EnvironmentMap<T, TBlendMode>::Execute(const ComplexShipElement *elements)
{
#	if (ENV_MAP_DEBUG_OUTPUT)
		ENV_MAP_DEBUG_LOG("Beginning new environment map update\nSources: {");
		for (size_t i = 0; i < m_sources.size(); ++i) ENV_MAP_DEBUG_LOG(concat("[Index=")(m_sources[i].Index)(", Value=")(m_sources[i].Value)("] ").str().c_str());
		ENV_MAP_DEBUG_LOG("}\n");
#	endif

	// Set initial value for all cells, UNLESS we have chosen to retain the existing data
	if (!m_preserve_existing_data)
	{
		ENV_MAP_DEBUG_LOG("Initialising all cells to starting values\n");
		if (m_fixed_initial_value)
			InitialiseCellValues(m_initial_value);

		else
			InitialiseCellValues(m_initial_value_min, m_initial_value_max);
	}
	else
	{
		// If we are preserving the existing data, apply any modifier that has been specified
		if (m_update_existing_data == ExistingDataUpdate::AdditiveUpdate)
		{
			ENV_MAP_DEBUG_LOG(concat("Applying additive pre-processing operation of ")(m_existing_data_additive_update)(" to all cells\n").str().c_str());
			for (std::vector<T>::size_type i = 0; i < m_elementcount; ++i) Data[i] = ((std::max)(m_zero, Data[i] + m_existing_data_additive_update));
		}
		else if (m_update_existing_data == ExistingDataUpdate::MultiplicativeUpdate)
		{
			ENV_MAP_DEBUG_LOG(concat("Applying multiplicative pre-processing operation of ")(m_existing_data_multiplicative_update)(" to all cells\n").str().c_str());
			for (std::vector<T>::size_type i = 0; i < m_elementcount; ++i) Data[i] = (T)((float)Data[i] * m_existing_data_multiplicative_update);
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
		ENV_MAP_DEBUG_LOG(concat("> Beginning update for source ")((*it).Index)("\n").str().c_str());
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
			ENV_MAP_DEBUG_LOG(concat(">> Processing queue element ")(queue_index)(" at index ")(index)(", current value = ")(Data[index])("\n").str().c_str());

			// Increment the queue index
			++queue_index;

			// We want to check all neighbours of this cell that have not yet been processed.  Select a random sequence
			// to ensure variation in the spread of value through the map
			const int *directions = EnvironmentMap<T, TBlendMode>::DirectionSequenceData.RandomSequence();
			for (int i = 0; i < Direction::_Count; ++i)
			{
				// Get the neighbour and perform basic checks of whether it should be processed
				Direction direction = (Direction)directions[i];
				int neighbour = el.GetNeighbour(direction);
				if (neighbour == -1) continue;									// Must be within the environment map area; -1 signifies not a valid element
				if (updated[neighbour] != 0) continue;							// Ignore any elements we have already updated
				updated[neighbour] = 1;											// Mark this neighbour as processed

				// Check against both transmission and blocking element properties
				bitstring neighbour_properties = elements[neighbour].GetProperties();
				if ((CheckBit_All_NotSet(neighbour_properties, m_transmission_properties)) ||
					(CheckBit_Any(neighbour_properties, m_blocking_properties))) continue;

				// We want to transmit to this cell.  Calculate the change in value between source and destination
				T neighbour_new = m_blend.Apply(Data[index], Data[neighbour]);
				T emitted = (neighbour_new - Data[neighbour]);
				if (emitted > m_transfer_limit) emitted = m_transfer_limit;

				// We cannot emit any of the value that *entered* the cell this cycle; this portion is reserved
				
				// Now calculate the transmitted value.  If it has fallen below the zero threshold then we can stop propogating it here
				T transmitted = m_falloff.ApplyFalloff(emitted, direction);
				transmitted = max(m_zero, transmitted);

				// Remove from the emitting cell (if applicable) and blend this value into the target cell
				ENV_MAP_DEBUG_LOG(concat(">>> Emitted ")(emitted)(", ")(transmitted)(" of which was transmitted to neighbour ")(neighbour)(" (direction: ")(DirectionToString(direction))(")\n").str().c_str());
				ENV_MAP_DEBUG_LOG(concat(">>> BEFORE: Cell value = ")(Data[index])(", neighbour value = ")(Data[neighbour])("\n").str().c_str());
				Data[index] -= (emitted * m_source_emission_multiplier);
				Data[neighbour] += transmitted;
				ENV_MAP_DEBUG_LOG(concat(">>> AFTER:  Cell value = ")(Data[index])(", neighbour value = ")(Data[neighbour])("\n").str().c_str());

				// We want to move on and process all this cell's neighbours in a future cycle, as long as 
				// it is above the zero threshold.  If not, we can stop propogating here
				if (Data[neighbour] > m_zero_threshold)
				{
					queue.push_back(neighbour);
				}
			}

		}

	}

	// Delete any temporarily-allocate memory
	delete[] updated;


	// Apply any post-processing value constraints
	if (m_value_constraints == MapValueConstraints::ValueClamped)
		for (std::vector<T>::size_type i = 0; i < m_elementcount; ++i) Data[i] = clamp(Data[i], m_value_floor, m_value_ceiling);
	else if (m_value_constraints == MapValueConstraints::ValueFloor)
		for (std::vector<T>::size_type i = 0; i < m_elementcount; ++i) Data[i] = max(Data[i], m_value_floor);
	else if (m_value_constraints == MapValueConstraints::ValueCeiling)
		for (std::vector<T>::size_type i = 0; i < m_elementcount; ++i) Data[i] = min(Data[i], m_value_ceiling);


	// Return success
	ENV_MAP_DEBUG_LOG("Environment map update completed\n");
	return ErrorCodes::NoError;
}



// Returns a debug string representation of the environment map contents, for the selected set of z-levels
// Internal method which returns a debug string output representing the set of element states, given 
// the specified tile orientation(s) to be returned. Shown as a 2D x/y representation, with z 
// values represented within an array at each element
template <typename T, template<typename> class TBlendMode>
std::string EnvironmentMap<T, TBlendMode>::DebugStringOutput(int z_start, int z_end, const char *format)
{
	if (z_start > z_end || z_start < 0 || z_end >= m_elementsize.z) return "";
	int zsize = ((z_end - z_start) + 1);
	concat result = concat("");

	for (int y = (m_elementsize.y - 1); y >= 0; --y)			// Note: decrement y from max>min, since elements are arranged with the origin at bottom-bottom-left 
	{
		for (int x = 0; x < m_elementsize.x; ++x)
		{
			result("[");
			for (int z = z_start; z <= z_end; ++z)
			{
				result((z != z_start ? "," : ""));
				if (!format)
					result(Data[ELEMENT_INDEX_EX(x, y, z, m_elementsize)]);			
				else
				{
					char str[128]; memset(str, 0, 128);
					sprintf(str, format, Data[ELEMENT_INDEX_EX(x, y, z, m_elementsize)]);
					result(str);
				}
			}
			result("] ");
		}
		result("\n");
	}
	return result.str();
}



#endif




