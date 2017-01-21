#pragma once
#ifndef __FastTrigC__
#define __FastTrigC__

#include "math.h"
#include "malloc.h"
#include <time.h>
#include <random>
#include <limits>
#include "GameVarsExtern.h"
#include "iObject.h"

#include "FastMath.h"

const float PI = 3.14159265358979f;
const float PIBY180 = 0.0174532925199433f;
const float _180BYPI = 57.2957795130823208f;
const float TWOPI = PI*2.0f;
const float PIOVER2 = PI / 2.0f;
const float ROOT2 = 1.414213562f;
const float ONE_BY_ROOT2 = 1.0f / ROOT2;
const float FLT_MAX_NEG = -(std::numeric_limits<float>::infinity());

const int INF_INT = std::numeric_limits<int>::infinity();
const unsigned int INF_UINT = std::numeric_limits<unsigned int>::infinity();
const short INF_SHORT = std::numeric_limits<short>::infinity();
const long INF_LONG = std::numeric_limits<long>::infinity();
const float INF_FLOAT = std::numeric_limits<float>::infinity();
const double INF_DOUBLE = std::numeric_limits<double>::infinity();


const INTVECTOR2 NULL_INTVECTOR2 = INTVECTOR2(0, 0);
const INTVECTOR3 NULL_INTVECTOR3 = INTVECTOR3(0, 0, 0);
const INTVECTOR2 ONE_INTVECTOR2 = INTVECTOR2(1, 1);
const INTVECTOR3 ONE_INTVECTOR3 = INTVECTOR3(1, 1, 1);
const XMVECTOR NULL_VECTOR = XMVectorZero();
const XMVECTOR NULL_VECTOR2 = XMVectorZero();
const XMVECTOR NULL_VECTOR3 = XMVectorZero();
const XMVECTOR NULL_VECTOR4 = XMVectorZero();
const XMVECTOR ZERO_VECTOR = XMVectorZero();
const XMVECTOR HALF_VECTOR_P = XMVectorReplicate(0.5f);
const XMVECTOR HALF_VECTOR_N = XMVectorNegate(HALF_VECTOR_P);
const XMVECTOR HALF_VECTOR = HALF_VECTOR_P;
const XMVECTOR ONE_VECTOR_P = XMVectorReplicate(1.0f);
const XMVECTOR ONE_VECTOR_N = XMVectorReplicate(-1.0f);
const XMVECTOR ONE_VECTOR = ONE_VECTOR_P;
const XMVECTOR LARGE_VECTOR_P = XMVectorReplicate(1e15f);
const XMVECTOR LARGE_VECTOR_N = XMVectorReplicate(-1e15f);
const XMVECTOR RAND_MAX_V = XMVectorReplicate(RAND_MAX);

const XMFLOAT2 NULL_FLOAT2 = XMFLOAT2(0.0f, 0.0f);
const XMFLOAT3 NULL_FLOAT3 = XMFLOAT3(0.0f, 0.0f, 0.0f);
const XMFLOAT4 NULL_FLOAT4 = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
const XMFLOAT2 ONE_FLOAT2 = XMFLOAT2(1.0f, 1.0f);
const XMFLOAT3 ONE_FLOAT3 = XMFLOAT3(1.0f, 1.0f, 1.0f);
const XMFLOAT4 ONE_FLOAT4 = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

const XMVECTOR BASIS_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
const XMVECTOR BASIS_VECTOR_NEGZ = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
const XMVECTOR BASIS_VECTOR2 = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
const XMVECTOR BASIS_VECTOR2_NEGY = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
const XMVECTOR UP_VECTOR = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
const XMVECTOR RIGHT_VECTOR = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
const XMVECTOR FORWARD_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

const XMFLOAT3 UP_VECTOR_F = XMFLOAT3(0.0f, 1.0f, 0.0f);
const XMFLOAT3 RIGHT_VECTOR_F = XMFLOAT3(1.0f, 0.0f, 0.0f);
const XMFLOAT3 FORWARD_VECTOR_F = XMFLOAT3(0.0f, 0.0f, 1.0f);

const XMVECTOR ID_QUATERNION = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
const XMFLOAT4 ID_QUATERNIONF = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
const XMMATRIX NULL_MATRIX = XMMatrixSet(0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f);
const XMMATRIX ID_MATRIX = XMMatrixSet(1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);
const XMMATRIX ELEMENT_SCALE_MATRIX = XMMatrixSet(Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f, 0.0f,
	0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f,
	0.0f, 0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);

const XMFLOAT4X4 ID_MATRIX_F = XMFLOAT4X4(1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);


const float SIN_ZERO = sinf(0.0f);
const float COS_ZERO = cosf(0.0f);

const AXMVECTOR VCTRL_0000 = XMVectorSelectControl(0U, 0U, 0U, 0U);
const AXMVECTOR VCTRL_0001 = XMVectorSelectControl(0U, 0U, 0U, 1U);
const AXMVECTOR VCTRL_0010 = XMVectorSelectControl(0U, 0U, 1U, 0U);
const AXMVECTOR VCTRL_0011 = XMVectorSelectControl(0U, 0U, 1U, 1U);
const AXMVECTOR VCTRL_0100 = XMVectorSelectControl(0U, 1U, 0U, 0U);
const AXMVECTOR VCTRL_0101 = XMVectorSelectControl(0U, 1U, 0U, 1U);
const AXMVECTOR VCTRL_0110 = XMVectorSelectControl(0U, 1U, 1U, 0U);
const AXMVECTOR VCTRL_0111 = XMVectorSelectControl(0U, 1U, 1U, 1U);
const AXMVECTOR VCTRL_1000 = XMVectorSelectControl(1U, 0U, 0U, 0U);
const AXMVECTOR VCTRL_1001 = XMVectorSelectControl(1U, 0U, 0U, 1U);
const AXMVECTOR VCTRL_1010 = XMVectorSelectControl(1U, 0U, 1U, 0U);
const AXMVECTOR VCTRL_1011 = XMVectorSelectControl(1U, 0U, 1U, 1U);
const AXMVECTOR VCTRL_1100 = XMVectorSelectControl(1U, 1U, 0U, 0U);
const AXMVECTOR VCTRL_1101 = XMVectorSelectControl(1U, 1U, 0U, 1U);
const AXMVECTOR VCTRL_1110 = XMVectorSelectControl(1U, 1U, 1U, 0U);
const AXMVECTOR VCTRL_1111 = XMVectorSelectControl(1U, 1U, 1U, 1U);

const XMMATRIX ROT_MATRIX_0 = XMMatrixSet(1.000000, 0.000000, -0.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	0.000000, 0.000000, 1.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000);
const XMMATRIX ROT_MATRIX_90 = XMMatrixSet(-0.000000, 0.000000, -1.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	1.000000, 0.000000, -0.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000);
const XMMATRIX ROT_MATRIX_180 = XMMatrixSet(-1.000000, 0.000000, 0.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	-0.000000, 0.000000, -1.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000);
const XMMATRIX ROT_MATRIX_270 = XMMatrixSet(0.000000, 0.000000, 1.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	-1.000000, 0.000000, 0.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000);
const XMFLOAT4X4 ROT_MATRIX_0_F = XMFLOAT4X4(1.000000, 0.000000, -0.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	0.000000, 0.000000, 1.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000);
const XMFLOAT4X4 ROT_MATRIX_90_F = XMFLOAT4X4(-0.000000, 0.000000, -1.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	1.000000, 0.000000, -0.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000);
const XMFLOAT4X4 ROT_MATRIX_180_F = XMFLOAT4X4(-1.000000, 0.000000, 0.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	-0.000000, 0.000000, -1.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000);
const XMFLOAT4X4 ROT_MATRIX_270_F = XMFLOAT4X4(0.000000, 0.000000, 1.000000, 0.000000,
	0.000000, 1.000000, 0.000000, 0.000000,
	-1.000000, 0.000000, 0.000000, 0.000000,
	0.000000, 0.000000, 0.000000, 1.000000);

const XMMATRIX* ROT_MATRICES[4] = { &ROT_MATRIX_0, &ROT_MATRIX_90, &ROT_MATRIX_180, &ROT_MATRIX_270 };
XMVECTOR ROT_QUATERNIONS[4];

// Orthornormal unit basis vectors
const XMVECTOR UNIT_BASES[3] = { XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };
const XMFLOAT3 UNIT_BASES_F[3] = { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) };

bool Float3NearEqual(const XMFLOAT3 & v1, const XMFLOAT3 & v2)
{
	return (fabs(v1.x - v2.x) < Game::C_EPSILON && fabs(v1.y - v2.y) < Game::C_EPSILON && fabs(v1.z - v2.z) < Game::C_EPSILON);
}
bool IsZeroFloat3(const XMFLOAT3 &v)
{
	return (fabs(v.x) < Game::C_EPSILON && fabs(v.y) < Game::C_EPSILON && fabs(v.z) < Game::C_EPSILON);
}





float *sintable;		// Contains pre-computed sine values
float *costable;		// Contains pre-computed cosine values
float *tantable;		// Contains pre-computed tangent values

std::tr1::ranlux_base_01					norm_reng;
std::tr1::normal_distribution<float>		norm_rdist;

// Sqrt cache values
float sqrt_cache[SQRT_CACHE_SIZE + 1];

// Element bounding sphere cache values
float element_bound_cache[ELEMENT_BOUND_CACHE_SIZE + 1];


void InitialiseMathFunctions()
{
	float f;

	// Seed the system random number generator
	srand((unsigned)time(NULL));

	// Allocate space for each trig table
	sintable = (float *)malloc(sizeof(float) * TRIG_TABLE_SIZE);
	costable = (float *)malloc(sizeof(float) * TRIG_TABLE_SIZE);
	tantable = (float *)malloc(sizeof(float) * TRIG_TABLE_SIZE);

	// Now calculate each element in turn
	for (int i=0; i<TRIG_TABLE_SIZE; ++i) {
		sintable[i] = sin(i * PIBY180);
		costable[i] = cos(i * PIBY180);
		tantable[i] = tan(i * PIBY180);
	}

	// Populate the sqrt cache table
	for (int i = 0; i <= SQRT_CACHE_SIZE; ++i) sqrt_cache[i] = sqrtf((float)i);

	// Populate the element bound cache table [ sphere_radius(n) = sqrt((n/2)^2 + (n/2)^2 + (n/2)^2) ]
	for (int i = 0; i <= ELEMENT_BOUND_CACHE_SIZE; ++i)
	{
		f = ((float)(i * Game::C_CS_ELEMENT_SCALE)) * 0.5f;		// (n/2)
		f *= f;													// (n/2)^2
		element_bound_cache[i] = sqrtf(f + f + f);				// sqrt((n/2)^2 + (n/2)^2 + (n/2)^2)
	}

	// Populate the pre-calculated quaternion arrays
	ROT_QUATERNIONS[0] = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	ROT_QUATERNIONS[1] = XMQuaternionRotationRollPitchYaw(0.0f, PIOVER2, 0.0f);
	ROT_QUATERNIONS[2] = XMQuaternionRotationRollPitchYaw(0.0f, PI, 0.0f);
	ROT_QUATERNIONS[3] = XMQuaternionRotationRollPitchYaw(0.0f, PI + PIOVER2, 0.0f);

	// Seed the normal distribution random number generator
	norm_reng.seed((unsigned long)time(NULL));
}

float _sin(const int theta) { return sintable[theta % TRIG_TABLE_SIZE]; }
float _cos(const int theta) { return costable[theta % TRIG_TABLE_SIZE]; }
float _tan(const int theta) { return tantable[theta % TRIG_TABLE_SIZE]; }


void TerminateMathFunctions()
{
	free(sintable); free(costable); free(tantable);
}

/*XMVECTOR XMVector3Rotate(const FXMVECTOR V, const FXMVECTOR Q)
{    
	D3DXQuaternionConjugate( &q1, pQ );    
	q2 = q1 * D3DXQUATERNION( pV->x, pV->y, pV->z, 1.0f ) * *pQ;	
	
	*pOut = D3DXVECTOR3( q2.x, q2.y, q2.z );    
	return pOut;
}*/

const XMMATRIX & GetRotationMatrix(Rotation90Degree rot)
{
	switch (rot)
	{
		case Rotation90Degree::Rotate90:		return ROT_MATRIX_90;		break;
		case Rotation90Degree::Rotate180:		return ROT_MATRIX_180;		break;
		case Rotation90Degree::Rotate270:		return ROT_MATRIX_270;		break;
		default:								return ROT_MATRIX_0;		break;
	}
}
XMMATRIX GetRotationMatrixInstance(Rotation90Degree rot)
{
	switch (rot)
	{
		case Rotation90Degree::Rotate90:		return ROT_MATRIX_90;		break;
		case Rotation90Degree::Rotate180:		return ROT_MATRIX_180;		break;
		case Rotation90Degree::Rotate270:		return ROT_MATRIX_270;		break;
		default:								return ROT_MATRIX_0;		break;
	}
}
const XMFLOAT4X4 *GetRotationMatrixF(Rotation90Degree rot)
{
	switch (rot)
	{
		case Rotation90Degree::Rotate90:		return &ROT_MATRIX_90_F;		break;
		case Rotation90Degree::Rotate180:		return &ROT_MATRIX_180_F;		break;
		case Rotation90Degree::Rotate270:		return &ROT_MATRIX_270_F;		break;
		default:								return &ROT_MATRIX_0_F;			break;
	}
}
XMFLOAT4X4 GetRotationMatrixInstanceF(Rotation90Degree rot)
{
	switch (rot)
		{
		case Rotation90Degree::Rotate90:		return ROT_MATRIX_90_F;		break;
		case Rotation90Degree::Rotate180:		return ROT_MATRIX_180_F;	break;
		case Rotation90Degree::Rotate270:		return ROT_MATRIX_270_F;	break;
		default:								return ROT_MATRIX_0_F;		break;
	}
}

// Returns a quaternion that represents the rotation value specified
XMVECTOR GetRotationQuaternion(Rotation90Degree rot)
{
	switch (rot)
	{
		case Rotation90Degree::Rotate90:		return ROT_QUATERNIONS[1];	break;
		case Rotation90Degree::Rotate180:		return ROT_QUATERNIONS[2];	break;
		case Rotation90Degree::Rotate270:		return ROT_QUATERNIONS[3];	break;
		default:								return ROT_QUATERNIONS[0];	break;
	}
}


XMVECTOR FloorVector(FXMVECTOR vec, float low)
{
	return XMVectorMax(vec, XMVectorReplicate(low));
}


XMVECTOR FloorVector(FXMVECTOR vec, const FXMVECTOR low)
{
	return XMVectorMax(vec, low);
}

XMVECTOR CeilVector(FXMVECTOR vec, float high)
{
	return XMVectorMin(vec, XMVectorReplicate(high));
}

XMVECTOR CeilVector(FXMVECTOR vec, const FXMVECTOR high)
{
	return XMVectorMin(vec, high);
}

XMVECTOR ClampVector(FXMVECTOR vec, float low, float high)
{
	return XMVectorClamp(vec, XMVectorReplicate(low), XMVectorReplicate(high));
}

XMVECTOR ClampVector(FXMVECTOR vec, const FXMVECTOR low, const FXMVECTOR high)
{
	return XMVectorClamp(vec, low, high);
}

// Scales a vector to the specified 'magnitude', so that one component is at +/- 'magnitude' with all other components scaled accordingly
// Near-zero vectors will not be scaled to avoid div/0 errors.
XMVECTOR ScaleVector3ToMagnitude(FXMVECTOR vec, float magnitude)
{	
	XMFLOAT3 v; XMStoreFloat3(&v, vec);
	float mx = max(max(fabs(v.x), fabs(v.y)), fabs(v.z));
	if (mx > Game::C_EPSILON) return XMVectorScale(vec, (magnitude / mx));
	return vec;
}

// Scales a vector to the specified 'magnitude', so that one component is at +/- 'magnitude' with all other components scaled accordingly
// Near-zero vectors will not be scaled to avoid div/0 errors.
void ScaleVector3ToMagnitude(XMFLOAT3 & vec, float magnitude)
{
	float mx = max(max(fabs(vec.x), fabs(vec.y)), fabs(vec.z));
	if (mx > Game::C_EPSILON)
	{
		float m = (magnitude / mx);
		vec.x *= m;
		vec.y *= m;
		vec.z *= m;
	}
}

// Scales a vector to ensure that the (absolute) value of any component does not exceed 'magnitude', with all other components scaled accordingly
XMVECTOR ScaleVector3WithinMagnitudeLimit(FXMVECTOR vec, float magnitude)
{
	XMFLOAT3 v; XMStoreFloat3(&v, vec);
	float mx = max(max(fabs(v.x), fabs(v.y)), fabs(v.z));
	if (mx > magnitude) return XMVectorScale(vec, (magnitude / mx));
	return vec;
}

// Scales a vector to ensure that the (absolute) value of any component does not exceed 'magnitude', with all other components scaled accordingly
void ScaleVector3WithinMagnitudeLimit(XMFLOAT3 &vec, float magnitude)
{
	float mx = max(max(fabs(vec.x), fabs(vec.y)), fabs(vec.z));
	if (mx > magnitude)
	{
		float m = (magnitude / mx);
		vec.x *= m;
		vec.y *= m;
		vec.z *= m;
	}
}

bool		IsZeroVector2(const FXMVECTOR vec) { return XMVector2NearEqual(vec, NULL_VECTOR2, Game::C_EPSILON_V); }
bool		IsZeroVector3(const FXMVECTOR vec) { return XMVector3NearEqual(vec, NULL_VECTOR3, Game::C_EPSILON_V); }
bool		IsZeroVector4(const FXMVECTOR vec) { return XMVector4NearEqual(vec, NULL_VECTOR4, Game::C_EPSILON_V); }
bool		IsZeroQuaternion(const FXMVECTOR q) { return XMVector4NearEqual(q, NULL_VECTOR4, Game::C_EPSILON_V); }
bool		IsIDQuaternion(const FXMVECTOR q) { return XMVector4NearEqual(q, ID_QUATERNION, Game::C_EPSILON_V); }


// Returns the squared diameter of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
float DetermineCuboidBoundingSphereDiameterSq(const FXMVECTOR xyz)
{
	// Use Phythagorus; cube diameter will be the 3D diagonal across the cuboid; D^2 = (A^2 + B^2 + C^2)
	//return ((xyz.x * xyz.x) + (xyz.y * xyz.y) + (xyz.z * xyz.z));
	return XMVectorGetX(XMVector3LengthSq(xyz));
}

// Returns the diameter of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
float DetermineCuboidBoundingSphereDiameter(const FXMVECTOR xyz)
{
	return sqrtf(DetermineCuboidBoundingSphereDiameterSq(xyz));
}

// Returns the squared radius of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
float DetermineCuboidBoundingSphereRadiusSq(const FXMVECTOR xyz)
{
	// [d = 2r] > [d^2 = (2r)^2] > [d^2 = (2*r*2*r)] > [d^2 = 4(r^2)] > [r^2 = (d^2)/4]
	return (DetermineCuboidBoundingSphereDiameterSq(xyz) * 0.25f);
}

// Returns the radius of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
float DetermineCuboidBoundingSphereRadius(const FXMVECTOR xyz)
{
	return sqrtf(DetermineCuboidBoundingSphereRadiusSq(xyz));
}

// Returns the squared diameter of a bounding sphere that completely encloses a cube with sides of the specified length
float DetermineCubeBoundingSphereDiameterSq(float cube_length)
{
	// Use shortcut to Pythagorus for efficiency; where A==B==C, D^2 = (A^2 + B^2 + C^2) >>> D^2 = 3(A^2)
	cube_length *= cube_length;
	return (cube_length + cube_length + cube_length);
}

// Returns the diameter of a bounding sphere that completely encloses a cube with sides of the specified length
float DetermineCubeBoundingSphereDiameter(float cube_length)
{
	return sqrt(DetermineCubeBoundingSphereDiameterSq(cube_length));
}

// Returns the squared radius of a bounding sphere that completely encloses a cube with sides of the specified length
float DetermineCubeBoundingSphereRadiusSq(float cube_length)
{
	// [d = 2r] > [d^2 = (2r)^2] > [d^2 = (2*r*2*r)] > [d^2 = 4(r^2)] > [r^2 = (d^2)/4]
	return (DetermineCubeBoundingSphereDiameterSq(cube_length) * 0.25f);
}

// Returns the radius of a bounding sphere that completely encloses a cube with sides of the specified length
float DetermineCubeBoundingSphereRadius(float cube_length)
{
	return sqrtf(DetermineCubeBoundingSphereRadiusSq(cube_length));
}

// Returns the radius of a bounding sphere that completely encloses a cube with sides of n ComplexShipElements in length
float DetermineElementBoundingSphereRadius(int n_elements)
{
	return DetermineCubeBoundingSphereRadius(((float)n_elements) * Game::C_CS_ELEMENT_SCALE);
}

XMVECTOR QuaternionBetweenVectors(const FXMVECTOR v1, const FXMVECTOR v2)
{
	// http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors

	// Take the cross product
	XMVECTOR q = XMVector3Cross(v1, v2);
	 
	// Build a quaternion based on the cross & dot products, and also incorporate the quaternion length
	//D3DXQUATERNION q = D3DXQUATERNION(w.x, w.y, w.z, D3DXVec3Dot(v1, v2));
	float w = XMVectorGetX(XMVector3Dot(v1, v2));
	q = XMVectorSetW(q, w);
	//q.w += D3DXQuaternionLength(&q);
	q = XMVectorSetW(q, w + XMVectorGetX(XMQuaternionLength(q)));
	
	// Normalise the vector and send the result to the output quaternion
	//D3DXQuaternionNormalize(pOutQuaternion, &q);
	return XMQuaternionNormalize(q);
}

XMFLOAT4 QuaternionMultiply(const XMFLOAT4 & q1, const XMFLOAT4 & q2)
{
	return XMFLOAT4(q1.y*q2.z - q1.z*q2.y + q1.x*q2.w + q1.w*q2.x,
					q1.z*q2.x - q1.x*q2.z + q1.y*q2.w + q1.w*q2.y,
					q1.x*q2.y - q1.y*q2.x + q1.z*q2.w + q1.w*q2.z,
					q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z);
}

void QuaternionMultiply(const XMFLOAT4 & q1, const XMFLOAT4 & q2, XMFLOAT4 & outProduct)
{
	outProduct.x = q1.y*q2.z - q1.z*q2.y + q1.x*q2.w + q1.w*q2.x;
	outProduct.y = q1.z*q2.x - q1.x*q2.z + q1.y*q2.w + q1.w*q2.y;
	outProduct.z = q1.x*q2.y - q1.y*q2.x + q1.z*q2.w + q1.w*q2.z;
	outProduct.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
}

XMFLOAT4 QuaternionAdd(const XMFLOAT4 & q1, const XMFLOAT4 & q2)
{
	return XMFLOAT4(q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w);
}

void QuaternionAdd(const XMFLOAT4 & q1, const XMFLOAT4 & q2, XMFLOAT4 & outSum)
{
	outSum.x = q1.x + q2.x;
	outSum.y = q1.y + q2.y;
	outSum.z = q1.z + q2.z;
	outSum.w = q1.w + q2.w;
}

void QuaternionNormalise(XMFLOAT4 & q)
{
	float one_over_mag = 1.0f / sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	q.x *= one_over_mag;
	q.y *= one_over_mag;
	q.z *= one_over_mag;
	q.w *= one_over_mag;
}

void QuaternionNormalise(const XMFLOAT4 & q, XMFLOAT4 & outQNorm)
{
	float one_over_mag = 1.0f / sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	outQNorm.x = (q.x * one_over_mag);
	outQNorm.y = (q.y * one_over_mag);
	outQNorm.z = (q.z * one_over_mag);
	outQNorm.w = (q.w * one_over_mag);
}

// Calculate the cosine of the angle between two 3D vectors
XMVECTOR CalculateCosAngleBetweenVectors(const FXMVECTOR a, const FXMVECTOR b)
{
	// dot(a,b) = (a.x*b.x) + (a.y*b.y) + (a.z*b.z) = |a|*|b|*cos(angle)
	// therefore, cos(angle) = (a.b) / (|a| * |b|)
	return (XMVectorDivide(
		XMVector3Dot(a, b),
		XMVectorAdd(XMVector3Length(a), XMVector3Length(b))));
}

// Calculate the cosine of the angle between two 3D vectors.  Not guarantee to be exact, but is faster
XMVECTOR CalculateCosAngleBetweenVectorsEst(const FXMVECTOR a, const FXMVECTOR b)
{
	// dot(a,b) = (a.x*b.x) + (a.y*b.y) + (a.z*b.z) = |a|*|b|*cos(angle)
	// therefore, cos(angle) = (a.b) / (|a| * |b|)
	return (XMVectorDivide(
		XMVector3Dot(a, b),
		XMVectorAdd(XMVector3LengthEst(a), XMVector3LengthEst(b))));
}

// Calculate the cosine of the angle between two NORMALISED 3D vectors (result is undefined where !(|a| == 1 && |b| == 1))
XMVECTOR CalculateCosAngleBetweenNormalisedVectors(const FXMVECTOR a, const FXMVECTOR b)
{
	// dot(a,b) = (a.x*b.x) + (a.y*b.y) + (a.z*b.z) = |a|*|b|*cos(angle)
	// therefore, cos(angle) = (a.b) / (|a| * |b|)
	// for unit vectors, |a|==|b|==1 so cos(angle) = (a.b)
	return XMVector3Dot(a, b);
}

// Calculate the angle between two 3D vectors
XMVECTOR CalculateAngleBetweenVectors(const FXMVECTOR a, const FXMVECTOR b)
{
	// dot(a,b) = (a.x*b.x) + (a.y*b.y) + (a.z*b.z) = |a|*|b|*cos(angle)
	// therefore, cos(angle) = (a.b) / (|a| * |b|)
	//			  angle = acos((a.b) / (|a| * |b|))
	return (XMVectorACos(
		XMVectorDivide(
		XMVector3Dot(a, b),
		XMVectorAdd(XMVector3Length(a), XMVector3Length(b)))));
}

// Calculate the angle between two 3D vectors.  Not guarantee to be exact, but is faster
XMVECTOR CalculateAngleBetweenVectorsEst(const FXMVECTOR a, const FXMVECTOR b)
{
	// dot(a,b) = (a.x*b.x) + (a.y*b.y) + (a.z*b.z) = |a|*|b|*cos(angle)
	// therefore, cos(angle) = (a.b) / (|a| * |b|)
	//			  angle = acos((a.b) / (|a| * |b|))
	return (XMVectorACosEst(
		XMVectorDivide(
		XMVector3Dot(a, b),
		XMVectorAdd(XMVector3LengthEst(a), XMVector3LengthEst(b)))));
}

// Calculate the angle between two NORMALISED 3D vectors (result is undefined where !(|a| == 1 && |b| == 1))
XMVECTOR CalculateAngleBetweenNormalisedVectors(const FXMVECTOR a, const FXMVECTOR b)
{
	// dot(a,b) = (a.x*b.x) + (a.y*b.y) + (a.z*b.z) = |a|*|b|*cos(angle)
	// therefore, cos(angle) = (a.b) / (|a| * |b|)
	// for unit vectors, |a|==|b|==1 so 
	//		cos(angle) = (a.b)
	//		angle = acos(a.b)
	return XMVectorACos(XMVector3Dot(a, b));
}

// Calculate the angle between two NORMALISED 3D vectors (result is undefined where !(|a| == 1 && |b| == 1))
// Not guaranteed to be exact, but is faster
XMVECTOR CalculateAngleBetweenNormalisedVectorsEst(const FXMVECTOR a, const FXMVECTOR b)
{
	// dot(a,b) = (a.x*b.x) + (a.y*b.y) + (a.z*b.z) = |a|*|b|*cos(angle)
	// therefore, cos(angle) = (a.b) / (|a| * |b|)
	// for unit vectors, |a|==|b|==1 so 
	//		cos(angle) = (a.b)
	//		angle = acos(a.b)
	return XMVectorACosEst(XMVector3Dot(a, b));
}

// Computes the approximate (but very close) inverse sqrt of a number VERY quickly.
// Requires use of 32-bit integer within the method for correct bitwise operations, so 
// set to use _int32 specifically.  Carmack.
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root
float fast_approx_invsqrt(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5F;
	y = number;
	i = *(__int32 *)&y;                      // evil floating point bit level hacking
	i = 0x5f3759df - (i >> 1);               // what the fuck?
	y = *(float *)&i;
	y = y * (threehalfs - (x2 * y * y));   // 1st iteration
	//  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

// Determines the yaw and pitch required to turn an object to face a point in space.  Assumes local object heading is [0 0 1] and performs
// test in local object coordinate space.  Both output values are [0.0-1.0] turn percentages
XMFLOAT2 DetermineYawAndPitchToTarget(const iObject & object, const FXMVECTOR target)
{
	// Determine the difference vector to this target, transform into local coordinate space (where our heading is the basis
	// vector [0, 0, 1], for mathematical simplicity) and normalise the difference vector
	XMVECTOR tgt = XMVector3TransformCoord(XMVectorSubtract(target, object.GetPosition()), object.GetInverseOrientationMatrix());
	tgt = XMVector3NormalizeEst(tgt);

	// Calculate the cross and dot products for ship yaw
	/*
	heading = BASIS_VECTOR;
	tgt = (s2->Location - ss->Location);	// (tgt is then transformed by the inverse ship orientation matrix)

	Optimisation: we know heading = the basis vector, so can substitute components for 0,0,1 and simplify accordingly
	ycross = (heading.z*tgt.x) - (heading.x*tgt.z);		= (1*tgt.x) - (0*tgt.z)		= tgt.x
	ydot = (heading.z*tgt.z) + (heading.x*tgt.x);		= (1*tgt.z) + (0*tgt.x)		= tgt.z
	pcross = (heading.y*tgt.z) - (heading.z*tgt.y);		= (0*tgt.z) - (1*tgt.y)		= -tgt.y
	pdot = (heading.x*tgt.x) + (heading.z*tgt.z);		= (0*tgt.x) + (1*tgt.z)		= tgt.z

	We therefore don't need to even maintain heading as a variable.  We can also just use tgt components in place of cross/dot
	*/

	XMFLOAT3 tgt_f;
	XMStoreFloat3(&tgt_f, tgt);

	// Determine yaw value depending on the current angle to target
	// outPitchYaw.x == pitch, outPitchYaw = yaw
	XMFLOAT2 result;
	if (fast_abs(tgt_f.x) > 0.01f)	result.y = tgt_f.x;		// Plot a yaw component proportionate to the angle the ship needs to cover
	else {
		if (tgt_f.z < 0.0f)			result.y = -1.0f;			// We are over 180deg from the target, so perform a full turn
		else						result.y = 0.0f;			// We are on the correct heading so maintain yaw
	}

	// Now determine pitch value, also based on the current angle to target
	if (fast_abs(tgt_f.y) > 0.01f)	result.x = -tgt_f.y;		// Plot a pitch component proportionate to the angle the ship needs to cover
	else {
		if (tgt_f.z < 0.0f)			result.x = -1.0f;			// We are over 180deg from the target, so perform a full turn
		else						result.x = 0.0f;			// We are on the correct heading so maintain pitch
	}

	return result;
}

// Determines the yaw and pitch required to turn an object to face a point in space.  Assumes local object heading is [0 0 1] and performs
// test in local object coordinate space.  Both output values are [0.0-1.0] turn percentages
XMFLOAT2 DetermineYawAndPitchToTarget(const FXMVECTOR position, const FXMVECTOR target, const FXMVECTOR invOrientation)
{
	// Determine the difference vector to this target, transform into local coordinate space (where our heading is the basis
	// vector [0, 0, 1], for mathematical simplicity) and normalise the difference vector
	XMVECTOR tgt = XMVector3Rotate(XMVectorSubtract(target, position), invOrientation);
	tgt = XMVector3NormalizeEst(tgt);		

	// Calculate the cross and dot products for ship yaw
	/*
	heading = BASIS_VECTOR;
	tgt = (s2->Location - ss->Location);	// (tgt is then transformed by the inverse ship orientation matrix)

	Optimisation: we know heading = the basis vector, so can substitute components for 0,0,1 and simplify accordingly
	ycross = (heading.z*tgt.x) - (heading.x*tgt.z);		= (1*tgt.x) - (0*tgt.z)		= tgt.x
	ydot = (heading.z*tgt.z) + (heading.x*tgt.x);		= (1*tgt.z) + (0*tgt.x)		= tgt.z
	pcross = (heading.y*tgt.z) - (heading.z*tgt.y);		= (0*tgt.z) - (1*tgt.y)		= -tgt.y
	pdot = (heading.x*tgt.x) + (heading.z*tgt.z);		= (0*tgt.x) + (1*tgt.z)		= tgt.z

	We therefore don't need to even maintain heading as a variable.  We can also just use tgt components in place of cross/dot
	*/

	XMFLOAT3 tgt_f;
	XMStoreFloat3(&tgt_f, tgt);

	// Determine yaw value depending on the current angle to target
	// outPitchYaw.x == pitch, outPitchYaw = yaw
	XMFLOAT2 result;
	if (fast_abs(tgt_f.x) > 0.01f)	result.y = tgt_f.x;		// Plot a yaw component proportionate to the angle the ship needs to cover
	else {
		if (tgt_f.z < 0.0f)			result.y = -1.0f;			// We are over 180deg from the target, so perform a full turn
		else						result.y = 0.0f;			// We are on the correct heading so maintain yaw
	}

	// Now determine pitch value, also based on the current angle to target
	if (fast_abs(tgt_f.y) > 0.01f)	result.x = -tgt_f.y;		// Plot a pitch component proportionate to the angle the ship needs to cover
	else {
		if (tgt_f.z < 0.0f)			result.x = -1.0f;			// We are over 180deg from the target, so perform a full turn
		else						result.x = 0.0f;			// We are on the correct heading so maintain pitch
	}

	return result;
}


// Determines the yaw and pitch required to turn an object to face a point in space.  Assumes local object heading is [0 0 1] and performs
// test in local object coordinate space.  Both output values are [0.0-1.0] turn percentages
XMFLOAT2 DetermineYawAndPitchToTarget(const FXMVECTOR position, const FXMVECTOR target, const CXMMATRIX invOrientMatrix)
{
	// Determine the difference vector to this target, transform into local coordinate space (where our heading is the basis
	// vector [0, 0, 1], for mathematical simplicity) and normalise the difference vector
	XMVECTOR tgt = XMVector3TransformCoord(XMVectorSubtract(target, position), invOrientMatrix);
	tgt = XMVector3NormalizeEst(tgt);		

	// Calculate the cross and dot products for ship yaw
	/*
	heading = BASIS_VECTOR;
	tgt = (s2->Location - ss->Location);	// (tgt is then transformed by the inverse ship orientation matrix)

	Optimisation: we know heading = the basis vector, so can substitute components for 0,0,1 and simplify accordingly
	ycross = (heading.z*tgt.x) - (heading.x*tgt.z);		= (1*tgt.x) - (0*tgt.z)		= tgt.x
	ydot = (heading.z*tgt.z) + (heading.x*tgt.x);		= (1*tgt.z) + (0*tgt.x)		= tgt.z
	pcross = (heading.y*tgt.z) - (heading.z*tgt.y);		= (0*tgt.z) - (1*tgt.y)		= -tgt.y
	pdot = (heading.x*tgt.x) + (heading.z*tgt.z);		= (0*tgt.x) + (1*tgt.z)		= tgt.z

	We therefore don't need to even maintain heading as a variable.  We can also just use tgt components in place of cross/dot
	*/

	XMFLOAT3 tgt_f;
	XMStoreFloat3(&tgt_f, tgt);

	// Determine yaw value depending on the current angle to target
	// outPitchYaw.x == pitch, outPitchYaw = yaw
	XMFLOAT2 result;
	if (fast_abs(tgt_f.x) > 0.01f)	result.y = tgt_f.x;			// Plot a yaw component proportionate to the angle the ship needs to cover
	else {
		if (tgt_f.z < 0.0f)			result.y = -1.0f;				// We are over 180deg from the target, so perform a full turn
		else						result.y = 0.0f;				// We are on the correct heading so maintain yaw
	}

	// Now determine pitch value, also based on the current angle to target
	if (fast_abs(tgt_f.y) > 0.01f)	result.x = -tgt_f.y;			// Plot a pitch component proportionate to the angle the ship needs to cover
	else {
		if (tgt_f.z < 0.0f)			result.x = -1.0f;				// We are over 180deg from the target, so perform a full turn
		else						result.x = 0.0f;				// We are on the correct heading so maintain pitch
	}

	return result;
}

// Determines the yaw and pitch required to turn an object to face a point in space.  Assumes local object heading is [0 0 1] and performs
// test in local object coordinate space.  Both output values are [0.0-1.0] turn percentages
XMFLOAT2 DetermineYawAndPitchToWorldVector(const FXMVECTOR target_vector, const FXMVECTOR object_inv_orient)
{
	// Transform the target vector into local coordinate space (where the parent object heading is the basis
	// vector [0, 0, 1], for mathematical simplicity) and normalise the resulting vector
	XMVECTOR tgt = XMVector3NormalizeEst(XMVector3Rotate(target_vector, object_inv_orient));

	// Calculate the cross and dot products for ship yaw
	/*
	heading = BASIS_VECTOR;
	tgt = (s2->Location - ss->Location);	// (tgt is then transformed by the inverse ship orientation matrix)

	Optimisation: we know heading = the basis vector, so can substitute components for 0,0,1 and simplify accordingly
	ycross = (heading.z*tgt.x) - (heading.x*tgt.z);		= (1*tgt.x) - (0*tgt.z)		= tgt.x
	ydot = (heading.z*tgt.z) + (heading.x*tgt.x);		= (1*tgt.z) + (0*tgt.x)		= tgt.z
	pcross = (heading.y*tgt.z) - (heading.z*tgt.y);		= (0*tgt.z) - (1*tgt.y)		= -tgt.y
	pdot = (heading.x*tgt.x) + (heading.z*tgt.z);		= (0*tgt.x) + (1*tgt.z)		= tgt.z

	We therefore don't need to even maintain heading as a variable.  We can also just use tgt components in place of cross/dot
	*/

	XMFLOAT3 tgt_f;
	XMStoreFloat3(&tgt_f, tgt);

	// Determine yaw value depending on the current angle to target
	// outPitchYaw.x == pitch, outPitchYaw = yaw
	XMFLOAT2 result;
	if (fast_abs(tgt_f.x) > 0.01f)	result.y = tgt_f.x;		// Plot a yaw component proportionate to the angle the ship needs to cover
	else {
		if (tgt_f.z < 0.0f)			result.y = -1.0f;			// We are over 180deg from the target, so perform a full turn
		else						result.y = 0.0f;			// We are on the correct heading so maintain yaw
	}

	// Now determine pitch value, also based on the current angle to target
	if (fast_abs(tgt_f.y) > 0.01f)	result.x = -tgt_f.y;		// Plot a pitch component proportionate to the angle the ship needs to cover
	else {
		if (tgt_f.z < 0.0f)			result.x = -1.0f;			// We are over 180deg from the target, so perform a full turn
		else						result.x = 0.0f;			// We are on the correct heading so maintain pitch
	}

	return result;
}

XMVECTOR OrientationFromPitchYawRollVector(const FXMVECTOR vec)
{
	XMFLOAT3 vecf; XMStoreFloat3(&vecf, vec);
	return (XMQuaternionMultiply(XMQuaternionMultiply(
		XMQuaternionRotationNormal(UP_VECTOR, vecf.y),
		XMQuaternionRotationNormal(RIGHT_VECTOR, vecf.x)),
		XMQuaternionRotationNormal(FORWARD_VECTOR, vecf.z)
	));
}


#endif