#include "IntersectionTests.h"

// Sphere vs sphere intersection
bool IntersectionTests::SphereSphere(const FXMVECTOR centre0, float radius0, const FXMVECTOR centre1, float radius1)
{
	float r0r1 = radius0 + radius1;
	XMVECTOR distsq = XMVector3LengthSq(XMVectorSubtract(centre1, centre0));
	return (XMVectorGetX(distsq) < (r0r1 * r0r1));
}

// Sphere vs sphere intersection
bool IntersectionTests::SphereSphere(const FXMVECTOR centre0, const FXMVECTOR radius0_repl, const FXMVECTOR centre1, const FXMVECTOR radius1_repl)
{
	XMVECTOR r0r1 = XMVectorAdd(radius0_repl, radius1_repl);
	XMVECTOR distsq = XMVector3LengthSq(XMVectorSubtract(centre1, centre0));
	return XMVector3Less(distsq, XMVectorMultiply(r0r1, r0r1));
}