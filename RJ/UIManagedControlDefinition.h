#pragma once

#ifndef __UIManagedControlDefinition__
#define __UIManagedControlDefinition__

#include <string>
#include <unordered_map>
#include "CompilerSettings.h"
#include "iUIControl.h"


class UIManagedControlDefinition
{
public:
	UIManagedControlDefinition(void);
	~UIManagedControlDefinition(void);

	// Methods to get and set key parameters
	CMPINLINE std::string			GetCode(void) { return m_code; }
	CMPINLINE void					SetCode(std::string code) { m_code = code; }
	CMPINLINE iUIControl::Type		GetClass(void) { return m_class; }
	CMPINLINE void					SetClass(iUIControl::Type c) { m_class = c; }

	// Determines whether we have an item with the key specified
	CMPINLINE bool					HaveComponent(std::string key) { return (m_components.count(key) > 0 && m_components[key] != NullString); }
	CMPINLINE std::string			GetComponent(std::string key) { return m_components[key]; }
	CMPINLINE void					AddComponent(std::string key, std::string val) { m_components[key] = val; }
	CMPINLINE void					RemoveComponent(std::string key) { m_components[key] = ""; }

	// Performs a validation of all components specified in this definition
	bool							ValidateDefinition(void);

	// Terminats the control definition and releases any resources
	void							Shutdown(void) { }


private:
	// The class of control this definition refers to, and its code
	iUIControl::Type										m_class;
	std::string												m_code;

	// Location of all files making up this control definition
	typedef std::unordered_map<std::string, std::string>	DefinitionComponentCollection;
	DefinitionComponentCollection							m_components;




};



#endif