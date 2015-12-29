#include "iObject.h"
#include "Octree.h"
#include "OctreePruner.h"

#include "GameSpatialPartitioningTrees.h"

namespace Game
{

	// Data structures used to maintain object / positioning data 
	//Octree<iSpaceObject*> *			SpatialPartitioningTree;
	//float							C_DEFAULT_SPATIAL_PARTITIONING_TREE_EXTENT = 200000.0f;

	// Central components used to periodically prune all spatial partitioning trees to remove redundant nodes
	OctreePruner<iObject*>			TreePruner = OctreePruner<iObject*>();

}