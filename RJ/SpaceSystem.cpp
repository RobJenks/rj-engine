#include <algorithm>
#include "Utility.h"
#include "FastMath.h"
#include "GameDataExtern.h"
#include "ObjectReference.h"
#include "GameSpatialPartitioningTrees.h"
#include "SimulationStateManager.h"
#include "LightSource.h"

#include "SpaceSystem.h"


// Default constructor
SpaceSystem::SpaceSystem(void)
{
	// Set all fields to NULL before initialisation
	m_initialised = false;
	m_code = m_name = "";
	m_size = m_minbounds = m_maxbounds = NULL_VECTOR;
	m_sizef = m_minboundsf = m_maxboundsf = NULL_FLOAT3;
	SpatialPartitioningTree = NULL;
	m_backdroplocation = "";
	m_backdrop = NULL;
}

Result SpaceSystem::InitialiseSystem(ID3D11Device *device)
{
	// The system can only be initialised once
	if (m_initialised) return ErrorCodes::CannotReinitialiseSystem;

	// Validate critical parameters
	if (m_code == NullString) return ErrorCodes::CannotInitialiseSystemWithoutValidCode;
	if (IsZeroVector3(m_size) || IsZeroVector3(m_minbounds) || IsZeroVector3(m_maxbounds)) return ErrorCodes::CannotInitialiseSystemWithInvalidSize;

	// Create a new spatial partitioning tree to track objects within the system.  Determine the largest dimension and generate to that extent
	// in each dimension to ensure coverage.  We should only generally have cubic systems (x==y==z) but this allows flexibility for the future
	float largestdimension = max(m_sizef.x, max(m_sizef.y, m_sizef.z));
	SpatialPartitioningTree = new Octree<iObject*>(
		XMVectorMultiply(XMVectorReplicate(largestdimension), HALF_VECTOR_N),						// Centre point is at -0.5f * size (e.g. -50k to +50k for a size of 100k)
		largestdimension);																			
	if (!SpatialPartitioningTree) return ErrorCodes::CouldNotInitialiseSpatialPartitioningTree;

	// Add the spatial partitioning tree for this system to the global tree-pruning maintenance process
	Game::TreePruner.AddNode(SpatialPartitioningTree);

	// Initialise the per-system projectile collection using default parameters
	Projectiles.Initialise();

	// Attempt to load the system backdrop, if a valid filename has been provided
	m_backdrop = new Texture();
	if (m_backdroplocation != NullString) 
	{
		string filename = BuildStrFilename(D::IMAGE_DATA, m_backdroplocation);
		if (FileExists(filename.c_str()))
		{
			m_backdrop->Initialise(filename.c_str());
		}
	}

	// Return success
	return ErrorCodes::NoError;
}

// Handles the entry of an object into the system, adding it to the system collections and updating the simulation state accordingly
Result SpaceSystem::AddObjectToSystem(iSpaceObject * object, const FXMVECTOR location)
{
	// Parameter check; ensure this is a valid object, and that it doesn't already belong to another system
	if (object == NULL)								return ErrorCodes::CannotAddNullObjectToSystem;
	if (object->GetSpaceEnvironment())				return ErrorCodes::ObjectAlreadyExistsInOtherSystem;
	if (object->GetSpatialTreeNode())				return ErrorCodes::ObjectAlreadyExistsInOtherSpatialTree;

	// Add to the list of system objects
	Objects.push_back(ObjectReference<iSpaceObject>(object));

	// Clamp the entry position to the bounds of the system
	XMVECTOR pos = XMVectorClamp(location, m_minbounds, m_maxbounds);

	// Store a reference to the new system this object exists in
	object->SetSpaceEnvironmentDirect(this);

	// Also update the system light collection if this object was a light
	if (object->GetObjectType() == iObject::ObjectType::LightSourceObject)
	{
		RegisterAllSystemLights();
	}

	// Set the position of the object in the system, and add it to the system spatial positioning tree
	object->SetPosition(pos);
	object->RefreshPositionImmediate();
	if (SpatialPartitioningTree)
	{
		SpatialPartitioningTree->AddItem(object, pos);
	}

	// Notify the state manager that this object has entered the system
	Game::StateManager.ObjectEnteringSpaceEnvironment(object, this);

	// Return success
	return ErrorCodes::NoError;
}

// Handles the exit of an object from the system, removing it from the system collections and updating the simulation state accordingly
Result SpaceSystem::RemoveObjectFromSystem(iSpaceObject * object)
{
	// Parameter check; ensure that this is a valid object
	if (!object) return ErrorCodes::CannotRemoveNullObjectFromSystem;

	// Remove from the spatial partitioning tree for this system
	if (SpatialPartitioningTree && object->GetSpatialTreeNode() != NULL)
	{
		object->GetSpatialTreeNode()->RemoveItem(object);
		object->SetSpatialTreeNode(NULL);
	}

	// Remove from the system collection itself; ensure that there are not multiple copies of the same object to be safe
	while (true)
	{
		std::vector<ObjectReference<iSpaceObject>>::iterator it = std::find_if(Objects.begin(), Objects.end(),
			[&object](const ObjectReference<iSpaceObject> & obj) { return (obj() == object); });
		if (it == Objects.end()) break;
		Objects.erase(it);
	}

	// Remove the reference to this system from the object
	object->SetSpaceEnvironmentDirect(NULL);

	// Register this object as a system light if is a directional light source
	if (object->GetObjectType() == iObject::ObjectType::LightSourceObject)
	{
		RegisterAllSystemLights();
	}

	// Notify the state manager so it can handle any necessary changes in state
	Game::StateManager.ObjectLeavingSpaceEnvironment(object, this);

	// Return success
	return ErrorCodes::NoError;
}

// Identifies and stores a reference to all directional system light sources in the system object collection
void SpaceSystem::RegisterAllSystemLights(void)
{
	// Clear the current record of directional light sources
	m_directional_lights.clear();

	// Now locate any directional lights and store them
	const iSpaceObject *obj;
	std::vector<ObjectReference<iSpaceObject>>::const_iterator it_end = Objects.end();
	for (std::vector<ObjectReference<iSpaceObject>>::const_iterator it = Objects.begin(); it != it_end; ++it)
	{
		obj = (*it)(); 
		if (obj && obj->GetObjectType() == iObject::ObjectType::LightSourceObject)
		{
			if (((LightSource*)obj)->GetLight().Data.Type == Light::LightType::Directional)
			{
				m_directional_lights.push_back((iObject*)obj);
			}
		}
	}
}


// Set the size of the system.  Cannot be changed post-initialisation.
void SpaceSystem::SetSize(const FXMVECTOR size)
{
	// We will only allow the size to be changed if the system has not yet been initialised; cannot be changed post-initialisation
	if (m_initialised) return;

	// Store the new size of the system
	m_size = size;

	// Calculate the minimum and maximum bounds for the system, maintained to save calculations at runtime
	m_maxbounds = XMVectorMultiply(m_size, HALF_VECTOR_P);
	m_minbounds = XMVectorNegate(m_maxbounds);

	// Also store local float representations for convenience
	XMStoreFloat3(&m_sizef, m_size);
	XMStoreFloat3(&m_minboundsf, m_minbounds);
	XMStoreFloat3(&m_maxboundsf, m_maxbounds);
}

void SpaceSystem::TerminateSystem(void)
{
	// Shutdown the spatial partitioning tree that tracks objects in the system.  Moves recursively down the 
	// tree, returning each node to the memory pool as it goes.  
	SpatialPartitioningTree->Shutdown();
	SpatialPartitioningTree = NULL;

	// Release the backdrop texture resource maintained for this system
	if (m_backdrop) 
	{
		m_backdrop->Shutdown();
		SafeDelete(m_backdrop);
	}

	// Finally, mark the system as uninitialised so it is not used in future (and can be re-initialised if required)
	m_initialised = false;
}


// Default destructor
SpaceSystem::~SpaceSystem(void)
{
}
