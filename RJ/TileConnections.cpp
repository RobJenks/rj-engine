#include "Utility.h"
#include "FastMath.h"

#include "TileConnections.h"


// Default constructor
TileConnections::TileConnections(void)
	:
	m_data(NULL), m_elementsize(NULL_INTVECTOR3), m_elementcount(0), m_xy_size(0)
{
}

// Initialise the connection data for an area of the specified size.  Clears all data
void TileConnections::Initialise(const INTVECTOR3 & element_size)
{
	// Deallocate any existing data
	DeallocateData();
	
	// Validate and store the desired area size
	m_elementsize = IntVector3Max(element_size, ONE_INTVECTOR3);
	m_xy_size = (m_elementsize.x * m_elementsize.y);
	m_elementcount = (m_xy_size * m_elementsize.z);

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
bool TileConnections::ConnectionExists(TileConnectionType type, const INTVECTOR3 & location, DirectionBS direction) const
{ 
	return (CheckBit_Any(m_data[(int)type][TCONN_EL_INDEX(location.x, location.y, location.z)], direction));
}

// Indicates whether a connection of the specified type exists, from the specified element (in any direction)
bool TileConnections::ConnectionExists(TileConnectionType type, const INTVECTOR3 & location) const
{
	return (CheckBit_Any(m_data[(int)type][TCONN_EL_INDEX(location.x, location.y, location.z)], DirectionBS_All));
}

// Indicates whether a connection of the specified type exists in a particular Direction (from any element)
bool TileConnections::ConnectionExists(TileConnectionType type, DirectionBS direction) const
{
	for (int i = 0; i < m_elementcount; ++i)
	{
		if (CheckBit_Any(m_data[(int)type][i], direction)) return true;
	}

	return false;
}

// Indicates whether a connection (of any type) exists from the specified element in a particular direction
bool TileConnections::ConnectionExists(const INTVECTOR3 & location, DirectionBS direction) const
{
	for (unsigned int i = 0; i < TileConnectionType::_COUNT; ++i)
	{
		if (CheckBit_Any(m_data[i][TCONN_EL_INDEX(location.x, location.y, location.z)], direction)) return true;
	}

	return false;
}

// Indicates whether a connection (of any type) exists from the specified element, in any direction
bool TileConnections::ConnectionExists(const INTVECTOR3 & location) const
{
	for (unsigned int i = 0; i < TileConnectionType::_COUNT; ++i)
	{
		if (CheckBit_Any(m_data[i][TCONN_EL_INDEX(location.x, location.y, location.z)], DirectionBS_All)) return true;
	}

	return false;
}

// Indicates whether a connection (of any type) exists in a particular direction, from any element
bool TileConnections::ConnectionExists(DirectionBS direction) const
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
	SetBit(m_data[(int)type][TCONN_EL_INDEX(location.x, location.y, location.z)], direction);
}

// Removes a particular connection
void TileConnections::RemoveConnection(TileConnectionType type, const INTVECTOR3 & location, DirectionBS direction) 
{
	ClearBit(m_data[(int)type][TCONN_EL_INDEX(location.x, location.y, location.z)], direction);
}

// Returns the complete connection state for a particular element & connection type
bitstring TileConnections::GetConnectionState(TileConnectionType type, const INTVECTOR3 & location) const
{
	return (m_data[(int)type][TCONN_EL_INDEX(location.x, location.y, location.z)]);
}

// Returns the complete connection state for a particular element & connection type
bitstring TileConnections::GetConnectionState(TileConnectionType type, unsigned int element_index) const
{
	return (m_data[(int)type][element_index]);
}

// Sets the complete connection state for a particular element & connection type
void TileConnections::SetConnectionState(TileConnectionType type, const INTVECTOR3 & location, bitstring state) 
{
	m_data[(int)type][TCONN_EL_INDEX(location.x, location.y, location.z)] = state;
}

// Sets the complete connection state based on some other connection state object
void TileConnections::SetConnectionState(const TileConnections & source) 
{ 
	bitstring ** const src = source.GetData();
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
	m_xy_size = 0;
}

// Default destructor
TileConnections::~TileConnections(void)
{
	// Deallocate all heap data before disposing of this object
	DeallocateData();
}
