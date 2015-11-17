#pragma once

#ifndef __DXLocaliserH__
#define __DXLocaliserH__

#include <string>
#include <vector>
#include <d3dcommon.h>
#include "ErrorCodes.h"
using namespace std;


// This class has no special alignment requirements
class DXLocaliser
{
public:
	// Enumeration of all supported DX levels, plus the default level that we start from
	enum DXLevel { DirectX_11_0, DirectX_9_1, Z__COUNT };
	static const DXLevel DX_LEVEL_DEFAULT			= DXLevel::DirectX_11_0;
	static const DXLevel DX_LEVEL_UNINITIALISED		= (DXLevel)-999;

	// Enumeration of all supported shader model levels
	enum SMLevel { SM_2_0 = 0, SM_4_0 = 1, SM_5_0 = 2 };

	// Structure storing the features of a particular D3D feature level
	struct LOCALISATION_LEVEL
	{
		D3D_FEATURE_LEVEL			FeatureLevel;	
		SMLevel						ShaderModelLevel;
		const char *				ShaderModelLevelDescription;
		const char *				VertexShaderLevelDesc;
		const char *				PixelShaderLevelDesc;
		D3D_DRIVER_TYPE				RendereringDeviceType;
	};

	// Constructor
	DXLocaliser(void);

	// Initialises the component, including localisation tables, and sets an initial localisation level
	Result					Initialise(void);
	Result					Initialise(D3D_FEATURE_LEVEL featurelevel);

	// Determines the maximum supported feature level of the current D3D device
	Result					DetermineMaxSupportedFeatureLevel(D3D_FEATURE_LEVEL & outFeatureLevel) const;

	// Applies a DX localisation level by transferring relevant values from the localisation tables
	Result					ApplyDXLocalisation(D3D_FEATURE_LEVEL featurelevel);

	// The current localisation level of this component
	LOCALISATION_LEVEL		Locale;

	// Set the localisation level to be used
	void					SetLocale(const LOCALISATION_LEVEL & locale);

	// Determines whether the supplied D3D feature level is valid
	bool					IsValidFeatureLevel(D3D_FEATURE_LEVEL featurelevel);

	// Returns a string representation of a D3D feature level
	static std::string		FeatureLevelToString(D3D_FEATURE_LEVEL featurelevel);

	// Returns a string representation of a D3D render device type
	static std::string		RenderDeviceTypeToString(D3D_DRIVER_TYPE drivertype);


	// Destructor
	~DXLocaliser(void);

protected:

	// Builds the localisation tables
	void								CompileDXLocalisationData(void);

	// The localiser maintains a list of capabilities provided at each feature level
	std::vector<LOCALISATION_LEVEL>		m_featurelevels;

};



#endif