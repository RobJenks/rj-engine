#include "CoreEngine.h"

#include "ObjectPicking.h"

// Method which generates a world-space ray based on the specified, normalised mouse location
void ObjectPicking::ScreenSpaceToWorldBasicRay(const XMFLOAT2 & norm_screen_location, BasicRay & outRay)
{
	// Projected ray will be from [x, y, 0] (near plane) to [x, y, 1] (far plane)
	// Transform these points by the inverse viewproj matrix to get world space points
	XMVECTOR pt = XMVectorSet(norm_screen_location.x, -norm_screen_location.y, 0.0f, 0.0f);
	XMVECTOR origin_pt =	XMVector3TransformCoord(pt,						Game::Engine->GetRenderInverseViewProjectionMatrix());
	XMVECTOR far_pt =		XMVector3TransformCoord(XMVectorSetZ(pt, 1.0f),	Game::Engine->GetRenderInverseViewProjectionMatrix());

	// Construct the ray using these points.  Ray direction will be norm(far - origin)
	outRay = BasicRay(origin_pt, XMVector3NormalizeEst(XMVectorSubtract(far_pt, origin_pt)));
}

// Method which generates a world-space ray based on the specified, normalised screen location
void ObjectPicking::ScreenSpaceToWorldRay(const XMFLOAT2 & norm_screen_location, Ray & outRay)
{
	// Generate the equivalent basic ray and wrap it before returning
	BasicRay basic_ray;
	ScreenSpaceToWorldBasicRay(norm_screen_location, basic_ray);
	outRay = Ray(basic_ray);
}


// Returns the object being selected at the specified mouse location, or NULL if none.  Expects
// normalised mouse location as input
iObject * ObjectPicking::GetObjectAtMouseLocation(const XMFLOAT2 & norm_mouse_location)
{
	// Generate a world-space ray for this mouse location
	BasicRay ray;
	ScreenSpaceToWorldBasicRay(norm_mouse_location, ray);

	// Return the result of the subsequent raycast
	return GetObjectIntersectedByMouseWorldRay(ray);
}

// Returns the object being selected at the current mouse location, or NULL if none.  Uses the precalculated
// mouse world ray and so is more efficient than overloads if we want to use the current mouse location
iObject * ObjectPicking::GetObjectAtMouseLocation(void)
{
	// We can use the precalculated mouse world-space ray
	return GetObjectIntersectedByMouseWorldRay(Game::Mouse.GetWorldSpaceMouseBasicRay());
}


// Returns the object being intersected by the specified mouse world ray, or NULL if none
iObject * ObjectPicking::GetObjectIntersectedByMouseWorldRay(const BasicRay & ray)
{
	// Perform a raycast using this world-space ray and return the closest possible intersection (or NULL if none)
	return Game::PhysicsEngine.PerformRaycastFull(ray, Game::VisibleObjects);
}




