#pragma once

#ifndef __XMLGeneratorH__
#define __XMLGeneratorH__

#include "XML\\tinyxml.h"
#include "Utility.h"


// This file contains no objects with special alignment requirements
namespace IO { namespace Data {

	TiXmlElement *				NewGameDataXMLNode();

	TiXmlElement *				NewFloatXMLElement(const string &name, float f);
	TiXmlElement *				NewDoubleXMLElement(const string &name, double d);
	TiXmlElement *				NewIntegerXMLElement(const string &name, int i);
	TiXmlElement *				NewStringXMLElement(const string &name, const string &s);
	TiXmlElement *				NewBoolXMLElement(const string &name, bool b);
	TiXmlElement *				NewVector2AttrXMLElement(const string &name, const FXMVECTOR v);
	TiXmlElement *				NewVector3AttrXMLElement(const string &name, const FXMVECTOR v);
	TiXmlElement *				NewVector4AttrXMLElement(const string &name, const FXMVECTOR v);
	TiXmlElement *				NewIntVector2AttrXMLElement(const string &name, const INTVECTOR2 & v);
	TiXmlElement *				NewIntVector3AttrXMLElement(const string &name, const INTVECTOR3 & v);
	TiXmlElement *				NewFloat2AttrXMLElement(const string &name, const XMFLOAT2 & v);
	TiXmlElement *				NewFloat3AttrXMLElement(const string &name, const XMFLOAT3 & v);
	TiXmlElement *				NewFloat4AttrXMLElement(const string &name, const XMFLOAT4 & v);
	TiXmlElement *				NewQuaternionAttrXMLElement(const string &name, const FXMVECTOR q)		
	{ 
		return NewVector4AttrXMLElement(name, q); 
	}


	void						LinkFloatXMLElement(const string &name, float f, TiXmlElement *parent);
	void						LinkDoubleXMLElement(const string &name, double d, TiXmlElement *parent);
	void						LinkIntegerXMLElement(const string &name, int i, TiXmlElement *parent);
	void						LinkStringXMLElement(const string &name, const string &s, TiXmlElement *parent);
	void						LinkBoolXMLElement(const string &name, bool b, TiXmlElement *parent);
	void						LinkVector2AttrXMLElement(const string &name, const FXMVECTOR v, TiXmlElement *parent);
	void						LinkVector3AttrXMLElement(const string &name, const FXMVECTOR v, TiXmlElement *parent);
	void						LinkVector4AttrXMLElement(const string &name, const FXMVECTOR v, TiXmlElement *parent);
	void						LinkIntVector2AttrXMLElement(const string &name, const INTVECTOR2 &v, TiXmlElement *parent);
	void						LinkIntVector3AttrXMLElement(const string &name, const INTVECTOR3 &v, TiXmlElement *parent);
	void						LinkQuaternionAttrXMLElement(const string &name, const FXMVECTOR q, TiXmlElement *parent) 
	{ 
		return LinkQuaternionAttrXMLElement(name, q, parent); 
	}

}}



#endif