#include "GameVarsExtern.h"
#include "ArticulatedModelComponent.h"

#include "ArticulatedModel.h"

// The static central collection of model data
ArticulatedModel::ModelCollection ArticulatedModel::Models;

// Constructor; must set the number of components at creation
ArticulatedModel::ArticulatedModel(int componentcount)
{
	// Validate parameters
	if (componentcount < 1 || componentcount > Game::C_MAX_ARTICULATED_MODEL_SIZE)
	{
		m_componentcount = m_attachcount = m_rootcomponent = 0;
		m_components = NULL; 
		m_attachments = NULL;
		return;
	}

	// Store the component counts (always one fewer attachment than number of components)
	m_componentcount = componentcount;
	m_attachcount = m_componentcount - 1;

	// Allocate space for the model components, and initialise each one
	m_components = new ArticulatedModelComponent*[m_componentcount];
	for (int i = 0; i < m_componentcount; ++i)
	{
		m_components[i] = new ArticulatedModelComponent();
	}
	
	// Allocate space for the attachments between model components
	m_attachments = new Attachment<ArticulatedModelComponent*>[m_attachcount];

	// Allocate space for the array of index pairs that describe these attachments
	m_attachment_indices = new int[m_attachcount * 2];
	memset(m_attachment_indices, 0, (sizeof(int) * m_attachcount * 2));

	// Initialise other data
	m_rootcomponent = 0;
}

// Set the definition of a particular model component.  Returns a value indicating whether the definition could be set
bool ArticulatedModel::SetComponentDefinition(int index, Model *model)
{
	// Parameter check
	if (index < 0 || index >= m_componentcount || !m_components[index]) return false;

	// Set the model definition and return success
	m_components[index]->Model = model;
	return true;
}

// Returns a pointer to the specified component, if valid
ArticulatedModelComponent * ArticulatedModel::GetComponent(int index)
{
	if (index < 0 || index >= m_componentcount) return NULL;
	return m_components[index];
}

// Returns a pointer to the specified attachment, if valid
Attachment<ArticulatedModelComponent*> * ArticulatedModel::GetAttachment(int index)
{
	if (index < 0 || index >= m_attachcount) return NULL;
	return &(m_attachments[index]);
}




// Destructor; deallocates all resources used by the articulated model
ArticulatedModel::~ArticulatedModel(void)
{
	// Deallocate each model component in turn
	if (m_components)
	{
		for (int i = 0; i < m_componentcount; ++i)
		{
			if (m_components[i]) SafeDelete(m_components[i]);
		}
	}

	// Deallocate the component collection itself
	SafeDeleteArray(m_components);

	// Deallocate the attachment collection
	SafeDeleteArray(m_attachments);
}


// Test whether a model exists in the central collection
bool ArticulatedModel::ModelExists(const std::string & code)
{
	return (code != NullString && ArticulatedModel::Models.count(code) > 0);
}

// Retrieve a model from the central collection based on its string code
ArticulatedModel *ArticulatedModel::GetModel(const std::string & code)
{
	if (code != NullString && ArticulatedModel::Models.count(code) > 0)	return ArticulatedModel::Models[code];
	else																return NULL;
}

// Add a new model to the central collection, indexed by its unique string code
void ArticulatedModel::AddModel(ArticulatedModel *model)
{
	// Make sure the model is valid, and that we do not already have a model with its unique code
	if (!model || model->GetCode() == NullString || ArticulatedModel::Models.count(model->GetCode()) > 0) return;

	// Add to the central collection, indexed by its string code
	ArticulatedModel::Models[model->GetCode()] = model;
}

void ArticulatedModel::TerminateAllModelData(void)
{
	// All standard models are contained within the model collection, so we can iterate over it and dispose
	// of objects one by one via their standard destructor
	ModelCollection::iterator it_end = ArticulatedModel::Models.end();
	for (ModelCollection::iterator it = ArticulatedModel::Models.begin(); it != it_end; ++it)
	{
		if (it->second)
		{
			delete it->second;
		}
	}

	// Clear the collection, now that it is simply full of null pointers
	ArticulatedModel::Models.clear();
}

// Attempts to create an attachment between the two specified components.  Returns an errorcode if not possible
Result ArticulatedModel::SetAttachment(int attach_index, int parent_index, int child_index)
{
	// Make sure indices are valid, and component indices are different
	if (attach_index < 0 || attach_index >= m_attachcount || parent_index < 0 || parent_index >= m_componentcount || 
		child_index < 0 || child_index >= m_componentcount || parent_index == child_index)
	{
		return ErrorCodes::CannotLoadAttachmentDataForArticulatedModel;
	}

	// Set the reference to each object in the current attachment
	Attachment<ArticulatedModelComponent*> *attach = &(m_attachments[attach_index]);
	attach->Parent = m_components[parent_index];
	attach->Child = m_components[child_index];

	// Make sure both parent and child are valid objects, are different, and also ensure the child is not already 
	// attached (objects can only ever be attached to one parent)
	if (!attach->Parent || !attach->Child || attach->Parent == attach->Child || attach->Child->HasParentAttachment())
	{
		return ErrorCodes::CannotLoadAttachmentDataForArticulatedModel;
	}

	// Also store the indices for more efficient lookups at runtime
	m_attachment_indices[attach_index]		= parent_index;
	m_attachment_indices[attach_index + 1]	= child_index;

	// Update the parent and child that they are now members of this attachment
	attach->Parent->SetChildAttachmentState(true);
	attach->Child->SetParentAttachmentState(true);

	// Return success
	return ErrorCodes::NoError;
}

// Replaces a component with a copy of the specified one
void ArticulatedModel::ReplaceComponent(int index, const ArticulatedModelComponent *copy_source)
{
	// Parameter check
	if (index < 0 || index >= m_componentcount || !copy_source) return;

	// Use the assignment operator to make a copy of the component
	(*m_components[index]) = (*copy_source);
}

// Initialises the model once all data has been loaded, to make sure it is ready for use in-game
void ArticulatedModel::PerformPostLoadInitialisation(void)
{
	// Determine the root component for this model
	m_rootcomponent = 0;
	for (int i = 0; i < m_componentcount; ++i)
	{
		if (!m_components[i]->HasParentAttachment())
		{
			m_rootcomponent = i;
			break;
		}
	}
}

// Creates and returns an exact copy of the specified articualated model
ArticulatedModel * ArticulatedModel::Copy(void)
{
	Result result;

	// Create a new model object of the required size
	int ccount = this->GetComponentCount();
	if (ccount <= 0 || ccount > Game::C_MAX_ARTICULATED_MODEL_SIZE) return NULL;
	ArticulatedModel *model = new ArticulatedModel(ccount);
	if (!model) return NULL;

	// Copy the model code by default
	model->SetCode(m_code);

	// Copy each component in turn
	for (int i = 0; i < ccount; ++i)
	{
		if (!m_components[i]) return NULL;
		model->ReplaceComponent(i, m_components[i]);	// TODO: Make sure this uses the operator= method
	}

	// Recreate each attachment in turn
	int acount = this->GetAttachmentCount();
	for (int i = 0; i < acount; ++i)
	{
		result = model->SetAttachment(i, m_attachment_indices[i], m_attachment_indices[i + 1]);
		if (result != ErrorCodes::NoError)
		{
			SafeDelete(model);
			return NULL;
		}
	}

	// Return a pointer to the new model
	return model;
}



