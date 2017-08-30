#pragma once

#ifndef __XMLGeneratorH__
#define __XMLGeneratorH__

#include "XML\\tinyxml.h"
#include "Utility.h"


// This file contains no objects with special alignment requirements
namespace IO { namespace Data {

	TiXmlElement *				NewGameDataXMLNode();

	TiXmlElement *				NewFloatXMLElement(const std::string &name, float f);
	TiXmlElement *				NewDoubleXMLElement(const std::string &name, double d);
	TiXmlElement *				NewIntegerXMLElement(const std::string &name, int i);
	TiXmlElement *				NewStringXMLElement(const std::string &name, const std::string &s);
	TiXmlElement *				NewBoolXMLElement(const std::string &name, bool b);
	TiXmlElement *				NewVector2AttrXMLElement(const std::string &name, const FXMVECTOR v);
	TiXmlElement *				NewVector3AttrXMLElement(const std::string &name, const FXMVECTOR v);
	TiXmlElement *				NewVector4AttrXMLElement(const std::string &name, const FXMVECTOR v);
	TiXmlElement *				NewIntVector2AttrXMLElement(const std::string &name, const INTVECTOR2 & v);
	TiXmlElement *				NewIntVector3AttrXMLElement(const std::string &name, const INTVECTOR3 & v);
	TiXmlElement *				NewFloat2AttrXMLElement(const std::string &name, const XMFLOAT2 & v);
	TiXmlElement *				NewFloat3AttrXMLElement(const std::string &name, const XMFLOAT3 & v);
	TiXmlElement *				NewFloat4AttrXMLElement(const std::string &name, const XMFLOAT4 & v);
	CMPINLINE TiXmlElement *	NewQuaternionAttrXMLElement(const std::string &name, const FXMVECTOR q)		
	{ 
		return NewVector4AttrXMLElement(name, q); 
	}


	void						LinkFloatXMLElement(const std::string &name, float f, TiXmlElement *parent);
	void						LinkDoubleXMLElement(const std::string &name, double d, TiXmlElement *parent);
	void						LinkIntegerXMLElement(const std::string &name, int i, TiXmlElement *parent);
	void						LinkStringXMLElement(const std::string &name, const std::string &s, TiXmlElement *parent);
	void						LinkBoolXMLElement(const std::string &name, bool b, TiXmlElement *parent);
	void						LinkVector2AttrXMLElement(const std::string &name, const FXMVECTOR v, TiXmlElement *parent);
	void						LinkVector3AttrXMLElement(const std::string &name, const FXMVECTOR v, TiXmlElement *parent);
	void						LinkVector4AttrXMLElement(const std::string &name, const FXMVECTOR v, TiXmlElement *parent);
	void						LinkIntVector2AttrXMLElement(const std::string &name, const INTVECTOR2 &v, TiXmlElement *parent);
	void						LinkIntVector3AttrXMLElement(const std::string &name, const INTVECTOR3 &v, TiXmlElement *parent);
	CMPINLINE void				LinkQuaternionAttrXMLElement(const std::string &name, const FXMVECTOR q, TiXmlElement *parent) 
	{ 
		return LinkVector4AttrXMLElement(name, q, parent); 
	}

}}



#endif