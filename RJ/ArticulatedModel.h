#pragma once
#ifndef __ArticulatedModelH__
#define __ArticulatedModelH__

#include <unordered_map>
#include "CompilerSettings.h"
#include "ArticulatedModelComponent.h"
#include "Attachment.h"

class ArticulatedModel
{
public:

	// Model data is stored in static unordered_map collections & indexed by unique string code
	typedef std::unordered_map<std::string, ArticulatedModel*> ModelCollection;

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

	// Replaces a component with a copy of the specified one
	void										ReplaceComponent(int index, const ArticulatedModelComponent *copy_source);

	// Returns a pointer to the specified attachment, if valid
	Attachment<ArticulatedModelComponent*> *	GetAttachment(int index);

	// Attempts to create an attachment between the two specified components.  Returns an errorcode if not possible
	Result										SetAttachment(int attach_index, int parent_index, int child_index);

	// Initialises the model once all data has been loaded, to make sure it is ready for use in-game
	void										PerformPostLoadInitialisation(void);

	// Performs an update of all components in the articulated model before rendering
	void										Update(	const D3DXVECTOR3 & position, const D3DXQUATERNION & orientation,
														const D3DXMATRIX * worldmatrix);

	// Rotates about the specified constraint.  If no constraint is defined at this attachment point (i.e. if this is a 
	// fixed attachment) no action will be taken
	void										RotateConstraint(int constraint_index, float radians);

	// Rotates the specified component about its parent.  Marginally less efficient than RotateConstraint since we have to locate
	// the appropriate constraint first.  If this component is not attached by a constraint (i.e. is the child of a fixed attachment)
	// then no action will be taken
	void										RotateComponent(int component_index, float radians);

	// Sets the rotation value about the specified constraint.  If no constraint is defined at this attachment point (i.e. if this is a 
	// fixed attachment) no action will be taken
	void										SetConstraintRotation(int constraint_index, float radians);

	// Sets the rotation of this component about its parent.  Marginally less efficient than RotateConstraint since we have to locate
	// the appropriate constraint first.  If this component is not attached by a constraint (i.e. is the child of a fixed attachment)
	// then no action will be taken
	void										SetComponentRotation(int component_index, float radians);

	// Destructor; deallocates all resources used by the articulated model
	~ArticulatedModel(void);

	// Static collection of all articulated models in the game, indexed by string code
	static ModelCollection						Models;

	// Central model storage methods
	static bool									ModelExists(const std::string & code);
	static ArticulatedModel *					GetModel(const std::string & code);
	static void									AddModel(ArticulatedModel *model);
	static void									TerminateAllModelData(void);

	// Creates and returns an exact copy of the specified articualated model
	ArticulatedModel *							Copy(void);

protected:

	// Unique string code to identify this articulated model
	std::string									m_code;

	// The number of components and attachments that make up this model (c == a + 1)
	int											m_componentcount, m_attachcount;

	// Collection of model components
	ArticulatedModelComponent **				m_components;

	// Store a reference to the root component for rendering efficiency
	int											m_rootcomponent;

	// Collection of attachments and constraints between each component
	Attachment<ArticulatedModelComponent*> *	m_attachments;

	// Collection of integer pairs describing the attachments, for efficient lookup
	int *										m_attachment_indices;
};




#endif