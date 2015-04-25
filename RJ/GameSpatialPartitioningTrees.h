#pragma once

#ifndef __GameSpatialPartitioningTreesH__
#define __GameSpatialPartitioningTreesH__


#include "iSpaceObject.h"
#include "Octree.h"
#include "OctreePruner.h"

namespace Game {

	// Data structures used to maintain object / positioning data 
	//extern Octree<iSpaceObject*> *SpatialPartitioningTree;
	//extern float C_DEFAULT_SPATIAL_PARTITIONING_TREE_EXTENT;

	// Central components used to periodically prune all spatial partitioning trees to remove redundant nodes
	extern OctreePruner<iSpaceObject*> TreePruner;
};





#endif