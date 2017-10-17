#pragma once

#include <assert.h>
#include "DataPorts.h"
#include "DataEnabledStaticTerrain.h"

template <unsigned int N>
class DataObjectRegisterN: public DataEnabledStaticTerrain
{
	// Acceptable value range for memory objects
	static_assert(N <= 10U, "Not a valid data register size");

public:

	// Default constructor
	DataObjectRegisterN(void);

	// Creates the new data-enabled object, including registration of all required data ports
	// Accepsts a terrain definition for the underlying object, which can be null for an object without any model
	static DataObjectRegisterN *		Create(const StaticTerrainDefinition *def);

	// Initialise the data ports required for this object
	void								InitialiseDataPorts(void);

	// Method invoked when this object receives data through one of its public input ports
	void								DataReceieved(DataPorts::PortIndex port_index, DataPorts::DataType data, DataPorts::PortID source_port);

	// Returns the value in a particular register, or zero if the given register index is invalid
	DataPorts::DataType					GetValue(unsigned int register_index) const;


private:

	// Value registers
	DataPorts::DataType					m_registers[N];

};

// Define concrete register types based on the internal register count
typedef DataObjectRegisterN<1U>			DataObjectRegister;
typedef DataObjectRegisterN<2U>			DataObjectRegister2;
typedef DataObjectRegisterN<3U>			DataObjectRegister3;
typedef DataObjectRegisterN<4U>			DataObjectRegister4;
typedef DataObjectRegisterN<5U>			DataObjectRegister5;
typedef DataObjectRegisterN<6U>			DataObjectRegister6;
typedef DataObjectRegisterN<7U>			DataObjectRegister7;
typedef DataObjectRegisterN<8U>			DataObjectRegister8;
typedef DataObjectRegisterN<9U>			DataObjectRegister9;
typedef DataObjectRegisterN<10U>		DataObjectRegister10;

