#pragma once

#ifndef __FastMathH__
#define __FastMathH__

#include <random>
#include <windows.h>
#include "DX11_Core.h"

#include <xnamath.h>
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
static const D3DXVECTOR2 NULL_VECTOR2 = D3DXVECTOR2(0.0f, 0.0f);
static const D3DXVECTOR3 NULL_VECTOR = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
static const D3DXVECTOR4 NULL_VECTOR4 = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
static const D3DXVECTOR3 BASIS_VECTOR = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
static const D3DXVECTOR3 BASIS_VECTOR_NEGZ = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
static const D3DXVECTOR2 BASIS_VECTOR2 = D3DXVECTOR2(0.0f, 1.0f);
static const D3DXVECTOR2 BASIS_VECTOR2_NEGY = D3DXVECTOR2(0.0f, -1.0f);
static const D3DXVECTOR3 UP_VECTOR = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
static const D3DXVECTOR3 RIGHT_VECTOR = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
static const D3DXVECTOR3 FORWARD_VECTOR = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
static const D3DXQUATERNION ID_QUATERNION = D3DXQUATERNION(0.0f, 0.0f, 0.0f, 1.0f);
static const D3DXMATRIX NULL_MATRIX = D3DXMATRIX(0.0f, 0.0f, 0.0f, 0.0f, 
												 0.0f, 0.0f, 0.0f, 0.0f,
												 0.0f, 0.0f, 0.0f, 0.0f, 
												 0.0f, 0.0f, 0.0f, 0.0f);
static const D3DXMATRIX ID_MATRIX =	  D3DXMATRIX(1.0f, 0.0f, 0.0f, 0.0f, 
												 0.0f, 1.0f, 0.0f, 0.0f,
												 0.0f, 0.0f, 1.0f, 0.0f, 
												 0.0f, 0.0f, 0.0f, 1.0f);
static const D3DXMATRIX ELEMENT_SCALE_MATRIX = D3DXMATRIX(	Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f, 0.0f,
															0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f,
															0.0f, 0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f,
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

D3DXVECTOR3			*D3DXVec3Rotate( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXQUATERNION *pQ );

float				GetVectorMagnitude(const D3DXVECTOR3 &vec);
void				ScaleVectorToMagnitude(D3DXVECTOR3 &vec, float magnitude);
void				ScaleVectorWithinMagnitudeLimit(D3DXVECTOR3 &vec, float magnitude);
void				NormaliseVector(D3DXVECTOR3 &vec);
bool				IsZeroVector(const D3DXVECTOR2 &vec);
bool				IsZeroVector(const D3DXVECTOR3 &vec);
bool				IsZeroVector(const D3DXVECTOR4 &vec);
bool				IsZeroQuaternion(const D3DXQUATERNION &q);
bool				IsFiniteVector(const D3DXVECTOR2 &vec);
bool				IsFiniteVector(const D3DXVECTOR3 &vec);
bool				IsFiniteQuaternion(const D3DXQUATERNION &q);
void				Clamp (float &v, float min, float max);
float				Clamp (float v, float min, float max);
void				FloorVector(D3DXVECTOR3 & vec, float low); 
void				FloorVector(D3DXVECTOR3 & vec, const D3DXVECTOR3 & low);
void				CeilVector(D3DXVECTOR3 & vec, float high);
void				CeilVector(D3DXVECTOR3 & vec, const D3DXVECTOR3 & high);
void				ClampVector(D3DXVECTOR3 & vec, float low, float high);
void				ClampVector(D3DXVECTOR3 & vec, const D3DXVECTOR3 & low, const D3DXVECTOR3 & high);
CMPINLINE void		VectorMin(D3DXVECTOR3 & vec, const D3DXVECTOR3 & minvalues) { CeilVector(vec, minvalues); }
CMPINLINE void		VectorMax(D3DXVECTOR3 & vec, const D3DXVECTOR3 & maxvalues) { FloorVector(vec, maxvalues); }
unsigned int		fast_sign(const float& v);
float				fast_abs(float v);
float				fast_approx_invsqrt(float number);
CMPINLINE float		GetRollAmount(const D3DXVECTOR3 & up, const D3DXVECTOR3 & right) { return atan2f(up.y, right.y) - PIOVER2; }
void				QuaternionBetweenVectors(D3DXQUATERNION *pOutQuaternion, const D3DXVECTOR3 *v1, const D3DXVECTOR3 *v2);


CMPINLINE void CalculateRotationBetweenQuaternions(D3DXQUATERNION & outQuaternion, const D3DXQUATERNION & qStart, const D3DXQUATERNION  & qEnd)
{
	// qRot = qEnd * Inverse(qStart)
	D3DXQUATERNION invStart;
	D3DXQuaternionInverse(&invStart, &qStart);
	outQuaternion = qEnd * invStart;
}

// Calculates the angle of rotation represented by a quaternion.  Requires both sqrt and atan2 so avoid using frequently
CMPINLINE float CalculateQuaternionRotationAngle(const D3DXQUATERNION & q)
{
	float length = (q.x*q.x + q.y*q.y + q.z*q.z); // D3DXQuaternionLength(&q);
	return (2.0f * atan2f(length, q.w));
}

CMPINLINE void RotateVectorByQuaternion(const D3DXVECTOR3 & v, const D3DXQUATERNION & q, D3DXVECTOR3 & outVPrime)
{
	// Extract the vector part of the quaternion, which will be used separately from the scalar (w) part
	D3DXVECTOR3 u = D3DXVECTOR3(q.x, q.y, q.z);

	// Calculate the rotate vector VPrime
	D3DXVECTOR3 cross;
	D3DXVec3Cross(&cross, &u, &v);
	outVPrime = 2.0f * D3DXVec3Dot(&u, &v) * u
		+ (q.w*q.w - D3DXVec3Dot(&u, &u)) * v
		+ 2.0f * q.w * cross;
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
const D3DXMATRIX *GetRotationMatrix(Rotation90Degree rot);
const D3DXQUATERNION & GetRotationQuaternion(Rotation90Degree rot);
D3DXMATRIX GetRotationMatrixInstance(Rotation90Degree rot);
static const D3DXMATRIX ROT_MATRIX_0 = D3DXMATRIX(1.000000, 0.000000, -0.000000, 0.000000, 
												  0.000000, 1.000000, 0.000000, 0.000000, 
												  0.000000, 0.000000, 1.000000, 0.000000, 
												  0.000000, 0.000000, 0.000000, 1.000000  );
static const D3DXMATRIX ROT_MATRIX_90 = D3DXMATRIX(-0.000000, 0.000000, -1.000000, 0.000000, 
												   0.000000, 1.000000, 0.000000, 0.000000, 
												   1.000000, 0.000000, -0.000000, 0.000000, 
												   0.000000, 0.000000, 0.000000, 1.000000 );
static const D3DXMATRIX ROT_MATRIX_180 = D3DXMATRIX(-1.000000, 0.000000, 0.000000, 0.000000, 
													0.000000, 1.000000, 0.000000, 0.000000, 
													-0.000000, 0.000000, -1.000000, 0.000000, 
													0.000000, 0.000000, 0.000000, 1.000000 );
static const D3DXMATRIX ROT_MATRIX_270 = D3DXMATRIX(0.000000, 0.000000, 1.000000, 0.000000, 
													0.000000, 1.000000, 0.000000, 0.000000, 
													-1.000000, 0.000000, 0.000000, 0.000000, 
													0.000000, 0.000000, 0.000000, 1.000000 );
static const D3DXMATRIX* ROT_MATRICES[4] = { &ROT_MATRIX_0, &ROT_MATRIX_90, &ROT_MATRIX_180, &ROT_MATRIX_270 };
static D3DXQUATERNION ROT_QUATERNIONS[4];

// Orthornormal unit basis vectors
static const D3DXVECTOR3 UNIT_BASES[3] = { D3DXVECTOR3(1.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f,1.0f) };


/* ********************** */
/* BEGIN INLINE FUNCTIONS */
/* ********************** */

CMPINLINE float GetVectorMagnitude(const D3DXVECTOR3 &vec)
{
	return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

CMPINLINE void NormaliseVector(D3DXVECTOR3 &vec)
{
	vec /= GetVectorMagnitude(vec);
}

// Uses the Padé approximation to avoid sqrt, where the approximation holds, within a certain tolerance 
// of the actual result.  If outside that tolerance then perform the sqrt to retain acceptable precision
// Most applicable where normalisation is performed very frequently, otherwise diverges from tolerance.
CMPINLINE void FastNormaliseQuaternion(D3DXQUATERNION &q)
{
	float magsq = (q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	if (abs(1.0f - magsq) < 2.107342e-08)
		q *= (2.0f / (1.0f + magsq));
	else
		q *= (1.0f / sqrt(magsq));
}

CMPINLINE bool IsZeroVector(const D3DXVECTOR2 &vec)
{
	// Determine whether an approximated vector magnitude is greater than EPSILON.  Avoid the squaring and rooting
	// since this adds significant computation time, and at very tiny values around EPSILON have little impact.
	return ((abs(vec.x) + abs(vec.y)) < Game::C_EPSILON);
}
CMPINLINE bool IsZeroVector(const D3DXVECTOR3 &vec)
{
	// Determine whether an approximated vector magnitude is greater than EPSILON.  Avoid the squaring and rooting
	// since this adds significant computation time, and at very tiny values around EPSILON have little impact.
	return ((abs(vec.x) + abs(vec.y) + abs(vec.z)) < Game::C_EPSILON);
}
CMPINLINE bool IsZeroVector(const D3DXVECTOR4 &vec)
{
	// Determine whether an approximated vector magnitude is greater than EPSILON.  Avoid the squaring and rooting
	// since this adds significant computation time, and at very tiny values around EPSILON have little impact.
	return ((abs(vec.x) + abs(vec.y) + abs(vec.z) + abs(vec.w)) < Game::C_EPSILON);
}
CMPINLINE bool IsZeroQuaternion(const D3DXQUATERNION &q)
{
	// Determine whether an approximated vector magnitude is greater than EPSILON.  Avoid the squaring and rooting
	// since this adds significant computation time, and at very tiny values around EPSILON have little impact.
	return ((abs(q.x) + abs(q.y) + abs(q.z) + abs(q.w)) < Game::C_EPSILON);
}
CMPINLINE bool VectorsApproximatelyEqual(const D3DXVECTOR3 & vec1, const D3DXVECTOR3 & vec2)
{
	// Determine whether the vector difference is approximately zero
	return ((abs(vec1.x - vec2.x) < Game::C_EPSILON) && (abs(vec1.y - vec2.y) < Game::C_EPSILON) && (abs(vec1.z - vec2.z) < Game::C_EPSILON));
}
CMPINLINE bool IsFiniteVector(const D3DXVECTOR2 &vec)
{
	// Uses the fact that (x == x) holds trivially true for all finite numbers, but (#-1.IND000 != #-1.IND000)
	return ((vec.x == vec.x) && (vec.y == vec.y));
}
CMPINLINE bool IsFiniteVector(const D3DXVECTOR3 &vec)
{
	// Uses the fact that (x == x) holds trivially true for all finite numbers, but (#-1.IND000 != #-1.IND000)
	return ((vec.x == vec.x) && (vec.y == vec.y) && (vec.z == vec.z));
}
CMPINLINE bool IsFiniteVector(const D3DXVECTOR4 &vec)
{
	// Uses the fact that (x == x) holds trivially true for all finite numbers, but (#-1.IND000 != #-1.IND000)
	return ((vec.x == vec.x) && (vec.y == vec.y) && (vec.z == vec.z) && (vec.w == vec.w));
}
CMPINLINE bool IsFiniteQuaternion(const D3DXQUATERNION &q)
{
	// Uses the fact that (x == x) holds trivially true for all finite numbers, but (#-1.IND000 != #-1.IND000)
	return ((q.x == q.x) && (q.y == q.y) && (q.z == q.z) && (q.w == q.w));
}

CMPINLINE bool IsIDQuaternion(const D3DXQUATERNION &q)
{
	// Determine whether a quaternion is within epsilon of the identify quaternion in each component
	return ((abs(q.x - ID_QUATERNION.x) < Game::C_EPSILON) && (abs(q.y - ID_QUATERNION.y) < Game::C_EPSILON) && 
			(abs(q.z - ID_QUATERNION.z) < Game::C_EPSILON) && (abs(q.w - ID_QUATERNION.w) < Game::C_EPSILON));
}



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

// Take the dot product of the two 2D vectors (in the xy plane).  Yields a scalar
CMPINLINE float DotProduct2D(const D3DXVECTOR2 & v1, const D3DXVECTOR2 & v2)
{
	return ((v1.x * v2.x) + (v1.y * v2.y));
}

// Take the cross product of the two 2D vectors (in the xy plane).  Yields the z component of a third vector perpendicular to xy
CMPINLINE float CrossProduct2D(const D3DXVECTOR2 & v1, const D3DXVECTOR2 & v2)
{
	return ((v1.x * v2.y) - (v1.y * v2.x));
}

CMPINLINE float Angle2D(const D3DXVECTOR2 & v1, const D3DXVECTOR2 & v2)
{
	// Since the dot product is proportional to the cosine of an angle, and the determinant is proportional to its sine, we can use
	// the more efficient method [Angle = atan2(det, dot), where [det = x1*y2 - y1*x2] & [dot = x1*x2 + y1*y2]
	return atan2( (v1.x*v2.y - v1.y*v2.x), (v1.x*v2.x + v1.y*v2.y) );
}

CMPINLINE float Angle3D(const D3DXVECTOR3 & v1, const D3DXVECTOR3 & v2)
{
	// cos(angle) = dot(v1,v2) / (v1.length * v2.length)
	float denom = (D3DXVec3Length(&v1) * D3DXVec3Length(&v2));
	if (fabs(denom) < Game::C_EPSILON) return 0.0f;

	float cos_angle = D3DXVec3Dot(&v1, &v2) / denom;
	cos_angle = clamp(cos_angle, -1.0f, 1.0f);

	return acosf(cos_angle);
}

// Calculate the inverse transpose of a matrix
CMPINLINE XMMATRIX MatrixInverseTranspose(CXMMATRIX M)
{
	// Inverse-transpose is just applied to normals.  So zero out 
	// translation row so that it doesn't get into our inverse-transpose
	// calculation--we don't want the inverse-transpose of the translation.
	XMMATRIX A = M;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR det = XMMatrixDeterminant(A);
	return XMMatrixTranspose(XMMatrixInverse(&det, A));
}

// Returns the squared diameter of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
CMPINLINE float DetermineCuboidBoundingSphereDiameterSq(const D3DXVECTOR3 & xyz)
{
	// Use Phythagorus; cube diameter will be the 3D diagonal across the cuboid; D^2 = (A^2 + B^2 + C^2)
	return ((xyz.x * xyz.x) + (xyz.y * xyz.y) + (xyz.z * xyz.z));
}

// Returns the diameter of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
CMPINLINE float DetermineCuboidBoundingSphereDiameter(const D3DXVECTOR3 & xyz)
{
	return sqrtf(DetermineCuboidBoundingSphereDiameterSq(xyz));
}

// Returns the squared radius of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
CMPINLINE float DetermineCuboidBoundingSphereRadiusSq(const D3DXVECTOR3 & xyz)
{
	// [d = 2r] > [d^2 = (2r)^2] > [d^2 = (2*r*2*r)] > [d^2 = 4(r^2)] > [r^2 = (d^2)/4]
	return (DetermineCuboidBoundingSphereDiameterSq(xyz) * 0.25f);
}

// Returns the radius of a bounding sphere that completely encloses a cuboid with sides of length x/y/z
CMPINLINE float DetermineCuboidBoundingSphereRadius(const D3DXVECTOR3 & xyz)
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
void DetermineYawAndPitchToTarget(const iObject *object, const D3DXVECTOR3 & target, float & outYaw, float & outPitch);
void DetermineYawAndPitchToTarget(const D3DXVECTOR3 & position, const D3DXQUATERNION & invOrientation, const D3DXVECTOR3 & target, float & outYaw, float & outPitch);
void DetermineYawAndPitchToTarget(const D3DXVECTOR3 & position, const D3DXMATRIX *invOrientMatrix, const D3DXVECTOR3 & target, float & outYaw, float & outPitch);




#endif


/* Angle in 2D: 
dot = x1*x2 + y1*y2 + z1*z2
lenSq1 = x1*x1 + y1*y1 + z1*z1
lenSq2 = x2*x2 + y2*y2 + z2*z2
angle = acos(dot/sqrt(lenSq1 * lenSq2))
*/