#pragma once

#ifndef __OrientedBoundingBoxH__
#define __OrientedBoundingBoxH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
class iObject;

/* 
	In this implementation, OBB is represented by
	- the centroid (vector3)
	- the three orthonormal, unit vectors forming its coordinate basis
	- the extent of the box in each of its three dimensions

	The 8 vertices can be determined from these values when required
*/

class OrientedBoundingBox
{

public:

	// Enumeration of bit-flags used in the OBB implementation
	enum OBBFlags
	{
		OBBHasOffset				= (1 << 0),
		OBBInvalidated				= (1 << 1),
		OBBAutoFitToObjectBounds	= (1 << 2)
	};

	// Struct containing the core OBB data, which can be reused in 'lighter' OBB implementations if required
	struct CoreOBBData
	{
		D3DXVECTOR3				Centre;				// Centre point of the OBB
		D3DXVECTOR3				Axis[3];			// The orthonormal unit vectors defining this OBB's coordinate basis
		D3DXVECTOR3				Extent;				// The half-extent of this OBB in each of its three dimensions (i.e. from centre to edge)
	};

	
	CoreOBBData					Data;				// Primary object fields are all contained within the core OBB data structure

	iObject *					Parent;				// The parent object that this OBB relates to 
	
	D3DXMATRIX					Offset;				// The offset of this OBB from its parent object, if applicable.  Offsets are always relative to the object 
													// position, rather than that of any OBB above it in the hierarchy

	OrientedBoundingBox *		Children;			// Array of child OBBs below this one, for hierarchical collision detection
	int							ChildCount;			// The number of children below this OBB

	short						Flags;				// Bitwise flags to store certain properties of the OBB
													// 0 = HasOffset, i.e. whether the pos/orient are absolute, or relative to the parent 
													// 1 = AutoFitObjectBounds, i.e. whether the OBB wil dynamically size itself based on the underlying model size

	// Flag to indicate whether an offset is being applied to this OBB
	CMPINLINE bool				HasOffset(void) const			{ return CheckBit_Single(Flags, OrientedBoundingBox::OBBFlags::OBBHasOffset); }
	CMPINLINE void				SetOffsetFlag(bool hasoffset)
	{
		if (hasoffset)			SetBit(Flags, OrientedBoundingBox::OBBFlags::OBBHasOffset);
		else					ClearBit(Flags, OrientedBoundingBox::OBBFlags::OBBHasOffset);
	}

	// Flag that indicates whether the OBB should be auto-calculated to fit object size
	CMPINLINE bool				AutoFitObjectBounds(void) const { return CheckBit_Single(Flags, OrientedBoundingBox::OBBFlags::OBBAutoFitToObjectBounds); }
	void						SetAutoFitMode(bool autofit);
	
	// Flag that indicates whether the OBB has been invalidated and needs to be recalculated next time it is used
	CMPINLINE bool				IsInvalidated(void) const { return CheckBit_Single(Flags, OrientedBoundingBox::OBBFlags::OBBInvalidated); }
	CMPINLINE void				Invalidate(void) { SetBit(Flags, OrientedBoundingBox::OBBFlags::OBBInvalidated); }
	CMPINLINE void				RemoveInvalidation(void) { ClearBit(Flags, OrientedBoundingBox::OBBFlags::OBBInvalidated); }
	
	// Intermediate calculations; basis axes * extent along each of those axes
	D3DXVECTOR3					ExtentAlongAxis[3];


	// Constructors to create a new OBB object
	OrientedBoundingBox(void);
	OrientedBoundingBox(iObject *parent);
	OrientedBoundingBox(iObject *parent, const D3DXVECTOR3 & size);
	OrientedBoundingBox(iObject *parent, const D3DXVECTOR3 & centre, const D3DXVECTOR3 & size);

	// Inline flags to return properties of the OBB
	CMPINLINE bool				IsRoot(void) const						{ return (Parent != NULL); }
	CMPINLINE bool				HasChildren(void) const					{ return (ChildCount != 0); }

	// Allocates space for new child OBBs below this one
	void						AllocateChildren(int children);

	// Deallocates space allocated to child nodes below this OBB
	void						DeallocateChildren(void);

	// Updates the extent (centre-to-bounds distance) of this bounding volume from the given extent values
	CMPINLINE void				UpdateExtent(const D3DXVECTOR3 & extent)
	{
		Data.Extent = extent;
		RecalculateData();
	}

	// Updates the extent (centre-to-bounds distance) of this bounding volume from a size (total -ve to +ve bounds size)
	CMPINLINE void				UpdateExtentFromSize(const D3DXVECTOR3 & size)
	{
		Data.Extent = (size * 0.5f);
		RecalculateData();
	}

	// Updates the matrix to offset this OBB from its parent
	CMPINLINE void				UpdateOffset(const D3DXMATRIX & offset)
	{
		Offset = offset;
		SetOffsetFlag(true);
		RecalculateData();
	}

	// Removes the offset applied to this child OBB
	CMPINLINE void				RemoveOffset(void)
	{
		SetOffsetFlag(false);
		RecalculateData();
	}

	// Update the OBB position and basis vectors based upon a world matrix
	void						UpdateFromObject(iObject & object);

	// Method to update this bounding volume based upon its underlying data
	CMPINLINE void				RecalculateData(void)
	{
		// Store intermediate calculations of basis axes * extent along those axes
		ExtentAlongAxis[0] = Data.Extent[0] * Data.Axis[0];
		ExtentAlongAxis[1] = Data.Extent[1] * Data.Axis[1];
		ExtentAlongAxis[2] = Data.Extent[2] * Data.Axis[2];
	}

	// Method to determine the vertices of this bounding box in world space
	void						DetermineVertices(D3DXVECTOR3 (&pOutVertices)[8]) const;

	// Generates a world matrix that will transform to the position & orientation of this OBB
	void						GenerateWorldMatrix(D3DXMATRIX *pOutMatrix) const;

	// Clear all data for this OBB, and deallocate any children in the tree below us
	void						Clear(void);

	// Reallocates child data, including one additional node at the end of the array.  Requires deallocation/reallocation
	// so this is a relatively costly operation.  Child data should generally be static after loading for this reason.
	void						AppendNewChildNode(void);

	// Removes a child node below this OBB.  Takes care of maintaining references to existing nodes and shrinking the child storage
	void						RemoveChildNode(int index);

	// Performs a deep copy of an OBB hierarchy into the specified destination
	static void					CloneOBBHierarchy(const OrientedBoundingBox & source, OrientedBoundingBox &dest);

	// Default destructor; deallocates memory assigned to any child OBBs below this one
	~OrientedBoundingBox(void);


};



#endif