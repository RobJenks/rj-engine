#pragma once

#include "EnvironmentOverlay.h"
class iSpaceObjectEnvironment;

class EnvironmentElementStrengthOverlay : public EnvironmentOverlay
{
public:
	static void Render(iSpaceObjectEnvironment & environment, int start, int end);

};
