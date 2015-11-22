#include "DX11_Core.h"

#include "FastMath.h"
#include "iObject.H"

#include "OrientedBoundingBox.h"

// Default constructor, where no parameters are provided
OrientedBoundingBox::OrientedBoundingBox(void) :
Parent(NULL), Offset(ID_MATRIX), Children(NULL), ChildCount(0), Flags(0)
{
	Data.Centre = NULL_VECTOR;
	Data.ExtentF = NULL_FLOAT3;
	Data.ExtentV = Data.Extent[0].value = Data.Extent[1].value = Data.Extent[2].value = NULL_VECTOR;
	Data.Axis[0].value = UNIT_BASES[0]; Data.Axis[1].value = UNIT_BASES[1]; Data.Axis[2].value = UNIT_BASES[2];

	RecalculateData();
}

// Default constructor, only parent pointer provided
OrientedBoundingBox::OrientedBoundingBox(iObject *parent) : 
	Parent(parent), Offset(ID_MATRIX), Children(NULL), ChildCount(0), Flags(0)
{
	Data.Centre = NULL_VECTOR;
	Data.ExtentF = NULL_FLOAT3;
	Data.ExtentV = Data.Extent[0].value = Data.Extent[1].value = Data.Extent[2].value = NULL_VECTOR;
	Data.Axis[0].value = UNIT_BASES[0]; Data.Axis[1].value = UNIT_BASES[1]; Data.Axis[2].value = UNIT_BASES[2];

	RecalculateData();
}

// Constructor to build an OBB based on position & size.  Assumes axis aligment.
OrientedBoundingBox::OrientedBoundingBox(iObject *parent, const FXMVECTOR centre, const FXMVECTOR size) :
	Parent(parent), Offset(ID_MATRIX), Children(NULL), ChildCount(0), Flags(0)
{
	Data.Centre = centre;
	Data.Axis[0].value = UNIT_BASES[0]; Data.Axis[1].value = UNIT_BASES[1]; Data.Axis[2].value = UNIT_BASES[2];
	UpdateExtentFromSize(size);

	RecalculateData();
}

// Constructor to build an OBB based on size only.  Assumes centred at the origin and axis-aligned.
OrientedBoundingBox::OrientedBoundingBox(iObject *parent, const FXMVECTOR size) :
	Parent(parent), Offset(ID_MATRIX), Children(NULL), ChildCount(0), Flags(0)
{
	Data.Centre = NULL_VECTOR;
	Data.Axis[0].value = UNIT_BASES[0]; Data.Axis[1].value = UNIT_BASES[1]; Data.Axis[2].value = UNIT_BASES[2];
	UpdateExtentFromSize(size);

	RecalculateData();
}


// Method to determine the vertices of this bounding box in world space.  [0-3] represent one end face in ccw order, [4-7] represent
// the second end face in ccw order.  v[x] and v[x+4] are opposite vertices in the two end faces
void OrientedBoundingBox::DetermineVertices(AXMVECTOR_P(&pOutVertices)[8]) const
{
	// We also need the negated versions of each axis extent
	NegAxisExtent[0].value = XMVectorNegate(ExtentAlongAxis[0].value);
	NegAxisExtent[1].value = XMVectorNegate(ExtentAlongAxis[1].value);
	NegAxisExtent[2].value = XMVectorNegate(ExtentAlongAxis[2].value);

	// Now use these values to determine the location of each bounding volume vertex in local space
	pOutVertices[0].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(Data.Centre, NegAxisExtent[0].value), NegAxisExtent[1].value), NegAxisExtent[2].value);		  // -x, -y, -z	(End face #1)
	pOutVertices[1].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(Data.Centre, ExtentAlongAxis[0].value), NegAxisExtent[1].value), NegAxisExtent[2].value);	  // +x, -y, -z	(End face #1)
	pOutVertices[2].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(Data.Centre, ExtentAlongAxis[0].value), ExtentAlongAxis[1].value), NegAxisExtent[2].value);   // +x, +y, -z	(End face #1)
	pOutVertices[3].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(Data.Centre, NegAxisExtent[0].value), ExtentAlongAxis[1].value), NegAxisExtent[2].value);  	  // -x, +y, -z	(End face #1)

	pOutVertices[4].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(Data.Centre, NegAxisExtent[0].value), NegAxisExtent[1].value), ExtentAlongAxis[2].value);	  // -x, -y, +z	(End face #2)
	pOutVertices[5].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(Data.Centre, ExtentAlongAxis[0].value), NegAxisExtent[1].value), ExtentAlongAxis[2].value);   // +x, -y, +z	(End face #2)
	pOutVertices[6].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(Data.Centre, ExtentAlongAxis[0].value), ExtentAlongAxis[1].value), ExtentAlongAxis[2].value); // +x, +y, +z	(End face #2)
	pOutVertices[7].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(Data.Centre, NegAxisExtent[0].value), ExtentAlongAxis[1].value), ExtentAlongAxis[2].value);	  // -x, +y, +z	(End face #2)
}

// Updates the extent (centre-to-bounds distance) of this bounding volume from a size (total -ve to +ve bounds size)
void OrientedBoundingBox::UpdateExtentFromSize(const FXMVECTOR size)
{
	UpdateExtent(XMVectorMultiply(size, HALF_VECTOR));
}

// Allocates space for new child OBBs below this one
void OrientedBoundingBox::AllocateChildren(int children)
{
	// Deallocate any existing space first, in the unlikely event something has been allocated already
	if (Children) DeallocateChildren();

	// Make sure the desired child count is valid
	if (children <= 0) return;

	// Allocate new space and initialise it to NULL
	Children = (OrientedBoundingBox*)malloc(sizeof(OrientedBoundingBox) * children);
	memset(Children, 0, sizeof(OrientedBoundingBox) * children);

	// Store the new child count
	ChildCount = children;
}

// Update the OBB position and basis vectors based upon a world matrix
void OrientedBoundingBox::UpdateFromObject(iObject & object)
{
	// Get a reference to the object world matrix
	const XMMATRIX & world = object.GetWorldMatrix();

	// Test whether this OBB incorporates an offset 
	if (!HasOffset())
	{
		// If we have no offset stored for this OBB then take directly from the world matrix components
		Data.Axis[0].value = XMVector3NormalizeEst(world.r[0]);
		Data.Axis[1].value = XMVector3NormalizeEst(world.r[1]);
		Data.Axis[2].value = XMVector3NormalizeEst(world.r[2]);
		Data.Centre = object.GetPosition();
	}
	else
	{
		// Generate a translation to the object centre point, to ensure OBBs are rotated correctly about their centre
		XMMATRIX centretrans = XMMatrixTranslationFromVector(XMVectorNegate(object.GetCentreOffsetTranslation()));
		
		// Apply the offset to this world matrix before deriving values
		XMMATRIX offsetworld = XMMatrixMultiply(XMMatrixMultiply(Offset, centretrans), world);
	
		// Basis vectors can be extracted directly from the new world matrix
		Data.Axis[0].value = XMVector3NormalizeEst(offsetworld.r[0]);
		Data.Axis[1].value = XMVector3NormalizeEst(offsetworld.r[1]);
		Data.Axis[2].value = XMVector3NormalizeEst(offsetworld.r[2]);

		// Transform the object centre offset by this new matrix, since this will negate the effect of any centre translation for
		// the object.  Using the world matrix to translate (0, 0, 0), for an object with position (0, 0, 10), would otherwise also 
		// incorporate the object centre translation and could lead to a position of e.g. (-1.5, -1.0, -7.5) if the object size if (3, 2, 5)
		// NOT TRUE ANY MORE: Now apply the centre offset earlier in the method, and simply transform the null vector at this point
		Data.Centre = XMVector3TransformCoord(NULL_VECTOR, offsetworld);
	}
	
	// Perform intermediate/cached calculations based on these new value
	RecalculateData();

	// The OBB is no longer invalidated
	RemoveInvalidation();

	// Now move recursively down to any children of this node
	for (int i = 0; i < ChildCount; ++i)
		Children[i].UpdateFromObject(object);
}


// Clear all data for this OBB, and deallocate any children in the tree below us
void OrientedBoundingBox::Clear(void)
{
	// Clear all primary fields and then recalculate to clear the derived fields
	Data.Centre = NULL_VECTOR;
	Data.ExtentF = NULL_FLOAT3;
	Data.ExtentV = Data.Extent[0].value = Data.Extent[1].value = Data.Extent[2].value = NULL_VECTOR;
	Data.Axis[0].value = UNIT_BASES[0]; Data.Axis[1].value = UNIT_BASES[1]; Data.Axis[2].value = UNIT_BASES[2];
	Flags = 0; Offset = ID_MATRIX;
	RecalculateData();

	// Deallocate any child data down the hierarchy 
	DeallocateChildren();
}

// Deallocates space allocated to child nodes below this OBB
void OrientedBoundingBox::DeallocateChildren(void)
{
	// Deallocate recursively down the tree before removing the data in this OBB
	if (Children)
	{
		for (int i = 0; i < ChildCount; ++i)
		{
			Children[i].DeallocateChildren();
		}
		free(Children);
		Children = NULL;
	}
	ChildCount = 0;
}

// Generates a world matrix that will transform to the position & orientation of this OBB
void OrientedBoundingBox::GenerateWorldMatrix(XMMATRIX & outMatrix) const
{
	// Build a rotation matrix from the OBB basis vectors; structure has xyz of each vector running down columns, so create
	// using the row-vector constructor and transpose.  We then invert since inverse rotation is required
	//rot = D3DXMATRIX(	Data.Axis[0].x, Data.Axis[1].x, Data.Axis[2].x, 0.0f, 
	//					Data.Axis[0].y, Data.Axis[1].y, Data.Axis[2].y, 0.0f, 
	//					Data.Axis[0].z, Data.Axis[1].z, Data.Axis[2].z, 0.0f, 
	//					0.0f, 0.0f, 0.0f, 1.0f );
	//D3DXMatrixInverse(&rot, 0, &rot);
	static const AXMVECTOR row4 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMMATRIX rot = XMMatrixInverse(NULL, XMMatrixTranspose(XMMATRIX(
		XMVectorSetW(Data.Axis[0].value, 0.0f), 
		XMVectorSetW(Data.Axis[1].value, 0.0f), 
		XMVectorSetW(Data.Axis[2].value, 0.0f), 
		row4)));

	// Compose the resulting world matrix as (rot * translation)
	outMatrix = XMMatrixMultiply(rot, XMMatrixTranslationFromVector(Data.Centre));
}

// Reallocates child data, including one additional node at the end of the array.  Requires deallocation/reallocation
// so this is a relatively costly operation.  Child data should generally be static after loading for this reason.
void OrientedBoundingBox::AppendNewChildNode(void)
{
	// Allocate new storage for (n+1) child nodes
	int newchildcount = ChildCount + 1;
	OrientedBoundingBox *obb = (OrientedBoundingBox*)malloc(sizeof(OrientedBoundingBox) * newchildcount);
	memset(obb, 0, sizeof(OrientedBoundingBox) * newchildcount);

	// Copy the existing OBB data element-by-element
	for (int i = 0; i < ChildCount; ++i)
	{
		obb[i] = Children[i];
	}
	
	// Now create a new OBB in the last element with default parameters
	obb[newchildcount - 1] = OrientedBoundingBox(NULL, NULL_VECTOR, ONE_VECTOR);
	obb[newchildcount - 1].SetOffsetFlag(true);

	// Keep a reference to the previous child data, then set the pointer to this newly-allocated child data
	OrientedBoundingBox *oldchildren = Children;
	Children = obb;
	ChildCount = newchildcount;

	// Deallocate the previous child data, assuming any existed
	if (oldchildren) SafeFree(oldchildren);
}

// Removes a child node below this OBB.  Takes care of maintaining references to existing nodes and reducing the child storage
void OrientedBoundingBox::RemoveChildNode(int index)
{
	// Parameter check
	if (ChildCount < 1 || index < 0 || index >= ChildCount) return;

	// If we have only one node, deallocate all child data and quit
	if (ChildCount == 1) { DeallocateChildren(); return; }

	// Allocate new storage for (n-1) child nodes
	int newchildcount = ChildCount - 1;
	OrientedBoundingBox *obb = (OrientedBoundingBox*)malloc(sizeof(OrientedBoundingBox) * newchildcount);
	memset(obb, 0, sizeof(OrientedBoundingBox) * newchildcount);

	// We want to dispose of the item being removed, as well as all data below it in the hierarchy.  We can't 
	// deallocate at this top level since the entire child data block was malloc-ed at once, and we only want to
	// deallocate one element from it.  Instead, deallocate individual data within that child object and allow it 
	// to propogate recursively at that point.  Add any other heap-allocated properties here as required.
	Children[index].DeallocateChildren();
	
	// Copy all data across, except for the item being removed
	int newindex = 0;
	for (int i = 0; i < ChildCount; ++i)
	{
		// We want to skip the object being removed
		if (i == index) continue;
		
		// Copy all other items directly and increment the index within this array
		obb[newindex] = Children[i];
		++newindex;
	}

	// Now deallocate the existing child data and replace it with this new pointer
	SafeFree(Children);
	Children = obb;

	// Update the child node count
	ChildCount = newchildcount;
}

// Updates the auto-fit mode for this OBB, recalculating the OBB bounds based on object size if auto-fit is enabled
void OrientedBoundingBox::SetAutoFitMode(bool autofit)
{
	if (autofit)
	{
		// Set the auto-fit flag and perform a recalculation
		SetBit(Flags, OrientedBoundingBox::OBBFlags::OBBAutoFitToObjectBounds);
		if (Parent) UpdateExtentFromSize(Parent->GetSize());
	}
	else
	{
		// Clear the auto-fit flag
		ClearBit(Flags, OrientedBoundingBox::OBBFlags::OBBAutoFitToObjectBounds);
	}
}


// Performs a deep copy of an OBB hierarchy into the specified destination
void OrientedBoundingBox::CloneOBBHierarchy(const OrientedBoundingBox & source, OrientedBoundingBox &dest)
{
	// Copy the primary fields by value from the source
	dest.Data.Centre = source.Data.Centre;
	dest.Data.ExtentV = source.Data.ExtentV;
	dest.Data.ExtentF = source.Data.ExtentF;
	dest.Data.Axis[0] = source.Data.Axis[0]; dest.Data.Axis[1] = source.Data.Axis[1]; dest.Data.Axis[2] = source.Data.Axis[2];
	dest.Data.Extent[0] = source.Data.Extent[0]; dest.Data.Extent[1] = source.Data.Extent[1]; dest.Data.Extent[2] = source.Data.Extent[2];
	dest.Parent = source.Parent;
	dest.Offset = source.Offset;
	dest.Flags = source.Flags;
	dest.ChildCount = source.ChildCount;

	// Allocate new memory for child objects and copy them, if any.  We perform this deep copy since the normal copy
	// constructor will only shallow copy the child data and result in aliasing of source & dest child data
	dest.Children = NULL;
	if (dest.ChildCount > 0 && source.Children != NULL)
	{
		// Allocate new memory and zero it
		dest.AllocateChildren(dest.ChildCount);
		memset(dest.Children, 0, sizeof(OrientedBoundingBox*) * dest.ChildCount);
		
		// Recursively clone each child in turn
		for (int i = 0; i < dest.ChildCount; ++i)
		{
			OrientedBoundingBox::CloneOBBHierarchy(source.Children[i], dest.Children[i]);
		}
	}

	// Recalculate the intermediate data based on these primary data fields
	dest.RecalculateData();
}

// Default destructor; don't deallocate data - this should be performed explicitly so that child data isn't inadvertantly
// deallocated during normal operation when a node is deleted
OrientedBoundingBox::~OrientedBoundingBox(void)
{
}


