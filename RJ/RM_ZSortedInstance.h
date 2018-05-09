#pragma once

#include "RM_Instance.h"
class ModelBuffer;


// Structure to hold z-sorted instance data, used where objects must be sorted before processing the render queue
struct RM_ZSortedInstance
{
	// Structure data
	int							Key;
	ModelBuffer *				ModelPtr;
	RM_Instance					Item;

	// Comparison operator for sorting
	CMPINLINE bool operator<(const RM_ZSortedInstance & val) const { return (Key < val.Key); }

	// Constructors
	CMPINLINE RM_ZSortedInstance(int key, ModelBuffer *model, const CXMMATRIX world) noexcept :
		Key(key), ModelPtr(model), Item(std::move(RM_Instance(world, RM_Instance::CalculateSortKey((float)key)))) { }

	CMPINLINE RM_ZSortedInstance(int key, ModelBuffer *model, RM_Instance && instance) noexcept :
		Key(key), ModelPtr(model), Item(std::move(instance)) { }

	// Copy constructor & assignment are disallowed
	CMPINLINE RM_ZSortedInstance(const RM_ZSortedInstance & other) = delete;
	CMPINLINE RM_ZSortedInstance & operator=(const RM_ZSortedInstance & other) = delete;

	// Move constructor; no-exception guarantee to ensure these objects are moved rather than copied by STL containers
	CMPINLINE RM_ZSortedInstance(RM_ZSortedInstance && other) noexcept
		:
		Key(other.Key), 
		ModelPtr(other.ModelPtr), 
		Item(std::move(other.Item))
	{
	}

	// Move assignment
	CMPINLINE RM_ZSortedInstance & RM_ZSortedInstance::operator=(RM_ZSortedInstance && other) noexcept
	{
		Key = other.Key;
		ModelPtr = other.ModelPtr;
		Item = std::move(other.Item);
		return *this;
	}

};





