#pragma once

#ifndef __BoundingObjectH__
#define __BoundingObjectH__

#include <string>
#include "DX11_Core.h"

#include "CompilerSettings.h"

// Class does NOT require special alignment declaration
class BoundingObject
{
public:
	enum Type { None, Point, Cube, Sphere, Cuboid };

	BoundingObject(void);
	~BoundingObject(void);

	// Create either point, cube or sphere bounding objects
	void CreatePointBound(void);
	void CreateCubeBound(float radius);
	void CreateSphereBound(float radius);
	
	// Creates a cuboid bounding object
	void CreateCuboidBound(float xsize, float ysize, float zsize);
	CMPINLINE void CreateCuboidBound(const XMFLOAT3 & size) { CreateCuboidBound(size.x, size.y, size.z); }

	// Accessor methods; get the relevant properties of each bounding object
	CMPINLINE BoundingObject::Type GetType(void) { return m_type; }
	CMPINLINE float GetCubeRadius(void) { return m_f[0]; }
	CMPINLINE float GetSphereRadius(void) { return m_f[0]; }
	CMPINLINE XMFLOAT3 GetCuboidSize(void) { return XMFLOAT3(m_f[0], m_f[1], m_f[2]); }
	CMPINLINE float GetCuboidXSize(void) { return m_f[0]; }
	CMPINLINE float GetCuboidYSize(void) { return m_f[1]; }
	CMPINLINE float GetCuboidZSize(void) { return m_f[2]; }

	// Also allow direct access to parameters, out of the context of any particular class of bounding object
	CMPINLINE void SetTypeDirect(BoundingObject::Type type) { m_type = type; }
	CMPINLINE float GetParameterDirect(int index) { return m_f[index]; }
	CMPINLINE void SetParameterDirect(int index, float value) { m_f[index] = value; }

	// Static method to make an exact copy of a bounding volume
	static BoundingObject *Copy(BoundingObject *src);

	// Translates between bounding object type and the string representation of that type
	static std::string TranslateTypeToString(BoundingObject::Type type);
	static BoundingObject::Type TranslateStringToType(const std::string & type);

private:
	BoundingObject::Type m_type;
	float m_f[3];					// We only need max three floats to represent all options
};


#endif