#pragma once

#ifndef __GameSpatialPartitioningTreesH__
#define __GameSpatialPartitioningTreesH__


#include "iObject.h"
#include "Octree.h"
#include "OctreePruner.h"


// This file contains no declarations with special alignment requirements
namespace Game {

	// Data structures used to maintain object / positioning data 
	//extern Octree<iSpaceObject*> *SpatialPartitioningTree;
	//extern float C_DEFAULT_SPATIAL_PARTITIONING_TREE_EXTENT;

	// Central components used to periodically prune all spatial partitioning trees to remove redundant nodes
	extern OctreePruner<iObject*> TreePruner;
};





#endif