#include "DefaultValues.h"
#include "DataObjectRegisterN.h"

// Default constructor
template<unsigned int N>
DataObjectRegisterN<N>::DataObjectRegisterN(void)
{
	for (unsigned int i = 0U; i < N; ++i)
	{
		m_registers[i] = DefaultValues<DataPorts::DataType>::NullValue();
	}
}


// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
template<unsigned int N>
DataObjectRegisterN<N> * DataObjectRegisterN<N>::Create(const StaticTerrainDefinition *def)
{
	// Create and initialise the underlying terrain object
	DataObjectRegisterN *object = new DataObjectRegisterN();
	object->InitialiseNewTerrain(def);

	// Initialise the data ports required for this object
	object->InitialiseDataPorts();

	// Return the new object
	return object;
}

// Initialise the data ports required for this object
template<unsigned int N>
void DataObjectRegisterN<N>::InitialiseDataPorts(void)
{
	for (unsigned int i = 0; i < N; ++i)
	{
		RegisterNewPort(DataPorts::PortType::InputPort);
	}
}


// Method invoked when this object receives data through one of its public input ports
template<unsigned int N>
void DataObjectRegisterN<N>::DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port)
{
	m_registers[port_index] = data;
}

// Returns the value in a particular register, or zero if the given register index is invalid
template <unsigned int N>
DataPorts::DataType DataObjectRegisterN<N>::GetValue(unsigned int register_index) const
{
	if (register_index < N) return m_registers[register_index];
	return DefaultValues<DataPorts::DataType>::NullValue();
}