#include "Utility.h"
#include "BasicRay.h"


// Transforms the ray by the given transformation matrix.  Accepts both an orientation transform
// matrix and the full world transform matrix to avoid rederivation within this method
void BasicRay::Transform(const XMMATRIX & orient_transform, const XMMATRIX world_transform)
{
	Origin = XMVector3TransformCoord(Origin, world_transform);
	Direction = XMVector3NormalizeEst(XMVector3TransformCoord(Direction, orient_transform));
}

// Returns a ray resulting from transformation of the current ray by the given transformation matrix
// Accepts both an orientation transform matrix and the full world transform matrix to 
// avoid rederivation within this method
BasicRay BasicRay::GetTransformedRay(const XMMATRIX & orient_transform, const XMMATRIX world_transform) const 
{
	return BasicRay(
		XMVector3TransformCoord(Origin, world_transform),
		XMVector3NormalizeEst(XMVector3TransformCoord(Direction, orient_transform))
	);
}

// Generates a string representation of the ray
std::string BasicRay::str(void) const
{
	return concat("BasicRay { ")(Vector3ToString(Origin))(" + k")(Vector3ToString(Direction))(" }").str();
}