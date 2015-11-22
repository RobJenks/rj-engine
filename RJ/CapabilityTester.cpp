#include <string>
#include "ErrorCodes.h"
#include "Utility.h"

#include "CapabilityTester.h"


// Determines the maximum supported feature level of the current D3D device
Result CapabilityTester::DetermineMaxSupportedFeatureLevel(D3D_FEATURE_LEVEL & outFeatureLevel) const
{
	HRESULT hr = E_FAIL;
	D3D_FEATURE_LEVEL MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
	const D3D_FEATURE_LEVEL FeatureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,				// TODO: this should be D3D_FEATURE_LEVEL_11_1, but not present in VS2010 for some reason
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Calling the device creation function with NULL in the ppDevice parameter will not create
	// any device, but rather store the highest supported feature level in the FeatureLevel parameter
	hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		(const D3D_FEATURE_LEVEL*)&FeatureLevels,
		ARRAYSIZE(FeatureLevels),
		D3D11_SDK_VERSION,
		NULL,
		&MaxSupportedFeatureLevel,
		NULL
		);

	// Report any failure in the D3D function, otherwise set the output parameter and return success
	if (FAILED(hr))
	{
		return ErrorCodes::CouldNotDetermineSupportedDXFeatureLevels;
	}
	else
	{
		outFeatureLevel = MaxSupportedFeatureLevel;
		return ErrorCodes::NoError;
	}
}

// Returns a string representation of a D3D feature level
std::string	CapabilityTester::FeatureLevelToString(D3D_FEATURE_LEVEL featurelevel)
{
	switch (featurelevel)
	{
	case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_1:	return "Direct3D 9.1";
	case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_2:	return "Direct3D 9.2";
	case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_3:	return "Direct3D 9.3";
	case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0:	return "Direct3D 10.0";
	case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_1:	return "Direct3D 10.1";
	case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0:	return "Direct3D 11.0";
	default:
		if (featurelevel > D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0)
			return "Direct3D 11.0 +";
		else
			return concat("(Unknown [")(featurelevel)("])").str();
	}
}

// Returns a string representation of a D3D render device type
std::string CapabilityTester::RenderDeviceTypeToString(D3D_DRIVER_TYPE drivertype)
{
	switch (drivertype)
	{
	case D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE:		return "Hardware";
	case D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_REFERENCE:	return "Reference device";
	case D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_SOFTWARE:		return "Software";
	case D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_WARP:			return "WARP";
	case D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN:		return "(Unknown)";
	case D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_NULL:			return "(NULL)";
	default:											return "<INVALID>";
	}
}
