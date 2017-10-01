#include "Utility.h"
#include "FastMath.h"

#include "TileConnections.h"


// Default constructor
TileConnections::TileConnections(void)
	:
	m_data(NULL), m_elementsize(NULL_INTVECTOR3), m_elementcount(0)
{
}

// Initialise the connection data for an area of the specified size.  Clears all data
void TileConnections::Initialise(const INTVECTOR3 & element_size)
{
	// Deallocate any existing data
	DeallocateData();
	
	// Validate and store the desired area size
	m_elementsize = IntVector3Max(element_size, ONE_INTVECTOR3);
	m_elementcount = (m_elementsize.x * m_elementsize.y * m_elementsize.z);

	// Allocate new data for this range of elements
	AllocateData(m_elementsize);
}


// Allocates memory to hold connection data, based on the target element size
bool TileConnections::AllocateData(const INTVECTOR3 & element_size)
{
	// First allocate data per connection type
	m_data = new bitstring*[TileConnections::TileConnectionType::_COUNT];
	if (!m_data) return false;

	// Now allocate and initialise data for all elements within each connection type
	for (unsigned int i = 0; i < TileConnections::TileConnectionType::_COUNT; ++i)
	{
		m_data[i] = new bitstring[m_elementcount];
		memset(m_data[i], 0, sizeof(bitstring) * m_elementcount);
	}

	// Return success
	return true;
}

// Indicates whether a connection of the specified type exists, from the specified element in a particular direction
bool TileConnections::ConnectionExists(TileConnectionType type, int element_index, DirectionBS direction) const
{ 
	return (CheckBit_Any(m_data[(int)type][element_index], direction));
}

// Indicates whether a connection of the specified type exists, from the specified element (in any direction)
bool TileConnections::ConnectionExistsInAnyDirection(TileConnectionType type, int element_index) const
{
	return (CheckBit_Any(m_data[(int)type][element_index], DirectionBS_All));
}

// Indicates whether a connection of the specified type exists in a particular Direction (from any element)
bool TileConnections::ConnectionExistsFromAnyElement(TileConnectionType type, DirectionBS direction) const
{
	for (int i = 0; i < m_elementcount; ++i)
	{
		if (CheckBit_Any(m_data[(int)type][i], direction)) return true;
	}

	return false;
}

// Indicates whether a connection (of any type) exists from the specified element in a particular direction
bool TileConnections::ConnectionExistsOfAnyType(int element_index, DirectionBS direction) const
{
	for (unsigned int i = 0; i < TileConnectionType::_COUNT; ++i)
	{
		if (CheckBit_Any(m_data[i][element_index], direction)) return true;
	}

	return false;
}

// Indicates whether a connection (of any type) exists from the specified element, in any direction
bool TileConnections::ConnectionExistsInAnyDirectionOfAnyType(int element_index) const
{
	for (unsigned int i = 0; i < TileConnectionType::_COUNT; ++i)
	{
		if (CheckBit_Any(m_data[i][element_index], DirectionBS_All)) return true;
	}

	return false;
}

// Indicates whether a connection (of any type) exists in a particular direction, from any element
bool TileConnections::ConnectionExistsFromAnyElementOfAnyType(DirectionBS direction) const
{
	for (unsigned int i = 0; i < TileConnectionType::_COUNT; ++i)
	{
		for (int e = 0; e < m_elementcount; ++e)
		{
			if (CheckBit_Any(m_data[i][e], direction)) return true;
		}
	}

	return false;
}


// Adds a particular connection
void TileConnections::AddConnection(TileConnectionType type, const INTVECTOR3 & location, DirectionBS direction) 
{
	SetBit(m_data[(int)type][ELEMENT_INDEX(location.x, location.y, location.z)], direction);
}

// Removes a particular connection
void TileConnections::RemoveConnection(TileConnectionType type, const INTVECTOR3 & location, DirectionBS direction) 
{
	ClearBit(m_data[(int)type][ELEMENT_INDEX(location.x, location.y, location.z)], direction);
}

// Returns the complete connection state for a particular element & connection type
bitstring TileConnections::GetConnectionState(TileConnectionType type, const INTVECTOR3 & location) const
{
	return (m_data[(int)type][ELEMENT_INDEX(location.x, location.y, location.z)]);
}

// Returns the complete connection state for a particular element & connection type
bitstring TileConnections::GetConnectionState(TileConnectionType type, unsigned int element_index) const
{
	return (m_data[(int)type][element_index]);
}

// Sets the complete connection state for a particular element & connection type
void TileConnections::SetConnectionState(TileConnectionType type, const INTVECTOR3 & location, bitstring state) 
{
	SetConnectionState(type, ELEMENT_INDEX_EX(location.x, location.y, location.z, m_elementsize), state);
}

// Sets the complete connection state for a particular element & connection type
void TileConnections::SetConnectionState(TileConnectionType type, int element_index, bitstring state)
{
	m_data[(int)type][element_index] = state;
}

// Replicate the connection state of one element to all others in the object
void TileConnections::ReplicateConnectionState(int source_element)
{
	if (source_element < 0 || source_element >= m_elementcount) return;
	for (int i = 0; i < m_elementcount; ++i)
	{
		if (i == source_element) continue;
		for (int c = 0; c < TileConnectionType::_COUNT; ++c)
		{
			m_data[c][i] = m_data[c][source_element];
		}
	}
}

// Sets the complete connection state for a particular element & connection type.  Also performs validation on the 
// input data before making any changes.  More appropriate for data read from external files.  Returns a flag
// indicating whether the data was valid and applied
bool TileConnections::ValidateAndSetConnectionState(TileConnectionType type, const INTVECTOR3 & location, bitstring state)
{
	// Validate our current state
	if (!m_data) return false;

	// Validate the input parameters
	if (type < (TileConnectionType)0 || type >= TileConnectionType::_COUNT ||
		!(location >= NULL_INTVECTOR3) || !(location < m_elementsize))
	{
		return false;
	}

	// Input parameters are valid, so set the connection state and return success
	SetConnectionState(type, location, state);
	return true;
}

// Sets the complete connection state based on some other connection state object
void TileConnections::SetConnectionState(const TileConnections & source) 
{ 
	// Parameter check
	bitstring ** const src = source.GetData();
	if (!src) return;

	// Copy each connection state entry in turn
	for (unsigned int i = 0; i < TileConnections::TileConnectionType::_COUNT; ++i)
	{
		for (int e = 0; e < m_elementcount; ++e)
		{
			m_data[i][e] = src[i][e];
		}
	}
}

// Resets the connection state across all elements
void TileConnections::ResetConnectionState(void)
{
	for (unsigned int i = 0; i < TileConnections::TileConnectionType::_COUNT; ++i)
	{
		for (int e = 0; e < m_elementcount; ++e)
		{
			m_data[i][e] = 0U;
		}
	}
}


// Copy constructor
TileConnections::TileConnections(const TileConnections & source)
	: m_data(NULL)
{
	// Initialise this object to the same size as the source object
	Initialise(source.GetElementSize());

	// Copy all data from the source object
	SetConnectionState(source);
}

// Assignment operator
TileConnections & TileConnections::operator=(const TileConnections & rhs)
{
	// Reinitialise the data within this object.  If we already have data then
	// it will be deallocated and reallocated
	Initialise(rhs.GetElementSize());

	// Copy all data from the source object into this object
	SetConnectionState(rhs);

	// Return a reference to this object
	return *this;
}

// Deallocates any existing data
void TileConnections::DeallocateData(void)
{
	// Deallocate the data itself
	if (m_data)
	{
		for (unsigned int i = 0; i < TileConnections::TileConnectionType::_COUNT; ++i)
		{
			if (m_data[i])
			{
				SafeDeleteArray(m_data[i]);		// Delete the data for each element
			}
		}
		SafeDeleteArray(m_data);				// Delete the aggregate element data for each connection type
	}

	// Reset size parameters accordingly
	m_elementsize = NULL_INTVECTOR3;
}

// Tests whether the data in this object is equivalent to the specified object
bool TileConnections::Equals(const TileConnections & other) const
{
	// The objects are clearly not equal if they are of different size
	if (m_elementsize != other.GetElementSize()) return false;

	// Iterate through all the data and return false if any differs
	bitstring ** const other_data = other.GetData();
	for (int i = 0; i < (int)TileConnectionType::_COUNT; ++i)
	{
		for (int e = 0; e < m_elementcount; ++e)
		{
			if (m_data[i][e] != other_data[i][e]) return false;
		}
	}

	// All data was identical
	return true;
}

// Translate a connection type to its string representation
std::string TileConnections::TranslateConnectionTypeToString(TileConnectionType type) const 
{
	switch (type)
	{
		case TileConnectionType::Walkable:		return "Walkable";
		default:								return "Unknown";
	}
}

// Translate a connection type from its string representation
TileConnections::TileConnectionType TileConnections::TranslateConnectionTypeFromString(const std::string & type) const 
{
	std::string str = StrLower(type);
	if (str == "walkable")						return TileConnectionType::Walkable;
	else										return TileConnectionType::_COUNT;
}

// Output a debug string representation of the connection state
std::string TileConnections::DebugString(void) const
{
	std::string str = "Connections { ";
	for (int x = 0; x < m_elementsize.x; ++x) {
		for (int y = 0; y < m_elementsize.y; ++y) {
			for (int z = 0; z < m_elementsize.z; ++z)
			{
				int index = ELEMENT_INDEX_EX(x, y, z, m_elementsize);
				for (int c = 0; c < (int)TileConnectionType::_COUNT; ++c)
				{
					bitstring conn = m_data[c][index];
					if (conn != 0) str = concat(str)("[")(x)(",")(y)(",")(z)("|")
						(TranslateConnectionTypeToString((TileConnectionType)c))("]=")(conn)("  ").str();
				}
			}
		}
	}

	return concat(str)("}").str();
}

// Default destructor
TileConnections::~TileConnections(void)
{
	// Deallocate all heap data before disposing of this object
	DeallocateData();
}



