#pragma once

#ifndef __OrientedBoundingBoxH__
#define __OrientedBoundingBoxH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "FastMath.h"
class iObject;

/* 
	In this implementation, OBB is represented by
	- the centroid (vector3)
	- the three orthonormal, unit vectors forming its coordinate basis
	- the extent of the box in each of its three dimensions

	The 8 vertices can be determined from these values when required
*/


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class OrientedBoundingBox : public ALIGN16<OrientedBoundingBox>
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
	// Class is 16-bit aligned to allow use of SIMD member variables
	__declspec(align(16))
	struct CoreOBBData
	{
		AXMVECTOR				Centre;				// Centre point of the OBB
		AXMVECTOR_P				Axis[3];			// The orthonormal unit vectors defining this OBB's coordinate basis
		AXMVECTOR				ExtentV;			// The half-extent of this OBB in each of its three dimensions (i.e. from centre to edge), 
		AXMVECTOR_P				Extent[3];			// The half-extent of this OBB in each of its three dimensions (i.e. from centre to edge), 
													// replicated in each vector. i.e. Extent[0] = { HalfExtent.x, HalfExtent.x, HalfExtent.x, HalfExtent.x }
		XMFLOAT3				ExtentF;			// Float representation of object extents, for easier access to opaque XMVECTOR Extent[3]

		// Updates the extent data and recalculates based on the new values
		CMPINLINE void			UpdateExtent(const FXMVECTOR extent)
		{
			ExtentV = extent;
			XMStoreFloat3(&ExtentF, extent);

			Extent[0].value = XMVectorReplicate(ExtentF.x);
			Extent[1].value = XMVectorReplicate(ExtentF.y);
			Extent[2].value = XMVectorReplicate(ExtentF.z);
		}

		// Updates the extent data and recalculates based on the new values
		CMPINLINE void			UpdateExtent(const XMFLOAT3 & extent)
		{
			ExtentF = extent;
			ExtentV = XMLoadFloat3(&extent);

			Extent[0].value = XMVectorReplicate(ExtentF.x);
			Extent[1].value = XMVectorReplicate(ExtentF.y);
			Extent[2].value = XMVectorReplicate(ExtentF.z);
		}

		// Updates the extent data by copying from another source data structure
		CMPINLINE void			UpdateExtent(const CoreOBBData & source)
		{
			ExtentV = source.ExtentV;
			ExtentF = source.ExtentF;
			Extent[0].value = source.Extent[0].value;
			Extent[1].value = source.Extent[1].value;
			Extent[2].value = source.Extent[2].value;
		}

		// Updates the extent (centre-to-bounds distance) of this bounding volume from a size (total -ve to +ve bounds size)
		CMPINLINE void UpdateExtentFromSize(const FXMVECTOR size)
		{
			UpdateExtent(XMVectorMultiply(size, HALF_VECTOR));
		}

		// Method to determine the vertices of this bounding box in world space
		void					DetermineVertices(AXMVECTOR_P(&pOutVertices)[8]) const;

		// Calculates the outer extents of this object in world space based on vertex positions.  Returns minimum
		// and maximum bounds via reference parameters
		void					DetermineWorldSpaceBounds(XMVECTOR & outMinBounds, XMVECTOR & outMaxBounds) const;

		// Intermediate calculations; basis axes * extent along each of those axes, and the corresponding
		// negation.  Permanent static allocation to reduce runtime allocations
		static AXMVECTOR_P		ExtentAlongAxis[3];
		static AXMVECTOR_P		NegAxisExtent[3];
	};

	const iObject *				Parent;				// The parent object that this OBB relates to 
	
	AXMMATRIX					Offset;				// The offset of this OBB from its parent object, if applicable.  Offsets are always relative to the object 
													// position, rather than that of any OBB above it in the hierarchy
	int							Index;				// Numeric index used to identify the OBB in some scenarios

	OrientedBoundingBox *		Children;			// Array of child OBBs below this one, for hierarchical collision detection
	int							ChildCount;			// The number of children below this OBB

	short						Flags;				// Bitwise flags to store certain properties of the OBB
													// 0 = HasOffset, i.e. whether the pos/orient are absolute, or relative to the parent 
													// 1 = AutoFitObjectBounds, i.e. whether the OBB wil dynamically size itself based on the underlying model size

	// Primary object fields are all contained within the core OBB data structure
	// Will update the OBB if it has become invalidated before returning the data
	CMPINLINE CoreOBBData &	Data(void)
	{
		if (IsInvalidated()) UpdateFromParent();
		return _Data;
	}

	// Primary object fields are all contained within the core OBB data structure
	// Simply returns the OBB data as-is; will NOT update the OBB based on invalidation
	CMPINLINE const CoreOBBData &	ConstData(void) const
	{
		return _Data;
	}

	// Updates the OBB data based on its parent object, if the OBB has become invalidated
	CMPINLINE void				UpdateIfRequired(void)			{ if (IsInvalidated()) UpdateFromParent(); }

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
	

	// Constructors to create a new OBB object
	OrientedBoundingBox(void);
	OrientedBoundingBox(iObject *parent);
	OrientedBoundingBox(iObject *parent, const FXMVECTOR size);
	OrientedBoundingBox(iObject *parent, const FXMVECTOR centre, const FXMVECTOR size);

	// Inline flags to return properties of the OBB
	CMPINLINE bool				HasChildren(void) const					{ return (ChildCount != 0); }

	// Allocates space for new child OBBs below this one
	void						AllocateChildren(int children);

	// Deallocates space allocated to child nodes below this OBB
	void						DeallocateChildren(void);

	// Updates the extent (centre-to-bounds distance) of this bounding volume from the given extent values
	CMPINLINE void				UpdateExtent(const FXMVECTOR extent)
	{
		_Data.UpdateExtent(extent);
		RecalculateData();
	}

	// Updates the extent (centre-to-bounds distance) of this bounding volume from a size (total -ve to +ve bounds size)
	CMPINLINE void				UpdateExtentFromSize(const FXMVECTOR size)
	{
		_Data.UpdateExtentFromSize(size);
		RecalculateData();
	}

	// Updates the matrix to offset this OBB from its parent.  Accepts an offset matrix as its parameter
	void RJ_XM_CALLCONV			UpdateOffset(const FXMMATRIX offset)
	{
		Offset = offset;
		SetOffsetFlag(true);
		RecalculateData();
	}

	// Updates the matrix to offset this OBB from its parent.  Accepts a position offset for a non-rotated offset
	void RJ_XM_CALLCONV			UpdateOffset(const FXMVECTOR offset)
	{
		UpdateOffset(XMMatrixTranslationFromVector(offset));
	}

	// Removes the offset applied to this child OBB
	CMPINLINE void				RemoveOffset(void)
	{
		SetOffsetFlag(false);
		RecalculateData();
	}

	// Update the OBB position and basis vectors based upon a world matrix
	void						UpdateFromObject(const iObject & object);

	// Update the OBB position and basis vectors based upon its parent object
	CMPINLINE void				UpdateFromParent(void) { if (Parent) UpdateFromObject(*Parent); }

	// Method to update this bounding volume based upon its underlying data
	CMPINLINE void				RecalculateData(void)
	{
		// Perform any required intermediate calculations
	}

	// Numeric index used to identify the OBB in some scenarios
	CMPINLINE int				GetIndex(void) const			{ return Index; }
	CMPINLINE void				SetIndex(int index)				{ Index = index; }

	// Method to determine the vertices of this bounding box in world space
	CMPINLINE void				DetermineVertices(AXMVECTOR_P(&pOutVertices)[8]) const
	{
		_Data.DetermineVertices(pOutVertices);
	}

	// Calculates the outer extents of this object in world space based on vertex positions.  Returns minimum
	// and maximum bounds via reference parameters
	CMPINLINE void				DetermineWorldSpaceBounds(XMVECTOR & outMinBounds, XMVECTOR & outMaxBounds) const
	{
		_Data.DetermineWorldSpaceBounds(outMinBounds, outMaxBounds);
	}

	// Generates a world matrix that will transform to the position & orientation of this OBB
	void						GenerateWorldMatrix(XMMATRIX & outMatrix) const;

	// Clear all data for this OBB, and deallocate any children in the tree below us
	void						Clear(void);

	// Reallocates child data, including one additional node at the end of the array.  Requires deallocation/reallocation
	// so this is a relatively costly operation.  Child data should generally be static after loading for this reason.
	void						AppendNewChildNode(void);

	// Removes a child node below this OBB.  Takes care of maintaining references to existing nodes and shrinking the child storage
	void						RemoveChildNode(int index);

	// Returns a string representation of the OBB node
	std::string					ToString(void) const;

	// Performs a deep copy of an OBB hierarchy into the specified destination
	static void					CloneOBBHierarchy(const OrientedBoundingBox & source, OrientedBoundingBox &dest, iObject *new_parent);

	// Default destructor; deallocates memory assigned to any child OBBs below this one
	~OrientedBoundingBox(void);

protected:

	// Primary object fields are all contained within the core OBB data structure
	CoreOBBData					_Data;
};



#endif