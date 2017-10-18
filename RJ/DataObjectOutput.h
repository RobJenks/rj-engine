#pragma once

#include "DynamicTerrain.h"


template <unsigned int N>
class DataObjectOutput : public DynamicTerrain
{
	// Acceptable output count
	static_assert(N <= 10U, "Not a valid data output size");

public:

	// Default constructor
	DataObjectOutput(void);

	// Creates the new data-enabled object, including registration of all required data ports
	// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
	static DataObjectOutput *			Create(const TerrainDefinition *def);

	// Initialise the data ports required for this object
	void								InitialiseDataPorts(void);

	// Method invoked when this object receives data through one of its public input ports
	// A value output has no inputs and is never able to receive any data
	CMPINLINE void						DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port) { }

	// Return port indices for the object
	CMPINLINE DataPorts::PortIndex		OutputPort(unsigned int output_index) const { return m_port_indices[output_index]; }

	// Output a value from the commponent through the given output
	void								SendOutput(unsigned int output_index, DataPorts::DataType value);


private:

	// Maintain port indices for convenience
	DataPorts::PortIndex				m_port_indices[N];

};


// Default constructor
template<unsigned int N>
DataObjectOutput<N>::DataObjectOutput(void)
{
	for (unsigned int i = 0U; i < N; ++i)
	{
		m_port_indices[i] = DefaultValues<DataPorts::PortIndex>::NullValue();
	}
}


// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
template <unsigned int N>
DataObjectOutput<N> * DataObjectOutput<N>::Create(const TerrainDefinition *def)
{
	// Create and initialise the underlying terrain object
	DataObjectOutput<N> *object = new DataObjectOutput<N>();
	object->InitialiseNewTerrain(def);

	// Initialise the data ports required for this object
	object->InitialiseDataPorts();

	// Return the new object
	return object;
}

// Initialise the data ports required for this object
template <unsigned int N>
void DataObjectOutput<N>::InitialiseDataPorts(void)
{
	for (unsigned int i = 0U; i < N; ++i)
	{
		m_port_indices[i] = RegisterNewPort(DataPorts::PortType::OutputPort);
	}
}

// Output a value from the commponent through the given output
template <unsigned int N>
void DataObjectOutput<N>::SendOutput(unsigned int output_index, DataPorts::DataType value)
{
	if (output_index < N) SendData(m_port_indices[output_index], value);
}


// Define aliases for small-sized value outputs
typedef DataObjectOutput<1>				DataObjectOutput1;
typedef DataObjectOutput<2>				DataObjectOutput2;
typedef DataObjectOutput<3>				DataObjectOutput3;
typedef DataObjectOutput<4>				DataObjectOutput4;
typedef DataObjectOutput<5>				DataObjectOutput5;
typedef DataObjectOutput<6>				DataObjectOutput6;
typedef DataObjectOutput<7>				DataObjectOutput7;
typedef DataObjectOutput<8>				DataObjectOutput8;
typedef DataObjectOutput<9>				DataObjectOutput9;
typedef DataObjectOutput<10>			DataObjectOutput10;


