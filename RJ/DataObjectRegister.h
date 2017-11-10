#pragma once

#include <assert.h>
#include "DefaultValues.h"
#include "DataPorts.h"
#include "DynamicTerrain.h"

template <unsigned int N>
DYNAMIC_TERRAIN_ABSTRACT_SUPERCLASS(DataObjectRegister)
//{
	// Acceptable register count
	static_assert(N <= 10U, "Not a valid data register size");

public:

	// Creates the new data-enabled object, including registration of all required data ports
	// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
	static DataObjectRegister *					Create(const TerrainDefinition *def);
	static DataObjectRegister *					Create(const TerrainDefinition *def, DataObjectRegister *instance);

	// Initialise the data ports required for this object
	void										InitialiseDataPorts(void);

	// Method invoked when this object receives data through one of its public input ports
	void										DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port);

	// Returns the number of registers available in this object
	CMPINLINE constexpr DataPorts::PortIndex	GetRegisterCount(void) const { return N; }

	// Returns the port index of the given register, or zero if the given register number is invalid
	DataPorts::PortIndex						InputPort(unsigned int register_index) const;

	// Returns the value in a particular register, or zero if the given register index is invalid
	DataPorts::DataType							GetValue(unsigned int register_index) const;


protected:

	// Default constructor
	DataObjectRegister(void);


private:

	// Value registers
	DataPorts::DataType							m_registers[N];

	// Port indices, though it is very likely that (index(k) == k) in all cases
	DataPorts::PortIndex						m_port_indices[N];

};


// Default constructor
template<unsigned int N>
DataObjectRegister<N>::DataObjectRegister(void)
{
	for (unsigned int i = 0U; i < N; ++i)
	{
		m_port_indices[i] = DataPorts::NO_PORT_INDEX;
		m_registers[i] = DataPorts::DataType::Zero();
	}
}

// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
template<unsigned int N>
DataObjectRegister<N> * DataObjectRegister<N>::Create(const TerrainDefinition *def)
{
	return Create(def, NULL);
}

// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
// This is a version of the Create() method that supports subclassing by other dynamic terrain types; it 
// will use 'instance' as the new object to be initialised if it is provided, otherwise a new object
// will be created as normal
template<unsigned int N>
DataObjectRegister<N> * DataObjectRegister<N>::Create(const TerrainDefinition *def, DataObjectRegister<N> *instance)
{
	// Create the underlying terrain object, if applicable
	DataObjectRegister *object = NULL;
	if (instance)		object = static_cast<DataObjectRegister*>(instance);
	else				object = new DataObjectRegister();

	// Initialise the new terrain object based on the provided definition
	object->InitialiseNewTerrain(def);

	// Initialise the data ports required for this object
	object->InitialiseDataPorts();

	// Return the new object
	return object;
}

// Initialise the data ports required for this object
template<unsigned int N>
void DataObjectRegister<N>::InitialiseDataPorts(void)
{
	for (unsigned int i = 0; i < N; ++i)
	{
		m_port_indices[i] = RegisterNewPort(DataPorts::PortType::InputPort);
	}
}


// Method invoked when this object receives data through one of its public input ports
template<unsigned int N>
void DataObjectRegister<N>::DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port)
{
	m_registers[port_index] = data;
}

// Returns the port index of the given register, or NO_PORT_INDEX if the given register number is invalid
template <unsigned int N>
DataPorts::PortIndex DataObjectRegister<N>::InputPort(unsigned int register_index) const
{
	if (register_index < N) return m_port_indices[register_index];
	return DataPorts::NO_PORT_INDEX;
}

// Returns the port index for a single-value register only
/*template <unsigned int N>
typename std::enable_if<std::is_same<N, 1>,
DataPorts::PortIndex> DataObjectRegister<N>::InputPort(void) const
{
return m_port_indices[0];
}*/


// Returns the value in a particular register, or zero if the given register index is invalid
template <unsigned int N>
DataPorts::DataType DataObjectRegister<N>::GetValue(unsigned int register_index) const
{
	if (register_index < N) return m_registers[register_index];
	return DataPorts::DataType::Zero();
}


// Define concrete register types based on the internal register count
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister1, DataObjectRegister<1U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister2, DataObjectRegister<2U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister3, DataObjectRegister<3U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister4, DataObjectRegister<4U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister5, DataObjectRegister<5U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister6, DataObjectRegister<6U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister7, DataObjectRegister<7U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister8, DataObjectRegister<8U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister9, DataObjectRegister<9U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectRegister10, DataObjectRegister<10U>);


