#include "FastMath.h"
#include "Utility.h"
#include "Model.h"
#include "ModelInstance.h"
#include "InstanceFlags.h"
#include "iObject.h"
#include "iSpaceObjectEnvironment.h"
#include "TerrainDefinition.h"
#include "OrientedBoundingBox.h"
#include "EnvironmentTree.h"
#include "CoreEngine.h"
#include "OverlayRenderer.h"
#include "DynamicTerrain.h"

#include "Terrain.h"


// Static record of the current maximum ID for terrain objects; allows all terrain objects to be given a uniquely-identifying reference
Game::ID_TYPE Terrain::InstanceCreationCount = 0;


// Default constructor; initialise fields to default values
Terrain::Terrain()
	:	m_definition(NULL), m_parent(NULL), m_sourcetype(TerrainSourceType::NoSource), m_orientation(ID_QUATERNION), m_worldmatrix(ID_MATRIX), 
		m_collisionradius(0.0f), m_collisionradiussq(0.0f), m_health(0.0f), m_element_min(NULL_INTVECTOR3), m_element_max(NULL_INTVECTOR3), 
		m_multielement(false), m_postponeupdates(false), m_env_treenode(NULL), m_parenttile(0), m_mass(1.0f), m_hardness(1.0f), 
		m_isdynamic(false), m_dataenabled(false), m_usable(false), m_articulated_model(NULL), m_has_model(false)
{
	m_data.Centre = NULL_VECTOR;
	m_data.ExtentF = NULL_FLOAT3;
	m_data.Extent[0].value = m_data.Extent[1].value = m_data.Extent[2].value = NULL_VECTOR;
	m_data.Axis[0].value = UNIT_BASES[0]; m_data.Axis[1].value = UNIT_BASES[1]; m_data.Axis[2].value = UNIT_BASES[2];

	// Assign a new unique ID for this terrain object
	m_id = Terrain::GenerateNewUniqueID();
}

// Sets the terrain definition for this object, updating other relevant fields in the process.  Will attempt to 
// look up a terrain definition with the given code, but if not found it will proceed with a null (i.e. no
// model or data) terrain definition
void Terrain::SetDefinition(const std::string & def)
{
	SetDefinition( D::TerrainDefinitions.Get(def) );
}

// Sets the terrain definition for this object, updating other relevant fields in the process
void Terrain::SetDefinition(const TerrainDefinition *d)
{
	// Store the new definition, which can be null (for non-renderable terrain such as pure collision boxes)
	m_definition = d;

	// We can derive further data if the definition is non-null (and if it has not been set manually already)
	if (m_definition)
	{
		// Take the model extents as our new bounds; this will also recalculate e.g. the collision radius
		SetExtent(XMLoadFloat3(&m_definition->GetDefaultExtent()));

		// Also initialise the health of this instance to the max health specified in its definition, if it has not already been set
		if (m_health == 0.0f) SetHealth(m_definition->GetMaxHealth());

		// Pull other data from the definition
		SetMass(m_definition->GetMass());
		SetHardness(m_definition->GetHardness());

		// Recalculate positional data, since e.g. the range of elements this object covers may now be different
		RecalculatePositionalData();

		// Update the flag that identifies whether we have a renderable model or not
		UpdateModelFlag();
	}
}

// Indicates whether this terrain object has a static model reference in its terrain definition
bool Terrain::HasStaticModel(void) const 
{ 
	return (m_definition && m_definition->HasModel()); 
}

// Return a reference to the static model assigned to this terrain object, or NULL if none is assigned
const Model * Terrain::GetStaticModel(void) const 
{ 
	return (m_definition ? m_definition->GetModel() : NULL); 
}

void Terrain::SetArticulatedModel(ArticulatedModel *m) 
{
	// Store the new model and update the flag indicating whether we have a model
	m_articulated_model = m; 
	UpdateModelFlag(); 

	// Take the consolidated model size if not already specified; this will also recalculate e.g. the collision radius
	if (m_articulated_model)
	{
		SetExtent(XMLoadFloat3(&m_articulated_model->GetExtent()));
	}
}


// Set the parent environment that contains this terrain object, or NULL if no environment
void Terrain::SetParentEnvironment(iSpaceObjectEnvironment *env)
{
	// Store a reference to the new environment (or NULL)
	m_parent = env;

	// Recalculate positional data, since this environment could influence e.g. the world matrix derivation
	RecalculatePositionalData();
}

// Changes the position of this terrain object, recalculating derived fields in the process
void Terrain::SetPosition(const FXMVECTOR pos)
{
	// Store the new position value
	m_data.Centre = pos;

	// Recalculate positional data, since e.g. the world matrix will now be different
	RecalculatePositionalData();
}

// Changes the orientation of this terrain object, recalculating derived fields in the process
void Terrain::SetOrientation(const FXMVECTOR orient)
{
	// Store the new orientation value
	m_orientation = orient;

	// Recalculate positional data, since e.g. the world matrix & axes will now be different
	RecalculatePositionalData();
}

// Adjusts the terrain orientation by the specified rotation value
void Terrain::ChangeOrientation(const FXMVECTOR delta)
{
	SetOrientation(XMQuaternionMultiply(m_orientation, delta));
}

// Changes the extents of this terrain object, recalculating derived fields in the process
void Terrain::SetExtent(const FXMVECTOR e)
{
	// Store the new extents; ensure no negative values
	XMVECTOR ex = XMVectorSetW(XMVectorMax(e, NULL_VECTOR), 0.0f);
	m_data.ExtentV = ex;
	XMStoreFloat3(&m_data.ExtentF, ex);
	m_data.Extent[0].value = XMVectorReplicate(m_data.ExtentF.x);
	m_data.Extent[1].value = XMVectorReplicate(m_data.ExtentF.y);
	m_data.Extent[2].value = XMVectorReplicate(m_data.ExtentF.z);

	// Recalculate the collision radius of this terrain object based on its maximum extents
	// Multiply all dimensions by 2 since we want the size, not the extent
	m_collisionradiussq = DetermineCuboidBoundingSphereRadiusSq(XMVectorMultiply(ex, XMVectorReplicate(2.0f)));	
	m_collisionradius = sqrtf(m_collisionradiussq);

	// Recalculate positional data, since e.g. the range of elements this object covers may now be different
	RecalculatePositionalData();
}

// Recalculates the positional data for this terrain following a change to its primary pos/orient data
void Terrain::RecalculatePositionalData(void)
{
	// We do not want to execute this method if updates are suspended
	if (m_postponeupdates) return;
	
	// Use a temporary ModelInstance object to calculate the required scaling factor for our current size.  Saves
	// storing a permanent base world matrix (ModelInstance::m_world) alongside every terrain instance
	XMMATRIX base_model_world;
	if (m_definition && m_definition->GetModel())
	{
		ModelInstance instance(m_definition->GetModel());
		instance.SetSize(XMVectorScale(GetExtentV(), 2.0f));
		base_model_world = instance.GetWorldMatrix();
	}
	else
	{
		base_model_world = ID_MATRIX;
	}

	// World = (base_model_world * rotation * translate_to_position)
	m_worldmatrix = XMMatrixMultiply(XMMatrixMultiply(
		base_model_world,
		XMMatrixRotationQuaternion(m_orientation)),
		XMMatrixTranslationFromVector(m_data.Centre));
		
	// Extract the basis vectors from the world matrix
	m_data.Axis[0].value = m_worldmatrix.r[0];
	m_data.Axis[1].value = m_worldmatrix.r[1];
	m_data.Axis[2].value = m_worldmatrix.r[2];

	// Determine the range of elements that this object covers
	XMVECTOR cradius = XMVectorReplicate(m_collisionradius);
	XMVECTOR elmin = XMVectorFloor(XMVectorMultiply(XMVectorSubtract(m_data.Centre, cradius), Game::C_CS_ELEMENT_SCALE_RECIP_V));
	XMVECTOR elmax = XMVectorFloor(XMVectorMultiply(XMVectorAdd(m_data.Centre, cradius), Game::C_CS_ELEMENT_SCALE_RECIP_V));

	// Convert vectors into integer elements
	m_element_location = Game::PhysicalPositionToElementLocation(m_data.Centre);
	Vector3ToIntVectorSwizzleYZ(elmin, m_element_min);
	Vector3ToIntVectorSwizzleYZ(elmax, m_element_max);

	// Set the flag that indicates whether we span multiple elements, for render-time efficiency
	m_multielement = (m_element_min != m_element_max);

	// Notify our parent environment (if applicable) that our position may have changed
	if (m_parent)
	{
		m_parent->TerrainPositionUpdated(this);
	}
}

// Returns a value indicating whether this terrain object overlaps the specified element
bool Terrain::OverlapsElement(const INTVECTOR3 & el) const
{
	return (m_multielement ? (m_element_min <= el && m_element_max >= el) : m_element_location == el);
}

// Determines the vertices of the surrounding collision volume
void Terrain::DetermineCollisionBoxVertices(iSpaceObjectEnvironment *parent, AXMVECTOR_P(&pOutVertices)[8]) const
{
	// Determine the extent along each basis vector
	static const int E_POS = 0; static const int E_NEG = 1;
	XMVECTOR AxisExtent[3][2];
	AxisExtent[0][E_POS] = XMVectorMultiply(m_data.Extent[0].value, m_data.Axis[0].value);
	AxisExtent[1][E_POS] = XMVectorMultiply(m_data.Extent[1].value, m_data.Axis[1].value);
	AxisExtent[2][E_POS] = XMVectorMultiply(m_data.Extent[2].value, m_data.Axis[2].value);

	// We also need the negated versions
	AxisExtent[0][E_NEG] = XMVectorNegate(AxisExtent[0][E_POS]);
	AxisExtent[1][E_NEG] = XMVectorNegate(AxisExtent[1][E_POS]);
	AxisExtent[2][E_NEG] = XMVectorNegate(AxisExtent[2][E_POS]);
	
	// Now use these values to determine the location of each bounding volume vertex in local space
	pOutVertices[0].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(m_data.Centre, AxisExtent[0][E_NEG]), AxisExtent[1][E_NEG]), AxisExtent[2][E_NEG]); // -x, -y, -z	(End face #1)
	pOutVertices[1].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(m_data.Centre, AxisExtent[0][E_POS]), AxisExtent[1][E_NEG]), AxisExtent[2][E_NEG]); // +x, -y, -z	(End face #1)
	pOutVertices[2].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(m_data.Centre, AxisExtent[0][E_POS]), AxisExtent[1][E_POS]), AxisExtent[2][E_NEG]); // +x, +y, -z	(End face #1)
	pOutVertices[3].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(m_data.Centre, AxisExtent[0][E_NEG]), AxisExtent[1][E_POS]), AxisExtent[2][E_NEG]); // -x, +y, -z	(End face #1)

	pOutVertices[4].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(m_data.Centre, AxisExtent[0][E_NEG]), AxisExtent[1][E_NEG]), AxisExtent[2][E_POS]); // -x, -y, +z	(End face #2)
	pOutVertices[5].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(m_data.Centre, AxisExtent[0][E_POS]), AxisExtent[1][E_NEG]), AxisExtent[2][E_POS]); // +x, -y, +z	(End face #2)
	pOutVertices[6].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(m_data.Centre, AxisExtent[0][E_POS]), AxisExtent[1][E_POS]), AxisExtent[2][E_POS]); // +x, +y, +z	(End face #2)
	pOutVertices[7].value = XMVectorAdd(XMVectorAdd(XMVectorAdd(m_data.Centre, AxisExtent[0][E_NEG]), AxisExtent[1][E_POS]), AxisExtent[2][E_POS]); // -x, +y, +z	(End face #2)

	// Now transform by the parent object's zero-element world matrix (since the terrain coordinates assume (0,0) equals
	// element (0,0,0) location, whereas in world space (0,0) is the model centre point), to yield the final world coordinates
	if (parent)
	{
		XMMATRIX zpworld = parent->GetZeroPointWorldMatrix();
		pOutVertices[0].value = XMVector3TransformCoord(pOutVertices[0].value, zpworld);
		pOutVertices[1].value = XMVector3TransformCoord(pOutVertices[1].value, zpworld);
		pOutVertices[2].value = XMVector3TransformCoord(pOutVertices[2].value, zpworld);
		pOutVertices[3].value = XMVector3TransformCoord(pOutVertices[3].value, zpworld);
		pOutVertices[4].value = XMVector3TransformCoord(pOutVertices[4].value, zpworld);
		pOutVertices[5].value = XMVector3TransformCoord(pOutVertices[5].value, zpworld);
		pOutVertices[6].value = XMVector3TransformCoord(pOutVertices[6].value, zpworld);
		pOutVertices[7].value = XMVector3TransformCoord(pOutVertices[7].value, zpworld);
	}
}

Terrain *Terrain::Create(const TerrainDefinition *def)
{
	// Create a new static terrain object with this definition (or NULL) and default parameters
	Terrain *terrain = new Terrain();
	if (!terrain) return NULL;

	// Initialise and return the terrain object
	terrain->InitialiseNewTerrain(def);
	return terrain;
}

// Initialise a newly-created terrain based on the given definition
void Terrain::InitialiseNewTerrain(const TerrainDefinition *def)
{
	// Postpone any internal recalculation, set all properties and then resume calculations
	PostponeUpdates();
	{
		// Set the terrain definition, plus default properties for other fields
		SetDefinition(def);
		SetPosition(NULL_VECTOR);
		SetOrientation(ID_QUATERNION);

		// Assign a default non-zero extent if none is specified in the definition
		if (IsZeroFloat3(GetExtentF())) SetExtent(ONE_VECTOR);
	}
	ResumeUpdates();
}

// Applies a highlight/alpha effect to the terrain object
void Terrain::Highlight(const XMFLOAT3 & colour, float alpha) const
{
	// Make sure we have all required data
	if (!m_parent) return;

	// Generate a world matrix for the required transformation
	XMMATRIX world = XMMatrixMultiply(	XMMatrixMultiply(XMMatrixRotationQuaternion(GetOrientation()), XMMatrixTranslationFromVector(GetPosition())),	// Env-local world
										m_parent->GetZeroPointWorldMatrix());																			// Global world
	XMFLOAT3 tsize = Float3MultiplyScalar(GetExtentF(), 2.1f);
	
	// Render the highlight effect around this terrain object
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, tsize.x, tsize.y, tsize.z, colour, 0.5f, 
		XMVector3TransformCoord(NULL_VECTOR, world));
}

// Applies a highlight effect to the terrain object.  No alpha blending is performed
void Terrain::Highlight(const XMFLOAT4 & colour) const
{
	// Make sure we have all required data
	if (!m_parent) return;

	// Generate a world matrix for the required transformation
	XMMATRIX world = XMMatrixMultiply(	XMMatrixMultiply(XMMatrixRotationQuaternion(GetOrientation()), XMMatrixTranslationFromVector(GetPosition())),	// Env-local world
										m_parent->GetZeroPointWorldMatrix());																			// Global world
	XMFLOAT3 tsize = Float3MultiplyScalar(GetExtentF(), 2.1f);

	// Render the highlight effect around this terrain object
	Game::Engine->GetOverlayRenderer()->RenderCuboid(world, tsize.x, tsize.y, tsize.z, colour);
}

// Returns the impact resistance of this object, i.e. the remaining force it can withstand from physical 
// impacts, with an impact point at the specified element
float Terrain::GetImpactResistance(void) const
{
	// Impact resistance is calculated as (mass * hardness)
	return (m_mass * m_hardness);
}

// Called by objects that are attempting to interact with the object.  Returns a flag indicating whether any 
// successful interaction was possible.  Forwards control to virtual subclass methods where terrain object is eligible
CMPINLINE bool Terrain::AttemptInteraction(iObject *interacting_object, DynamicTerrainInteractionType interaction_type)
{
	// Only dynamic terrain objects can be interacted with
	if (m_isdynamic)
	{
		// Only a subset of dynamic terrain may be explicitly marked as usable
		if (m_usable)
		{
			// This appears to be an eligible terrain object, so promote and pass control
			return ToDynamicTerrain()->AttemptInteraction(interacting_object, interaction_type);
		}
	}

	// The terrain object is not eligible for interaction, so return false immediately
	return false;
}

// Creates a copy of the terrain object and returns a pointer.  Uses default copy constructor and modifies result
Terrain * Terrain::Copy(void) const
{
	// Take different action depending on whether this is a static or dynamic terrain object
	Terrain *terrain = NULL; 
	if (IsDynamic())
	{
		// This is a dynamic terrain object, so call the virtual clone constructor to create a new instance
		const DynamicTerrain *source = this->ToDynamicTerrain();
		terrain = static_cast<Terrain*>(source->Clone());
	}
	else
	{
		// This is a static terrain object, so use the base Terrain clone method to construct a new instance
		terrain = this->Clone();
	}
	
	// Return the new instance (which may be null if the instantiation failed)
	return terrain;
}

// Clone method for regular static terrain objects; not applicable for dynamic terrain otherwise dynamic terrain data will be lost
Terrain * Terrain::Clone(void) const
{
	// Debug assert that this is not a dynamic terrain type, otherwise we will end up losing data
	assert(!IsDynamic());

	// Perform a shallow copy of the terrain object via its copy constructor
	Terrain *terrain = new Terrain(*this);

	// Assign a new unique ID to the object, and remove its parent pointer since it does not initially belong to any environment
	terrain->SetID(Terrain::GenerateNewUniqueID());
	terrain->SetParentEnvironment(NULL);

	// If any per-instance articulated model is present then clone it for this new instance
	if (terrain->GetArticulatedModel()) terrain->SetArticulatedModel(terrain->GetArticulatedModel()->Copy());

	// Return the new terrain instance
	return terrain;
}

// Event triggered upon destruction of the entity
void Terrain::DestroyObject(void)
{
	// If we are already destroyed then take no further action
	if (IsDestroyed()) return;

	// Apply any destruction effect

	// Now (repairably) destroy the object
	MarkObjectAsDestroyed();
}

// Custom debug string function
std::string Terrain::DebugString(void) const
{
	return concat("Terrain [ID=")(m_id)(", Type=")(m_definition ? m_definition->GetCode() : "<null>").str();
}


// Shutdown method to deallocate resources and remove the terrain object
void Terrain::Shutdown(void)
{
	// Remove this object from its parent environment, which will also deallocate this object
	if (m_parent)
	{
		m_parent->RemoveTerrainObject(this);
	}
	else
	{
		// This is an error.  Object must always be part of a parent environment, since the parent environment
		// has ownership for its terrain and is responsible for deallocating it
		Game::Log << LOG_ERROR << "ERROR: Terrain object" << m_id << " is being shut down but has no parent environment\n";
	}
}

// Default destructor
Terrain::~Terrain()
{
}







