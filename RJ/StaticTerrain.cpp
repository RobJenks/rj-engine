#include "FastMath.h"
#include "Utility.h"
#include "Model.h"
#include "iObject.h"
#include "iSpaceObjectEnvironment.h"
#include "StaticTerrainDefinition.h"
#include "OrientedBoundingBox.h"
#include "StaticTerrain.h"


// Static record of the current maximum ID for terrain objects; allows all terrain objects to be given a uniquely-identifying reference
Game::ID_TYPE StaticTerrain::InstanceCreationCount = 0;


// Default constructor; initialise fields to default values
StaticTerrain::StaticTerrain()
	:	m_definition(NULL), m_parent(NULL), m_orientation(ID_QUATERNION), m_worldmatrix(ID_MATRIX), m_collisionradius(0.0f), m_collisionradiussq(0.0f),
		m_health(0.0f), m_element_min(NULL_INTVECTOR3), m_element_max(NULL_INTVECTOR3), m_multielement(false), m_postponeupdates(false), m_parenttile(0)
{
	m_data.Centre = NULL_VECTOR;
	m_data.Extent = NULL_VECTOR;
	m_data.Axis[0] = m_data.Axis[1] = m_data.Axis[2] = NULL_VECTOR;

	// Assign a new unique ID for this terrain object
	m_id = StaticTerrain::GenerateNewUniqueID();
}


// Sets the terrain definition for this object, updating other relevant fields in the process
void StaticTerrain::SetDefinition(const StaticTerrainDefinition *d)
{
	// Store the new definition, which can be null (for non-renderable terrain such as pure collision boxes)
	m_definition = d;

	// We can derive further data if the definition is non-null (and if it has not been set manually already)
	if (m_definition)
	{
		// Take the standard object size if not already specified; this will also recalculate e.g. the collision radius
		if (IsZeroVector(m_data.Extent)) SetExtent(m_definition->GetDefaultExtent());

		// Also initialise the health of this instance to the max health specified in its definition, if it has not already been set
		if (m_health == 0.0f) SetHealth(m_definition->GetMaxHealth());

		// Recalculate positional data, since e.g. the range of elements this object covers may now be different
		RecalculatePositionalData();
	}
}

// Set the parent environment that contains this terrain object, or NULL if no environment
void StaticTerrain::SetParentEnvironment(const iSpaceObjectEnvironment *env)
{
	// Store a reference to the new environment (or NULL)
	m_parent = env;

	// Recalculate positional data, since this environment could influence e.g. the world matrix derivation
	RecalculatePositionalData();
}

// Changes the position of this terrain object, recalculating derived fields in the process
void StaticTerrain::SetPosition(const D3DXVECTOR3 & pos)
{
	// Store the new position value
	m_data.Centre = pos;

	// Recalculate positional data, since e.g. the world matrix will now be different
	RecalculatePositionalData();
}

// Changes the orientation of this terrain object, recalculating derived fields in the process
void StaticTerrain::SetOrientation(const D3DXQUATERNION & orient)
{
	// Store the new orientation value
	m_orientation = orient;

	// Recalculate positional data, since e.g. the world matrix & axes will now be different
	RecalculatePositionalData();
}

// Changes the extents of this terrain object, recalculating derived fields in the process
void StaticTerrain::SetExtent(const D3DXVECTOR3 & e)
{
	// Store the new extents; ensure no negative values
	m_data.Extent = D3DXVECTOR3(max(e.x, 0.0f), max(e.y, 0.0f), max(e.z, 0.0f));

	// Recalculate the collision radius of this terrain object based on its maximum extents
	// Multiply all dimensions by 2 since we want the size, not the extent
	m_collisionradiussq = DetermineCuboidBoundingSphereRadiusSq(m_data.Extent * 2.0f);	
	m_collisionradius = sqrtf(m_collisionradiussq);

	// Recalculate positional data, since e.g. the range of elements this object covers may now be different
	RecalculatePositionalData();
}

// Recalculates the positional data for this terrain following a change to its primary pos/orient data
void StaticTerrain::RecalculatePositionalData(void)
{
	// We do not want to execute this method if updates are suspended
	if (m_postponeupdates) return;

	// Create matrices for the translation (position), rotation (orientation) and centre-translation (modelcentre)
	D3DXMATRIX trans, rot, centretrans;
	
	// First, the centre translation.  Only relevant if we have a definition & associated model
	if (m_definition && m_definition->GetModel())
	{
		D3DXVECTOR3 ctrans = (m_definition->GetModel()->GetModelCentre() * -1.0f);
		D3DXMatrixTranslation(&centretrans, ctrans.x, ctrans.y, ctrans.z);
	}
	else
	{
		centretrans = ID_MATRIX;
	}

	// Construct the rotation matrix based on our orientation
	D3DXMatrixRotationQuaternion(&rot, &m_orientation);

	// Finally the translation matrix; combine our position with the parent environment zero-point translation to generate a 
	// translation matrix from the environment (0,0,0) point.  Otherwise, without adding the zero-point translation, the terrain
	// object position would instead be translated from the environment centre point.
	if (m_parent)
	{
		const D3DXVECTOR3 & offset = NULL_VECTOR; // m_parent->GetZeroPointTranslation();
		D3DXMatrixTranslation(&trans, offset.x + m_data.Centre.x, offset.y + m_data.Centre.y, offset.z + m_data.Centre.z);
	}
	else
	{
		D3DXMatrixTranslation(&trans, m_data.Centre.x, m_data.Centre.y, m_data.Centre.z);
	}	

	// Multiply all matrices together to yield the overall world matrix
	m_worldmatrix = (centretrans * rot * trans);

	// Extract the basis vectors from the world matrix
	m_data.Axis[0] = D3DXVECTOR3(m_worldmatrix._11, m_worldmatrix._12, m_worldmatrix._13);
	m_data.Axis[1] = D3DXVECTOR3(m_worldmatrix._21, m_worldmatrix._22, m_worldmatrix._23);
	m_data.Axis[2] = D3DXVECTOR3(m_worldmatrix._31, m_worldmatrix._32, m_worldmatrix._33);

	// Determine the range of elements that this object covers
	m_element_min.x = (int)floorf((m_data.Centre.x - m_collisionradius) * Game::C_CS_ELEMENT_SCALE_RECIP);
	m_element_min.y = (int)floorf((m_data.Centre.z - m_collisionradius) * Game::C_CS_ELEMENT_SCALE_RECIP);
	m_element_min.z = (int)floorf((m_data.Centre.y - m_collisionradius) * Game::C_CS_ELEMENT_SCALE_RECIP);

	m_element_max.x = (int)floorf((m_data.Centre.x + m_collisionradius) * Game::C_CS_ELEMENT_SCALE_RECIP);
	m_element_max.y = (int)floorf((m_data.Centre.z + m_collisionradius) * Game::C_CS_ELEMENT_SCALE_RECIP);
	m_element_max.z = (int)floorf((m_data.Centre.y + m_collisionradius) * Game::C_CS_ELEMENT_SCALE_RECIP);

	// Set the flag that indicates whether we span multiple elements, for render-time efficiency
	m_multielement = (m_element_min != m_element_max);
}

// Determines the vertices of the surrounding collision volume
void StaticTerrain::DetermineCollisionBoxVertices(iSpaceObjectEnvironment *parent, D3DXVECTOR3(&pOutVertices)[8]) const
{
	// Determine the extent along each basis vector
	D3DXVECTOR3 ExtentAlongAxis[3];
	ExtentAlongAxis[0] = m_data.Extent[0] * m_data.Axis[0];
	ExtentAlongAxis[1] = m_data.Extent[1] * m_data.Axis[1];
	ExtentAlongAxis[2] = m_data.Extent[2] * m_data.Axis[2];

	// Now use these values to determine the location of each bounding volume vertex in local space
	pOutVertices[0] = m_data.Centre - ExtentAlongAxis[0] - ExtentAlongAxis[1] - ExtentAlongAxis[2];	// -x, -y, -z	(End face #1)
	pOutVertices[1] = m_data.Centre + ExtentAlongAxis[0] - ExtentAlongAxis[1] - ExtentAlongAxis[2];	// +x, -y, -z	(End face #1)
	pOutVertices[2] = m_data.Centre + ExtentAlongAxis[0] + ExtentAlongAxis[1] - ExtentAlongAxis[2];	// +x, +y, -z	(End face #1)
	pOutVertices[3] = m_data.Centre - ExtentAlongAxis[0] + ExtentAlongAxis[1] - ExtentAlongAxis[2];	// -x, +y, -z	(End face #1)

	pOutVertices[4] = m_data.Centre - ExtentAlongAxis[0] - ExtentAlongAxis[1] + ExtentAlongAxis[2];	// -x, -y, +z	(End face #2)
	pOutVertices[5] = m_data.Centre + ExtentAlongAxis[0] - ExtentAlongAxis[1] + ExtentAlongAxis[2];	// +x, -y, +z	(End face #2)
	pOutVertices[6] = m_data.Centre + ExtentAlongAxis[0] + ExtentAlongAxis[1] + ExtentAlongAxis[2];	// +x, +y, +z	(End face #2)
	pOutVertices[7] = m_data.Centre - ExtentAlongAxis[0] + ExtentAlongAxis[1] + ExtentAlongAxis[2];	// -x, +y, +z	(End face #2)

	// Now transform by the parent object's zero-element world matrix (since the terrain coordinates assume (0,0) equals
	// element (0,0,0) location, whereas in world space (0,0) is the model centre point), to yield the final world coordinates
	if (parent)
	{
		D3DXVec3TransformCoordArray(pOutVertices, sizeof(D3DXVECTOR3), pOutVertices, sizeof(D3DXVECTOR3), parent->GetZeroPointWorldMatrix(), 8U);
	}
}

StaticTerrain *StaticTerrain::Create(const StaticTerrainDefinition *def)
{
	// Create a new static terrain object with this definition (or NULL) and default parameters
	StaticTerrain *terrain = new StaticTerrain();
	if (!terrain) return NULL;

	// Postpone any internal recalculation, set all properties and then resume calculations
	terrain->PostponeUpdates();
	{
		// Set the terrain definition, plus default properties for other fields
		terrain->SetDefinition(def);
		terrain->SetPosition(NULL_VECTOR);
		terrain->SetOrientation(ID_QUATERNION);

		// Assign a default non-zero extent if none is specified in the definition
		if (IsZeroVector(terrain->GetExtent())) terrain->SetExtent(D3DXVECTOR3(1.0f, 1.0f, 1.0f));
	}
	terrain->ResumeUpdates();

	// Return the new terrain object
	return terrain;
}

// Creates a copy of the terrain object and returns a pointer.  Uses default copy constructor and modifies result
StaticTerrain * StaticTerrain::Copy(void) const
{
	// Create a shallow copy of the object and assign a new unique ID
	StaticTerrain *t = new StaticTerrain(*this);
	t->SetID(StaticTerrain::GenerateNewUniqueID());

	// Alter the parent object references, since this object will start off unassigned
	t->SetParentEnvironment(NULL);

	// Return the new terrain object
	return t;
}

// Default destructor
StaticTerrain::~StaticTerrain()
{
}







