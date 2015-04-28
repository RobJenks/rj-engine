#pragma once

#ifndef __ObjectManagerH__
#define __ObjectManagerH__

#include "Utility.h"
#include "Octree.h"
class iSpaceObject;

class SimulationObjectManager
{
public:

	// Bit flags specifying object search options
	enum ObjectSearchOptions
	{
		NoSearchOptions						= 0x00,
		OnlyCollidingObjects				= 0x01,
		IncludeFocalObjectBoundary			= 0x02,
		IncludeTargetObjectBoundaries		= 0x04
	};

	// Bit flags specifying the conditions for partitioning an Octree node during object search
	enum NodeSubdiv
	{
		X_NEG_SEG = 0x01,
		X_POS_SEG = 0x02,
		X_BOTH = (X_NEG_SEG | X_POS_SEG),
		Y_NEG_SEG = 0x04,
		Y_POS_SEG = 0x08,
		Y_BOTH = (Y_NEG_SEG | Y_POS_SEG),
		Z_NEG_SEG = 0x10,
		Z_POS_SEG = 0x20,
		Z_BOTH = (Z_NEG_SEG | Z_POS_SEG)
	};
		

	// Searches for all items within the specified distance of an object.  Returns the number of items located
	// Allows use of certain flags to limit results during the search; more efficient that returning everything and then removing items later
	int GetAllObjectsWithinDistance(const iSpaceObject *focalobject, float distance, std::vector<iSpaceObject*> & outResult,
									int options);


protected:


	// Data used in the process of searching for objects
	std::vector<Octree<iSpaceObject*>*> search_candidate_nodes;
	std::vector<iSpaceObject*>::const_iterator search_obj_end, search_obj_it;
	std::vector<iSpaceObject*> search_large_objects;
	iSpaceObject *search_last_large_obj;
	bool search_found_large_object;

};




#endif