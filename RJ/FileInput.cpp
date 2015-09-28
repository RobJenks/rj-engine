#include "FileInput.h"

#include <malloc.h>
#include <string.h>
#include "ErrorCodes.h"
#include "Utility.h"
#include "FastMath.h"
#include "AdjustableParameter.h"


const char *	IO::___tmp_loading_cchar;
std::string		IO::___tmp_loading_string;


D3DXVECTOR2 IO::GetVector2FromAttr(TiXmlElement *node)
{
	char *name; double val; 
	D3DXVECTOR2 vec = NULL_VECTOR2;

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

	return vec;
}


void IO::GetVector2FromAttr(TiXmlElement *node, D3DXVECTOR2 *out)
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

D3DXVECTOR3 IO::GetVector3FromAttr(TiXmlElement *node)
{
	char *name; double val; 
	D3DXVECTOR3 vec = NULL_VECTOR;

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

	return vec;
}

void IO::GetVector3FromAttr(TiXmlElement *node, D3DXVECTOR3 *out)
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

D3DXVECTOR4 IO::GetVector4FromAttr(TiXmlElement *node)
{
	char *name; double val; 
	D3DXVECTOR4 vec = NULL_VECTOR4;

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

	return vec;
}

void IO::GetVector4FromAttr(TiXmlElement *node, D3DXVECTOR4 *out)
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


D3DXQUATERNION IO::GetD3DXQUATERNIONFromAttr(TiXmlElement *node)
{
	char *name; double val; 
	D3DXQUATERNION q = ID_QUATERNION;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "w") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) q.w = (FLOAT)val;
		}
		else if (strcmp(name, "x") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) q.x = (FLOAT)val;
		}
		else if (strcmp(name, "y") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) q.y = (FLOAT)val;
		}
		else if (strcmp(name, "z") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) q.z = (FLOAT)val;
		}

		free(name);
		attr = attr->Next();
	}

	return q;
}

void IO::GetD3DXQUATERNIONFromAttr(TiXmlElement *node, D3DXQUATERNION *out)
{
	char *name; double val;

	TiXmlAttribute *attr = node->FirstAttribute();
	while (attr) {
		name = lower(attr->Name());
		if (strcmp(name, "w") == 0) {
			if (attr->QueryDoubleValue(&val) == TIXML_SUCCESS) out->w = (FLOAT)val;
		}
		else if (strcmp(name, "x") == 0) {
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

// Get the specified integer attribute, returning 0 if the attribute does not exist
// "node" and "attribute" must exist or exception is thrown
int IO::GetIntegerAttribute(TiXmlElement *node, const char *attribute)
{
	const char *cattr = node->Attribute(attribute);
	if (!cattr) return 0;
	return atoi(cattr);
}

// Get the specified integer attribute, returning 'defaultvalue' if the attribute does not exist
// "node" and "attribute" must exist or exception is thrown
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




