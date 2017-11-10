#pragma once

#include "DynamicTerrain.h"


template <unsigned int N>
DYNAMIC_TERRAIN_ABSTRACT_SUPERCLASS(DataObjectOutput)
//{
	// Acceptable output count
	static_assert(N <= 10U, "Not a valid data output size");

public:

	// Creates the new data-enabled object, including registration of all required data ports
	// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
	static DataObjectOutput *			Create(const TerrainDefinition *def);
	static DataObjectOutput *			Create(const TerrainDefinition *def, DataObjectOutput<N> *instance);
	
	// Initialise the data ports required for this object
	void								InitialiseDataPorts(void);

	// Method invoked when this object receives data through one of its public input ports
	// A value output has no inputs and is never able to receive any data
	CMPINLINE void						DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port) { }

	// Return port indices for the object
	CMPINLINE DataPorts::PortIndex		OutputPort(unsigned int output_index) const { return m_port_indices[output_index]; }

	// Output a value from the commponent through the given output
	void								SendOutput(unsigned int output_index, DataPorts::DataType value);


protected:

	// Default constructor
	DataObjectOutput(void);


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

// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
template <unsigned int N>
DataObjectOutput<N> * DataObjectOutput<N>::Create(const TerrainDefinition *def)
{
	return Create(def, NULL);
}

// Creates the new data-enabled object, including registration of all required data ports
// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
// This is a version of the Create() method that supports subclassing by other dynamic terrain types; it 
// will use 'instance' as the new object to be initialised if it is provided, otherwise a new object
// will be created as normal
template <unsigned int N>
DataObjectOutput<N> * DataObjectOutput<N>::Create(const TerrainDefinition *def, DataObjectOutput<N> *instance)
{
	// Create the underlying terrain object, if applicable
	DataObjectOutput *	object = NULL;
	if (instance)		object = static_cast<DataObjectOutput*>(instance);
	else				object = new DataObjectOutput();

	// Initialise the new terrain object based on the provided definition
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


// Define concrete output component types based on total port count
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput1, DataObjectOutput<1U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput2, DataObjectOutput<2U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput3, DataObjectOutput<3U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput4, DataObjectOutput<4U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput5, DataObjectOutput<5U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput6, DataObjectOutput<6U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput7, DataObjectOutput<7U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput8, DataObjectOutput<8U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput9, DataObjectOutput<9U>);
DYNAMIC_TERRAIN_DERIVED_CLASS(DataObjectOutput10, DataObjectOutput<10U>);

