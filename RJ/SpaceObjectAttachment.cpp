#include "iSpaceObject.h"
#include "FastMath.h"

#include "SpaceObjectAttachment.h"

// Applies the effect of this attachment to the child object, recursively if necessary
void SpaceObjectAttachment::ApplyAttachment(void)
{
	D3DXMATRIX world;
	D3DXVECTOR3 pos, nullvec;
	D3DXQUATERNION orient;

	// Multiply the child offset matrix by it's parent's world matrix, yielding the child world matrix.  Repeat ad infinitum
	D3DXMatrixMultiply(&world, &m_mat_offset, Parent->GetWorldMatrix());
//	D3DXMatrixMultiply(&world, Parent->GetWorldMatrix(), &m_mat_offset);
	Child->SetWorldMatrix(world);

	// Decompose to yield the world position and orientation of the child object
	D3DXMatrixDecompose(&nullvec, &orient, &pos, &world);
	Child->SetPosition(pos);
	Child->SetOrientation(&orient);
}

// Precalculates offset matrices in respose to a change in offset pos/orient, to save rendering time
void SpaceObjectAttachment::RecalculateOffsetParameters(void)
{
	// Recalculate component matrices from the attachment offsets
	D3DXMatrixTranslation(&m_mat_posoffset, m_posoffset.x, m_posoffset.y, m_posoffset.z);
	D3DXMatrixRotationQuaternion(&m_mat_orientoffset, &m_orientoffset);

	// Also calculate the combined offset matrix
	D3DXMatrixMultiply(&m_mat_offset, &m_mat_orientoffset, &m_mat_posoffset);
}

// Sets position offset and recalculates parameters accordingly
void SpaceObjectAttachment::SetPositionOffset(D3DXVECTOR3 off)
{
	m_posoffset = off;
	RecalculateOffsetParameters();
}

// Sets orientation offset and recalculates parameters accordingly
void SpaceObjectAttachment::SetOrientationOffset(D3DXQUATERNION off)
{
	m_orientoffset = off;
	RecalculateOffsetParameters();
}

// Sets position and orientation offsets and recalculates parameters accordingly
void SpaceObjectAttachment::SetOffset(D3DXVECTOR3 poff, D3DXQUATERNION qoff)
{
	m_posoffset = poff;
	m_orientoffset = qoff;
	RecalculateOffsetParameters();
}

Result SpaceObjectAttachment::Create(iSpaceObject* parent, iSpaceObject* child)
{
	// Call the overridden method with no offset specified
	return SpaceObjectAttachment::Create(parent, child, NULL_VECTOR, ID_QUATERNION);
}

Result SpaceObjectAttachment::Create(iSpaceObject* parent, iSpaceObject* child, const D3DXVECTOR3 poff, const D3DXQUATERNION qoff)
{
	// Make sure the child object is not already attached; this also covers the case where parent & child are already attached
	if (child->ParentAttachment()) return ErrorCodes::ChildIsAlreadyAttachedToAnObject;

	// Otherwise create a new attachment object
	SpaceObjectAttachment *attach = new SpaceObjectAttachment(parent, child, poff, qoff);

	// Assign this attachment to both the parent and the child
	parent->ChildAttachments()->push_back(attach);
	child->SetParentAttachment(attach);
	
	// Return success
	return ErrorCodes::NoError;
}

int SpaceObjectAttachment::FindInList(vector<SpaceObjectAttachment*> *list, SpaceObjectAttachment *attach)
{
	// Iterate through the list to locate this attachment
	int n = list->size();
	for (int i=0; i<n; i++)
	{
		// If this is the item then return its index
		if (list->at(i) == attach) return i;
	}

	// If no success then return -1
	return -1;
}

// Break the attachment that exists between two objects, if one exists
Result SpaceObjectAttachment::Detach(iSpaceObject *parent, iSpaceObject *child)
{
	// Make sure the objects passed in are valid
	if (!parent || !child) return ErrorCodes::CannotBreakAttachmentToNullObject;

	// Attempt to locate the attachment object by looking up the child 'parent' pointer (since there is only one to consider)
	if (!(child->ParentAttachment())) return ErrorCodes::ChildDoesNotHaveAttachmentToBreak;
	SpaceObjectAttachment *attach = child->ParentAttachment();

	// Validate that the attachment does connect these two objects
	if (attach->Parent != parent) return ErrorCodes::CannotBreakAttachmentParentMismatch;
	if (attach->Child != child) return ErrorCodes::CannotBreakAttachmentChildMismatch;

	// Invoke the detach method on this attachment and return the result
	return attach->Detach();
}

// Break this attachment, updating both the parent and child
Result SpaceObjectAttachment::Detach(void)
{	
	// Remove this attachment from the parent
	int i = SpaceObjectAttachment::FindInList(this->Parent->ChildAttachments(), this);
	if (i == -1) return ErrorCodes::CannotRemoveAttachmentThatDoesNotExist; 
	
	// Swap and pop to remove this item from the vector
	std::swap(this->Parent->ChildAttachments()->at(i), this->Parent->ChildAttachments()->at(this->Parent->ChildAttachments()->size()-1));
	this->Parent->ChildAttachments()->pop_back();

	// Remove this attachment from the child
	this->Child->SetParentAttachment(NULL);

	// Deallocate the memory used for this attachment and return success
	delete this;
	return ErrorCodes::NoError;
}



SpaceObjectAttachment::SpaceObjectAttachment(void)
{
	Parent = NULL; 
	Child = NULL;
	m_posoffset = D3DXVECTOR3(NULL_VECTOR);
	m_orientoffset = D3DXQUATERNION(ID_QUATERNION);

	RecalculateOffsetParameters();
}

SpaceObjectAttachment::SpaceObjectAttachment(iSpaceObject *parent, iSpaceObject *child)
{
	Parent = parent; 
	Child = child;
	m_posoffset = D3DXVECTOR3(NULL_VECTOR);
	m_orientoffset = D3DXQUATERNION(ID_QUATERNION);

	RecalculateOffsetParameters();
}

SpaceObjectAttachment::SpaceObjectAttachment(iSpaceObject *parent, iSpaceObject *child, const D3DXVECTOR3 posoffset, const D3DXQUATERNION orientoffset)
{
	Parent = parent; 
	Child = child;
	m_posoffset = D3DXVECTOR3(posoffset);
	m_orientoffset = D3DXQUATERNION(orientoffset);

	RecalculateOffsetParameters();
}

SpaceObjectAttachment::~SpaceObjectAttachment(void) { }
