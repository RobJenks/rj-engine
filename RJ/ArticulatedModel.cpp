#include "GameVarsExtern.h"
#include "Model.h"
#include "ArticulatedModelComponent.h"

#include "ArticulatedModel.h"

// The static central collection of model data
ArticulatedModel::ModelCollection ArticulatedModel::Models;

// Constructor; must set the number of components at creation
ArticulatedModel::ArticulatedModel(int componentcount)
	:
	m_code(NullString), m_componentcount(0), m_attachcount(0), m_components(NULL), 
	m_rootcomponent(0), m_attachments(NULL), m_attachment_indices(NULL), m_extent(NULL_FLOAT3)
{
	// Validate parameters
	if (componentcount < 1 || componentcount > Game::C_MAX_ARTICULATED_MODEL_SIZE) return;

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
	m_attachment_indices[(attach_index * 2)]		= parent_index;
	m_attachment_indices[(attach_index * 2) + 1]	= child_index;

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
	XMVECTOR extent = NULL_VECTOR;
	m_rootcomponent = 0;

	// Process each component in turn
	for (int i = 0; i < m_componentcount; ++i)
	{
		// Add component extents to the consolidate terrain bounding box
		Model *model = m_components[i]->Model;
		if (model)
		{
			XMVECTOR largest_bound = XMVectorMax(
				XMVectorAbs(XMVectorAdd(m_components[i]->GetPosition(), XMLoadFloat3(&model->GetModelMinBounds()))),
				XMVectorAbs(XMVectorAdd(m_components[i]->GetPosition(), XMLoadFloat3(&model->GetModelMaxBounds()))));

			extent = XMVectorMax(extent, largest_bound);
		}

		// Also attempt to determine the root component for this model
		if (!m_components[i]->HasParentAttachment())
		{
			m_rootcomponent = i;
		}
	}

	// Update the consolidated model extent based on the per-component calculations above
	XMStoreFloat3(&m_extent, extent);
	
}

// Performs an update of all components in the articulated model before rendering
void ArticulatedModel::Update(	const FXMVECTOR position, const FXMVECTOR orientation,
								const CXMMATRIX worldmatrix)
{
	// Update the root component with this data
	m_components[m_rootcomponent]->SetAllSpatialData(position, orientation, worldmatrix);

	// Now apply all attachments; no need to apply in sequence, the components will resolve any timing
	// differences within a couple of frames.  NOTE: there will be no timing differences if attachment
	// order is specified correctly in xml data (i.e. from root to children)
	for (int i = 0; i < m_attachcount; ++i)
	{
		// Apply the attachment to move the child into its correct position and orientation
		m_attachments[i].Apply();

		// Attachment will only recalculate child world matrix if it has children of its own, by default,
		// since normal objects will recalculate their world matrix on the next cycle.  It would
		// only then be necessary to recalculate it here if the matrix is needed for updating its child
		// objects during this cycle.  Articulated model components do not automatically recalculate
		// their world matrix on per-frame simulation, so force a recalculation for any components that 
		// wouldn't be covered by the normal attachment logic (i.e. those without children of their own)
		if (!m_attachments[i].Child->HasChildAttachments()) m_attachments[i].Child->RefreshPositionImmediate();
	}
}

// Rotates about the specified constraint.  If no constraint is defined at this attachment point (i.e. if this is a 
// fixed attachment) no action will be taken
void ArticulatedModel::RotateConstraint(int constraint_index, float radians)
{
	// Make sure the constraint is valid
	if (constraint_index < 0 || constraint_index >= m_attachcount) return;

	// Update the constraint
	m_attachments[constraint_index].RotateChildAboutConstraint(radians);
}

// Rotates the specified component about its parent.  Marginally less efficient than RotateConstraint since we have to locate
// the appropriate constraint first.  If this component is not attached by a constraint (i.e. is the child of a fixed attachment)
// then no action will be taken
void ArticulatedModel::RotateComponent(int component_index, float radians)
{
	// Find the attachment that influences this component (no need to validate component index; if invalid, we won't find an attachment).  Start at 1
	// and move += 2 each time to only check child indices.  (i-1) is therefore always even and ((i-1)/2) always give the correct attachment index
	for (int i = 1; i < m_attachcount; i += 2)
	{
		if (m_attachment_indices[i] == component_index)
		{
			m_attachments[(i - 1) / 2].RotateChildAboutConstraint(radians);
			break;
		}
	}
}

// Sets the rotation value about the specified constraint.  If no constraint is defined at this attachment point (i.e. if this is a 
// fixed attachment) no action will be taken
void ArticulatedModel::SetConstraintRotation(int constraint_index, float radians)
{
	// Make sure the constraint is valid
	if (constraint_index < 0 || constraint_index >= m_attachcount) return;

	// Update the constraint
	m_attachments[constraint_index].SetChildRotationAboutConstraint(radians);
}

// Return the current rotation value for a particular constraint (or 0.0 if the given constraint is not valid or is fixed)
float ArticulatedModel::GetConstraintRotation(int constraint_index)
{
	// Make sure the constraint is valid
	if (constraint_index < 0 || constraint_index >= m_attachcount) return 0.0f;

	// Return the current rotation about this constraint
	return m_attachments[constraint_index].GetChildRotationAboutConstraint();
}

// Sets the rotation of this component about its parent.  Marginally less efficient than RotateConstraint since we have to locate
// the appropriate constraint first.  If this component is not attached by a constraint (i.e. is the child of a fixed attachment)
// then no action will be taken
void ArticulatedModel::SetComponentRotation(int component_index, float radians)
{
	// Find the attachment that influences this component (no need to validate component index; if invalid, we won't find an attachment).  Start at 1
	// and move += 2 each time to only check child indices.  (i-1) is therefore always even and ((i-1)/2) always give the correct attachment index
	for (int i = 1; i < m_attachcount; i += 2)
	{
		if (m_attachment_indices[i] == component_index)
		{
			m_attachments[(i - 1) / 2].SetChildRotationAboutConstraint(radians);
			break;
		}
	}
}

// Add a new model tag, assuming it is valid
void ArticulatedModel::AddConstraintTag(int parent, int child, const std::string & tag)
{
	if (parent < 0 || parent >= m_componentcount || child < 0 || child >= m_componentcount) return;
	Tags.push_back(ArticulatedModel::ModelTag(parent, child, tag));
}

// Add a new model tag, assuming it is valid
void ArticulatedModel::AddComponentTag(int component, const std::string & tag)
{
	if (component < 0 || component >= m_componentcount) return;
	Tags.push_back(ArticulatedModel::ModelTag(component, -1, tag));
}

// Retrieve the index of the constraint with the specified tag, or -1 if no such tag exists
int ArticulatedModel::GetConstraintWithTag(const std::string & tag)
{
	int parent = -1, child = -1;

	// Attempt to locate this tag, and the parent/child indices associated to it
	std::vector<ModelTag>::const_iterator it_end = Tags.end();
	for (std::vector<ModelTag>::const_iterator it = Tags.begin(); it != it_end; ++it)
	{
		if ((*it).Tag == tag)
		{
			parent = (*it).Parent;
			child = (*it).Child;
			break;
		}
	}

	// Return an error if we have no matching tag
	if (parent < 0 || parent >= m_componentcount || child < 0 || child >= m_componentcount) return -1; 

	// Now attempt to locate this constraint in the attachment array
	int n = (m_attachcount * 2);
	for (int i = 0; i < n; i += 2)
	{
		if (m_attachment_indices[i] == parent && m_attachment_indices[i + 1] == child)
		{
			return (i / 2);			// Will always be a whole number; the index of the relevant attachment
		}
	}

	// We could not locate the constraint; return -1 to signal error
	return -1;
}

// Retrieve the index of the component with the specified tag, or -1 if no such tag exists
int ArticulatedModel::GetComponentWithTag(const std::string & tag)
{
	int component = -1;

	// Attempt to locate this tag, and the parent/child indices associated to it
	std::vector<ModelTag>::const_iterator it_end = Tags.end();
	for (std::vector<ModelTag>::const_iterator it = Tags.begin(); it != it_end; ++it)
	{
		if ((*it).Tag == tag)
		{
			component = (*it).Parent;
			break;
		}
	}

	// Return an error if we have no matching tag, or if index is invalid
	if (component < 0 || component >= m_componentcount) return -1;

	// Return the index of the component with this tag
	return component;
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
		// Link the two relevant objects using this attachment
		result = model->SetAttachment(i, m_attachment_indices[(i*2)], m_attachment_indices[(i*2) + 1]);
		if (result != ErrorCodes::NoError)
		{
			SafeDelete(model);
			return NULL;
		}

		// Copy other attachment data directly; take different action depending on whether we have a constraint or fixed attachment
		Attachment<ArticulatedModelComponent*> *new_attach = model->GetAttachment(i);
		AttachmentConstraint *constraint = m_attachments[i].Constraint;
		if (constraint)
		{
			new_attach->CreateConstraint(constraint->Axis, constraint->ParentPoint, constraint->ChildPoint, constraint->BaseChildOrient);
		}
		else
		{
			new_attach->SetOffset(m_attachments[i].GetPositionOffset(), m_attachments[i].GetOrientationOffset());
		}
	}

	// Copy all model tag data to the new instance
	model->Tags = Tags;

	// Recalculate model-related data, such as the consolidated model extents
	model->PerformPostLoadInitialisation();

	// Return a pointer to the new model
	return model;
}



