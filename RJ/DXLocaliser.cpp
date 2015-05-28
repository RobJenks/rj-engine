#include <string>
#include <vector>
#include <d3dcommon.h>
#include "ErrorCodes.h"
#include "DX11_Core.h"
#include "Utility.h"
#include "LogManager.h"
using namespace std;

#include "DXLocaliser.h"

DXLocaliser::DXLocaliser(void)
{
	// Build the localisation tables, which enumerate the capabilities for each level of DX device
	CompileDXLocalisationData();

}

// Initalise the localiser.  Will determine the capabilities provided by the current
// device and initialise accordingly
Result DXLocaliser::Initialise(void)
{
	// Determine the maximum supported feature level of the current device
	D3D_FEATURE_LEVEL maxfeaturelevel;
	Result result = DetermineMaxSupportedFeatureLevel(maxfeaturelevel);
	if (result != ErrorCodes::NoError) 
	{
		Game::Log << LOG_INIT_START << "Error: Could not determine supported D3D feature levels\n";
		return result;
	}

	// Report the max supported feature level
	Game::Log	<< LOG_INIT_START << "Determined maximum supported D3D feature level: " 
				<< DXLocaliser::FeatureLevelToString(maxfeaturelevel) << "\n";

	// Now call the feature level-specific initialisation method
	return Initialise(maxfeaturelevel);
}

Result DXLocaliser::Initialise(D3D_FEATURE_LEVEL featurelevel)
{
	// Perform an initial localisation to the provided DX level
	Result result = ApplyDXLocalisation(featurelevel);
	if (result != ErrorCodes::NoError) return result;

	// Return success
	return ErrorCodes::NoError;
}

void DXLocaliser::CompileDXLocalisationData(void)
{
	// Determine whether the hardware render device is being overriden; apply this value
	// to each entry in the localisation tables later so that they are propogated to 
	// the game engine when one localisation is selected
	D3D_DRIVER_TYPE renderdevice = (Game::ForceWARPRenderDevice ? D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_WARP : 
																  D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE);

	// Define a list of capabilities provided at each feature level
	DXLocaliser::LOCALISATION_LEVEL levels[] = 
	{
		/* FEATURE LEVEL            SHADER MODEL     SM DESC   VS MODEL LEVEL      PS MODEL LEVEL      RENDERING DEVICE TYPE    */
		{ D3D_FEATURE_LEVEL_9_1,    SMLevel::SM_2_0, "SM 2.0", "vs_4_0_level_9_1", "ps_4_0_level_9_1", renderdevice },
		{ D3D_FEATURE_LEVEL_9_2,    SMLevel::SM_2_0, "SM 2.0", "vs_4_0_level_9_1", "ps_4_0_level_9_1", renderdevice },
		{ D3D_FEATURE_LEVEL_9_3,    SMLevel::SM_2_0, "SM 2.0", "vs_4_0_level_9_3", "ps_4_0_level_9_3", renderdevice },
		//{ D3D_FEATURE_LEVEL_10_0,    SMLevel::SM_2_0, "SM 2.0", "vs_4_0_level_9_1", "ps_4_0_level_9_1", renderdevice },
		{ D3D_FEATURE_LEVEL_10_0,   SMLevel::SM_4_0, "SM 4.0", "vs_4_0",		   "ps_4_0",           renderdevice },
		{ D3D_FEATURE_LEVEL_10_1,   SMLevel::SM_4_0, "SM 4.0", "vs_4_0",		   "ps_4_0",           renderdevice },
		{ D3D_FEATURE_LEVEL_11_0,   SMLevel::SM_5_0, "SM 5.0", "vs_5_0",           "ps_5_0",           renderdevice }
	};

	// Maintain these levels in a vector for ease-of-use
	int count = sizeof(levels) / sizeof(DXLocaliser::LOCALISATION_LEVEL);
	for (int i = 0; i < count; ++i)
	{
		m_featurelevels.push_back(levels[i]);
	}

}

// Determines the maximum supported feature level of the current D3D device
Result DXLocaliser::DetermineMaxSupportedFeatureLevel(D3D_FEATURE_LEVEL & outFeatureLevel) const
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
	if(FAILED(hr))
	{
		return ErrorCodes::CouldNotDetermineSupportedDXFeatureLevels;
	}
	else
	{
		outFeatureLevel = MaxSupportedFeatureLevel;
		return ErrorCodes::NoError;
	}
}

// Determines whether the supplied D3D feature level is valid
bool DXLocaliser::IsValidFeatureLevel(D3D_FEATURE_LEVEL featurelevel)
{
	// Check whether the feature level is one of those explictly supported
	for (int i = 0; i < (int)m_featurelevels.size(); ++i)
		if (featurelevel == m_featurelevels[i].FeatureLevel)
			return true;

	// Also support if the feature level is greater than our maximum (e.g. when DX12 is released, we 
	// still want to support it but just at the DX11 feature set level)
	if (featurelevel > m_featurelevels[m_featurelevels.size() - 1].FeatureLevel) return true;

	// This is not a valid, or sufficiently-new, feature level
	return false;
}

Result DXLocaliser::ApplyDXLocalisation(D3D_FEATURE_LEVEL featurelevel)
{
	// Perform validation on the DX level parameter provided
	if (!DXLocaliser::IsValidFeatureLevel(featurelevel))
		return ErrorCodes::InvalidDXLevelPassedToLocaliser;

	// If the device supports a higher feature level than at the time of release, we want to scale
	// it back to the maximum supported feature level
	if (featurelevel > m_featurelevels[m_featurelevels.size() - 1].FeatureLevel)
	{
		featurelevel = m_featurelevels[m_featurelevels.size() - 1].FeatureLevel;
		Game::Log << LOG_INIT_START << "Note: Device supports feature levels greater than maximum required; " <<
			"constraining feature level to " << DXLocaliser::FeatureLevelToString(featurelevel) << "\n";
	}

	// Otherwise, we want to find the relevant feature set and use its values
	for (int i = 0; i < (int)m_featurelevels.size(); ++i)
	{
		if (featurelevel == m_featurelevels[i].FeatureLevel)
		{
			SetLocale(m_featurelevels[i]);
			return ErrorCodes::NoError;
		}
	}
	
	// If we reached here we could not apply a feature level for some reason; report the error and quit
	return ErrorCodes::FailedToApplyDesiredD3DFeatureLevel;
}

// Set the localisation level to be used
void DXLocaliser::SetLocale(const LOCALISATION_LEVEL & locale)
{
	// Store the new locale
	Locale = locale;

	// Report out the settings that this locale will change
	Game::Log << LOG_INIT_START << "Applying DX Localisation for " 
		<< FeatureLevelToString(locale.FeatureLevel) << "\n";
	Game::Log << LOG_INIT_START << "Using Shader Model version " << (locale.ShaderModelLevelDescription) << "\n";
	Game::Log << LOG_INIT_START << "Enabling Vertex Shader \"" << locale.VertexShaderLevelDesc << "\" capabilities\n";
	Game::Log << LOG_INIT_START << "Enabling Pixel Shader \"" << locale.PixelShaderLevelDesc << "\" capabilities\n"; 
	Game::Log << LOG_INIT_START << "Using " << RenderDeviceTypeToString(locale.RendereringDeviceType) << " rendering\n";
}

// Returns a string representation of a D3D feature level
std::string	DXLocaliser::FeatureLevelToString(D3D_FEATURE_LEVEL featurelevel)
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
std::string DXLocaliser::RenderDeviceTypeToString(D3D_DRIVER_TYPE drivertype)
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

DXLocaliser::~DXLocaliser(void)
{
}
