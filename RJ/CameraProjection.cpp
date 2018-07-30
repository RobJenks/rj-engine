#include "CameraProjection.h"

// Derive a perspective projection matrix base based on the given parameters
XMMATRIX CameraProjection::Perspective(float fov, float aspect_ratio, float near_plane, float far_plane)
{
	return XMMatrixPerspectiveFovLH(fov, aspect_ratio, near_plane, far_plane);
}

// Derive an orthographic projection matrix base based on the given parameters
XMMATRIX CameraProjection::Orthographic(XMFLOAT2 view_size, float near_plane, float far_plane)
{
	return XMMatrixOrthographicLH(view_size.x, view_size.y, near_plane, far_plane);
}