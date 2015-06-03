#include "malloc.h"
#include <string>
#include <sstream>
#include <cmath>
#include "FastMath.h"

#include "Utility.h"

using namespace std;

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

string BuildStrFilename(const string &path, const string &filename)
{
	string result = path + "\\" + filename;
	return result;
}

void StrLowerC(string &str)
{
	int length = (int)str.length();
	for (int i=0; i<length; i++)
		if (str[i] >= 0x41 && str[i] <= 0x5A) str[i] += 0x20;
}

void StrUpperC(string &str)
{
	int length = (int)str.length();
	for (int i=0; i<length; i++)
		if (str[i] >= 0x61 && str[i] <= 0x7A) str[i] -= 0x20;
}

string StrLower(const string &str)
{
	string tmp = str;
	StrLowerC(tmp);
	return tmp;
}

string StrUpper(const string &str)
{
	string tmp = str;
	StrUpperC(tmp);
	return tmp;
}

bool FileExists(const char *szPath)
{
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool DirectoryExists(const char *szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		   (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void QuaternionRotationYawPitch(D3DXQUATERNION *q, float yaw, float pitch)
{
	float halfyaw, halfpitch, sinY, cosY, sinP, cosP;

	halfyaw = yaw * 0.5f;
	halfpitch = pitch * 0.5f;

	sinY = sinf(halfyaw);
	cosY = cosf(halfyaw);

	sinP = sinf(halfpitch);
	cosP = cosf(halfpitch);

	q->x = cosY * sinP * COS_ZERO + sinY * cosP * SIN_ZERO;
	q->y = sinY * cosP * COS_ZERO - cosY * sinP * SIN_ZERO;
	q->z = cosY * cosP * SIN_ZERO - sinY * sinP * COS_ZERO;
	q->w = cosY * cosP * COS_ZERO + sinY * sinP * SIN_ZERO;
}

void MatrixToCharStream(const D3DXMATRIX *m, char *out)
{
	sprintf(out, "[ %.2f, %.2f, %.2f, %.2f // %.2f, %.2f, %.2f, %.2f // %.2f, %.2f, %.2f, %.2f // %.2f, %.2f, %.2f, %.2f ]",
		m->_11, m->_12, m->_13, m->_14, m->_21, m->_22, m->_23, m->_24, m->_31, m->_32, m->_33, m->_34, m->_41, m->_42, m->_43, m->_44); 
}

void MatrixToCharStreamHighPrecision(const D3DXMATRIX *m, char *out)
{
	sprintf(out, "[ %.6f, %.6f, %.6f, %.6f // %.6f, %.6f, %.6f, %.6f // %.6f, %.6f, %.6f, %.6f // %.6f, %.6f, %.6f, %.6f ]",
		m->_11, m->_12, m->_13, m->_14, m->_21, m->_22, m->_23, m->_24, m->_31, m->_32, m->_33, m->_34, m->_41, m->_42, m->_43, m->_44); 
}

void RotationMatrixFromBasisVectors(D3DXVECTOR3 (&bases)[3], D3DXMATRIX & outMatrix)
{
	outMatrix = D3DXMATRIX(	bases[0].x, bases[0].y, bases[0].z, 0.0f,
							bases[1].x, bases[1].y, bases[1].z, 0.0f,
							bases[2].x, bases[2].y, bases[2].z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f);
}

Rotation90Degree TranslateRotation90Degree(string rotvalue)
{
	if		(rotvalue == "0")				return Rotation90Degree::Rotate0;
	else if	(rotvalue == "90")				return Rotation90Degree::Rotate90;
	else if (rotvalue == "180")				return Rotation90Degree::Rotate180;
	else if (rotvalue == "270")				return Rotation90Degree::Rotate270;
	else									return Rotation90Degree::Rotate0;
}

Rotation90Degree RotateBy90Degrees(Rotation90Degree current)
{
	switch (current)
	{
		case Rotation90Degree::Rotate0:
			return Rotation90Degree::Rotate90;
		case Rotation90Degree::Rotate90:
			return Rotation90Degree::Rotate180;
		case Rotation90Degree::Rotate180:
			return Rotation90Degree::Rotate270;
		default:
			return Rotation90Degree::Rotate0;
	}
}

string DirectionToString(Direction edge)
{
	switch (edge) {
		case Direction::Left:		return "left";
		case Direction::Up:			return "up";
		case Direction::Right:		return "right";
		case Direction::Down:		return "down";
		case Direction::UpLeft:		return "upleft";
		case Direction::UpRight:	return "upright";
		case Direction::DownRight:	return "downright";
		case Direction::DownLeft:	return "downleft";
		case Direction::ZUp:		return "zup";
		case Direction::ZDown:		return "zdown";
		default:					return NullString;
	}
}
Direction DirectionFromString(string edge)
{
	// All comparisons are case-insensitive
	string s = StrLower(edge);
	if		(s == "left")			return Direction::Left;
	else if (s == "up")				return Direction::Up;
	else if (s == "right")			return Direction::Right;
	else if (s == "down")			return Direction::Down;
	else if (s == "upleft")			return Direction::UpLeft; 
	else if (s == "upright")		return Direction::UpRight; 
	else if (s == "downright")		return Direction::DownRight; 
	else if (s == "downleft")		return Direction::DownLeft;
	else if (s == "zup")			return Direction::ZUp;
	else if (s == "zdown")			return Direction::ZDown;
	else							return Direction::None;
}

Direction GetOppositeDirection(Direction dir)
{
	switch (dir)
	{
		case Direction::Left:		return Direction::Right;
		case Direction::Up:			return Direction::Down;
		case Direction::Right:		return Direction::Left;
		case Direction::Down:		return Direction::Up;
		case Direction::UpLeft:		return Direction::DownRight;
		case Direction::UpRight:	return Direction::DownLeft;
		case Direction::DownRight:	return Direction::UpLeft; 
		case Direction::DownLeft:	return Direction::UpRight;
		case Direction::ZUp:		return Direction::ZDown;
		case Direction::ZDown:		return Direction::ZUp;
		default:					return Direction::None;
	}
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
	if (input == NullString) return;

	// Use a string stream to process each element in turn
	std::string item;
	std::stringstream ss(input);
	while (std::getline(ss, item, delimiter)) {
		if (!skip_empty || !item.empty()) 
			outElements.push_back(item);
	}
}

// Concatenates a series of strings together, optionally with the supplied string as a delimiter
std::string ConcatenateStrings(const std::vector<std::string> & elements, const std::string & delimiter)
{
	// Determine whether a delimiter is required
	bool delimit = (delimiter != NullString);

	// Append each string to a string stream in turn
	std::ostringstream ss;
	int ubound = elements.size() - 1;
	for (int i = 0; i <= ubound; ++i)
	{
		ss << elements[i];
		if (delimit && i < ubound) ss << delimiter;
	}

	// Return a string output from the stringstream
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
