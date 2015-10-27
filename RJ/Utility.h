#pragma once

#ifndef __UtilityH__
#define __UtilityH__

#include "DX11_Core.h"

#include <time.h>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "CompilerSettings.h"
using namespace std;
using namespace std::tr1;


// Define memory tracking macros for use in the application
#define MemReg(cp) _CrtMemState cp; _CrtMemCheckpoint( &cp );
#define MemDiffReport(c1, c2, c3) _CrtMemState c3; if ( _CrtMemDifference(&c3, &c1, &c2)) { OutputDebugString("\n\n*** WARNING: ACCUMULATING MEMORY ALLOCATIONS.  INCREMENTAL ALLOCATIONS BELOW ***\n\n"); _CrtMemDumpStatistics( &c3 ); } 
#define MemDump(label,cp) MemReg(cp);OutputDebugString("**** ");OutputDebugString(label);OutputDebugString(": \n");_CrtMemDumpStatistics(&cp);

// Convenience macro to combine multiple arguments, including commas, into one macro argument
#define SINGLE_ARG(...) __VA_ARGS__

// Convenience macro for releasing COM objects.
#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

// Convenience macros for deleting objects.
#define SafeDelete(x) { delete x; x = 0; }
#define SafeDeleteArray(x) { delete[] x; x = 0; }

// Convenience macro for freeing malloc-ed memory
#define SafeFree(x) { free(x); x = 0; }

// Convenience macro for releasing non-null COM resources
#define ReleaseIfExists(x) { if (x) { x->Release(); x = 0; } }

// Accepts either a reference or a pointer and universally returns a pointer
template<typename T>
CMPINLINE T * ptr(T & obj) { return &obj; }	// If object is a reference type then return a pointer
template<typename T>
CMPINLINE T * ptr(T * obj) { return obj; }	// Otherwise if object is already a pointer type then simply return it

// Convenience macros to manipulate bitstrings
#define SetBit(bitstring, bit) bitstring |= bit
#define ClearBit(bitstring, bit) bitstring &= ~bit
#define SetBitState(bitstring, bit, bool_value) (bool_value ? (bitstring |= bit) : (bitstring &= ~bit))

#define CheckBit_All(bitstring, bit) ((bitstring & bit) == bit)
#define CheckBit_Any(bitstring, bit) ((bitstring & bit) != 0)
#define CheckBit_AllExclusive(bitstring, bit) (bitstring == bit)
#define CheckBit_AnyExclusive(bitstring, bit) ((bitstring & bit) != 0 && (bitstring & ~bit) == 0)
#define CheckBit_Single CheckBit_Any

#define CheckBit_All_NotSet(bitstring, bit) ((bitstring & bit) == 0)
#define CheckBit_Any_NotSet(bitstring, bit) ((bitstring & bit) != bit)
#define CheckBit_Single_NotSet CheckBit_All_NotSet

// Convenience macro for min/max clamping of values
#define clamp(x, min_val, max_val) (min((max_val), max((min_val), (x))))

// Convenience macros for listing each component of a vector sequentially, e.g. for component-wise printing
#define Vector2Components(v)	(v.x, v.y)
#define Vector3Components(v)	(v.x, v.y, v.z)
#define Vector4Components(v)	(v.x, v.y, v.z, v.w)

struct INTVECTOR2
{ 
	int x; int y;

	INTVECTOR2(void) { x = 0; y = 0; }
	INTVECTOR2(int _x, int _y) { x = _x; y = _y; }
	INTVECTOR2(float _x, float _y) { x = (int)_x; y = (int)_y; }
	INTVECTOR2(D3DXVECTOR2 v) { x = (int)v.x; y = (int)v.y; }
	INTVECTOR2(int _xy) { x = _xy; y = _xy; }						// For efficiency; allows setting both components to same value

	bool IsZeroVector(void) { return (x == 0 && y == 0); }

	INTVECTOR2& operator +=(const INTVECTOR2& rhs) { this->x += rhs.x; this->y += rhs.y; return *this; }
	INTVECTOR2& operator -=(const INTVECTOR2& rhs) { this->x -= rhs.x; this->y -= rhs.y; return *this; }
	INTVECTOR2& operator *=(const int scalar) { this->x *= scalar; this->y *= scalar; return *this; }
	INTVECTOR2& operator *=(const float scalar) { this->x = (int)(this->x * scalar); this->y = (int)(this->y * scalar); return *this; }
	bool operator ==(const INTVECTOR2& rhs) { return (this->x == rhs.x && this->y == rhs.y); }
	bool operator !=(const INTVECTOR2& rhs) { return (this->x != rhs.x || this->y != rhs.y); }
};
inline INTVECTOR2 operator +(INTVECTOR2 lhs, const INTVECTOR2& rhs) { lhs += rhs; return lhs; }
inline INTVECTOR2 operator -(INTVECTOR2 lhs, const INTVECTOR2& rhs) { lhs -= rhs; return lhs; }
inline INTVECTOR2 operator *(INTVECTOR2 lhs, const int scalar) { lhs *= scalar; return lhs; }
inline INTVECTOR2 operator *(INTVECTOR2 lhs, const float scalar) { lhs *= scalar; return lhs; }


struct INTVECTOR3 
{ 
	int x; int y; int z; 

	INTVECTOR3(void) { x = 0; y = 0; z = 0; }
	INTVECTOR3(int _x, int _y, int _z) { x = _x; y = _y; z = _z; }
	INTVECTOR3(float _x, float _y, float _z) { x = (int)_x; y = (int)_y; z = (int)_z; }
	INTVECTOR3(const D3DXVECTOR3 &v) { x = (int)v.x; y = (int)v.y; z = (int)v.z; }

	bool IsZeroVector(void) { return (x == 0 && y == 0 && z == 0); }

	INTVECTOR3& operator +=(const INTVECTOR3& rhs) { this->x += rhs.x; this->y += rhs.y; this->z += rhs.z; return *this; }
	INTVECTOR3& operator -=(const INTVECTOR3& rhs) { this->x -= rhs.x; this->y -= rhs.y; this->z -= rhs.z; return *this; }	
	INTVECTOR3& operator *=(const int scalar) { this->x *= scalar; this->y *= scalar; this->z *= scalar; return *this; }
	INTVECTOR3& operator *=(const float scalar) { this->x = (int)(this->x * scalar); this->y = (int)(this->y * scalar); 
												  this->z = (int)(this->z * scalar); return *this; }
	INTVECTOR3& operator /=(const int scalar) { this->x /= scalar; this->y /= scalar; this->z /= scalar; return *this; }
	INTVECTOR3& operator /=(const float scalar) { this->x = (int)((float)this->x / scalar); this->y = (int)((float)this->y / scalar); 
												  this->z = (int)((float)this->z / scalar); return *this; }
	INTVECTOR3& operator +=(const int scalar) { this->x += scalar; this->y += scalar; this->z += scalar; return *this; }
	INTVECTOR3& operator -=(const int scalar) { this->x -= scalar; this->y -= scalar; this->z -= scalar; return *this; }
	bool operator ==(const INTVECTOR3& rhs) { return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z); }
	bool operator !=(const INTVECTOR3& rhs) { return (this->x != rhs.x || this->y != rhs.y || this->z != rhs.z); }
	bool operator ==(const INTVECTOR3& rhs) const { return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z); }
	bool operator !=(const INTVECTOR3& rhs) const { return (this->x != rhs.x || this->y != rhs.y || this->z != rhs.z); }
};
inline INTVECTOR3 operator +(INTVECTOR3 lhs, const INTVECTOR3& rhs) { lhs += rhs; return lhs; }
inline INTVECTOR3 operator -(INTVECTOR3 lhs, const INTVECTOR3& rhs) { lhs -= rhs; return lhs; }
inline INTVECTOR3 operator *(INTVECTOR3 lhs, const int scalar) { lhs *= scalar; return lhs; }
inline INTVECTOR3 operator *(INTVECTOR3 lhs, const float scalar) { lhs *= scalar; return lhs; }
inline INTVECTOR3 operator /(INTVECTOR3 lhs, const int scalar) { lhs /= scalar; return lhs; }
inline INTVECTOR3 operator /(INTVECTOR3 lhs, const float scalar) { lhs /= scalar; return lhs; }

void clower(char *);			// Converts a string to lowercase in-place
void cupper(char *);			// Converts a string to uppercase in-place
char *lower(const char *);		// Converts a string to lowercase as a new string
char *upper(const char *);		// Converts a string to uppercase as a new string

void StrLowerC(string &);				// Converts a std::string to lowercase in-place
void StrUpperC(string &);				// Converts a std::string to uppercase in-place
string StrLower(const string &);		// Converts a std::string to lowercase as a new string
string StrUpper(const string &);		// Converts a std::string to uppercase as a new string

char *BuildFilename(const char *, const char *);
string BuildStrFilename(const string &, const string &);

// Enumeration of possible comparison results
enum ComparisonResult { Equal = 0, LessThan, GreaterThan };

// Enumeration of possible 90-degree rotations
enum Rotation90Degree { Rotate0 = 0, Rotate90 = 1, Rotate180 = 2, Rotate270 = 3 };
Rotation90Degree TranslateRotation90Degree(string rotvalue);
Rotation90Degree RotateBy90Degrees(Rotation90Degree current);

// Enumeration of possible 90-degree directions
enum Direction { None = 0, Left, Up, Right, Down, UpLeft, UpRight, DownRight, DownLeft, ZUp, ZDown };
const int DirectionCount = 10;

// Structure holding the basic directions, for iteration over the possible movement directions
const int Directions[DirectionCount] = { Direction::Left, Direction::Up, Direction::Right, Direction::Down, Direction::UpLeft, 
										 Direction::UpRight, Direction::DownRight, Direction::DownLeft,  Direction::ZUp, Direction::ZDown };

// Structure that holds the effect of applying a rotation to each direction value (in 2D only)
const int RotatedDirections[9][4] = { 
	{ Direction::None,	Direction::None,	Direction::None,	Direction::None },
	{ Direction::Left,	Direction::Up,		Direction::Right,	Direction::Down },
	{ Direction::Up,	Direction::Right,	Direction::Down,	Direction::Left },
	{ Direction::Right, Direction::Down,	Direction::Left,	Direction::Up },
	{ Direction::Down,	Direction::Left,	Direction::Up,		Direction::Right }, 
	{ Direction::UpLeft, Direction::UpRight, Direction::DownRight, Direction::DownLeft }, 
	{ Direction::UpRight, Direction::DownRight, Direction::DownLeft, Direction::UpLeft }, 
	{ Direction::DownRight, Direction::DownLeft, Direction::UpLeft, Direction::UpRight }, 
	{ Direction::DownLeft, Direction::UpLeft, Direction::UpRight, Direction::DownRight }
};


// Method to return the effect of a rotation on a 2D direction, using the above predefined structure
CMPINLINE Direction GetRotatedDirection(Direction direction, Rotation90Degree rotation) {
	if ((int)direction >= 0 && (int)direction < 9 && (int)rotation >= 0 && (int)rotation < 4) 
		return (Direction)RotatedDirections[direction][rotation];
	else
		return Direction::None;
}

// Returns a value indicating whether this is a diagonal direction or not
CMPINLINE bool IsDiagonalDirection(Direction direction) { 
	return (direction == Direction::UpLeft || direction == Direction::UpRight || 
			direction == Direction::DownLeft || direction == Direction::DownRight);
}

// Methods to manipulate direction values and translate to/from their string representations
string DirectionToString(Direction direction);
Direction DirectionFromString(string direction);
Direction GetOppositeDirection(Direction dir);

// Null string, to avoid repeated instantiation at runtime
const std::string NullString = "";

// String concatenation class
class concat
{
public:
	template <typename T>
	explicit concat(const T & t)
	{
		m_out << t ;
	}

	template <typename T>
	concat & operator()(const T & t)
	{
		m_out << t ;
		return *this ;
	}

	std::string str() const
	{
		return m_out.str() ;
	}
private:
	std::ostringstream m_out ;
} ;

template <typename T>
void StreamToDebugOutput(T obj)
{
	std::ostringstream s;
	s << obj << "\n";

	OutputDebugString(s.str().c_str());
}

CMPINLINE string IntVectorToString(INTVECTOR2 *v) { return (concat("(")(v->x)(", ")(v->y)(")").str()); }
CMPINLINE string IntVectorToString(INTVECTOR3 *v) { return (concat("(")(v->x)(", ")(v->y)(", ")(v->z)(")").str()); }
CMPINLINE string VectorToString(D3DXVECTOR2 *v) { return (concat("(")(v->x)(", ")(v->y)(")").str()); }
CMPINLINE string VectorToString(D3DXVECTOR3 *v) { return (concat("(")(v->x)(", ")(v->y)(", ")(v->z)(")").str()); }
CMPINLINE string VectorToString(D3DXVECTOR4 *v) { return (concat("(")(v->x)(", ")(v->y)(", ")(v->z)(", ")(v->w)(")").str()); }
CMPINLINE string QuaternionToString(D3DXQUATERNION *q) { return (concat("(")(q->x)(", ")(q->y)(", ")(q->z)(", ")(q->w)(")").str()); }

CMPINLINE string VectorToString(D3DXVECTOR3 v) { return (concat("(")(v.x)(", ")(v.y)(", ")(v.z)(")").str()); }

bool PointWithinBounds(INTVECTOR2 point, INTVECTOR2 arealocation, INTVECTOR2 areasize);

void MatrixToCharStream(const D3DXMATRIX *m, char *out);
void MatrixToCharStreamHighPrecision(const D3DXMATRIX *m, char *out);

void RotationMatrixFromBasisVectors(D3DXVECTOR3(&bases)[3], D3DXMATRIX & outMatrix);

void QuaternionRotationYawPitch(D3DXQUATERNION *q, float yaw, float pitch);

struct D3DXFINITEPLANE
{
	D3DXVECTOR3		TL;		// Top-left vertex
	D3DXVECTOR3		TR;		// Top-right vertex
	D3DXVECTOR3		BR;		// Bottom-right vertex
	D3DXVECTOR3		BL;		// Bottom-left vertex
};

bool FileExists(const char *szPath);
bool DirectoryExists(const char *szPath);

template <typename TKey, typename TVal>
void VectorFromUnorderedMap(unordered_map<TKey, TVal> &map, vector<TVal> *pOutVector)
{
	// Make sure we have a valid output vector
	if (!pOutVector) return;

	// Iterate through the map in a linear fashion and copy items to the vector
	unordered_map<TKey,TVal>::iterator it_end = map.end();
	for (unordered_map<TKey, TVal>::iterator it = map.begin(); it != it_end; ++it)
	{
		pOutVector->push_back(it->second);
	}
}

template <typename T>
void InsertIntoVector(std::vector<T> &vec, T obj, typename std::vector<T>::size_type index)
{
	// Get initial parameters
	std::vector<T>::size_type n = vec.size();

	// Make sure the desired index is within the (future) bounds of this vector.  If not, adjust it
	if (index < 0) index = 0;
	if (index > n) index = n;

	// If we are trying to add to the last element then simply append it; otherwise we need to adjust the memory
	if (index == n)
	{
		// Simply append this item to the end of the vector
		vec.push_back(obj);
	} 
	else
	{
		// Otherwise, first add a new entry to the vector so that we have another element to expand into
		vec.push_back(T());

		// Now move the contiguous block of memory between index and (n-1) down by one element
		memmove(&(vec[index+1]), &(vec[index]), (n-index) * sizeof(T));
		//*** MAKE SURE THIS IS THE VERSION THAT PRESERVES OVERLAPPING MEMORY SEGMENTS ***

		// Insert the new object into this desired index
		vec[index] = obj;
	}
}

// Inserts an item into a sorted vector using binary search.  Search operates in O(logN) although the insert 
// can be up to O(N).  Effcicient enough for small datasets, consider 'set' for large amounts of data.
template <typename T, typename Pred>
void InsertIntoSortedVector(std::vector<T> & vec, T const& item, Pred pred)
{
	vec.insert(std::upper_bound(vec.begin(), vec.end(), item, pred), item);
}

// Inserts an item into a sorted vector using binary search.  Search operates in O(logN) although the insert 
// can be up to O(N).  Effcicient enough for small datasets, consider 'set' for large amounts of data.
template <typename T>
void InsertIntoSortedVector(std::vector<T> & vec, T const& item)
{
	vec.insert(std::upper_bound(vec.begin(), vec.end(), item), item);
}

// Inserts an item into a sorted vector using binary search, if it does not already exist.  Returns 
// 'true' if the object was inserted, or 'false' if the object already existed in the vector.  Saves performing 
// the search and insert phases separately where we want to maintain a unique sorted vector.
template <typename T, typename Pred>
bool InsertIntoSortedVectorIfNotPresent(std::vector<T> & vec, T const& item, Pred pred)
{
	// Use lower_bound to find the first element <= item
	std::vector<T>::iterator it = std::lower_bound(vec.begin(), vec.end(), item, pred);

	// If it == end() then all items are less than 'item', and 'item' does not exist, so push onto the back and quit
	if (it == vec.end())
	{
		vec.push_back(item);
		return true;
	}

	// Otherwise, test if 'it', which is the nearest element <= 'item', is actually == 'item'
	if ((*it) == item)
	{
		// The item already exists, so do nothing and return false
		return false;
	}
	else
	{
		// The item does not exist. We know at least one item exists beyond 'it', since otherwise we would
		// have hit the (it == end()) condition above.  So we can safely insert at (it + 1)
		vec.insert(it + 1, item);
		return true;
	}
}

// Inserts an item into a sorted vector using binary search, if it does not already exist.  Returns 
// 'true' if the object was inserted, or 'false' if the object already existed in the vector.  Saves performing 
// the search and insert phases separately where we want to maintain a unique sorted vector.
template <typename T>
bool InsertIntoSortedVectorIfNotPresent(std::vector<T> & vec, T const& item)
{
	// Use lower_bound to find the first element >= item
	std::vector<T>::iterator it = std::lower_bound(vec.begin(), vec.end(), item);

	// If it == end() then all items are less than 'item', and 'item' does not exist, so push onto the back and quit
	if (it == vec.end())
	{
		vec.push_back(item);
		return true;
	}

	// Otherwise, test if 'it', which is the nearest element >= 'item', is actually == 'item'
	if ((*it) == item)
	{
		// The item already exists, so do nothing and return false
		return false;
	}
	else
	{
		// The item does not exist, so 'it' must therefore be the first element beyond the correct
		// location.  We know there is one since we already passed the (it != end) test above.  We
		// can therefore insert directly at this location to maintain ordering.
		vec.insert(it, item);
		return true;
	}
}

// Locates a specific item in a vector, using the defined equality operator for type T.  Simple linear search.
template <typename T>
int FindInVector(vector<T> &vec, T obj)
{
	// Loop through the vector and look for this element
	int i = 0; vector<T>::const_iterator it, it_end = vec.end();
	for (it = vec.begin(); it != it_end; ++it, ++i)
	{
		if ((*it) == obj) return i;
	}

	// We did not find a match
	return -1;
}

// TODO: FindInVector method returns an integer (with -1 for no result) however must be case to vector::size_type for methods
// which use it.  Can't use size_type for FindInVector since size_type is unsigned.  Better solution to avoid casting and
// allow use of full size_type value range?
template <typename T>
CMPINLINE void RemoveFromVector(vector<T> &vec, T obj)
{
	// First make sure the item exists within this vector
	int index = FindInVector(vec, obj);
	if (index == -1) return;

	// Now remove the element at this index
	_RemoveFromVectorAtIndex_Unchecked(vec, index);
}

// Removes the specified element from a vector, based on its index within the vector
template <typename T>
CMPINLINE void RemoveFromVectorAtIndex(std::vector<T> &vec, typename std::vector<T>::size_type index)
{
	// Parameter check
	if (index < 0 || index >= vec.size()) return;

	// Remove the element at this index
	vec.erase(vec.begin() + index);
}

// Remove an element from the specified vector.  No bounds checking; should only be used internally once
// index is confirmed as valid
template <typename T>
CMPINLINE void _RemoveFromVectorAtIndex_Unchecked(std::vector<T> &vec, typename std::vector<T>::size_type index)
{
	// Remove the element at this index
	vec.erase(vec.begin() + index);
}

// Clears and deallocates all memory maintained by a vector
template <typename T>
CMPINLINE void DeallocateVector(std::vector<T> & t)
{
	std::vector<T> tmp;
	tmp.swap(t);
}

// Returns the local system time
CMPINLINE struct tm * GetLocalDateTime(void)
{
	time_t t = time(0);
	return localtime(&t);
}

// Return a formatted date string.  Local system date.
CMPINLINE std::string GetLocalDateString(void) 
{ 
	const struct tm *t = GetLocalDateTime();
	return concat(t->tm_year + 1900)("-")(t->tm_mon + 1)("-")(t->tm_mday).str();
}

// Return a formatted time string.  Local system time.
CMPINLINE std::string GetLocalTimeString(void)
{
	const struct tm *t = GetLocalDateTime();
	return concat(t->tm_hour)(":")(t->tm_min)(":")(t->tm_sec).str();
}

// Return a formatted date/time string.  Local system date & time.
CMPINLINE std::string GetLocalDateTimeString(void)
{
	const struct tm *t = GetLocalDateTime();
	return concat(t->tm_year + 1900)("-")(t->tm_mon + 1)("-")(t->tm_mday)(" ")(t->tm_hour)(":")(t->tm_min)(":")(t->tm_sec).str();
}

// Splits a string based upon the supplied delimeter, optionally skipping empty items
void SplitString(const std::string & input, char delimiter, bool skip_empty, std::vector<std::string> & outElements);

// Concatenates a series of strings together, optionally with the supplied string as a delimiter
std::string ConcatenateStrings(const std::vector<std::string> & elements, const std::string & delimiter);

// Enumeration of possible object visibility testing methods
enum VisibilityTestingModeType { UseBoundingSphere = 0, UseOrientedBoundingBox };

// Utility methods to translate visibility mode to/from its string representation
VisibilityTestingModeType		TranslateVisibilityModeFromString(const std::string & mode);
std::string 					TranslateVisibilityModeToString(const VisibilityTestingModeType mode);



#endif