#include "malloc.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <shlwapi.h>
#include "FastMath.h"
#include "GameDataExtern.h"

#include "Utility.h"


// C++ standard object for converting between UTF8 (std::string) and UTF16 (std::wstring)
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf_converter;

// Set of all 90 degree rotations
const Rotation90Degree ROT90_VALUES[4] = { Rotation90Degree::Rotate0, Rotation90Degree::Rotate90, Rotation90Degree::Rotate180, Rotation90Degree::Rotate270 };

void clower(char *str) 
{
	for (char *tmp=str; *tmp != '\0'; tmp++)
		if (*tmp >= 0x41 && *tmp <= 0x5A) *tmp += 0x20;
}

void cupper(char *str) 
{
	for (char *tmp=str; *tmp != '\0'; tmp++)
		if (*tmp >= 0x61 && *tmp <= 0x7A) *tmp -= 0x20;
}

char *lower(const char *str) 
{
	size_t length = strlen(str);
	char *tmp = (char *)malloc(sizeof(char) * (length + 1));
	strcpy(tmp, str);

	for ( ; *tmp != '\0'; tmp++)
		if (*tmp >= 0x41 && *tmp <= 0x5A) *tmp += 0x20;

	tmp -= length;
	return tmp;
}

char *upper(const char *str) 
{
	size_t length = strlen(str);
	char *tmp = (char *)malloc(sizeof(char) * (length + 1));
	strcpy(tmp, str);

	for ( ; *tmp != '\0'; tmp++)
		if (*tmp >= 0x61 && *tmp <= 0x7A) *tmp -= 0x20;

	tmp -= length;
	return tmp;
}

char *BuildFilename(const char *path, const char *filename)
{
	char *r = (char*)malloc(sizeof(char) * (strlen(path) + strlen("\\") + strlen(filename) + 1));
	strcpy(r, path);
	strcpy(r+strlen(path), "\\");
	strcpy(r+strlen(path) + strlen("\\"), filename);

	return r;
}

std::string BuildStrFilename(const std::string &path, const std::string &filename)
{
	std::string result = path + "\\" + filename;
	return result;
}

std::string DataRelativeFile(const std::string & relative_path)
{
	return (D::DATA_S + "\\" + relative_path);
}

void StrLowerC(std::string &str)
{
	int length = (int)str.length();
	for (int i=0; i<length; i++)
		if (str[i] >= 0x41 && str[i] <= 0x5A) str[i] += 0x20;
}

void StrUpperC(std::string &str)
{
	int length = (int)str.length();
	for (int i=0; i<length; i++)
		if (str[i] >= 0x61 && str[i] <= 0x7A) str[i] -= 0x20;
}

std::string StrLower(std::string str)
{
	StrLowerC(str);
	return str;
}

std::string StrUpper(std::string str)
{
	StrUpperC(str);
	return str;
}



void MatrixToCharStream(const XMFLOAT4X4 *m, char *out)
{
	sprintf(out, "[ %.2f, %.2f, %.2f, %.2f // %.2f, %.2f, %.2f, %.2f // %.2f, %.2f, %.2f, %.2f // %.2f, %.2f, %.2f, %.2f ]",
		m->_11, m->_12, m->_13, m->_14, m->_21, m->_22, m->_23, m->_24, m->_31, m->_32, m->_33, m->_34, m->_41, m->_42, m->_43, m->_44); 
}

void MatrixToCharStreamHighPrecision(const XMFLOAT4X4 *m, char *out)
{
	sprintf(out, "[ %.6f, %.6f, %.6f, %.6f // %.6f, %.6f, %.6f, %.6f // %.6f, %.6f, %.6f, %.6f // %.6f, %.6f, %.6f, %.6f ]",
		m->_11, m->_12, m->_13, m->_14, m->_21, m->_22, m->_23, m->_24, m->_31, m->_32, m->_33, m->_34, m->_41, m->_42, m->_43, m->_44); 
}

std::string MatrixToString(const XMFLOAT4X4 & m)
{
	char *c = new char[1024];
	if (!c) return "";

	MatrixToCharStream(&m, c);

	std::string s = std::string(c);

	delete[] c;
	return s;
}

XMMATRIX RotationMatrixFromBasisVectors(XMFLOAT3 (&bases)[3])
{
	return XMMatrixSet(	bases[0].x, bases[0].y, bases[0].z, 0.0f,
						bases[1].x, bases[1].y, bases[1].z, 0.0f,
						bases[2].x, bases[2].y, bases[2].z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f); 
}
void RotationMatrixFromBasisVectors(XMFLOAT3 (&bases)[3], XMFLOAT4X4 & outMatrix)
{
	outMatrix = XMFLOAT4X4(	bases[0].x, bases[0].y, bases[0].z, 0.0f,
							bases[1].x, bases[1].y, bases[1].z, 0.0f,
							bases[2].x, bases[2].y, bases[2].z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f);
}

// Transforms a location by the specified rotation and returns the new location
INTVECTOR3 GetRotatedElementLocation(const INTVECTOR3 & element_location, Rotation90Degree rotate_by, const INTVECTOR3 & element_size)
{
	switch (rotate_by)
	{
		case Rotation90Degree::Rotate90:	return INTVECTOR3(element_location.y, (element_size.y - 1) - element_location.x, element_location.z);						 // [y, (sy-1)-x]
		case Rotation90Degree::Rotate180:	return INTVECTOR3((element_size.x - 1) - element_location.x, (element_size.y - 1) - element_location.y, element_location.z); // [(sx-1)-x, (sy-1)-y
		case Rotation90Degree::Rotate270:	return INTVECTOR3((element_size.x - 1) - element_location.y, element_location.x, element_location.z);						 // [(sx-1)-y, x]
		default:							return element_location;																									 // [x, y]
	}
}

// Returns a unit integer offset in the indicated direction
INTVECTOR3 DirectionUnitOffset(Direction direction)
{
	switch (direction)
	{
		case Direction::Left:			return INTVECTOR3(-1, 0, 0);
		case Direction::Right:			return INTVECTOR3(+1, 0, 0);
		case Direction::Up:				return INTVECTOR3(0, +1, 0);
		case Direction::Down:			return INTVECTOR3(0, -1, 0);
		case Direction::ZUp:			return INTVECTOR3(0, 0, +1);
		case Direction::ZDown:			return INTVECTOR3(0, 0, -1);

		case Direction::UpLeft:			return INTVECTOR3(-1, +1, 0);
		case Direction::UpRight:		return INTVECTOR3(+1, +1, 0);
		case Direction::DownLeft:		return INTVECTOR3(-1, -1, 0);
		case Direction::DownRight:		return INTVECTOR3(+1, -1, 0);

		default:						return NULL_INTVECTOR3;
	}
}

// Rotates a bitstring representing directional data
bitstring RotateDirectionBitString(bitstring b, Rotation90Degree rotation)
{
	bitstring newb = 0U;
	SetBitState(newb, GetRotatedBSDirection(DirectionBS::Left_BS, rotation), CheckBit_All(b, DirectionBS::Left_BS)); 
	SetBitState(newb, GetRotatedBSDirection(DirectionBS::Up_BS, rotation), CheckBit_All(b, DirectionBS::Up_BS));
	SetBitState(newb, GetRotatedBSDirection(DirectionBS::Down_BS, rotation), CheckBit_All(b, DirectionBS::Down_BS));
	SetBitState(newb, GetRotatedBSDirection(DirectionBS::Right_BS, rotation), CheckBit_All(b, DirectionBS::Right_BS));
	SetBitState(newb, GetRotatedBSDirection(DirectionBS::UpLeft_BS, rotation), CheckBit_All(b, DirectionBS::UpLeft_BS));
	SetBitState(newb, GetRotatedBSDirection(DirectionBS::UpRight_BS, rotation), CheckBit_All(b, DirectionBS::UpRight_BS));
	SetBitState(newb, GetRotatedBSDirection(DirectionBS::DownLeft_BS, rotation), CheckBit_All(b, DirectionBS::DownLeft_BS));
	SetBitState(newb, GetRotatedBSDirection(DirectionBS::DownRight_BS, rotation), CheckBit_All(b, DirectionBS::DownRight_BS));
	return newb;
}

bool PointWithinBounds(INTVECTOR2 point, INTVECTOR2 arealocation, INTVECTOR2 areasize)
{
	return (point.x >= arealocation.x && point.y >= arealocation.y && 
			point.x <= (arealocation.x + areasize.x) &&
			point.y <= (arealocation.y + areasize.y) );
}

// Splits a string based upon the supplied delimeter, optionally skipping empty items
void SplitString(const std::string & input, char delimiter, bool skip_empty, std::vector<std::string> & outElements)
{
	// Return if there is no data to process
	if (input.empty()) return;

	// Use a string stream to process each element in turn
	std::string item;
	std::stringstream ss(input);
	while (std::getline(ss, item, delimiter)) {
		if (!skip_empty || !item.empty()) 
			outElements.push_back(item);
	}
}

// Splits a string around spaces, observing the presence of quote marks
void SplitStringQuoted(const std::string & input, std::vector<std::string> & outElements)
{
	std::string item;
	std::istringstream ss(input);

	while (ss >> std::quoted(item, '\"', (char)0))		// Escape=0 seems to prevent this choking on \\ in path strings
	{
		outElements.push_back(item);
	}
}

// Return a new string with spaces removed
std::string TrimString(const std::string & str)
{
	auto start = str.find_first_not_of(' ');
	auto end = str.find_last_not_of(' ');
	return str.substr(start, end - start + 1U);
}

// Replace all instances of a substring in a string with another; returns a copy, original string is unmodified
std::string StringReplace(std::string str, const std::string & original, const std::string & replacement)
{
	size_t index = 0U;
	while (true)
	{
		index = str.find(original, index);
		if (index == std::string::npos) break;

		str.replace(index, original.size(), replacement);
		index += replacement.size();
	}

	return str;
}


// Concatenates a series of strings together, optionally with the supplied string as a delimiter
std::string ConcatenateStrings(const std::vector<std::string> & elements, const std::string & delimiter)
{
	// Determine whether a delimiter is required
	bool delimit = (delimiter != NullString);

	// Append each string to a string stream in turn
	std::ostringstream ss;
	std::vector<std::string>::size_type ubound = (elements.size() - 1);
	for (std::vector<std::string>::size_type i = 0; i <= ubound; ++i)
	{
		ss << elements[i];
		if (delimit && i < ubound) ss << delimiter;
	}

	// Return a string output from the stringstream
	return ss.str();
}

// Normalise the given string into one suitable for use in object codes
std::string NormaliseString(const std::string & string_value)
{
	std::ostringstream ss;
	for (std::string::value_type s : string_value)
	{
		if ((s >= '0' && s <= '9') || (s >= 'A' && s <= 'Z') || (s >= 'a' && s <= 'z') || s == '_')
			ss << s;
	}

	return ss.str();
}

// Static method to translate visibility mode from its string representation
VisibilityTestingModeType TranslateVisibilityModeFromString(const std::string & mode)
{
	std::string s = StrLower(mode);
	
	// Test for each potential testing mode; default is to use the bounding sphere
	if (s == "useorientedboundingbox")		return VisibilityTestingModeType::UseOrientedBoundingBox;

	else									return VisibilityTestingModeType::UseBoundingSphere;
}

// Static method to translate visibility mode to its string representation
std::string TranslateVisibilityModeToString(const VisibilityTestingModeType mode)
{
	// Test for each potential testing mode; default is to use the bounding sphere
	switch (mode)
	{
		case VisibilityTestingModeType::UseOrientedBoundingBox:		
			return "useorientedboundingbox";

		default:
			return "useboundingsphere";
	}
}
