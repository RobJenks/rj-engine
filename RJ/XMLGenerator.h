#pragma once

#ifndef __XMLGeneratorH__
#define __XMLGeneratorH__

#include "XML\\tinyxml.h"
#include "Utility.h"


namespace IO { namespace Data {

	TiXmlElement *				NewGameDataXMLNode();

	TiXmlElement *				NewFloatXMLElement(const string &name, float f);
	TiXmlElement *				NewDoubleXMLElement(const string &name, double d);
	TiXmlElement *				NewIntegerXMLElement(const string &name, int i);
	TiXmlElement *				NewStringXMLElement(const string &name, const string &s);
	TiXmlElement *				NewBoolXMLElement(const string &name, bool b);
	TiXmlElement *				NewVectorAttrXMLElement(const string &name, D3DXVECTOR3 *v);
	TiXmlElement *				NewVectorAttrXMLElement(const string &name, D3DXVECTOR3 &v);
	TiXmlElement *				NewIntVectorAttrXMLElement(const string &name, INTVECTOR3 &v);
	TiXmlElement *				NewQuaternionAttrXMLElement(const string &name, D3DXQUATERNION &q);

	void						LinkFloatXMLElement(const string &name, float f, TiXmlElement *parent);
	void						LinkDoubleXMLElement(const string &name, double d, TiXmlElement *parent);
	void						LinkIntegerXMLElement(const string &name, int i, TiXmlElement *parent);
	void						LinkStringXMLElement(const string &name, const string &s, TiXmlElement *parent);
	void						LinkBoolXMLElement(const string &name, bool b, TiXmlElement *parent);
	void						LinkVectorAttrXMLElement(const string &name, D3DXVECTOR3 *v, TiXmlElement *parent);
	void						LinkVectorAttrXMLElement(const string &name, D3DXVECTOR3 &v, TiXmlElement *parent);
	void						LinkIntVectorAttrXMLElement(const string &name, INTVECTOR3 &v, TiXmlElement *parent);
	void						LinkQuaternionAttrXMLElement(const string &name, D3DXQUATERNION &q, TiXmlElement *parent);

}}



#endif