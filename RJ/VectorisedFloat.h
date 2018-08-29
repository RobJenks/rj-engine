#pragma once

#include "DX11_Core.h"
#include "Utility.h"


struct VectorisedFloat
{
private:

	XMVECTOR			_V;
	float				_F;

public:

	// Accessor
	CMPINLINE float F(void) const { return _F; }
	CMPINLINE XMVECTOR V(void) const { return _V; }

	// Assignment
	CMPINLINE VectorisedFloat & operator=(float f)
	{
		_F = f;
		_V = XMVectorReplicate(f);
		return *this;
	}

	// Assignment
	CMPINLINE VectorisedFloat & operator=(const FXMVECTOR v)
	{
		_V = v;
		_F = XMVectorGetX(v);
		return *this;
	}


	// Constructor
	CMPINLINE VectorisedFloat(void) { }

	// Constructor
	CMPINLINE VectorisedFloat(float f)
		:
		_F(f),
		_V(XMVectorReplicate(f))
	{
	}

	// Constructor
	CMPINLINE VectorisedFloat(const FXMVECTOR v)
		:
		_V(v),
		_F(XMVectorGetX(v))
	{
	}

	// Copy constructor
	CMPINLINE VectorisedFloat(const VectorisedFloat & other)
		:
		_F(other._F),
		_V(other._V)
	{
	}

	// Copy assignment
	CMPINLINE VectorisedFloat & operator=(const VectorisedFloat & other)
	{
		_F = other._F;
		_V = other._V;
		return *this;
	}

	// Move constructor
	CMPINLINE VectorisedFloat(VectorisedFloat && other)
		:
		_F(other._F),
		_V(std::move(other._V))
	{
	}

	// Move assignment
	CMPINLINE VectorisedFloat & operator=(VectorisedFloat && other)
	{
		_F = other._F;
		_V = std::move(other._V);
		return *this;
	}

	// Destructor
	CMPINLINE ~VectorisedFloat(void) { }

};