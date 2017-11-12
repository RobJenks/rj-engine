#pragma once

#include "DynamicTerrain.h"


template <unsigned int N>
DYNAMIC_TERRAIN_CLASS(DataObjectInput)
//{
// Acceptable output count
static_assert(N <= 10U, "Not a valid data input size");

public:

	// Default constructor
	DataObjectInput(void);

	// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
	// as registering new port assignments; all general data should be retained through clone copy-construction
	void								InitialiseDynamicTerrain(void);

	// Initialise the data ports required for this object
	void								InitialiseDataPorts(void);

	// Method invoked when this object receives data through one of its public input ports
	// By default the input object will simply consume inputs, unless behaviour is overridden in a derived class
	CMPINLINE void						DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port) { }

	// Return port indices for the object
	CMPINLINE DataPorts::PortIndex		InputPort(unsigned int input_index) const { return m_port_indices[input_index]; }


private:

	// Maintain port indices for convenience
	DataPorts::PortIndex				m_port_indices[N];

};


// Default constructor
template<unsigned int N>
DataObjectInput<N>::DataObjectInput(void)
{
	for (unsigned int i = 0U; i < N; ++i)
	{
		m_port_indices[i] = DataPorts::NO_PORT_INDEX;
	}
}

// Initialises a new instance after it has been created.  Primarily respsonsible for per-instance data such
// as registering new port assignments; all general data should be retained through clone copy-construction
template <unsigned int N>
void DataObjectInput<N>::InitialiseDynamicTerrain(void)
{
	// Initialise the data ports required for this object
	InitialiseDataPorts();
}

// Initialise the data ports required for this object
template <unsigned int N>
void DataObjectInput<N>::InitialiseDataPorts(void)
{
	for (unsigned int i = 0U; i < N; ++i)
	{
		m_port_indices[i] = RegisterNewPort(DataPorts::PortType::InputPort);
	}
}


// Define concrete output component types based on total port count
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput1, DataObjectInput<1U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput2, DataObjectInput<2U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput3, DataObjectInput<3U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput4, DataObjectInput<4U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput5, DataObjectInput<5U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput6, DataObjectInput<6U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput7, DataObjectInput<7U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput8, DataObjectInput<8U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput9, DataObjectInput<9U>);
DYNAMIC_TERRAIN_DERIVED_CLASS_COMPLETE(DataObjectInput10, DataObjectInput<10U>);

