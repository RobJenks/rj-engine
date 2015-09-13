#pragma once
#ifndef __ArticulatedModelH__
#define __ArticulatedModelH__

#include "CompilerSettings.h"
#include "ArticulatedModelComponent.h"
#include "Attachment.h"

class ArticulatedModel
{
public:

	// Constructor; must set the number of components at creation
	ArticulatedModel(int componentcount);

	// Get or set the unique string code for this articulated model
	CMPINLINE std::string						GetCode(void) const							{ return m_code; }
	CMPINLINE void								SetCode(const std::string &code)			{ m_code = code; }

	// Returns the number of components or attachments in this articulated model
	CMPINLINE int								GetComponentCount(void) const				{ return m_componentcount; }
	CMPINLINE int								GetAttachmentCount(void) const				{ return m_attachcount; }

	// Set the definition of a particular model component.  Returns a value indicating whether the definition could be set
	bool										SetComponentDefinition(int index, Model *model);

	// Returns a pointer to the specified component, if valid
	ArticulatedModelComponent *					GetComponent(int index);

	// Returns a pointer to the specified attachment, if valid
	Attachment<ArticulatedModelComponent*> *	GetAttachment(int index);

	// Destructor; deallocates all resources used by the articulated model
	~ArticulatedModel(void);


protected:

	// Unique string code to identify this articulated model
	std::string									m_code;

	// The number of components and attachments that make up this model (c == a + 1)
	int											m_componentcount, m_attachcount;

	// Collection of model components
	ArticulatedModelComponent **				m_components;

	// Collection of attachments and constraints between each component
	Attachment<ArticulatedModelComponent*> *	m_attachments;
};




#endif