#pragma once

#include "DX11_Core.h"
#include "CompilerSettings.h"


// Stores per-instance data that is only required in the render queue, and does not need to be passed through the GPU pipeline
struct RM_InstanceMetadata
{
public:

	// Instance metadata
	XMVECTOR					Position;
	float						BoundingSphereRadius;


	// Constructor
	CMPINLINE RM_InstanceMetadata(void) { }

	// Constructor
	CMPINLINE RM_InstanceMetadata(FXMVECTOR position, float bounding_sphere_radius)
		:
		Position(position),
		BoundingSphereRadius(bounding_sphere_radius)
	{
	}

	// Copy assignment is disallowed
	CMPINLINE RM_InstanceMetadata & operator=(const RM_InstanceMetadata & other) = delete;

	// Copy constructor
	CMPINLINE RM_InstanceMetadata(const RM_InstanceMetadata & other)
		:
		Position(other.Position),
		BoundingSphereRadius(other.BoundingSphereRadius)
	{
	}

	// Move constructor
	CMPINLINE RM_InstanceMetadata(RM_InstanceMetadata && other)
		:
		Position(std::move(other.Position)),
		BoundingSphereRadius(other.BoundingSphereRadius)
	{
	}

	// Move assignment
	CMPINLINE RM_InstanceMetadata & operator=(RM_InstanceMetadata && other)
	{
		Position = other.Position;
		BoundingSphereRadius = other.BoundingSphereRadius;

		return *this;
	}

	// Destructor
	CMPINLINE ~RM_InstanceMetadata(void) { }



};