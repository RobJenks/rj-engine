#pragma once

#ifndef __FastMathH__
#define __FastMathH__

#include <random>
#include <windows.h>
#include "DX11_Core.h"

#include "GameVarsExtern.h"
#include "CompilerSettings.h"

static const float PI = 3.14159265358979f;
static const float PIBY180 = 0.0174532925199433f;
static const float _180BYPI = 57.2957795130823208f;
static const float TWOPI = PI*2.0f;
static const float PIOVER2 = PI / 2.0f;
static const float ROOT2 = 1.414213562f;
static const float ONE_BY_ROOT2 = 1.0f / ROOT2;

#define DegToRad(x) ((x)*PIBY180)
#define RadToDeg(x) ((x)*_180BYPI)

#define DOT_2D(v0, v1) ((v0.x*v1.x) + (v0.y*v1.y))
#define DOT_3D(v0, v1) ((v0.x*v1.x) + (v0.y*v1.y) + (v0.z*v1.z))
#define DOTPERP_2D(v0, v1) ((v0.x*v1.y) + (v0.y*-v1.x))				// DotPerp(v0,v1) == Dot(v0, Perp(v1)), where Perp(u,v) == (v, -u).  [v1.x=v1.y, v1.y=-v1.x]

static const INTVECTOR2 NULL_INTVECTOR2 = INTVECTOR2(0, 0);
static const INTVECTOR3 NULL_INTVECTOR3 = INTVECTOR3(0, 0, 0);
static const XMVECTOR NULL_VECTOR = XMVectorZero(); 
static const XMVECTOR NULL_VECTOR2 = XMVectorZero();
static const XMVECTOR NULL_VECTOR3 = XMVectorZero();
static const XMVECTOR NULL_VECTOR4 = XMVectorZero();
static const XMVECTOR HALF_VECTOR_P = XMVectorReplicate(0.5f);
static const XMVECTOR HALF_VECTOR_N = XMVectorNegate(HALF_VECTOR_P);
static const XMVECTOR HALF_VECTOR = HALF_VECTOR_P;
static const XMVECTOR ONE_VECTOR = XMVectorReplicate(1.0f);
static const XMVECTOR LARGE_VECTOR_P = XMVectorReplicate(1e15f);
static const XMVECTOR LARGE_VECTOR_N = XMVectorReplicate(-1e15f);
static const XMVECTOR RAND_MAX_V = XMVectorReplicate(RAND_MAX);
static const XMFLOAT2 NULL_FLOAT2 = XMFLOAT2(0.0f, 0.0f);
static const XMFLOAT3 NULL_FLOAT3 = XMFLOAT3(0.0f, 0.0f, 0.0f);
static const XMFLOAT4 NULL_FLOAT4 = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
static const XMVECTOR BASIS_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
static const XMVECTOR BASIS_VECTOR_NEGZ = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
static const XMVECTOR BASIS_VECTOR2 = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
static const XMVECTOR BASIS_VECTOR2_NEGY = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
static const XMVECTOR UP_VECTOR = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
static const XMVECTOR RIGHT_VECTOR = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
static const XMVECTOR FORWARD_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
static const XMVECTOR ID_QUATERNION = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
static const XMFLOAT4 ID_QUATERNIONF = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
static const XMMATRIX NULL_MATRIX = XMMatrixSet( 0.0f, 0.0f, 0.0f, 0.0f, 
												 0.0f, 0.0f, 0.0f, 0.0f,
												 0.0f, 0.0f, 0.0f, 0.0f, 
												 0.0f, 0.0f, 0.0f, 0.0f);
static const XMMATRIX ID_MATRIX = XMMatrixSet(	 1.0f, 0.0f, 0.0f, 0.0f,
												 0.0f, 1.0f, 0.0f, 0.0f,
												 0.0f, 0.0f, 1.0f, 0.0f, 
												 0.0f, 0.0f, 0.0f, 1.0f);
static const XMMATRIX ELEMENT_SCALE_MATRIX = XMMatrixSet(	Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f, 0.0f,
															0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f,
															0.0f, 0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f,
															0.0f, 0.0f, 0.0f, 1.0f);

static const XMFLOAT4X4 ID_MATRIX_F = XMFLOAT4X4(	1.0f, 0.0f, 0.0f, 0.0f,
													0.0f, 1.0f, 0.0f, 0.0f,
													0.0f, 0.0f, 1.0f, 0.0f,
													0.0f, 0.0f, 0.0f, 1.0f);


static const float SIN_ZERO = sinf(0.0f);
static const float COS_ZERO = cosf(0.0f);

// Random number generation functions
// TODO: Replace with better-optimised versions?
#define frand()			((float)rand()/(float)RAND_MAX)
#define frand_h(h)		((float)rand()/((float)RAND_MAX/(h)))
#define frand_lh(l,h)	((l) + (float)rand()/((float)RAND_MAX/((h)-(l))))


#define TRIG_TABLE_SIZE 360

void				InitialiseMathFunctions();
void				TerminateMathFunctions();

XMVECTOR			ScaleVector3ToMagnitude(FXMVECTOR vec, float magnitude);
void				ScaleVector3ToMagnitude(XMFLOAT3 & vec, float magnitude);
XMVECTOR			ScaleVector3WithinMagnitudeLimit(FXMVECTOR vec, float magnitude);
void				ScaleVector3WithinMagnitudeLimit(XMFLOAT3 &vec, float magnitude);
CMPINLINE bool		IsZeroVector2(const FXMVECTOR vec) { return XMVector2NearEqual(vec, NULL_VECTOR2, Game::C_EPSILON_V); }
CMPINLINE bool		IsZeroVector3(const FXMVECTOR vec) { return XMVector3NearEqual(vec, NULL_VECTOR3, Game::C_EPSILON_V); }
CMPINLINE bool		IsZeroVector4(const FXMVECTOR vec) { return XMVector4NearEqual(vec, NULL_VECTOR4, Game::C_EPSILON_V); }
CMPINLINE bool		IsZeroQuaternion(const FXMVECTOR q) { return XMVector4NearEqual(q, NULL_VECTOR4, Game::C_EPSILON_V); }
CMPINLINE bool		IsIDQuaternion(const FXMVECTOR q) { return XMVector4NearEqual(q, ID_QUATERNION, Game::C_EPSILON_V); }
CMPINLINE bool		IsFiniteVector2(const FXMVECTOR vec) { return XMVector2Equal(vec, vec); }
CMPINLINE bool		IsFiniteVector3(const FXMVECTOR vec) { return XMVector3Equal(vec, vec); }
CMPINLINE bool		IsFiniteVector4(const FXMVECTOR vec) { return XMVector4Equal(vec, vec); }
CMPINLINE bool		IsFiniteQuaternion(const FXMVECTOR q) { return XMVector4Equal(q, q); }
void				Clamp (float &v, float min, float max);
float				Clamp (float v, float min, float max);
XMVECTOR			FloorVector(FXMVECTOR vec, float low);
XMVECTOR			FloorVector(FXMVECTOR vec, const FXMVECTOR low);
XMVECTOR			CeilVector(FXMVECTOR vec, float high);
XMVECTOR			CeilVector(FXMVECTOR vec, const FXMVECTOR high);
XMVECTOR			ClampVector(FXMVECTOR vec, float low, float high);
XMVECTOR			ClampVector(FXMVECTOR vec, const FXMVECTOR low, const FXMVECTOR high);
CMPINLINE XMVECTOR	VectorMin(FXMVECTOR vec, const FXMVECTOR minvalues) { return CeilVector(vec, minvalues); }
CMPINLINE XMVECTOR	VectorMax(FXMVECTOR vec, const FXMVECTOR maxvalues) { return FloorVector(vec, maxvalues); }
unsigned int		fast_sign(const float& v);
float				fast_abs(float v);
float				fast_approx_invsqrt(float number);
CMPINLINE float		GetRollAmount(const FXMVECTOR up, const FXMVECTOR right) { return atan2f(XMVectorGetY(up), XMVectorGetY(right)) - PIOVER2; }
CMPINLINE float		GetRollAmount(const XMFLOAT3 & up, const XMFLOAT3 & right) { return atan2f(up.y, right.y) - PIOVER2; }
CMPINLINE float		GetRollAmount(float upY, float rightY) { return atan2f(upY, rightY) - PIOVER2; }
XMVECTOR			QuaternionBetweenVectors(const FXMVECTOR v1, const FXMVECTOR v2);
XMFLOAT4			QuaternionAdd(const XMFLOAT4 & q1, const XMFLOAT4 & q2);
void				QuaternionAdd(const XMFLOAT4 & q1, const XMFLOAT4 & q2, XMFLOAT4 & outSum);
XMFLOAT4			QuaternionMultiply(const XMFLOAT4 & q1, const XMFLOAT4 & q2);
void				QuaternionMultiply(const XMFLOAT4 & q1, const XMFLOAT4 & q2, XMFLOAT4 & outProduct);
void				QuaternionNormalise(XMFLOAT4 & q);
void				QuaternionNormalise(const XMFLOAT4 & q, XMFLOAT4 & outQNorm);
CMPINLINE XMVECTOR	VectorFromIntVector2(const INTVECTOR2 & v) { return XMVectorSet((float)v.x, (float)v.y, 0.0f, 0.0f); }
CMPINLINE XMVECTOR	VectorFromIntVector3(const INTVECTOR3 & v) { return XMVectorSet((float)v.x, (float)v.y, (float)v.z, 0.0f); }
CMPINLINE XMVECTOR	VectorFromIntVector3SwizzleYZ(const INTVECTOR3 & v) { return XMVectorSet((float)v.x, (float)v.z, (float)v.y, 0.0f); }
CMPINLINE void		Vector3ToIntVector(const FXMVECTOR vec, INTVECTOR3 & outVec);
CMPINLINE void		Vector3ToIntVectorSwizzleYZ(const FXMVECTOR vec, INTVECTOR3 & outVec);
XMVECTOR			Vector2Random(void);
XMVECTOR			Vector3Random(void);
XMVECTOR			Vector4Random(void);
XMVECTOR			Vector2Random(float rmin, float rmax);
XMVECTOR			Vector3Random(float rmin, float rmax);
XMVECTOR			Vector4Random(float rmin, float rmax);
XMVECTOR			Vector2Random(const FXMVECTOR vmin, const FXMVECTOR vmax);
XMVECTOR			Vector3Random(const FXMVECTOR vmin, const FXMVECTOR vmax);
XMVECTOR			Vector4Random(const FXMVECTOR vmin, const FXMVECTOR vmax);

CMPINLINE bool		Float3NearEqual(const XMFLOAT3 & v1, const XMFLOAT3 & v2)
{
	return (fabs(v1.x - v2.x) < Game::C_EPSILON && fabs(v1.y - v2.y) < Game::C_EPSILON && fabs(v1.z - v2.z) < Game::C_EPSILON);
}
CMPINLINE bool		IsZeroFloat3(const XMFLOAT3 &v) 
{ 
	return (fabs(v.x) < Game::C_EPSILON && fabs(v.y) < Game::C_EPSILON && fabs(v.z) < Game::C_EPSILON);
}

CMPINLINE XMFLOAT3	Float3Add(const XMFLOAT3 & v1, const XMFLOAT3 & v2)
{
	return XMFLOAT3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

CMPINLINE XMFLOAT2	Float2Subtract(const XMFLOAT2 & v1, const XMFLOAT2 & v2)
{
	return XMFLOAT2(v1.x - v2.x, v1.y - v2.y);
}
CMPINLINE XMFLOAT3	Float3Subtract(const XMFLOAT3 & v1, const XMFLOAT3 & v2)
{
	return XMFLOAT3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

CMPINLINE XMFLOAT3	Float3MultiplyScalar(const XMFLOAT3 & v, const float s)
{
	return XMFLOAT3(v.x * s, v.y * s, v.z * s);
}

CMPINLINE void Vector3ToIntVector(const FXMVECTOR vec, INTVECTOR3 & outVec)
{
	XMFLOAT3 vecf; XMStoreFloat3(&vecf, vec);
	outVec.x = (int)vecf.x; outVec.y = (int)vecf.y; outVec.z = (int)vecf.z;
}
CMPINLINE void Vector3ToIntVectorSwizzleYZ(const FXMVECTOR vec, INTVECTOR3 & outVec)
{
	XMFLOAT3 vecf; XMStoreFloat3(&vecf, vec);
	outVec.x = (int)vecf.x; outVec.y = (int)vecf.z; outVec.z = (int)vecf.y;
}

CMPINLINE XMVECTOR CalculateRotationBetweenQuaternions(const FXMVECTOR qStart, const FXMVECTOR qEnd)
{
	// qRot = qEnd * Inverse(qStart)
	return XMQuaternionMultiply(qEnd, XMQuaternionInverse(qStart));

	/*D3DXQUATERNION invStart;
	D3DXQuaternionInverse(&invStart, &qStart);
	outQuaternion = qEnd * invStart;*/
}

// Calculates the angle of rotation represented by a quaternion.  Requires both sqrt and atan2 so avoid using frequently
CMPINLINE float CalculateQuaternionRotationAngle(const FXMVECTOR q)
{
	//float length = (q.x*q.x + q.y*q.y + q.z*q.z); // D3DXQuaternionLength(&q);
	float length = XMVectorGetX(XMVector3Length(q));

	return (2.0f * atan2f(length, XMVectorGetW(q)));
}

//CMPINLINE void RotateVectorByQuaternion(const D3DXVECTOR3 & v, const D3DXQUATERNION & q, D3DXVECTOR3 & outVPrime)
//{
//	// Extract the vector part of the quaternion, which will be used separately from the scalar (w) part
//	D3DXVECTOR3 u = D3DXVECTOR3(q.x, q.y, q.z);
//
//	// Calculate the rotate vector VPrime
//	D3DXVECTOR3 cross;
//	D3DXVec3Cross(&cross, &u, &v);
//	outVPrime = 2.0f * D3DXVec3Dot(&u, &v) * u
//		+ (q.w*q.w - D3DXVec3Dot(&u, &u)) * v
//		+ 2.0f * q.w * cross;
//}

//-----------------------------------------------------------------------------
// Return TRUE if any of the elements of a 3 vector are equal to 0xffffffff.
// Slightly more efficient than using XMVector3EqualInt.
//-----------------------------------------------------------------------------
CMPINLINE bool XMVector3AnyTrue(FXMVECTOR V)
{
	// Duplicate the fourth element from the first element.
	XMVECTOR C = XMVectorSwizzle(V, 0, 1, 2, 0);

	return XMComparisonAnyTrue(XMVector4EqualIntR(C, XMVectorTrueInt()));
}



//-----------------------------------------------------------------------------
// Return TRUE if all of the elements of a 3 vector are equal to 0xffffffff.
// Slightly more efficient than using XMVector3EqualInt.
//-----------------------------------------------------------------------------
CMPINLINE bool XMVector3AllTrue(FXMVECTOR V)
{
	// Duplicate the fourth element from the first element.
	XMVECTOR C = XMVectorSwizzle(V, 0, 1, 2, 0);

	return XMComparisonAllTrue(XMVector4EqualIntR(C, XMVectorTrueInt()));
}

float _sin(const int theta);
float _cos(const int theta);
float _tan(const int theta);

// Peforms a sqrt, with fast (lookup) efficiency for all positive integers to SQRT_CACHE_SIZE.  Cached version behaviour is undefined for <0 or >SQRT_CACHE_SIZE
#define SQRT_CACHE_SIZE 10000
extern float sqrt_cache[SQRT_CACHE_SIZE + 1];
CMPINLINE float fast_cached_sqrt(int n) { return sqrt_cache[n]; }
CMPINLINE float fast_sqrt(int n) { return (n <= SQRT_CACHE_SIZE ? sqrt_cache[n] : sqrtf((float)n)); }

extern std::tr1::ranlux_base_01						norm_reng;
extern std::tr1::normal_distribution<float>			norm_rdist;

#define NormalDist()								(norm_rdist(norm_reng))
#define NormalDistRange(l, h)						(l + (((norm_rdist(norm_reng)+4.0f)*(1.0f/8.0f)) * (h-l)))

// Fast rotation functions: precalculated matrices & function for common 90-degree rotations
XMVECTOR GetRotationQuaternion(Rotation90Degree rot); 
XMMATRIX GetRotationMatrix(Rotation90Degree rot);
const XMFLOAT4X4 * GetRotationMatrixF(Rotation90Degree rot);
XMFLOAT4X4 GetRotationMatrixInstance(Rotation90Degree rot);

static const XMMATRIX ROT_MATRIX_0 = XMMatrixSet(1.000000, 0.000000, -0.000000, 0.000000, 
												  0.000000, 1.000000, 0.000000, 0.000000, 
												  0.000000, 0.000000, 1.000000, 0.000000, 
												  0.000000, 0.000000, 0.000000, 1.000000  );
static const XMMATRIX ROT_MATRIX_90 = XMMatrixSet(-0.000000, 0.000000, -1.000000, 0.000000,
												   0.000000, 1.000000, 0.000000, 0.000000, 
												   1.000000, 0.000000, -0.000000, 0.000000, 
												   0.000000, 0.000000, 0.000000, 1.000000 );
static const XMMATRIX ROT_MATRIX_180 = XMMatrixSet(-1.000000, 0.000000, 0.000000, 0.000000,
													0.000000, 1.000000, 0.000000, 0.000000, 
													-0.000000, 0.000000, -1.000000, 0.000000, 
													0.000000, 0.000000, 0.000000, 1.000000 );
static const XMMATRIX ROT_MATRIX_270 = XMMatrixSet(0.000000, 0.000000, 1.000000, 0.000000,
													0.000000, 1.000000, 0.000000, 0.000000, 
													-1.000000, 0.000000, 0.000000, 0.000000, 
													0.000000, 0.000000, 0.000000, 1.000000 );
static const XMFLOAT4X4 ROT_MATRIX_0_F = XMFLOAT4X4(1.000000, 0.000000, -0.000000, 0.000000,
													0.000000, 1.000000, 0.000000, 0.000000,
													0.000000, 0.000000, 1.000000, 0.000000,
													0.000000, 0.000000, 0.000000, 1.000000);
static const XMFLOAT4X4 ROT_MATRIX_90_F = XMFLOAT4X4(-0.000000, 0.000000, -1.000000, 0.000000,
													0.000000, 1.000000, 0.000000, 0.000000,
													1.000000, 0.000000, -0.000000, 0.000000,
													0.000000, 0.000000, 0.000000, 1.000000);
static const XMFLOAT4X4 ROT_MATRIX_180_F = XMFLOAT4X4(-1.000000, 0.000000, 0.000000, 0.000000,
													0.000000, 1.000000, 0.000000, 0.000000,
													-0.000000, 0.000000, -1.000000, 0.000000,
													0.000000, 0.000000, 0.000000, 1.000000);
static const XMFLOAT4X4 ROT_MATRIX_270_F = XMFLOAT4X4(0.000000, 0.000000, 1.000000, 0.000000,
													0.000000, 1.000000, 0.000000, 0.000000,
													-1.000000, 0.000000, 0.000000, 0.000000,
													0.000000, 0.000000, 0.000000, 1.000000);

static const XMMATRIX* ROT_MATRICES[4] = { &ROT_MATRIX_0, &ROT_MATRIX_90, &ROT_MATRIX_180, &ROT_MATRIX_270 };
static XMVECTOR ROT_QUATERNIONS[4];

// Orthornormal unit basis vectors
static const XMVECTOR UNIT_BASES[3] = { XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f) };


// Uses the Padé approximation to avoid sqrt, where the approximation holds, within a certain tolerance 
// of the actual result.  If outside that tolerance then perform the sqrt to retain acceptable precision
// Most applicable where normalisation is performed very frequently, otherwise diverges from tolerance.
/*CMPINLINE XMVECTOR FastNormaliseQuaternion(const FXMVECTOR q)
{
	float magsq = (q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	if (abs(1.0f - magsq) < 2.107342e-08)
		q *= (2.0f / (1.0f + magsq));
	else
		q *= (1.0f / sqrt(magsq));
}*/

CMPINLINE void Clamp (float &v, float min, float max)
{
	if		(v > max)	v = max;
	else if (v < min)	v = min;
}

CMPINLINE float Clamp (float v, float min, float max)
{
	if		(v > max)	return max;
	else if (v < min)	return min;
	return v;
}

// Fast determination of floating point sign
CMPINLINE unsigned int fast_sign(const float& v)
{	
	return (((unsigned int&)v) & 0x80000000);
}

// Fast floating point absolute value.
CMPINLINE float fast_abs(float v)
{
	*(int *)&v &= 0x7fffffff;
	return v;
}

// Calculate the inverse transpose of a matrix
CMPINLINE XMMATRIX MatrixInverseTranspose(FXMMATRIX M)
{
	// Inverse-transpose is just applied to normals.  So zero out 
	// translation row so that it doesn't get into our inverse-transpose
	// calculation--we don't want the inverse-transpose of the translation.
	XMMATRIX A = M;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR det = XMMatrixDeterminant(A);
	return XMMatrixTranspose(XMMatrixInverse(&det, A));
}

// Returns a vector with random components in the range [0 1)
XMVECTOR			Vector2Random(void)	{ return XMVectorSet(frand(), frand(), 0.0f, 0.0f); }
// Returns a vector with random components in the range [0 1)
XMVECTOR			Vector3Random(void) { return XMVectorSet(frand(), frand(), frand(), 0.0f); }
// Returns a vector with random components in the range [0 1)
XMVECTOR			Vector4Random(void) { return XMVectorSet(frand(), frand(), frand(), frand()); }

// Returns a vector with random components in the range [rmin rmax)
XMVECTOR			Vector2Random(float rmin, float rmax)	{ return XMVectorSet(frand_lh(rmin, rmax), frand_lh(rmin, rmax), 0.0f, 0.0f); }
// Returns a vector with random components in the range [rmin rmax)
XMVECTOR			Vector3Random(float rmin, float rmax)	{ return XMVectorSet(frand_lh(rmin, rmax), frand_lh(rmin, rmax), frand_lh(rmin, rmax), 0.0f); }
// Returns a vector with random components in the range [rmin rmax)
XMVECTOR			Vector4Random(float rmin, float rmax)	{ return XMVectorSet(frand_lh(rmin, rmax), frand_lh(rmin, rmax), frand_lh(rmin, rmax), frand_lh(rmin, rmax)); }

// Returns a vector with random components in the range [rmin._ rmax._) for each component individually
// Vectorised version of "#define frand_lh(l,h)	((l) + (float)rand()/((float)RAND_MAX/((h)-(l))))"
XMVECTOR			Vector2Random(const FXMVECTOR vmin, const FXMVECTOR vmax)
{
	return XMVectorAdd(vmin, XMVectorDivide(Vector2Random(), XMVectorDivide(RAND_MAX_V, XMVectorSubtract(vmax, vmin))));
}
// Returns a vector with random components in the range [rmin._ rmax._) for each component individually
// Vectorised version of "#define frand_lh(l,h)	((l) + (float)rand()/((float)RAND_MAX/((h)-(l))))"
XMVECTOR			Vector3Random(const FXMVECTOR vmin, const FXMVECTOR vmax)
{
	return XMVectorAdd(vmin, XMVectorDivide(Vector3Random(), XMVectorDivide(RAND_MAX_V, XMVectorSubtract(vmax, vmin))));
}
// Returns a vector with random components in the range [rmin._ rmax._) for each component individually
// Vectorised version of "#define frand_lh(l,h)	((l) + (float)rand()/((float)RAND_MAX/((h)-(l))))"
XMVECTOR			Vector4Random(const FXMVECTOR vmin, const FXMVECTOR vmax)
{
	return XMVectorAdd(vmin, XMVectorDivide(Vector4Random(), XMVectorDivide(RAND_MAX_V, XMVectorSubtract(vmax, vmin))));
}

// Returns the squared diameter of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
CMPINLINE float DetermineCuboidBoundingSphereDiameterSq(const FXMVECTOR xyz)
{
	// Use Phythagorus; cube diameter will be the 3D diagonal across the cuboid; D^2 = (A^2 + B^2 + C^2)
	//return ((xyz.x * xyz.x) + (xyz.y * xyz.y) + (xyz.z * xyz.z));
	return XMVectorGetX(XMVector3LengthSq(xyz));
}

// Returns the diameter of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
CMPINLINE float DetermineCuboidBoundingSphereDiameter(const FXMVECTOR xyz)
{
	return sqrtf(DetermineCuboidBoundingSphereDiameterSq(xyz));
}

// Returns the squared radius of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
CMPINLINE float DetermineCuboidBoundingSphereRadiusSq(const FXMVECTOR xyz)
{
	// [d = 2r] > [d^2 = (2r)^2] > [d^2 = (2*r*2*r)] > [d^2 = 4(r^2)] > [r^2 = (d^2)/4]
	return (DetermineCuboidBoundingSphereDiameterSq(xyz) * 0.25f);
}

// Returns the radius of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
CMPINLINE float DetermineCuboidBoundingSphereRadius(const FXMVECTOR xyz)
{
	return sqrtf(DetermineCuboidBoundingSphereRadiusSq(xyz));
}

// Returns the squared diameter of a bounding sphere that completely encloses a cube with sides of the specified length
CMPINLINE float DetermineCubeBoundingSphereDiameterSq(float cube_length)
{
	// Use shortcut to Pythagorus for efficiency; where A==B==C, D^2 = (A^2 + B^2 + C^2) >>> D^2 = 3(A^2)
	cube_length *= cube_length;
	return (cube_length + cube_length + cube_length);
}

// Returns the diameter of a bounding sphere that completely encloses a cube with sides of the specified length
CMPINLINE float DetermineCubeBoundingSphereDiameter(float cube_length)
{
	return sqrt(DetermineCubeBoundingSphereDiameterSq(cube_length));
}

// Returns the squared radius of a bounding sphere that completely encloses a cube with sides of the specified length
CMPINLINE float DetermineCubeBoundingSphereRadiusSq(float cube_length)
{
	// [d = 2r] > [d^2 = (2r)^2] > [d^2 = (2*r*2*r)] > [d^2 = 4(r^2)] > [r^2 = (d^2)/4]
	return (DetermineCubeBoundingSphereDiameterSq(cube_length) * 0.25f);
}

// Returns the radius of a bounding sphere that completely encloses a cube with sides of the specified length
CMPINLINE float DetermineCubeBoundingSphereRadius(float cube_length)
{
	return sqrtf(DetermineCubeBoundingSphereRadiusSq(cube_length));
}

// Returns the radius of a bounding sphere that completely encloses a cube with sides of n ComplexShipElements in length
CMPINLINE float DetermineElementBoundingSphereRadius(int n_elements)
{
	return DetermineCubeBoundingSphereRadius(((float)n_elements) * Game::C_CS_ELEMENT_SCALE);
}

// Returns the radius of a sphere completely bounding a cube with sides of n ComplexShipElements in length
#define ELEMENT_BOUND_CACHE_SIZE 1000
extern float element_bound_cache[ELEMENT_BOUND_CACHE_SIZE + 1];
CMPINLINE float GetElementBoundingSphereRadius_Unchecked(int n_elements) { return element_bound_cache[n_elements]; }
CMPINLINE float GetElementBoundingSphereRadius(int n_elements)
{
	return ((n_elements >= 0 && n_elements <= ELEMENT_BOUND_CACHE_SIZE) ? element_bound_cache[n_elements] : DetermineElementBoundingSphereRadius(n_elements));
}

// Determines the yaw and pitch required to turn an object to face a point in space.  Assumes local object heading is [0 0 1] and performs
// test in local object coordinate space.  Both output values are [0.0-1.0] turn percentages
void DetermineYawAndPitchToTarget(const iObject *object, const FXMVECTOR target, float & outYaw, float & outPitch);
void DetermineYawAndPitchToTarget(const FXMVECTOR position, const FXMVECTOR target, const FXMVECTOR invOrientation, float & outYaw, float & outPitch);
void DetermineYawAndPitchToTarget(const FXMVECTOR position, const FXMVECTOR target, const CXMMATRIX invOrientMatrix, float & outYaw, float & outPitch);




#endif


/* Angle in 2D: 
dot = x1*x2 + y1*y2 + z1*z2
lenSq1 = x1*x1 + y1*y1 + z1*z1
lenSq2 = x2*x2 + y2*y2 + z2*z2
angle = acos(dot/sqrt(lenSq1 * lenSq2))
*/