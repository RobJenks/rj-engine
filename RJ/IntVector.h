#pragma once

#include <string>
#include <sstream>
#include "DX11_Core.h"


struct INTVECTOR2
{
	int x; int y;

	INTVECTOR2(void) { x = 0; y = 0; }
	INTVECTOR2(int _x, int _y) { x = _x; y = _y; }
	INTVECTOR2(float _x, float _y) { x = (int)_x; y = (int)_y; }
	INTVECTOR2(const XMFLOAT2 & v) { x = (int)v.x; y = (int)v.y; }
	INTVECTOR2(int _xy) { x = _xy; y = _xy; }						// For efficiency; allows setting both components to same value

	bool IsZeroVector(void) { return (x == 0 && y == 0); }
	std::string ToString(void) const { std::ostringstream s; s << "[" << x << ", " << y << ", " << "]"; return s.str(); }

	INTVECTOR2& operator +=(const INTVECTOR2& rhs) { this->x += rhs.x; this->y += rhs.y; return *this; }
	INTVECTOR2& operator -=(const INTVECTOR2& rhs) { this->x -= rhs.x; this->y -= rhs.y; return *this; }
	INTVECTOR2& operator *=(const int scalar) { this->x *= scalar; this->y *= scalar; return *this; }
	INTVECTOR2& operator *=(const float scalar) { this->x = (int)(this->x * scalar); this->y = (int)(this->y * scalar); return *this; }
	bool operator ==(const INTVECTOR2& rhs) { return (this->x == rhs.x && this->y == rhs.y); }
	bool operator !=(const INTVECTOR2& rhs) { return (this->x != rhs.x || this->y != rhs.y); }
};
inline INTVECTOR2 operator +(INTVECTOR2 lhs, const INTVECTOR2& rhs) { lhs += rhs; return lhs; }
inline INTVECTOR2 operator -(INTVECTOR2 lhs, const INTVECTOR2& rhs) { lhs -= rhs; return lhs; }
inline INTVECTOR2 operator *(INTVECTOR2 lhs, const int scalar) { lhs *= scalar; return lhs; }
inline INTVECTOR2 operator *(INTVECTOR2 lhs, const float scalar) { lhs *= scalar; return lhs; }


struct INTVECTOR3
{
	int x; int y; int z;

	INTVECTOR3(void) { x = 0; y = 0; z = 0; }
	INTVECTOR3(int _x, int _y, int _z) { x = _x; y = _y; z = _z; }
	INTVECTOR3(float _x, float _y, float _z) { x = (int)_x; y = (int)_y; z = (int)_z; }
	INTVECTOR3(const XMFLOAT3 & v) { x = (int)v.x; y = (int)v.y; z = (int)v.z; }
	INTVECTOR3(int val) { x = y = z = val; }

	bool IsZeroVector(void) { return (x == 0 && y == 0 && z == 0); }

	INTVECTOR3& operator +=(const INTVECTOR3& rhs) { this->x += rhs.x; this->y += rhs.y; this->z += rhs.z; return *this; }
	INTVECTOR3& operator -=(const INTVECTOR3& rhs) { this->x -= rhs.x; this->y -= rhs.y; this->z -= rhs.z; return *this; }
	INTVECTOR3& operator *=(const int scalar) { this->x *= scalar; this->y *= scalar; this->z *= scalar; return *this; }
	INTVECTOR3& operator *=(const float scalar) {
		this->x = (int)(this->x * scalar); this->y = (int)(this->y * scalar);
		this->z = (int)(this->z * scalar); return *this;
	}
	INTVECTOR3& operator /=(const int scalar) { this->x /= scalar; this->y /= scalar; this->z /= scalar; return *this; }
	INTVECTOR3& operator /=(const float scalar) {
		this->x = (int)((float)this->x / scalar); this->y = (int)((float)this->y / scalar);
		this->z = (int)((float)this->z / scalar); return *this;
	}
	INTVECTOR3& operator +=(const int scalar) { this->x += scalar; this->y += scalar; this->z += scalar; return *this; }
	INTVECTOR3& operator -=(const int scalar) { this->x -= scalar; this->y -= scalar; this->z -= scalar; return *this; }
	bool operator ==(const INTVECTOR3& rhs) { return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z); }
	bool operator !=(const INTVECTOR3& rhs) { return (this->x != rhs.x || this->y != rhs.y || this->z != rhs.z); }
	bool operator ==(const INTVECTOR3& rhs) const { return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z); }
	bool operator !=(const INTVECTOR3& rhs) const { return (this->x != rhs.x || this->y != rhs.y || this->z != rhs.z); }

	bool operator<(const INTVECTOR3 &rhs) const { return (x < rhs.x && y < rhs.y && z < rhs.z); }
	bool operator<=(const INTVECTOR3 &rhs) const { return (x <= rhs.x && y <= rhs.y && z <= rhs.z); }
	bool operator>(const INTVECTOR3 &rhs) const { return (x > rhs.x && y > rhs.y && z > rhs.z); }
	bool operator>=(const INTVECTOR3 &rhs) const { return (x >= rhs.x && y >= rhs.y && z >= rhs.z); }

	XMFLOAT3 ToFloat3(void) const { return XMFLOAT3((float)x, (float)y, (float)z); }
	std::string ToString(void) const { std::ostringstream s; s << "[" << x << ", " << y << ", " << z << "]"; return s.str(); }
};
inline INTVECTOR3 operator +(INTVECTOR3 lhs, const INTVECTOR3& rhs) { lhs += rhs; return lhs; }
inline INTVECTOR3 operator -(INTVECTOR3 lhs, const INTVECTOR3& rhs) { lhs -= rhs; return lhs; }
inline INTVECTOR3 operator *(INTVECTOR3 lhs, const int scalar) { lhs *= scalar; return lhs; }
inline INTVECTOR3 operator *(INTVECTOR3 lhs, const float scalar) { lhs *= scalar; return lhs; }
inline INTVECTOR3 operator /(INTVECTOR3 lhs, const int scalar) { lhs /= scalar; return lhs; }
inline INTVECTOR3 operator /(INTVECTOR3 lhs, const float scalar) { lhs /= scalar; return lhs; }