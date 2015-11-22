#pragma once

#ifndef __CapabilityTesterH__
#define __CapabilityTesterH__

#include <string>
#include "DX11_Core.h"
#include "ErrorCodes.h"

// This class has no special alignment requirements
class CapabilityTester
{
public:

	// Determines the maximum supported feature level of the current D3D device
	Result					DetermineMaxSupportedFeatureLevel(D3D_FEATURE_LEVEL & outFeatureLevel) const;

	// Returns a string representation of a D3D feature level
	static std::string		FeatureLevelToString(D3D_FEATURE_LEVEL featurelevel);

	// Returns a string representation of a D3D render device type
	static std::string		RenderDeviceTypeToString(D3D_DRIVER_TYPE drivertype);

};


#endif