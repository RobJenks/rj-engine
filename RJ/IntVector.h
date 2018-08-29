#pragma once

#include <string>
#include <sstream>
#include "DX11_Core.h"


template <typename T>
struct IntegralVector2
{
	T x; T y;

	IntegralVector2(void) : x(0), y(0) { }
	IntegralVector2(T _x, T _y) : x(_x), y(_y) { }
	IntegralVector2(float _x, float _y) : x(static_cast<T>(_x)), y(static_cast<T>(_y)) { }
	IntegralVector2(const XMFLOAT2 & v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)) { }
	IntegralVector2(T _xy) : x(_xy), y(_xy) { }

	IntegralVector2 Abs(void) const { return IntegralVector2(std::abs(x), std::abs(y)); }
	bool IsZeroVector(void) { return (x == 0 && y == 0); }
	XMFLOAT2 ToFloat(void) const { return XMFLOAT2(static_cast<float>(x), static_cast<float>(y)); }
	std::string ToString(void) const { std::ostringstream s; s << "[" << x << ", " << y << "]"; return s.str(); }

	IntegralVector2& operator +=(const IntegralVector2& rhs) { this->x += rhs.x; this->y += rhs.y; return *this; }
	IntegralVector2& operator -=(const IntegralVector2& rhs) { this->x -= rhs.x; this->y -= rhs.y; return *this; }
	IntegralVector2& operator *=(const T scalar) { this->x *= scalar; this->y *= scalar; return *this; }
	IntegralVector2& operator *=(const float scalar) { this->x = static_cast<T>(this->x * scalar); this->y = static_cast<T>(this->y * scalar); return *this; }
	bool operator ==(const IntegralVector2& rhs) { return (this->x == rhs.x && this->y == rhs.y); }
	bool operator !=(const IntegralVector2& rhs) { return (this->x != rhs.x || this->y != rhs.y); }

	template <typename U>
	IntegralVector2<U> Convert(void) const { return IntegralVector2<U>(static_cast<U>(x), static_cast<U>(y)); }
};
template <typename T> inline IntegralVector2<T> operator +(IntegralVector2<T> lhs, const IntegralVector2<T> & rhs) { lhs += rhs; return lhs; }
template <typename T> inline IntegralVector2<T> operator -(IntegralVector2<T> lhs, const IntegralVector2<T> & rhs) { lhs -= rhs; return lhs; }
template <typename T> inline IntegralVector2<T> operator *(IntegralVector2<T> lhs, const T scalar) { lhs *= scalar; return lhs; }
template <typename T> inline IntegralVector2<T> operator *(IntegralVector2<T> lhs, const float scalar) { lhs *= scalar; return lhs; }

template <typename T> std::ostream & operator<<(std::ostream & out, IntegralVector2<T> const& x) { return out << x.ToString(); }

typedef IntegralVector2<int>			INTVECTOR2;
typedef IntegralVector2<unsigned int>	UINTVECTOR2;


template <typename T>
struct IntegralVector3
{
	T x; T y; T z;

	IntegralVector3(void) : x(0), y(0), z(0) { }
	IntegralVector3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) { }
	IntegralVector3(float _x, float _y, float _z) : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)) { }
	IntegralVector3(const XMFLOAT3 & v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)) { }
	IntegralVector3(T xyz) : x(xyz), y(xyz), z(xyz) { }

	IntegralVector3 Abs(void) const { return IntegralVector3(std::abs(x), std::abs(y), std::abs(z)); }
	bool IsZeroVector(void) { return (x == 0 && y == 0 && z == 0); }
	XMFLOAT3 ToFloat(void) const { return XMFLOAT3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)); }

	IntegralVector3 & operator +=(const IntegralVector3 & rhs) { this->x += rhs.x; this->y += rhs.y; this->z += rhs.z; return *this; }
	IntegralVector3 & operator -=(const IntegralVector3 & rhs) { this->x -= rhs.x; this->y -= rhs.y; this->z -= rhs.z; return *this; }
	IntegralVector3 & operator *=(const int scalar) { this->x *= scalar; this->y *= scalar; this->z *= scalar; return *this; }
	IntegralVector3 & operator *=(const float scalar) {
		this->x = static_cast<T>(this->x * scalar); this->y = static_cast<T>(this->y * scalar);
		this->z = static_cast<T>(this->z * scalar); return *this;
	}
	IntegralVector3& operator /=(const T scalar) { this->x /= scalar; this->y /= scalar; this->z /= scalar; return *this; }
	IntegralVector3& operator /=(const float scalar) {
		this->x = static_cast<T>((float)this->x / scalar); this->y = static_cast<T>((float)this->y / scalar);
		this->z = static_cast<T>((float)this->z / scalar); return *this;
	}
	IntegralVector3& operator +=(const T scalar) { this->x += scalar; this->y += scalar; this->z += scalar; return *this; }
	IntegralVector3& operator -=(const T scalar) { this->x -= scalar; this->y -= scalar; this->z -= scalar; return *this; }
	bool operator ==(const IntegralVector3 & rhs) { return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z); }
	bool operator !=(const IntegralVector3 & rhs) { return (this->x != rhs.x || this->y != rhs.y || this->z != rhs.z); }
	bool operator ==(const IntegralVector3 & rhs) const { return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z); }
	bool operator !=(const IntegralVector3 & rhs) const { return (this->x != rhs.x || this->y != rhs.y || this->z != rhs.z); }

	bool operator<(const IntegralVector3 & rhs) const { return (x < rhs.x && y < rhs.y && z < rhs.z); }
	bool operator<=(const IntegralVector3 & rhs) const { return (x <= rhs.x && y <= rhs.y && z <= rhs.z); }
	bool operator>(const IntegralVector3 & rhs) const { return (x > rhs.x && y > rhs.y && z > rhs.z); }
	bool operator>=(const IntegralVector3 & rhs) const { return (x >= rhs.x && y >= rhs.y && z >= rhs.z); }

	std::string ToString(void) const { std::ostringstream s; s << "[" << x << ", " << y << ", " << z << "]"; return s.str(); }

	template <typename U>
	IntegralVector3<U> Convert(void) const { return IntegralVector3<U>(static_cast<U>(x), static_cast<U>(y), static_cast<U>(z)); }
};
template <typename T> inline IntegralVector3<T> operator +(IntegralVector3<T> lhs, const IntegralVector3<T>& rhs) { lhs += rhs; return lhs; }
template <typename T> inline IntegralVector3<T> operator -(IntegralVector3<T> lhs, const IntegralVector3<T>& rhs) { lhs -= rhs; return lhs; }
template <typename T> inline IntegralVector3<T> operator *(IntegralVector3<T> lhs, const int scalar) { lhs *= scalar; return lhs; }
template <typename T> inline IntegralVector3<T> operator *(IntegralVector3<T> lhs, const float scalar) { lhs *= scalar; return lhs; }
template <typename T> inline IntegralVector3<T> operator /(IntegralVector3<T> lhs, const int scalar) { lhs /= scalar; return lhs; }
template <typename T> inline IntegralVector3<T> operator /(IntegralVector3<T> lhs, const float scalar) { lhs /= scalar; return lhs; }

template <typename T> std::ostream & operator<<(std::ostream & out, IntegralVector3<T> const& x) { return out << x.ToString(); }


typedef IntegralVector3<int>			INTVECTOR3;
typedef IntegralVector3<UINT>			UINTVECTOR3;





