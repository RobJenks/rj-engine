#pragma once

#ifndef __AABBH__
#define __AABBH__

#include "CompilerSettings.h"
#include "DX11_Core.h"
#include "OrientedBoundingBox.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class AABB : public ALIGN16<AABB>
{
public:
	
	// AABB can be defined using only two points; P0 = min point, P1 = max point
	AXMVECTOR							P0;
	AXMVECTOR							P1;

	// Convenience methods to get/set the min and max AABB points
	CMPINLINE const XMVECTOR &			MinPoint(void) const			{ return P0; }
	CMPINLINE const XMVECTOR &			MaxPoint(void) const			{ return P1; }
	CMPINLINE void						SetMin(const FXMVECTOR v)		{ P0 = v; }
	CMPINLINE void						SetMax(const FXMVECTOR v)		{ P1 = v; }

	// Default constructor
	CMPINLINE AABB(void) noexcept { }

	// Constructor that builds an AABB based upon its minimum and maximum points
	CMPINLINE AABB(const FXMVECTOR vmin, const FXMVECTOR vmax) noexcept
		:  P0(vmin), P1(vmax)
	{
	}

	// Constructor to build an AABB from an OBB.  Generates an AABB centred about (0,0,0) in the LOCAL
	// OBJECT SPACE.  This does NOT create an AABB oriented in world space.
	CMPINLINE AABB(const OrientedBoundingBox & obb) noexcept
		:
		P1(obb.ConstData().ExtentV),
		P0(XMVectorNegate(obb.ConstData().ExtentV))
	{
	}

	// Constructor to build an AABB from OBB core data.  Generates an AABB centred about (0,0,0) in the LOCAL
	// OBJECT SPACE.  This does NOT create an AABB oriented in world space.
	CMPINLINE AABB(const OrientedBoundingBox::CoreOBBData & obb) noexcept
		:
		P1(obb.ExtentV),
		P0(XMVectorNegate(obb.ExtentV))
	{
	}

	// Constructor to build an AABB from OBB core data.  Accepts a parameter used to expand/contract the AABB 
	// bounds (centre-to-edge, i.e. half of overall size) by a specified amount on construction
	CMPINLINE AABB(const OrientedBoundingBox::CoreOBBData & obb, const FXMVECTOR bounds_adjustment) noexcept
		:
		P1(XMVectorAdd(obb.ExtentV, bounds_adjustment))
	{
		P0 = XMVectorNegate(P1);
	}

	// Constructor to build an AABB from a set of vertices.  Generates an AABB that encloses all vertices
	CMPINLINE AABB(const std::vector<XMFLOAT3> & vertices)
	{
		if (vertices.empty()) { P0 = P1 = NULL_VECTOR; return; }

		P0 = P1 = XMLoadFloat3(&(vertices[0]));
		size_t n = vertices.size();
		for (size_t i = 1U; i < n; ++i)
		{
			XMVECTOR v = XMLoadFloat3(&(vertices[i]));
			P0 = XMVectorMin(P0, v);
			P1 = XMVectorMax(P1, v);
		}
	}

	// Transform the AABB by the given transformation matrix
	CMPINLINE void Transform(const FXMMATRIX transform)
	{
		P0 = XMVector3TransformCoord(P0, transform);
		P1 = XMVector3TransformCoord(P1, transform);
	}

	// Copy constructor
	CMPINLINE AABB(const AABB & other) noexcept
		:
		P0(other.P0), P1(other.P1)
	{
	}

	// Copy assignment
	CMPINLINE AABB & operator=(const AABB & other) noexcept
	{
		P0 = other.P0; 
		P1 = other.P1;
		return *this;
	}

	// Move constructor
	CMPINLINE AABB(AABB && other) noexcept
		:
		P0(other.P0), P1(other.P1)
	{
	}

	// Move assignment
	CMPINLINE AABB & operator=(AABB && other) noexcept
	{
		P0 = other.P0;
		P1 = other.P1;
		return *this;
	}

	// Destructor
	CMPINLINE ~AABB(void) noexcept { }


	// String representation of the AABB
	CMPINLINE std::string str(void) const
	{
		return concat("AABB { min=")(Vector3ToString(P0))(", max=")(Vector3ToString(P1))(" }").str();
	}
};



#endif