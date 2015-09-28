#ifndef __FileInputH__
#define __FileInputH__

#include "DX11_Core.h"

#include "XML\\tinyxml.h"
#include "ErrorCodes.h"
#include "Utility.h"

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

	D3DXVECTOR2		GetVector2FromAttr(TiXmlElement *node);
	void			GetVector2FromAttr(TiXmlElement *node, D3DXVECTOR2 *out);

	D3DXVECTOR3		GetVector3FromAttr(TiXmlElement *node);
	void			GetVector3FromAttr(TiXmlElement *node, D3DXVECTOR3 *out);

	D3DXVECTOR4		GetVector4FromAttr(TiXmlElement *node);
	void			GetVector4FromAttr(TiXmlElement *node, D3DXVECTOR4 *out);

	D3DXQUATERNION	GetD3DXQUATERNIONFromAttr(TiXmlElement *node);
	void			GetD3DXQUATERNIONFromAttr(TiXmlElement *node, D3DXQUATERNION *out);

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

	CMPINLINE int GetIntValue(TiXmlElement *node)
	{
		___tmp_loading_cchar = node->GetText();
		if (___tmp_loading_cchar) return atoi(___tmp_loading_cchar); else return 0;
	}

	CMPINLINE std::string GetLCString(TiXmlElement *node)
	{
		___tmp_loading_string = node->GetText();
		if (___tmp_loading_string != NullString) {
			StrLowerC(___tmp_loading_string); return ___tmp_loading_string;
		}
		else return "";
	}
	CMPINLINE bool GetBoolValue(TiXmlElement *node)
	{
		___tmp_loading_string = node->GetText();
		if (___tmp_loading_string != NullString) {
			StrLowerC(___tmp_loading_string); return (___tmp_loading_string == "true");
		} 
		else {
			return false;
		}
	}



}

#endif