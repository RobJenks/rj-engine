#pragma once

#ifndef __DXLocaliserH__
#define __DXLocaliserH__

#include <string>
#include <d3dcommon.h>
#include "ErrorCodes.h"
using namespace std;


class DXLocaliser
{
public:
	// Enumeration of all supported DX levels, plus the default level that we start from
	enum DXLevel { DirectX_11_0, DirectX_9_1, Z__COUNT };
	static const DXLevel DX_LEVEL_DEFAULT			= DXLevel::DirectX_11_0;
	static const DXLevel DX_LEVEL_UNINITIALISED		= (DXLevel)-999;

	// Enumeration of all supported shader model levels
	enum SMLevel { SM_5_0, SM_2_0 };

	// Constructor
	DXLocaliser(void);

	// Initialises the component, including localisation tables, and sets an initial localisation level
	Result					Initialise(void);
	Result					Initialise(DXLevel DirectXLevel);

	// Applies a DX localisation level by transferring relevant values from the localisation tables
	Result					ApplyDXLocalisation(DXLevel DirectXLevel);

	// Destructor
	~DXLocaliser(void);

	// The parameters that will be modified by the DX localisation component
	D3D_FEATURE_LEVEL				DXL_D3D_FEATURE_LEVEL;
	SMLevel							DXL_SM_LEVEL;
	const char *					DXL_VERTEX_SHADER_LEVEL_S;
	const char *					DXL_PIXEL_SHADER_LEVEL_S;
	D3D_DRIVER_TYPE					DXL_DEVICE_DRIVER_TYPE;

private:
	// Builds the localisation tables
	Result DXLocaliser::BuildDXLocalisationTables(void);

	// The current localisation level of this component
	DXLevel							m_DXLevel;

	// The localisation tables for each of the parameters controlled by this localisation component
	D3D_FEATURE_LEVEL				LT_D3D_FEATURE_LEVEL				[DXLevel::Z__COUNT];
	SMLevel							LT_SM_LEVEL							[DXLevel::Z__COUNT];
	string							LT_VERTEX_SHADER_LEVEL_S			[DXLevel::Z__COUNT];
	string							LT_PIXEL_SHADER_LEVEL_S				[DXLevel::Z__COUNT];
	D3D_DRIVER_TYPE					LT_DEVICE_DRIVER_TYPE				[DXLevel::Z__COUNT];
};



#endif