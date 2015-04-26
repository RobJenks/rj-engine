#pragma once
#ifndef __FastTrigC__
#define __FastTrigC__

#include "math.h"
#include "malloc.h"
#include <time.h>
#include <random>
#include "GameVarsExtern.h"

#include "FastMath.h"


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
	D3DXQuaternionRotationYawPitchRoll(&(ROT_QUATERNIONS[0]), 0.0f,			0.0f, 0.0f);
	D3DXQuaternionRotationYawPitchRoll(&(ROT_QUATERNIONS[1]), PIOVER2,		0.0f, 0.0f);
	D3DXQuaternionRotationYawPitchRoll(&(ROT_QUATERNIONS[2]), PI,			0.0f, 0.0f);
	D3DXQuaternionRotationYawPitchRoll(&(ROT_QUATERNIONS[3]), PI + PIOVER2, 0.0f, 0.0f);

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

D3DXVECTOR3 *D3DXVec3Rotate( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXQUATERNION *pQ )
{    
	D3DXQUATERNION q1, q2;    
	D3DXQuaternionConjugate( &q1, pQ );    
	q2 = q1 * D3DXQUATERNION( pV->x, pV->y, pV->z, 1.0f ) * *pQ;	
	
	*pOut = D3DXVECTOR3( q2.x, q2.y, q2.z );    
	return pOut;
}

const D3DXMATRIX *GetRotationMatrix(Rotation90Degree rot)
{
	switch (rot)
	{
		case Rotation90Degree::Rotate90:		return &ROT_MATRIX_90;		break;
		case Rotation90Degree::Rotate180:		return &ROT_MATRIX_180;		break;
		case Rotation90Degree::Rotate270:		return &ROT_MATRIX_270;		break;
		default:								return &ROT_MATRIX_0;		break;
	}
}

// Returns a quaternion that represents the rotation value specified
const D3DXQUATERNION & GetRotationQuaternion(Rotation90Degree rot)
{
	switch (rot)
	{
		case Rotation90Degree::Rotate90:		return ROT_QUATERNIONS[1];	break;
		case Rotation90Degree::Rotate180:		return ROT_QUATERNIONS[2];	break;
		case Rotation90Degree::Rotate270:		return ROT_QUATERNIONS[3];	break;
		default:								return ROT_QUATERNIONS[0];	break;
	}
}

D3DXMATRIX GetRotationMatrixInstance(Rotation90Degree rot)
{
	switch (rot)
	{
		case Rotation90Degree::Rotate0:			return ROT_MATRIX_0;		break;
		case Rotation90Degree::Rotate90:		return ROT_MATRIX_90;		break;
		case Rotation90Degree::Rotate180:		return ROT_MATRIX_180;		break;
		case Rotation90Degree::Rotate270:		return ROT_MATRIX_270;		break;
		default:								return ROT_MATRIX_0;		break;
	}
}


void FloorVector(D3DXVECTOR3 & vec, float low)
{
	vec.x = max(vec.x, low);
	vec.y = max(vec.y, low);
	vec.z = max(vec.z, low);
}

void FloorVector(D3DXVECTOR3 & vec, const D3DXVECTOR3 & low)
{
	vec.x = max(vec.x, low.x);
	vec.y = max(vec.y, low.y);
	vec.z = max(vec.z, low.z);
}

void CeilVector(D3DXVECTOR3 & vec, float high)
{
	vec.x = min(vec.x, high);
	vec.y = min(vec.y, high);
	vec.z = min(vec.z, high);
}

void CeilVector(D3DXVECTOR3 & vec, const D3DXVECTOR3 & high)
{
	vec.x = min(vec.x, high.x);
	vec.y = min(vec.y, high.y);
	vec.z = min(vec.z, high.z);
}

void ClampVector(D3DXVECTOR3 &vec, float low, float high)
{
	vec.x = max(min(vec.x, high), low);
	vec.y = max(min(vec.y, high), low);
	vec.z = max(min(vec.z, high), low);
}

void ClampVector(D3DXVECTOR3 & vec, const D3DXVECTOR3 & low, const D3DXVECTOR3 & high)
{
	vec.x = max(min(vec.x, high.x), low.x);
	vec.y = max(min(vec.y, high.y), low.y);
	vec.z = max(min(vec.z, high.z), low.z);
}

// Scales a vector to the specified 'magnitude', so that one component is at +/- 'magnitude' with all other components scaled accordingly
// Near-zero vectors will not be scaled to avoid div/0 errors.
void ScaleVectorToMagnitude(D3DXVECTOR3 &vec, float magnitude)
{
	float mx = max(max(fabs(vec.x), fabs(vec.y)), fabs(vec.z));
	if (mx > Game::C_EPSILON) vec *= (magnitude / mx);
}


// Scales a vector to ensure that the (absolute) value of any component does not exceed 'magnitude', with all other components scaled accordingly
void ScaleVectorWithinMagnitudeLimit(D3DXVECTOR3 &vec, float magnitude)
{
	float mx = max(max(fabs(vec.x), fabs(vec.y)), fabs(vec.z));
	if (mx > magnitude) vec *= (magnitude / mx);
}

void QuaternionBetweenVectors(D3DXQUATERNION *pOutQuaternion, const D3DXVECTOR3 *v1, const D3DXVECTOR3 *v2)
{
	// http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors

	// Take the cross product
	D3DXVECTOR3 w;
	D3DXVec3Cross(&w, v1, v2);
	 
	// Build a quaternion based on the cross & dot products, and also incorporate the quaternion length
	D3DXQUATERNION q = D3DXQUATERNION(w.x, w.y, w.z, D3DXVec3Dot(v1, v2));
	q.w += D3DXQuaternionLength(&q);

	// Normalise the vector and send the result to the output quaternion
	D3DXQuaternionNormalize(pOutQuaternion, &q);
}

// Computes the approximate (but very close) inverse sqrt of a number VERY quickly.
// Requires use of 32-bit integer within the method for correct bitwise operations, so 
// set to use _int32 specifically.  Carmack.
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root
float fast_approx_invsqrt(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(__int32 *)&y;                      // evil floating point bit level hacking
	i = 0x5f3759df - (i >> 1);               // what the fuck?
	y = *(float *)&i;
	y = y * (threehalfs - (x2 * y * y));   // 1st iteration
	//  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}



#endif