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
	TiXmlElement *el = NewFloatXMLElement(name, f);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkDoubleXMLElement(const std::string &name, double d, TiXmlElement *parent)
{
	TiXmlElement *el = NewDoubleXMLElement(name, d);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkIntegerXMLElement(const std::string &name, int i, TiXmlElement *parent)
{
	TiXmlElement *el = NewIntegerXMLElement(name, i);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkStringXMLElement(const std::string &name, const std::string &s, TiXmlElement *parent)
{
	TiXmlElement *el = NewStringXMLElement(name, s);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkBoolXMLElement(const std::string &name, bool b, TiXmlElement *parent)
{
	TiXmlElement *el = NewBoolXMLElement(name, b);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkVector2AttrXMLElement(const std::string &name, const FXMVECTOR v, TiXmlElement *parent)
{
	TiXmlElement *el = NewVector2AttrXMLElement(name, v);
	if (parent) parent->LinkEndChild(el);
}
void IO::Data::LinkVector3AttrXMLElement(const std::string &name, const FXMVECTOR v, TiXmlElement *parent)
{
	TiXmlElement *el = NewVector3AttrXMLElement(name, v);
	if (parent) parent->LinkEndChild(el);
}
void IO::Data::LinkVector4AttrXMLElement(const std::string &name, const FXMVECTOR v, TiXmlElement *parent)
{
	TiXmlElement *el = NewVector4AttrXMLElement(name, v);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkIntVector2AttrXMLElement(const std::string &name, const INTVECTOR2 & v, TiXmlElement *parent)
{
	TiXmlElement *el = NewIntVector2AttrXMLElement(name, v);
	if (parent) parent->LinkEndChild(el);
}
void IO::Data::LinkIntVector3AttrXMLElement(const std::string &name, const INTVECTOR3 & v, TiXmlElement *parent)
{
	TiXmlElement *el = NewIntVector3AttrXMLElement(name, v);
	if (parent) parent->LinkEndChild(el);
}
