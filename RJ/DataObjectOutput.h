#pragma once

#include "DynamicTerrain.h"


template <unsigned int N>
DYNAMIC_TERRAIN_CLASS(DataObjectOutput)
//{
	// Acceptable output count
	static_assert(N <= 10U, "Not a valid data output size");

public:

	// Default constructor
	DataObjectOutput(void);

	// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
	// as registering new port assignments; all general data should be retained through clone copy-construction
	void								InitialiseDynamicTerrain(void);

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
		m_port_indices[i] = DataPorts::NO_PORT_INDEX;
	}
}

// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
// as registering new port assignments; all general data should be retained through clone copy-construction
template <unsigned int N>
void DataObjectOutput<N>::InitialiseDynamicTerrain(void)
{
	// Initialise the data ports required for this object
	InitialiseDataPorts();
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


// Define concrete output component types based on total port count
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput1, DataObjectOutput<1U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput2, DataObjectOutput<2U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput3, DataObjectOutput<3U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput4, DataObjectOutput<4U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput5, DataObjectOutput<5U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput6, DataObjectOutput<6U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput7, DataObjectOutput<7U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput8, DataObjectOutput<8U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput9, DataObjectOutput<9U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectOutput10, DataObjectOutput<10U>);

