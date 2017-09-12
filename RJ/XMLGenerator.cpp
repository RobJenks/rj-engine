#include "DX11_Core.h"

#include "XML\\tinyxml.h"
#include "Utility.h"
#include "GameDataExtern.h"

#include "XMLGenerator.h"


TiXmlElement *IO::Data::NewGameDataXMLNode(void)
{
	// Create a new root gamedata element and return a pointer to it
	TiXmlElement *root = new TiXmlElement(D::NODE_GameData);
	return root;
}

TiXmlElement *IO::Data::NewFloatXMLElement(const std::string &name, float f)
{
	char *str = (char *)malloc(sizeof(char) * 32);
	sprintf(str, "%.6f", f);

	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->LinkEndChild(new TiXmlText(str));

	free(str);
	return el;
}

TiXmlElement *IO::Data::NewDoubleXMLElement(const std::string &name, double d)
{
	char *str = (char *)malloc(sizeof(char) * 32);
	sprintf(str, "%.6f", d);

	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->LinkEndChild(new TiXmlText( str ));	

	free(str);
	return el;
}

TiXmlElement *IO::Data::NewIntegerXMLElement(const std::string &name, int i)
{
	char *str = (char *)malloc(sizeof(char) * 32);
	sprintf(str, "%d", i);

	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->LinkEndChild(new TiXmlText( str ));	

	free(str);
	return el;
}

TiXmlElement *IO::Data::NewStringXMLElement(const std::string &name, const std::string &s)
{
	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->LinkEndChild(new TiXmlText( s.c_str() ));
	return el;
}

TiXmlElement *IO::Data::NewBoolXMLElement(const std::string &name, bool b)
{
	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->LinkEndChild(new TiXmlText( (b ? "True" : "False") ));
	return el;
}

TiXmlElement *IO::Data::NewVector2AttrXMLElement(const std::string &name, const FXMVECTOR v)
{
	XMFLOAT2 vf; XMStoreFloat2(&vf, v);

	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->SetDoubleAttribute("x", vf.x);
	el->SetDoubleAttribute("y", vf.y);

	return el;
}
TiXmlElement *IO::Data::NewVector3AttrXMLElement(const std::string &name, const FXMVECTOR v)
{
	XMFLOAT3 vf; XMStoreFloat3(&vf, v);

	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->SetDoubleAttribute("x", vf.x);
	el->SetDoubleAttribute("y", vf.y);
	el->SetDoubleAttribute("z", vf.z);

	return el;
}
TiXmlElement *IO::Data::NewVector4AttrXMLElement(const std::string &name, const FXMVECTOR v)
{
	XMFLOAT4 vf; XMStoreFloat4(&vf, v);

	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->SetDoubleAttribute("x", vf.x);
	el->SetDoubleAttribute("y", vf.y);
	el->SetDoubleAttribute("z", vf.z);
	el->SetDoubleAttribute("w", vf.w);

	return el;
}

TiXmlElement *IO::Data::NewIntAttributeElement(const std::string & name, const std::string & attribute_name, int v)
{
	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->SetAttribute(attribute_name.c_str(), v);

	return el;
}

TiXmlElement *IO::Data::NewFloatAttributeElement(const std::string & name, const std::string & attribute_name, float v)
{
	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->SetDoubleAttribute(attribute_name.c_str(), (double)v);

	return el;
}

TiXmlElement *IO::Data::NewStringAttributeElement(const std::string & name, const std::string & attribute_name, const std::string & v)
{
	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->SetAttribute(attribute_name.c_str(), v.c_str());

	return el;
}

TiXmlElement *IO::Data::NewIntVector2AttrXMLElement(const std::string &name, const INTVECTOR2 & v)
{
	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->SetAttribute("x", v.x);
	el->SetAttribute("y", v.y);

	return el;
}
TiXmlElement *IO::Data::NewIntVector3AttrXMLElement(const std::string &name, const INTVECTOR3 & v)
{
	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->SetAttribute("x", v.x);
	el->SetAttribute("y", v.y);
	el->SetAttribute("z", v.z);

	return el;
}

void IO::Data::LinkFloatXMLElement(const std::string &name, float f, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewFloatXMLElement(name, f));
}

void IO::Data::LinkDoubleXMLElement(const std::string &name, double d, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewDoubleXMLElement(name, d));
}

void IO::Data::LinkIntegerXMLElement(const std::string &name, int i, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewIntegerXMLElement(name, i));
}

void IO::Data::LinkStringXMLElement(const std::string &name, const std::string &s, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewStringXMLElement(name, s));
}

void IO::Data::LinkBoolXMLElement(const std::string &name, bool b, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewBoolXMLElement(name, b));
}

void IO::Data::LinkVector2AttrXMLElement(const std::string &name, const FXMVECTOR v, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewVector2AttrXMLElement(name, v));
}
void IO::Data::LinkVector3AttrXMLElement(const std::string &name, const FXMVECTOR v, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewVector3AttrXMLElement(name, v));
}
void IO::Data::LinkVector4AttrXMLElement(const std::string &name, const FXMVECTOR v, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewVector4AttrXMLElement(name, v));
}
void IO::Data::LinkIntAttributeElement(const std::string &name, const std::string & attribute_name, int v, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewIntAttributeElement(name, attribute_name, v));
}
void IO::Data::LinkFloatAttributeElement(const std::string &name, const std::string & attribute_name, float v, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewFloatAttributeElement(name, attribute_name, v));
}
void IO::Data::LinkStringAttributeElement(const std::string &name, const std::string & attribute_name, std::string & v, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewStringAttributeElement(name, attribute_name, v));
}

void IO::Data::LinkIntVector2AttrXMLElement(const std::string &name, const INTVECTOR2 & v, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewIntVector2AttrXMLElement(name, v));
}
void IO::Data::LinkIntVector3AttrXMLElement(const std::string &name, const INTVECTOR3 & v, TiXmlElement *parent)
{
	if (!parent) return;
	parent->LinkEndChild(NewIntVector3AttrXMLElement(name, v));
}
