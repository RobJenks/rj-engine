#include "FileInput.h"

#include <malloc.h>
#include <string.h>
#include "ErrorCodes.h"
#include "Utility.h"
#include "FastMath.h"
#include "AdjustableParameter.h"


const char *	IO::___tmp_loading_cchar;
std::string		IO::___tmp_loading_string;

XMVECTOR IO::GetVector2FromAttr(TiXmlElement *node)
{
	char *name; double val;
	XMFLOAT2 vec = NULL_FLOAT2;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.y = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}

	return XMLoadFloat2(&vec);
}

XMFLOAT2 IO::GetFloat2FromAttr(TiXmlElement *node)
{
	char *name; double val; 
	float x = 0.0f, y = 0.0f;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) y = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}

	return XMFLOAT2(x, y);
}


void IO::GetFloat2FromAttr(TiXmlElement *node, XMFLOAT2 *out)
{
	char *name; double val;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->y = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}
}


XMVECTOR IO::GetVector3FromAttr(TiXmlElement *node)
{
	char *name; double val;
	XMFLOAT3 vec = NULL_FLOAT3;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.y = (FLOAT)val;
		}
		else if (strcmp(name, "z") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.z = (FLOAT)val;
		}
		
		free(name);
		attr = attr->Next();
	}

	return XMLoadFloat3(&vec);
}

XMFLOAT3 IO::GetFloat3FromAttr(TiXmlElement *node)
{
	char *name; double val;
	float x = 0.0f, y = 0.0f, z = 0.0f;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) y = (FLOAT)val;
		}
		else if (strcmp(name, "z") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) z = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}

	return XMFLOAT3(x, y, z);
}

void IO::GetFloat3FromAttr(TiXmlElement *node, XMFLOAT3 *out)
{
	char *name; double val;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->y = (FLOAT)val;
		}
		else if (strcmp(name, "z") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->z = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}
}

XMVECTOR IO::GetVector4FromAttr(TiXmlElement *node)
{
	char *name; double val; 
	XMFLOAT4 vec = NULL_FLOAT4;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.y = (FLOAT)val;
		}
		else if (strcmp(name, "z") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.z = (FLOAT)val;
		}
		else if (strcmp(name, "w") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.w = (FLOAT)val;
		}
		
		free(name);
		attr = attr->Next();
	}

	return XMLoadFloat4(&vec);
}

XMFLOAT4 IO::GetFloat4FromAttr(TiXmlElement *node)
{
	char *name; double val;
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f; 

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) y = (FLOAT)val;
		}
		else if (strcmp(name, "z") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) z = (FLOAT)val;
		}
		else if (strcmp(name, "w") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) w = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}

	return XMFLOAT4(x, y, z, w);
}

void IO::GetFloat4FromAttr(TiXmlElement *node, XMFLOAT4 *out)
{
	char *name; double val;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->y = (FLOAT)val;
		}
		else if (strcmp(name, "z") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->z = (FLOAT)val;
		}
		else if (strcmp(name, "w") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->w = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}
}

XMVECTOR IO::GetColourVectorFromAttr(TiXmlElement *node)
{
	char *name; double val;
	XMFLOAT4 vec = NULL_FLOAT4;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "r") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.x = (FLOAT)val;
		}
		else if (strcmp(name, "g") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.y = (FLOAT)val;
		}
		else if (strcmp(name, "b") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.z = (FLOAT)val;
		}
		else if (strcmp(name, "a") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) vec.w = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}

	return XMLoadFloat4(&vec);
}

XMFLOAT4 IO::GetColourFloatFromAttr(TiXmlElement *node)
{
	char *name; double val;
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "r") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) x = (FLOAT)val;
		}
		else if (strcmp(name, "g") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) y = (FLOAT)val;
		}
		else if (strcmp(name, "b") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) z = (FLOAT)val;
		}
		else if (strcmp(name, "a") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) w = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}

	return XMFLOAT4(x, y, z, w);
}

void IO::GetColourFloatFromAttr(TiXmlElement *node, XMFLOAT4 *out)
{
	char *name; double val;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "r") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->x = (FLOAT)val;
		}
		else if (strcmp(name, "g") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->y = (FLOAT)val;
		}
		else if (strcmp(name, "b") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->z = (FLOAT)val;
		}
		else if (strcmp(name, "a") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->w = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}
}

void IO::GetInt2CoordinatesFromAttr(TiXmlElement *node, int *x, int *y)
{
	char *name; int val = 0;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) *x = val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) *y = val;
		}

		free(name);
		attr = attr->Next();
	}
}

void IO::GetInt3CoordinatesFromAttr(TiXmlElement *node, int *x, int *y, int *z)
{
	char *name; int val;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) *x = val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) *y = val;
		}
		else if (strcmp(name, "z") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) *z = val;
		}

		free(name);
		attr = attr->Next();
	}
}

INTVECTOR2 IO::GetInt2CoordinatesFromAttr(TiXmlElement *node)
{
	char *name; int val = 0;
	INTVECTOR2 vec;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) vec.x = val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) vec.y = val;
		}

		free(name);
		attr = attr->Next();
	}

	return vec;
}

INTVECTOR3 IO::GetInt3CoordinatesFromAttr(TiXmlElement *node)
{
	char *name; int val;
	INTVECTOR3 vec;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "x") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) vec.x = val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) vec.y = val;
		}
		else if (strcmp(name, "z") == 0) {
			if (attr->QueryIntValue(&val) == TIXML_SUCCESS) vec.z = val;
		}

		free(name);
		attr = attr->Next();
	}

	return vec;
}

// Get the specified float attribute
float IO::GetFloatAttribute(TiXmlElement *node, const char *attribute)
{
	float val = 0.0f;
	node->QueryFloatAttribute(attribute, &val);
	return val;
}

// Get the specified float attribute
float IO::GetFloatAttribute(TiXmlElement *node, const char *attribute, float defaultvalue)
{
	float val = defaultvalue;
	node->QueryFloatAttribute(attribute, &val);
	return val;
}

// Get the specified integer attribute, returning 0 if the attribute does not exist
// "node" and "attribute" must exist
int IO::GetIntegerAttribute(TiXmlElement *node, const char *attribute)
{
	const char *cattr = node->Attribute(attribute);
	if (!cattr) return 0;
	return atoi(cattr);
}

// Get the specified integer attribute, returning 'defaultvalue' if the attribute does not exist
// "node" and "attribute" must exist
int	IO::GetIntegerAttribute(TiXmlElement *node, const char *attribute, int defaultvalue)
{
	const char *cattr = node->Attribute(attribute);
	if (!cattr) return defaultvalue;
	return atoi(cattr);
}

// Attempt to get the specified integer attribute, returning 0 and an error code != NoError if the attribute does not exist
// "node" and "attribute" must exist or exception is thrown
Result IO::TryGetIntegerAttribute(TiXmlElement *node, const char *attribute, int & outValue)
{
	const char *cattr = node->Attribute(attribute);
	if (!cattr)
	{
		outValue = 0;
		return ErrorCodes::AttributeDoesNotExist;
	}

	outValue = atoi(cattr);
	return ErrorCodes::NoError;
}

// Attempt to get the specified integer attribute, returning 'defaultvalue' and an error code != NoError if the attribute does not exist
// "node" and "attribute" must exist or exception is thrown
Result IO::TryGetIntegerAttribute(TiXmlElement *node, const char *attribute, int defaultvalue, int & outValue)
{
	const char *cattr = node->Attribute(attribute);
	if (!cattr)
	{
		outValue = defaultvalue;
		return ErrorCodes::AttributeDoesNotExist;
	}

	outValue = atoi(cattr);
	return ErrorCodes::NoError;
}

// Get the specified bool attribute
bool IO::GetBoolAttribute(TiXmlElement *node, const char *attribute, bool defaultvalue)
{
	const char *cattr = node->Attribute(attribute);
	return (cattr ? (StrLower(cattr) == "true") : defaultvalue);
}

// Get the specified attribute and return as a std::string, or the default value if no attribute with that key exisfts
std::string IO::GetStringAttribute(TiXmlElement *node, const char *attribute, const std::string & defaultValue)
{
	const char *cattr = node->Attribute(attribute);
	return (cattr ? cattr : defaultValue);
}

// Read a Direction attribute, which will either be the string name of the direction (preferred) or the integer index (secondary)
Direction IO::GetDirectionAttribute(TiXmlElement *node, const char *attribute)
{
	if (!attribute) return Direction::_Count;
	const char *cdir = node->Attribute(attribute);
	if (!cdir) return Direction::_Count;

	// Attempt to parse a string direction value first
	std::string lc = cdir; StrLowerC(lc);
	Direction direction = DirectionFromString(lc);
	if (direction != Direction::_Count) return direction;
	
	// If we could not match a string direction name, attempt to parse as an integer index instead
	int direction_index;
	if (node->QueryIntAttribute(attribute, &direction_index) == TIXML_SUCCESS)
	{
		if (direction_index >= 0 && direction_index < Direction::_Count) return (Direction)direction_index;
	}

	return Direction::_Count;
}



