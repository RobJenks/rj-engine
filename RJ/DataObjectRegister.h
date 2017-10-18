#pragma once

#include <assert.h>
#include "DefaultValues.h"
#include "DataPorts.h"
#include "DataEnabledStaticTerrain.h"

template <unsigned int N>
class DataObjectRegister: public DataEnabledStaticTerrain
{
	// Acceptable register count
	static_assert(N <= 10U, "Not a valid data register size");

public:

	// Default constructor
	DataObjectRegister(void);

	// Creates the new data-enabled object, including registration of all required data ports
	// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
	static DataObjectRegister *					Create(const StaticTerrainDefinition *def);

	// Initialise the data ports required for this object
	void										InitialiseDataPorts(void);

	// Method invoked when this object receives data through one of its public input ports
	void										DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port);

	// Returns the number of registers available in this object
	CMPINLINE constexpr DataPorts::PortIndex	GetRegisterCount(void) const { return N; }

	// Returns the port index of the given register, or zero if the given register number is invalid
	DataPorts::PortIndex						InputPort(unsigned int register_index) const;

	// Returns the port index for a single-value register only
//	typename std::enable_if<std::is_same<N, 1>,
	//	DataPorts::PortIndex>					InputPort(void) const;

	// Returns the value in a particular register, or zero if the given register index is invalid
	DataPorts::DataType							GetValue(unsigned int register_index) const;


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
		m_port_indices[i] = DefaultValues<DataPorts::PortIndex>::NullValue();
		m_registers[i] = DataPorts::DataType::Zero();
	}
}


// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
template<unsigned int N>
DataObjectRegister<N> * DataObjectRegister<N>::Create(const StaticTerrainDefinition *def)
{
	// Create and initialise the underlying terrain object
	DataObjectRegister *object = new DataObjectRegister();
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

// Returns the port index of the given register, or zero if the given register number is invalid
template <unsigned int N>
DataPorts::PortIndex DataObjectRegister<N>::InputPort(unsigned int register_index) const
{
	if (register_index < N) return m_port_indices[register_index];
	return DefaultValues<DataPorts::PortIndex>::NullValue();
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
typedef DataObjectRegister<1U>					DataObjectRegister1;
typedef DataObjectRegister<2U>	   				DataObjectRegister2;
typedef DataObjectRegister<3U>		  			DataObjectRegister3;
typedef DataObjectRegister<4U>   				DataObjectRegister4;
typedef DataObjectRegister<5U>   				DataObjectRegister5;
typedef DataObjectRegister<6U>					DataObjectRegister6;
typedef DataObjectRegister<7U>   				DataObjectRegister7;
typedef DataObjectRegister<8U>   				DataObjectRegister8;
typedef DataObjectRegister<9U>   				DataObjectRegister9;
typedef DataObjectRegister<10U>   				DataObjectRegister10;
