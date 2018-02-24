#pragma once

#ifndef __UtilityH__
#define __UtilityH__

#pragma comment(lib, "shlwapi.lib")
#include "DX11_Core.h"

#include <time.h>
#include <sstream>
#include <string>
#include <locale>
#include <codecvt>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include "CompilerSettings.h"
#include "IntVector.h"
#include "Direction.h"
class iObject;


// Define memory tracking macros for use in the application
#define MemReg(cp) _CrtMemState cp; _CrtMemCheckpoint( &cp );
#define MemDiffReport(c1, c2, c3) _CrtMemState c3; if ( _CrtMemDifference(&c3, &c1, &c2)) { OutputDebugString("\n\n*** WARNING: ACCUMULATING MEMORY ALLOCATIONS.  INCREMENTAL ALLOCATIONS BELOW ***\n\n"); _CrtMemDumpStatistics( &c3 ); } 
#define MemDump(label,cp) MemReg(cp);OutputDebugString("**** ");OutputDebugString(label);OutputDebugString(": \n");_CrtMemDumpStatistics(&cp);

// Convenience macro for generating string literal data
#define STRING(x) #x

// String macro to force macro expansion before resolving as string (https://stackoverflow.com/questions/2653214/stringification-of-a-macro-value)
#define MSTRING(x) STRING(x)

// Convenience macro to combine multiple arguments, including commas, into one macro argument
#define SINGLE_ARG(...) __VA_ARGS__

// Convenience macro for releasing COM objects.
#define ReleaseCOM(x) { if(x){ x->Release(); (x) = 0; } }

// Convenience macros for deleting objects.
#define SafeDelete(x) { delete (x); (x) = 0; }
#define SafeDeleteArray(x) { delete[] (x); (x) = 0; }

// Convenience macro for freeing malloc-ed memory
#define SafeFree(x) { free(x); (x) = 0; }

// Convenience macro for releasing non-null COM resources
#define ReleaseIfExists(x) { if (x) { (x)->Release(); (x) = 0; } }

// Debug logging macro; logs to the debug output stream before returning
#ifdef _DEBUG
#	define RETURN_LOG(return_value, msg)		OutputDebugString(msg); return return_value
#else
#	define RETURN_LOG(return_value, msg)		return return_value
#endif

// Accepts either a reference or a pointer and universally returns a pointer
template<typename T>
CMPINLINE T * ptr(T & obj) { return &obj; }	// If object is a reference type then return a pointer
template<typename T>
CMPINLINE T * ptr(T * obj) { return obj; }	// Otherwise if object is already a pointer type then simply return it

// Type definition for bitstring data
typedef unsigned int bitstring;
typedef unsigned long bitstring_l;

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

// Formula to translate x/y/z coordinates into an index in the element collection
// Optimise "x + (y*sx) + (z*sx*sy)" -> "x + sx(y + z*sy)"
#define ELEMENT_INDEX(_x, _y, _z) (_x + m_elementsize.x * (_y + (_z * m_elementsize.y)))

// Special overloaded formula to translate x/y/z coordinates into an index in the element collection, which also
// accepts the element space size.  Allows use on element spaces other than our own, e.g. 
// when allocating a new space with different dimensions
// Optimise "x + (y*sx) + (z*sx*sy)" -> "x + sx(y + z*sy)" 
#define ELEMENT_INDEX_EX(_x, _y, _z, _size) (_x + _size.x * (_y + (_z * _size.y)))

// Transforms a location by the specified rotation and returns the new location
INTVECTOR3 GetRotatedElementLocation(const INTVECTOR3 & element_location, Rotation90Degree rotate_by, const INTVECTOR3 & element_size);

// Returns a unit integer offset in the indicated direction
INTVECTOR3 DirectionUnitOffset(Direction direction);


void clower(char *);			// Converts a string to lowercase in-place
void cupper(char *);			// Converts a string to uppercase in-place
char *lower(const char *);		// Converts a string to lowercase as a new string
char *upper(const char *);		// Converts a string to uppercase as a new string

void StrLowerC(std::string &);				// Converts a std::string to lowercase in-place
void StrUpperC(std::string &);				// Converts a std::string to uppercase in-place
std::string StrLower(std::string);		// Converts a std::string to lowercase as a new string
std::string StrUpper(std::string);		// Converts a std::string to uppercase as a new string
std::string TrimString(const std::string & str);

char *BuildFilename(const char *, const char *);
std::string BuildStrFilename(const std::string &, const std::string &);

std::string DataRelativeFile(const std::string & relative_path);

// Enumeration of possible comparison results
enum ComparisonResult { Equal = 0, LessThan, GreaterThan };

// Rotates a bitstring representing directional data
bitstring RotateDirectionBitString(bitstring b, Rotation90Degree rotation);

// Null string, to avoid repeated instantiation at runtime
const std::string NullString = "";

// C++ standard object for converting between UTF8 (std::string) and UTF16 (std::wstring)
extern std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf_converter;

// Convert a UTF8 string (std::string) to UTF16 wstring (std::wstring)
CMPINLINE std::wstring ConvertStringToWString(const std::string & utf8_string)
{
	return utf_converter.from_bytes(utf8_string);
}

// Convert a UTF16 wstring (std::wstring) to UTF8 string (std::string)
CMPINLINE std::string ConvertWStringToString(const std::wstring & utf16_string)
{
	return utf_converter.to_bytes(utf16_string);
}


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
	std::wstring wstr() const
	{
		return ConvertStringToWString(m_out.str());
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

CMPINLINE std::string IntVectorToString(INTVECTOR2 *v) { return v->ToString(); }
CMPINLINE std::string IntVectorToString(INTVECTOR3 *v) { return v->ToString(); }
CMPINLINE std::string IntVectorToString(INTVECTOR2 & v) { return v.ToString(); }
CMPINLINE std::string IntVectorToString(INTVECTOR3 & v) { return v.ToString(); }
CMPINLINE std::string Vector2ToString(const XMFLOAT2 & v) { return (concat("(")(v.x)(", ")(v.y)(")").str()); }
CMPINLINE std::string Vector3ToString(const XMFLOAT3 & v) { return (concat("(")(v.x)(", ")(v.y)(", ")(v.z)(")").str()); }
CMPINLINE std::string Vector4ToString(const XMFLOAT4 & v) { return (concat("(")(v.x)(", ")(v.y)(", ")(v.z)(", ")(v.w)(")").str()); }
CMPINLINE std::string QuaternionToString(const XMFLOAT4 & v) { return (concat("(")(v.x)(", ")(v.y)(", ")(v.z)(", ")(v.w)(")").str()); }
CMPINLINE std::string Vector2ToString(const FXMVECTOR v) { XMFLOAT2 vf; XMStoreFloat2(&vf, v); return Vector2ToString(vf); }
CMPINLINE std::string Vector3ToString(const FXMVECTOR v) { XMFLOAT3 vf; XMStoreFloat3(&vf, v); return Vector3ToString(vf); }
CMPINLINE std::string Vector4ToString(const FXMVECTOR v) { XMFLOAT4 vf; XMStoreFloat4(&vf, v); return Vector4ToString(vf); }
CMPINLINE std::string QuaternionToString(const FXMVECTOR v) { XMFLOAT4 vf; XMStoreFloat4(&vf, v); return Vector4ToString(vf); }
CMPINLINE std::string VectorToString(const XMFLOAT2 & v) { return Vector2ToString(v); }
CMPINLINE std::string VectorToString(const XMFLOAT3 & v) { return Vector3ToString(v); }
CMPINLINE std::string VectorToString(const XMFLOAT4 & v) { return Vector4ToString(v); }

void MatrixToCharStream(const XMFLOAT4X4 *m, char *out);
void MatrixToCharStreamHighPrecision(const XMFLOAT4X4 *m, char *out);
std::string MatrixToString(const XMFLOAT4X4 & m);
CMPINLINE std::string MatrixToString(const XMMATRIX & m) { XMFLOAT4X4 mf; XMStoreFloat4x4(&mf, m); return MatrixToString(mf); }

// Value ptr is a pointer to the first element in contiguous structure data; in the case of XMFLOAT#, this is always the x component
CMPINLINE const float* ValuePtr(const XMFLOAT2 & xmf) { return &(xmf.x); }
CMPINLINE const float* ValuePtr(const XMFLOAT3 & xmf) { return &(xmf.x); }
CMPINLINE const float* ValuePtr(const XMFLOAT4 & xmf) { return &(xmf.x); }

// Generic 'ToString' method that can be specialised as required
template <typename T> CMPINLINE std::string		StringValue(T value) { return concat(value).str(); }
template <> CMPINLINE std::string				StringValue<bool>(bool value) { return (value ? "true" : "false"); }
template <> CMPINLINE std::string				StringValue<XMFLOAT2>(XMFLOAT2 value) { return VectorToString(value); }
template <> CMPINLINE std::string				StringValue<XMFLOAT3>(XMFLOAT3 value) { return VectorToString(value); }
template <> CMPINLINE std::string				StringValue<XMFLOAT4>(XMFLOAT4 value) { return VectorToString(value); }
template <> CMPINLINE std::string				StringValue<XMVECTOR>(XMVECTOR value) { return Vector4ToString(value); }
template <> CMPINLINE std::string				StringValue<INTVECTOR2>(INTVECTOR2 value) { return IntVectorToString(value); }
template <> CMPINLINE std::string				StringValue<INTVECTOR3>(INTVECTOR3 value) { return IntVectorToString(value); }
template <> CMPINLINE std::string				StringValue<XMMATRIX>(XMMATRIX value) { return MatrixToString(value); }
template <> CMPINLINE std::string				StringValue<const XMMATRIX &>(const XMMATRIX & value) { return MatrixToString(value); }


bool PointWithinBounds(INTVECTOR2 point, INTVECTOR2 arealocation, INTVECTOR2 areasize);

XMMATRIX RotationMatrixFromBasisVectors(XMFLOAT3(&bases)[3]);
void RotationMatrixFromBasisVectors(XMFLOAT3(&bases)[3], XMFLOAT4X4 & outMatrix);

// Struct is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
struct D3DXFINITEPLANE
{
	XMVECTOR		TL;		// Top-left vertex
	XMVECTOR		TR;		// Top-right vertex
	XMVECTOR		BR;		// Bottom-right vertex
	XMVECTOR		BL;		// Bottom-left vertex
};


template <typename TKey, typename TVal>
void VectorFromUnorderedMap(std::unordered_map<TKey, TVal> &map, std::vector<TVal> & pOutVector)
{
	// Iterate through the map in a linear fashion and copy items to the vector
	std::unordered_map<TKey,TVal>::iterator it_end = map.end();
	for (std::unordered_map<TKey, TVal>::iterator it = map.begin(); it != it_end; ++it)
	{
		pOutVector.push_back(it->second);
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
// Returns an iterator pointing to the newly-added element
template <typename T, typename Pred>
typename std::vector<T>::iterator InsertIntoSortedVector(std::vector<T> & vec, T const& item, Pred pred)
{
	return vec.insert(std::upper_bound(vec.begin(), vec.end(), item, pred), item);
}

// Inserts an item into a sorted vector using binary search.  Search operates in O(logN) although the insert 
// can be up to O(N).  Effcicient enough for small datasets, consider 'set' for large amounts of data.
template <typename T>
typename std::vector<T>::iterator InsertIntoSortedVector(std::vector<T> & vec, T const& item)
{
	return vec.insert(std::upper_bound(vec.begin(), vec.end(), item), item);
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

// Locates a specific item in a vector, using the defined equality operator for type T.  Returns iterator to the 
// item, or vector.end() if not found
template <typename T>
typename std::vector<T>::const_iterator FindInVector(std::vector<T> &vec, T obj)
{
	return (std::find(vec.begin(), vec.end(), obj));
}

// Locates a specific item in a vector, using the defined equality operator for type T.  Returns iterator to the 
// item, or vector.end() if not found
template <typename Tvec, typename Tobj>
typename Tvec::const_iterator FindInVector(Tvec &vec, Tobj obj)
{
	return (std::find(vec.begin(), vec.end(), obj));
}

// Locates a specific item in a vector, using the defined equality operator for type T.  Returns iterator to the 
// item, or vector.end() if not found
template <typename T>
int FindIndexInVector(std::vector<T> &vec, T obj)
{
	typename std::vector<T>::const_iterator it = FindInVector<T>(vec, obj);
	return (it == vec.end() ? -1 : (int)(it - vec.begin()));
}

// Locates a specific item in a vector, using the defined equality operator for type T.  Returns iterator to the 
// item, or vector.end() if not found
template <typename Tvec, typename Tobj>
int FindIndexInVector(Tvec &vec, Tobj obj)
{
	typename Tvec::const_iterator it = FindInVector<Tvec, Tobj>(vec, obj);
	return (it == vec.end() ? -1 : (int)(it - vec.begin()));
}

// TODO: FindInVector method returns an integer (with -1 for no result) however must be case to vector::size_type for methods
// which use it.  Can't use size_type for FindInVector since size_type is unsigned.  Better solution to avoid casting and
// allow use of full size_type value range?
template <typename T>
CMPINLINE void RemoveFromVector(std::vector<T> &vec, T obj)
{
	typename std::vector<T>::const_iterator it = FindInVector(vec, obj);
	if (it != vec.end()) vec.erase(it);
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

// Removes the specified element from a vector, based on its index within the vector.  Accepts the vector type
// as an additional template parameter, to allow us to handle e.g. vectors with custom allocators
template <typename T, typename vector_type>
CMPINLINE void RemoveFromVectorAtIndex(vector_type &vec, typename vector_type::size_type index)
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

// Splits a string around spaces, observing the presence of quote marks
void SplitStringQuoted(const std::string & input, std::vector<std::string> & outElements);

// Concatenates a series of strings together, optionally with the supplied string as a delimiter
std::string ConcatenateStrings(const std::vector<std::string> & elements, const std::string & delimiter);

// Replace all instances of a substring in a string with another; returns a copy, original string is unmodified
std::string StringReplace(std::string str, const std::string & original, const std::string & replacement);

// Normalise the given string into one suitable for use in object codes
std::string NormaliseString(const std::string & string_value);

// Enumeration of possible object visibility testing methods
enum VisibilityTestingModeType { UseBoundingSphere = 0, UseOrientedBoundingBox };

// Utility methods to translate visibility mode to/from its string representation
VisibilityTestingModeType		TranslateVisibilityModeFromString(const std::string & mode);
std::string 					TranslateVisibilityModeToString(const VisibilityTestingModeType mode);

// Debug macro to output the size of a given type
#ifdef _DEBUG
#	define DBG_OUTPUT_SIZE(x) { OutputDebugString(concat("Sizeof("#x") == ")(sizeof(x))("\n").str().c_str()); }
#endif

#define DBGVAL(x) ss << name << ": " << x << ", "
CMPINLINE void DbgValue(std::ostringstream & ss, const std::string & name, int x) { DBGVAL(x); }
CMPINLINE void DbgValue(std::ostringstream & ss, const std::string & name, float x) { DBGVAL(x); }
CMPINLINE void DbgValue(std::ostringstream & ss, const std::string & name, double x) { DBGVAL(x); }
CMPINLINE void DbgValue(std::ostringstream & ss, const std::string & name, char x) { DBGVAL(x); }
CMPINLINE void DbgValue(std::ostringstream & ss, const std::string & name, const char *x) { DBGVAL(x); }
CMPINLINE void DbgValue(std::ostringstream & ss, const std::string & name, const XMFLOAT2 & x) { DBGVAL("[" << x.x << ", " << x.y << "]"); }
CMPINLINE void DbgValue(std::ostringstream & ss, const std::string & name, const XMFLOAT3 & x) { DBGVAL("[" << x.x << ", " << x.y << ", " << x.z << "]"); }
CMPINLINE void DbgValue(std::ostringstream & ss, const std::string & name, const FXMVECTOR x) 
{ 
	XMFLOAT4 f; XMStoreFloat4(&f, x);
	DBGVAL("[" << f.x << ", " << f.y << ", " << f.z << ", " << f.w << "]"); 
}


// Convenience macros used to invoke debug functions on objects
#	define INIT_DEBUG_FN_TESTING(command)		std::string fn = StrLower(command.Parameter(1)); if (false) { } 
#	define REGISTER_DEBUG_FN(fn_name, ...)										\
						else if (fn == StrLower(#fn_name)) { fn_name(SINGLE_ARG(__VA_ARGS__));		\
			command.SetSuccessOutput(concat("Function \"")(#fn_name)("\" executed on object \"")(command.Parameter(0))("\"").str()); } 
#	define REGISTER_DEBUG_ACCESSOR_FN(fn_name, ...)										\
						else if (fn == StrLower(#fn_name)) { std::string tmp_output_##fn_name = concat(StringValue(fn_name(SINGLE_ARG(__VA_ARGS__)))).str();		\
			command.SetSuccessOutput(concat("Obj(")(command.Parameter(0))(").")(#fn_name)("(...) == ")(tmp_output_##fn_name).str()); } 

// Redirects debug processing based on the specified trigger command
#	define REGISTER_DEBUG_FN_REDIRECT(trigger, redirect_to)										\
						else if (fn == #trigger) { redirect_to(command); }		\

// Redirected debug testing allows us to specify "parameter_level", which is the level at which 
// useful parameters (firstly, the command name) begin in the parameter list
#	define INIT_DEBUG_FN_TESTING_AT_LEVEL(command, parameter_level)		std::string fn = command.Parameter(parameter_level); if (false) { } 


// Convenience macros used to invoke debug functions on tiles
#	define INIT_DEBUG_TILE_FN_TESTING(command)		std::string fn = StrLower(command.Parameter(3)); if (false) { } 
#	define REGISTER_DEBUG_TILE_FN(fn_name, ...)										\
						else if (fn == StrLower(#fn_name)) { fn_name(SINGLE_ARG(__VA_ARGS__));		\
			command.SetSuccessOutput(concat("Function \"")(#fn_name)("\" executed on object \"")(command.Parameter(0))("\", tile ")(command.Parameter(2)).str()); } 
#	define REGISTER_DEBUG_TILE_ACCESSOR_FN(fn_name, ...)										\
						else if (fn == StrLower(#fn_name)) { std::string tmp_output_##fn_name = concat(StringValue(fn_name(SINGLE_ARG(__VA_ARGS__)))).str();		\
			command.SetSuccessOutput(concat("Obj(")(command.Parameter(0))(").Tile(")(command.Parameter(2))(").")(#fn_name)("(...) == ")(tmp_output_##fn_name).str()); } 



#endif