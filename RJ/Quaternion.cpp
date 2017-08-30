//#pragma once
//#ifndef __QuaternionC__
//#define __QuaternionC__
//
//#include <string>
//
//#include "Utility.h"
//
//
//#include "Quaternion.h"
//
//// TODO: RE-INLINE
//void Quaternion::FastDRotateX(const int theta)
//{
//	if (theta % 2 == 0) { 
//		int halft = (theta >> 1);
//		w = _cos(halft);
//		x = _sin(halft);
//	} else {
//		float halft = theta / 2.0f;
//		w = cos(halft);
//		x = sin(halft);
//	}
//
//	y = 0; z = 0;
//}
//
//string Quaternion::ToString()
//{
//	return concat("(")(w)(", ")(x)(", ")(y)(", ")(z).str();
//}
//
//
//
//
//
//#endif
//
//
//
