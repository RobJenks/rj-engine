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
		typedef float						TFloat;
		typedef UINT32						TUInt;
		typedef INT32						TInt;
		typedef bool						TBool;

		// The 4-byte payload can support UInt, Int or Float data types as standard
		union
		{
			TFloat							FloatValue;
			TUInt							UIntValue;
			TInt							IntValue;
		};

		// We also define a boolean representation based upon the underlying UInt value
		CMPINLINE bool						BoolValue(void) const { return (UIntValue != 0U); }

		// Constructors
		CMPINLINE DataType(void) noexcept				: UIntValue(0U) { }
		CMPINLINE DataType(TFloat value) noexcept		: FloatValue(value) { }
		CMPINLINE DataType(TUInt value) noexcept		: UIntValue(value) { }
		CMPINLINE DataType(TInt value) noexcept			: IntValue(value) { }
		CMPINLINE DataType(TBool value) noexcept		: UIntValue(value ? 1U : 0U) { }

		// Non-const assignment operators
		CMPINLINE DataType & DataType::operator=(TFloat value) { FloatValue = value; return *this; }
		CMPINLINE DataType & DataType::operator=(TUInt value) { UIntValue = value; return *this; }
		CMPINLINE DataType & DataType::operator=(TInt value) { IntValue = value; return *this; }
		CMPINLINE DataType & DataType::operator=(TBool value) { UIntValue = (value ? 1U : 0U); return *this; }

		// Non-const conversion operators
		CMPINLINE operator TUInt&(void) { return UIntValue; }
		CMPINLINE explicit operator TFloat&(void) { return FloatValue; }
		CMPINLINE explicit operator TInt&(void) { return IntValue; }

		// Const conversion operators
		CMPINLINE operator const TUInt&(void) const { return UIntValue; }
		CMPINLINE explicit operator const TFloat&(void) const { return FloatValue; }
		CMPINLINE explicit operator const TInt&(void) const { return IntValue; }
		CMPINLINE explicit operator const TBool&(void) const { return (UIntValue != 0U); }

		// Equality comparisons
		CMPINLINE bool operator==(const DataType &rhs) const { return (UIntValue == rhs.UIntValue); }
		CMPINLINE bool operator!=(const DataType &rhs) const { return (UIntValue != rhs.UIntValue); }

		// Returns a zero-valued data object
		CMPINLINE static DataType Zero(void) { return DataType(0U); }

		// Custom string serialisation for data values
		std::string str(void) const;

		// Overload for standard ostream output 
		CMPINLINE friend std::ostream & operator<<(std::ostream & lhs, const DataType & rhs)
		{
			return lhs.operator<<(rhs.str().c_str());
		}
	};

private:


};
