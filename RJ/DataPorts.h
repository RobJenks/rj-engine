#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include "CompilerSettings.h"
#include "Logging.h"

class DataPorts
{
public:

	// Uniquely-identifying index to a port within a specific environment
	typedef size_t							PortID;

	// Indicates that the object has not yet been assigned a unique port ID
	static const PortID						NO_PORT_ID = ((PortID)0U - (PortID)1U);

	// Index of a port within the scope of its parent object
	typedef size_t							PortIndex;

	// Indicates that the object has not yet been assigned a port index within its parent object
	static const PortIndex					NO_PORT_INDEX = ((PortIndex)0U - (PortIndex)1U);

	// Available port types
	enum									PortType
	{
											InputPort = 0,
											OutputPort
	};

	// Indicates whether a connection can be made between the specified port types
	static bool								PortTypesAreCompatible(PortType port0, PortType port1);

	// The type of data transmitted by all data ports
	struct									DataType
	{
	public:
		typedef float						TFloat;
		typedef UINT32						TUInt;
		typedef INT32						TInt;
		typedef bool						TBool;

		// The 4-byte payload is stored as a float by standard
	private:
		TFloat								Data;

	public:

		// Constructors
		CMPINLINE DataType(void) noexcept				: Data(0.0f) { }
		CMPINLINE DataType(TFloat value) noexcept		: Data(value) { }
		CMPINLINE DataType(TUInt value) noexcept		: Data((TFloat)value) { }
		CMPINLINE DataType(TInt value) noexcept			: Data((TFloat)value) { }
		CMPINLINE DataType(TBool value) noexcept		: Data(value ? 1.0f : 0.0f) { }

		// Return the data packet in the specified format
		CMPINLINE TFloat								FloatValue(void) const { return Data; }
		CMPINLINE TUInt									UIntValue(void)  const { return (TUInt)Data; }
		CMPINLINE TInt									IntValue(void)   const { return (TInt)Data; }
		CMPINLINE TBool									BoolValue(void)  const { return (Data != 0.0f); }

		// Set the value of the data packet in the given format
		CMPINLINE void									Set(TFloat value) { Data = value; }
		CMPINLINE void									Set(TUInt value) { Data = (TFloat)value; }
		CMPINLINE void									Set(TInt value) { Data = (TFloat)value; }
		CMPINLINE void									Set(TBool value) { Data = (value ? 1.0f : 0.0f); }

		// Non-const assignment operators
		CMPINLINE DataType & DataType::operator=(TFloat value) { Data = value; return *this; }
		CMPINLINE DataType & DataType::operator=(TUInt value) { Data = (TFloat)value; return *this; }
		CMPINLINE DataType & DataType::operator=(TInt value) { Data = (TFloat)value; return *this; }
		CMPINLINE DataType & DataType::operator=(TBool value) { Data = (value ? 1.0f : 0.0f); return *this; }
		
		// Conversion operators directly to float data types
		CMPINLINE operator TFloat&(void)				{ return Data; }
		CMPINLINE operator const TFloat&(void) const	{ return Data; }

		// Equality comparisons
		CMPINLINE bool operator==(const DataType &rhs) const { return (Data == rhs.Data); }
		CMPINLINE bool operator!=(const DataType &rhs) const { return (Data != rhs.Data); }

		// Returns a zero-valued data object
		CMPINLINE static DataType Zero(void) { return DataType(0.0f); }

		// Custom string serialisation for data values
		std::string str(void) const;

		// Overload for standard ostream output 
		CMPINLINE friend std::ostream & operator<<(std::ostream & lhs, DataType & rhs)
		{
			return lhs.operator<<(rhs.str().c_str());
		}
		CMPINLINE friend std::ostream & operator<<(std::ostream & lhs, const DataType & rhs)
		{
			return lhs.operator<<(rhs.str().c_str());
		}

	};

private:


};
