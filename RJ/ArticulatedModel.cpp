#include "GameVarsExtern.h"
#include "ArticulatedModelComponent.h"

#include "ArticulatedModel.h"

// Constructor; must set the number of components at creation
ArticulatedModel::ArticulatedModel(int componentcount)
{
	// Validate parameters
	if (componentcount < 1 || componentcount > Game::C_MAX_ARTICULATED_MODEL_SIZE)
	{
		m_componentcount = m_attachcount = 0;
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

}

// Set the definition of a particular model component.  Returns a value indicating whether the definition could be set
bool ArticulatedModel::SetComponentDefinition(int index, Model *model)
{
	// Parameter check
	if (index < 0 || index >= m_componentcount || m_components[index]) return false;

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
	SafeDelete(m_components);

	// Deallocate the attachment collection
	SafeDelete(m_attachments);
}



