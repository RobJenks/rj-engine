#ifndef __FileInputH__
#define __FileInputH__

#include "DX11_Core.h"

#include "XML\\tinyxml.h"
#include "ErrorCodes.h"
#include "Utility.h"


// This file contains no objects with special alignment requirements
namespace IO 
{
	extern const char *		___tmp_loading_cchar;
	extern std::string		___tmp_loading_string;

	const int		MAX_LINE_SIZE = 1024;

/*
	bool			IsHeader(char *s);
	bool			IsData(char *s);

	char*			GetHeader(char *s);
	void			GetHeader(char *s, char *out);

	char*			GetLabel(char *s);
	void			GetLabel(char *s, char *out);

	char*			GetValue(char *s);
	void			GetValue(char *s, char *out);

	D3DXVECTOR3		GetVector(char *s);
	void			GetVector(char *s, D3DXVECTOR3 *out);

	char**			GetStringList(char *s, int *count);
	void			GetStringList(char *s, char **list, int *count);
*/

	XMVECTOR		GetVector2FromAttr(TiXmlElement *node);
	XMFLOAT2		GetFloat2FromAttr(TiXmlElement *node);
	void			GetFloat2FromAttr(TiXmlElement *node, XMFLOAT2 *out);

	XMVECTOR		GetVector3FromAttr(TiXmlElement *node);
	XMFLOAT3		GetFloat3FromAttr(TiXmlElement *node);
	void			GetFloat3FromAttr(TiXmlElement *node, XMFLOAT3 *out);

	XMVECTOR		GetVector4FromAttr(TiXmlElement *node);
	XMFLOAT4		GetFloat4FromAttr(TiXmlElement *node);
	void			GetFloat4FromAttr(TiXmlElement *node, XMFLOAT4 *out);

	XMVECTOR		GetColourVectorFromAttr(TiXmlElement *node);
	XMFLOAT4		GetColourFloatFromAttr(TiXmlElement *node);
	void			GetColourFloatFromAttr(TiXmlElement *node, XMFLOAT4 *out);

	CMPINLINE XMVECTOR	GetQuaternionFromAttr(TiXmlElement *node)					{ return GetVector4FromAttr(node); }
	CMPINLINE XMFLOAT4	GetQuaternionFromAttrF(TiXmlElement *node)					{ return GetFloat4FromAttr(node); }
	CMPINLINE void		GetQuaternionFromAttrF(TiXmlElement *node, XMFLOAT4 *out)	{ return GetFloat4FromAttr(node, out); }

	INTVECTOR2		GetInt2CoordinatesFromAttr(TiXmlElement *node);
	INTVECTOR3		GetInt3CoordinatesFromAttr(TiXmlElement *node);

	void			GetInt2CoordinatesFromAttr(TiXmlElement *node, int *x, int *y);
	void			GetInt3CoordinatesFromAttr(TiXmlElement *node, int *x, int *y, int *z);

	// Get the specified integer attribute, returning 0 if the attribute does not exist.  "node" must exist or exception is thrown
	int				GetIntegerAttribute(TiXmlElement *node, const char *attribute);

	// Get the specified integer attribute, returning 'defaultvalue' if the attribute does not exist.  "node" must exist or exception is thrown
	int				GetIntegerAttribute(TiXmlElement *node, const char *attribute, int defaultvalue);

	// Attempt to get the specified integer attribute, returning 0 and an error code != NoError if the attribute does not exist
	// "node" must exist or exception is thrown
	Result			TryGetIntegerAttribute(TiXmlElement *node, const char *attribute, int & outValue);

	// Attempt to get the specified integer attribute, returning 'defaultvalue' and an error code != NoError if the attribute does not exist
	// "node" must exist or exception is thrown
	Result			TryGetIntegerAttribute(TiXmlElement *node, const char *attribute, int defaultvalue, int & outValue);

	CMPINLINE float	GetFloatValue(TiXmlElement *node) 
	{ 
		___tmp_loading_cchar = node->GetText();  
		if (___tmp_loading_cchar) return (float)atof(___tmp_loading_cchar); else return 0.0f;
	}

	CMPINLINE float	GetFloatValue(TiXmlElement *node, float default_value)
	{
		___tmp_loading_cchar = node->GetText();
		if (___tmp_loading_cchar) return (float)atof(___tmp_loading_cchar); else return default_value;
	}

	CMPINLINE int GetIntValue(TiXmlElement *node)
	{
		___tmp_loading_cchar = node->GetText();
		if (___tmp_loading_cchar) return atoi(___tmp_loading_cchar); else return 0;
	}

	CMPINLINE std::string GetLCString(TiXmlElement *node)
	{
		___tmp_loading_cchar = node->GetText();
		if (!___tmp_loading_cchar) return "";

		___tmp_loading_string = ___tmp_loading_cchar;
		if (___tmp_loading_string != NullString) {
			StrLowerC(___tmp_loading_string); 
			return ___tmp_loading_string;
		}
		else return "";
	}
	CMPINLINE bool GetBoolValue(TiXmlElement *node)
	{
		___tmp_loading_cchar = node->GetText();
		if (!___tmp_loading_cchar) return "";

		___tmp_loading_string = ___tmp_loading_cchar;
		if (___tmp_loading_string != NullString) {
			StrLowerC(___tmp_loading_string); 
			return (___tmp_loading_string == "true");
		} 
		else {
			return false;
		}
	}



}

#endif