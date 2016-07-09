#include "DX11_Core.h"
#include "FastMath.h"
#include "Ray.h"

// Set the ray direction, updating derived fields in the process
void Ray::SetDirection(const FXMVECTOR direction)
{
	Direction = direction;
	InvDirection = XMVectorReciprocal(Direction);
	Sign = XMVectorSelect(ONE_VECTOR_P, ONE_VECTOR_N, XMVectorLess(InvDirection, ZERO_VECTOR));
}

// Transforms the ray into a different coordinate system
void Ray::TransformIntoCoordinateSystem(const FXMVECTOR origin, const AXMVECTOR_P(&bases)[3])
{
	XMVECTOR diff = XMVectorSubtract(Origin, origin);

	// Transform the ray origin
	Origin = XMVectorSet(XMVectorGetX(XMVector3Dot(diff, bases[0].value)),
		XMVectorGetX(XMVector3Dot(diff, bases[1].value)),
		XMVectorGetX(XMVector3Dot(diff, bases[2].value)), 0.0f);

	// Transform the ray direction
	Direction = XMVectorSet(XMVectorGetX(XMVector3Dot(Direction, bases[0].value)),
		XMVectorGetX(XMVector3Dot(Direction, bases[1].value)),
		XMVectorGetX(XMVector3Dot(Direction, bases[2].value)), 0.0f);

	// Recalculate derived fields
	InvDirection = XMVectorReciprocal(Direction);
	Sign = XMVectorSelect(ONE_VECTOR_P, ONE_VECTOR_N, XMVectorLess(InvDirection, ZERO_VECTOR));
}