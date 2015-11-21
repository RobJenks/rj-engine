//#pragma once
//
//#ifndef __QuaternionH__
//#define __QuaternionH__
//
//#include "DX11_Core.h"
//#include "FastMath.h"
//#include "CompilerSettings.h"
//
//typedef struct Quaternion
//{
//	public:
//		Quaternion() {};
//		Quaternion(float w, float x, float y, float z);
//		Quaternion(const Quaternion&);
//
//		Quaternion	operator * (const Quaternion&) const;
//		D3DXVECTOR3 operator * (const D3DXVECTOR3 &vec) const;
//		Quaternion& operator *= (const Quaternion &q);
//
//		Quaternion	GetConjugate() const;
//		void		Normalise();
//
//		void RotateX(const float theta);
//		void FastDRotateX(const int theta);
//		void RotateY(const float theta);
//		void FastDRotateY(const int theta);
//		void RotateZ(const float theta);
//		void FastDRotateZ(const int theta);
//		void RotateAxis(const float x, const float y, const float z, const float theta);
//		void FastDRotateAxis(const float x, const float y, const float z, const int theta);
//
//		void ToMatrix(D3DXMATRIX&);
//		void DeriveFromMatrix(const D3DXMATRIX&);
//
//		string ToString();
//
//		float w, x, y, z;
//
//} Quaternion;
//
//
//// The identity Quaternion; static const for reference only
//static const Quaternion Quaternion_IDENTITY = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
//
//
///* ********************** */
///* BEGIN INLINE FUNCTIONS */
///* ********************** */
//
//
//CMPINLINE 
//Quaternion::Quaternion(float fw, float fx, float fy, float fz)
//{
//	w = fw; x = fx; y = fy; z = fz;
//}
//
//CMPINLINE 
//Quaternion::Quaternion(const Quaternion &q)
//{
//	w = q.w; x = q.x; y = q.y; z = q.z;
//}
//
//CMPINLINE 
//Quaternion& Quaternion::operator *= (const Quaternion &q)
//{
//	(*this) = q * (*this);
//	return (*this);
//}
//
//CMPINLINE 
//Quaternion Quaternion::operator * (const Quaternion &q) const
//{
//	return Quaternion(w*q.w - x*q.x - y*q.y - z*q.z,
//					  w*q.x + x*q.w + y*q.z - z*q.y,
//					  w*q.y - x*q.z + y*q.w + z*q.x,
//					  w*q.z + x*q.y - y*q.x + z*q.w);
//}
//
//CMPINLINE 
//D3DXVECTOR3 Quaternion::operator * (const D3DXVECTOR3 &vec) const
//{
//	D3DXVECTOR3 vn(vec);
//	NormaliseVector(vn);
// 
//	Quaternion vecQuat, resQuat;
//	vecQuat.x = vn.x;
//	vecQuat.y = vn.y;
//	vecQuat.z = vn.z;
//	vecQuat.w = 0.0f;
// 
//	resQuat = vecQuat * GetConjugate();
//	resQuat = *this * resQuat;
// 
//	return (D3DXVECTOR3(resQuat.x, resQuat.y, resQuat.z));
//}
// 
//CMPINLINE Quaternion Quaternion::GetConjugate() const { return Quaternion(w, -x, -y, -z); }
//
//CMPINLINE void Quaternion::Normalise() 
//{
//	float norm = sqrt(w*w + x*x + y*y + z*z);
//	w /= norm;
//	x /= norm;
//	y /= norm;
//	z /= norm;
//}
//
//CMPINLINE
//void Quaternion::ToMatrix(D3DXMATRIX &m)
//{
//	m._11 = 1 - 2*y*y - 2*z*z;
//	m._12 = 2*x*y - 2*w*z;
//	m._13 = 2*x*z + 2*w*y;
//	m._14 = 0;
//
//	m._21 = 2*x*y + 2*w*z;
//	m._22 = 1 - 2*x*x - 2*z*z;
//	m._23 = 2*y*z - 2*w*x;
//	m._24 = 0;
//
//	m._31 = 2*x*z - 2*w*y;
//	m._32 = 2*y*z - 2*w*x;
//	m._33 = 1 - 2*x*x - 2*y*y;
//	m._34 = 0;
//
//	m._41 = 0;
//	m._42 = 0;
//	m._43 = 0;
//	m._44 = 1;
//
//	// TODO: Generate the transpose matrix.  Can do this by simply rearranging term assignment above?
//	D3DXMatrixTranspose(&m, &m);
//}
//
//CMPINLINE
//void Quaternion::DeriveFromMatrix(const D3DXMATRIX &m)
//{
//	w = sqrt( max( 0, 1 + m._11 + m._22 + m._33 ) ) / 2;
//	x = sqrt( max( 0, 1 + m._11 - m._22 - m._33 ) ) / 2;
//	y = sqrt( max( 0, 1 - m._11 + m._22 - m._33 ) ) / 2;
//	z = sqrt( max( 0, 1 - m._11 - m._22 + m._33 ) ) / 2;
//	
//	x = (float)_copysign( x, m._32 - m._21 );
//	y = (float)_copysign( y, m._13 - m._31 );
//	z = (float)_copysign( z, m._21 - m._12 );
//}
//
//CMPINLINE 
//void Quaternion::RotateX(const float theta)
//{
//	float halft = theta / 2.0f;
//	w = cos(halft);
//	x = sin(halft);
//	y = 0; z = 0;
//}
//
////********************************************************************
//
//CMPINLINE 
//void Quaternion::RotateY(const float theta)
//{
//	float halft = theta / 2.0f;
//	w = cos(halft);
//	y = sin(halft);
//	x = 0; z = 0;
//}
//
//CMPINLINE 
//void Quaternion::FastDRotateY(const int theta)
//{
//	if (theta % 2 == 0) { 
//		int halft = (theta >> 1);
//		w = _cos(halft);
//		y = _sin(halft);
//	} else {
//		float halft = (float)theta / 2.0f;
//		w = cos(halft);
//		y = sin(halft);
//	}
//
//	x = 0; z = 0;
//}
//
//CMPINLINE 
//void Quaternion::RotateZ(const float theta)
//{
//	float halft = theta / 2.0f;
//	w = cos(halft);
//	z = sin(halft);
//	x = 0; y = 0;
//}
//
//CMPINLINE 
//void Quaternion::FastDRotateZ(const int theta)
//{
//	if (theta % 2 == 0) { 
//		int halft = (theta >> 1);
//		w = _cos(halft);
//		z = _sin(halft);
//	} else {
//		float halft = theta / 2.0f;
//		w = cos(halft);
//		z = sin(halft);
//	}
//
//	x = 0; y = 0;
//}
//
//CMPINLINE 
//void Quaternion::RotateAxis(const float ax, const float ay, const float az, const float theta)
//{
//	float halft = theta / 2.0f;
//	float tmp = sin(halft);
//	w = cos(halft);
//	x = ax * tmp;
//	y = ay * tmp;
//	z = az * tmp;
//}
//
//CMPINLINE 
//void Quaternion::FastDRotateAxis(const float ax, const float ay, const float az, const int theta)
//{
//	float tmp;
//
//	if (theta % 2 == 0) { 
//		int halft = (theta >> 1);
//		tmp = _sin(halft);
//		w = _cos(halft);
//	} else {
//		float halft = theta / 2.0f;
//		tmp = sin(halft);
//		w = cos(halft);
//	}
//
//	x = ax * tmp; y = ay * tmp; z = az * tmp;
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//#endif