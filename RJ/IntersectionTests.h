#pragma once

#include "DX11_Core.h"

// Information from https://github.com/gszauer/GamePhysicsCookbook
class IntersectionTests
{
public:

	// Sphere vs sphere intersection
	static bool SphereSphere(const FXMVECTOR centre0, float radius0, const FXMVECTOR centre1, float radius1);
	static bool SphereSphere(const FXMVECTOR centre0, const FXMVECTOR radius0_repl, const FXMVECTOR centre1, const FXMVECTOR radius1_repl);


private:
};