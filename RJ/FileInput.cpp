#include "FileInput.h"

#include <malloc.h>
#include <string.h>
#include "Utility.h"
#include "FastMath.h"
#include "AdjustableParameter.h"


const char *	IO::___tmp_loading_cchar;
std::string		IO::___tmp_loading_string;

/*
bool IO::IsHeader(char *s) { return (s[0] == '[' && s[strlen(s)-1] == ']'); }
bool IO::IsData(char *s) { return strchr(s, '=') != 0; }

char *IO::GetHeader(char *s)
{
	int l = strlen(s);
	char *r = (char *)malloc(sizeof(char)*(l-1));
	strncpy(r, &s[1], l-2); 
	r[l-2] = '\0';

	return r;
}

void IO::GetHeader(char *s, char *out)
{
	int l = strlen(s);
	strncpy(out, &s[1], l-2); 
	out[l-2] = '\0';
}

char *IO::GetLabel(char *s)
{
	int l = strlen(s);
	char *eq = strchr(s, '=');

	char *r = (char *)malloc(sizeof(char)*(eq-s+1));		// nb assuming sizeof(char)==1
	strncpy(r, s, eq-s);
	r[eq-s] = '\0';

	return r;
}

void IO::GetLabel(char *s, char *out)
{
	int l = strlen(s);
	char *eq = strchr(s, '=');

	strncpy(out, s, eq-s);
	out[eq-s] = '\0';
}

char *IO::GetValue(char *s)
{
	int l = strlen(s);
	char *eq = strchr(s, '=');
	int ln = s+l-eq-1;

	char *r = (char *)malloc(sizeof(char)*(ln+1));		// nb assuming sizeof(char)==1
	strncpy(r, eq+1, ln);
	r[ln] = '\0';

	return r;
}

void IO::GetValue(char *s, char *out)
{
	int l = strlen(s);
	char *eq = strchr(s, '=');
	int ln = s+l-eq-1;

	strncpy(out, eq+1, ln);
	out[ln] = '\0';
}

D3DXVECTOR3 IO::GetVector(char *s)
{
	D3DXVECTOR3 v(0.0f, 0.0f, 0.0f);
	IO::GetVector(s, &v);
	return v;
}

void IO::GetVector(char *s, D3DXVECTOR3 *out)
{
	// Create a copy of the string as it may be altered by strtok
	char *sc = (char *)malloc(sizeof(char) * (strlen(s)+1));
	strcpy(sc, s);

	char *c = strtok(sc, ", ");
	if (c != NULL) out->x = atof(c);
	
	c = strtok(NULL, ", ");
	if (c != NULL) out->y = atof(c);

	c = strtok(NULL, ", ");
	if (c != NULL) out->z = atof(c);

	free(sc);
}

char **IO::GetStringList(char *s, int *count)
{
	// Get the required length of array by counting commas
	int n = 0;
	char *c = strchr(s, ',');
	while (c != NULL) {
		n++;
		c = strchr(c+1, ',');
	}

	// Now allocate space for the elements
	char **list = (char **)malloc(sizeof(char*) * n);

	// Call the overloaded method with this list and then return the result
	IO::GetStringList(s, list, count);
	return list;
}

void IO::GetStringList(char *s, char **list, int *count)
{
	// Create a copy of the string as it may be altered by strtok
	char *sc = (char *)malloc(sizeof(char) * (strlen(s)+1));
	strcpy(sc, s);

	int n = 0;
	char *c = strtok(sc, ",");
	while (c != NULL) {
		char *el = (char *)malloc(sizeof(char) * (strlen(c)+1));
		strcpy(el, c);

		list[n++] = el;
		c = strtok(NULL, ",");
	}

	// Report back the number of elements that were added if required
	if (count != NULL) *count = n;

	// Deallocate memory for the string copy
	free(sc);
}*/

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