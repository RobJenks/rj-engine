#include <string>

#include "BoundingObject.h"


BoundingObject::BoundingObject(void)
{
	m_type = BoundingObject::Type::None;
	m_f[0] = m_f[1] = m_f[2] = 0.0f;
}

BoundingObject *BoundingObject::Copy(BoundingObject *src)
{
	// Create a new bounding object
	BoundingObject *obj = new BoundingObject();

	// Copy each property directly
	obj->SetTypeDirect(src->GetType());
	for (int i=0; i<3; i++)				
		obj->SetParameterDirect(i, src->GetParameterDirect(i));

	// Return a pointer to the new object
	return obj;
}

BoundingObject::~BoundingObject(void)
{
}

void BoundingObject::CreatePointBound(void)
{
	// Simply set the type; no floats required
	m_type = BoundingObject::Type::Point;
}

void BoundingObject::CreateCubeBound(float radius)
{
	// Set the type, and parameter 1 as the cube radius
	m_type = BoundingObject::Type::Cube;
	m_f[0] = radius;
}

void BoundingObject::CreateSphereBound(float radius)
{
	// Set the type, and parameter 1 as the sphere radius
	m_type = BoundingObject::Type::Sphere;
	m_f[0] = radius;
}
	
void BoundingObject::CreateCuboidBound(float xsize, float ysize, float zsize)
{
	// Set the type, and parameters 1-3 as the size in each dimension
	m_type = BoundingObject::Type::Cuboid;
	m_f[0] = xsize;
	m_f[1] = ysize;
	m_f[2] = zsize;
}

std::string BoundingObject::TranslateTypeToString(BoundingObject::Type type)
{
	switch (type) 
	{
		case BoundingObject::Type::Point:
			return "point";						break;
		case BoundingObject::Type::Sphere:
			return "sphere";					break;
		case BoundingObject::Type::Cube:
			return "cube";						break;
		case BoundingObject::Type::Cuboid:
			return "cuboid";					break;
		default:
			return "";							break;
	}
}

BoundingObject::Type BoundingObject::TranslateStringToType(const std::string & type)
{
	if (type == "point")
		return BoundingObject::Type::Point;
	else if (type == "sphere")
		return BoundingObject::Type::Sphere;
	else if (type == "cube")
		return BoundingObject::Type::Cube;
	else if (type == "cuboid")
		return BoundingObject::Type::Cuboid;
	else
		return BoundingObject::Type::None;
}