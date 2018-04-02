#pragma once

#include "CompilerSettings.h"
#include "RM_InstanceData.h"
class MaterialDX11;



struct RM_MaterialInstanceMapping
{
	const MaterialDX11 *				Material;
	RM_InstanceData						InstanceCollection;


	// Constructors
	CMPINLINE RM_MaterialInstanceMapping(void) { }
	CMPINLINE RM_MaterialInstanceMapping(const MaterialDX11 *material)
		:
		Material(material)
	{
	}
	
	// Copy constructor/assignment is disallowed
	CMPINLINE RM_MaterialInstanceMapping(const RM_MaterialInstanceMapping & other) = delete;
	CMPINLINE RM_MaterialInstanceMapping & operator=(const RM_MaterialInstanceMapping & other) = delete;

	// Move constructor
	CMPINLINE RM_MaterialInstanceMapping(RM_MaterialInstanceMapping && other)
		:
		Material(other.Material), 
		InstanceCollection(std::move(other.InstanceCollection))
	{
	}

	// Move assignment
	CMPINLINE RM_MaterialInstanceMapping & operator=(RM_MaterialInstanceMapping && other)
	{
		Material = other.Material;
		InstanceCollection = std::move(other.InstanceCollection);
	}

	// Destructor
	CMPINLINE ~RM_MaterialInstanceMapping(void)
	{
	}
};
