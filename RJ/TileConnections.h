#pragma once

#ifndef __TileConnectionsH__
#define __TileConnectionsH__

#include "Utility.h"


class TileConnections
{
public:

	// Enumeration of possible tile connection types
	enum TileConnectionType { Walkable = 0, _COUNT };

	// Default constructor
	TileConnections(void);

	// Copy constructor
	TileConnections(const TileConnections & source);

	// Assignment operator
	TileConnections & operator=(const TileConnections & rhs);

	// Initialise the connection data for an area of the specified size.  Clears all data
	void							Initialise(const INTVECTOR3 & element_size);

	// Indicates whether a connection of the specified type exists, from the specified element in a particular direction
	bool							ConnectionExists(TileConnectionType type, const INTVECTOR3 & location, DirectionBS direction) const;

	// Indicates whether a connection of the specified type exists, from the specified element (in any direction)
	bool							ConnectionExists(TileConnectionType type, const INTVECTOR3 & location) const;

	// Indicates whether a connection of the specified type exists in a particular direction (from any element)
	bool							ConnectionExists(TileConnectionType type, DirectionBS direction) const;

	// Indicates whether a connection (of any type) exists from the specified element in a particular direction
	bool							ConnectionExists(const INTVECTOR3 & location, DirectionBS direction) const;

	// Indicates whether a connection (of any type) exists from the specified element, in any direction
	bool							ConnectionExists(const INTVECTOR3 & location) const;

	// Indicates whether a connection (of any type) exists in a particular direction, from any element
	bool							ConnectionExists(DirectionBS direction) const;

	// Adds a particular connection
	void							AddConnection(TileConnectionType type, const INTVECTOR3 & location, DirectionBS direction);

	// Removes a particular connection
	void							RemoveConnection(TileConnectionType type, const INTVECTOR3 & location, DirectionBS direction);

	// Returns the complete connection state for a particular element & connection type
	bitstring						GetConnectionState(TileConnectionType type, const INTVECTOR3 & location) const;

	// Returns the complete connection state for a particular element & connection type
	bitstring						GetConnectionState(TileConnectionType type, unsigned int element_index) const;

	// Sets the complete connection state for a particular element & connection type
	void							SetConnectionState(TileConnectionType type, const INTVECTOR3 & location, bitstring state);

	// Sets the complete connection state for a particular element & connection type.  Also performs validation on the 
	// input data before making any changes.  More appropriate for data read from external files.  Returns a flag
	// indicating whether the data was valid and applied
	bool							ValidateAndSetConnectionState(TileConnectionType type, const INTVECTOR3 & location, bitstring state);

	// Sets the complete connection state based on some other connection state object
	void							SetConnectionState(const TileConnections & source);


	// Resets the connection state across all elements
	void							ResetConnectionState(void);

	// Returns an immutable pointer to the raw connection data
	CMPINLINE bitstring **			GetData(void) const						{ return m_data; }

	// Returns the size of the area covered by this object
	CMPINLINE INTVECTOR3			GetElementSize(void) const				{ return m_elementsize; }

	// Returns the number of elements covered by this object
	CMPINLINE int					GetElementCount(void) const				{ return m_elementcount; }

	// Tests whether the data in this object is equivalent to the specified object
	bool							Equals(const TileConnections & other) const;

	// Default destructor
	~TileConnections(void);


protected:

	// Allocates memory to hold connection data, based on the target element size
	bool							AllocateData(const INTVECTOR3 & element_size);

	// Deallocates any existing data
	void							DeallocateData(void);



	// Generates an index into the data collection based on x/y/z coordinates
#	define TCONN_EL_INDEX(_x, _y, _z) (_x + (_y * m_elementsize.x) + (_z * m_xy_size))

	// Raw data used to store the connection information
	bitstring **					m_data;		// (data[el_index])[connection_type], where each bitstring specifies the directions

	// Size of the area covered
	INTVECTOR3						m_elementsize;

	// Total number of elements covered
	int								m_elementcount;

	// Precalculated for efficiency
	int								m_xy_size;
};


#endif




