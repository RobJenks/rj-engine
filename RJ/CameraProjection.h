#pragma once

#include "DX11_Core.h"


class CameraProjection
{
public:

	// Derive a perspective projection matrix base based on the given parameters
	static XMMATRIX							Perspective(float fov, float aspect_ratio, float near_plane, float far_plane);

	// Derive an orthographic projection matrix base based on the given parameters
	static XMMATRIX							Orthographic(XMFLOAT2 view_size, float near_plane, float far_plane);


private:


};