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

TiXmlElement *IO::Data::NewFloatXMLElement(const string &name, float f)
{
	char *str = (char *)malloc(sizeof(char) * 32);
	sprintf(str, "%.6f", f);

	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->LinkEndChild(new TiXmlText(str));

	free(str);
	return el;
}

TiXmlElement *IO::Data::NewDoubleXMLElement(const string &name, double d)
{
	char *str = (char *)malloc(sizeof(char) * 32);
	sprintf(str, "%.6f", d);

	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->LinkEndChild(new TiXmlText( str ));	

	free(str);
	return el;
}

TiXmlElement *IO::Data::NewIntegerXMLElement(const string &name, int i)
{
	char *str = (char *)malloc(sizeof(char) * 32);
	sprintf(str, "%d", i);

	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->LinkEndChild(new TiXmlText( str ));	

	free(str);
	return el;
}

TiXmlElement *IO::Data::NewStringXMLElement(const string &name, const string &s)
{
	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->LinkEndChild(new TiXmlText( s.c_str() ));
	return el;
}

TiXmlElement *IO::Data::NewBoolXMLElement(const string &name, bool b)
{
	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->LinkEndChild(new TiXmlText( (b ? "True" : "False") ));
	return el;
}

TiXmlElement *IO::Data::NewVectorAttrXMLElement(const string &name, D3DXVECTOR3 *v)
{
	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->SetDoubleAttribute("x", v->x);
	el->SetDoubleAttribute("y", v->y);
	el->SetDoubleAttribute("z", v->z);

	return el;
}

TiXmlElement *IO::Data::NewVectorAttrXMLElement(const string &name, D3DXVECTOR3 &v)
{
	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->SetDoubleAttribute("x", v.x);
	el->SetDoubleAttribute("y", v.y);
	el->SetDoubleAttribute("z", v.z);

	return el;
}

TiXmlElement *IO::Data::NewIntVectorAttrXMLElement(const string &name, INTVECTOR3 &v)
{
	TiXmlElement *el = new TiXmlElement( name.c_str() );
	el->SetAttribute("x", v.x);
	el->SetAttribute("y", v.y);
	el->SetAttribute("z", v.z);

	return el;
}

TiXmlElement *IO::Data::NewQuaternionAttrXMLElement(const string &name, D3DXQUATERNION &q)
{
	TiXmlElement *el = new TiXmlElement(name.c_str());
	el->SetDoubleAttribute("x", q.x);
	el->SetDoubleAttribute("y", q.y);
	el->SetDoubleAttribute("z", q.z);
	el->SetDoubleAttribute("w", q.w);

	return el;
}

void IO::Data::LinkFloatXMLElement(const string &name, float f, TiXmlElement *parent)
{
	TiXmlElement *el = NewFloatXMLElement(name, f);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkDoubleXMLElement(const string &name, double d, TiXmlElement *parent)
{
	TiXmlElement *el = NewDoubleXMLElement(name, d);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkIntegerXMLElement(const string &name, int i, TiXmlElement *parent)
{
	TiXmlElement *el = NewIntegerXMLElement(name, i);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkStringXMLElement(const string &name, const string &s, TiXmlElement *parent)
{
	TiXmlElement *el = NewStringXMLElement(name, s);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkBoolXMLElement(const string &name, bool b, TiXmlElement *parent)
{
	TiXmlElement *el = NewBoolXMLElement(name, b);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkVectorAttrXMLElement(const string &name, D3DXVECTOR3 *v, TiXmlElement *parent)
{
	TiXmlElement *el = NewVectorAttrXMLElement(name, v);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkVectorAttrXMLElement(const string &name, D3DXVECTOR3 &v, TiXmlElement *parent)
{
	TiXmlElement *el = NewVectorAttrXMLElement(name, &v);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkIntVectorAttrXMLElement(const string &name, INTVECTOR3 &v, TiXmlElement *parent)
{
	TiXmlElement *el = NewIntVectorAttrXMLElement(name, v);
	if (parent) parent->LinkEndChild(el);
}

void IO::Data::LinkQuaternionAttrXMLElement(const string &name, D3DXQUATERNION &q, TiXmlElement *parent)
{
	TiXmlElement *el = NewQuaternionAttrXMLElement(name, q);
	if (parent) parent->LinkEndChild(el);
}
