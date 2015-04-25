#include <string>
#include <d3dcommon.h>
#include "ErrorCodes.h"
using namespace std;

#include "DXLocaliser.h"

DXLocaliser::DXLocaliser(void)
{
	// Set the current localisation level to an out-of-bounds value, to signify that we are not yet initialised
	this->m_DXLevel = DXLocaliser::DX_LEVEL_UNINITIALISED;
}

Result DXLocaliser::Initialise(void)
{
	// If not given an initial DX level then initialise to the default level
	return Initialise(DXLocaliser::DX_LEVEL_DEFAULT);
}

Result DXLocaliser::Initialise(DXLevel DirectXLevel)
{
	Result result;

	// Build the localisation tables, prior to any DX level being applied
	result = BuildDXLocalisationTables();
	if (result != ErrorCodes::NoError) return result;

	// Perform an initial localisation to the provided DX level
	result = ApplyDXLocalisation(DirectXLevel);
	if (result != ErrorCodes::NoError) return result;

	// Return success
	return ErrorCodes::NoError;
}

Result DXLocaliser::BuildDXLocalisationTables(void)
{
	/* Build each table in turn */

	// DX Feature Level
	LT_D3D_FEATURE_LEVEL				[DXLevel::DirectX_11_0]		= D3D_FEATURE_LEVEL_11_0;
	LT_D3D_FEATURE_LEVEL				[DXLevel::DirectX_9_1 ]		= D3D_FEATURE_LEVEL_9_1;
	
	// Shader model level
	LT_SM_LEVEL							[DXLevel::DirectX_11_0]		= SMLevel::SM_5_0;
	LT_SM_LEVEL							[DXLevel::DirectX_9_1]		= SMLevel::SM_2_0;

	// Pixel shader model level - string representation
	LT_PIXEL_SHADER_LEVEL_S				[DXLevel::DirectX_11_0]		= "ps_5_0";
	LT_PIXEL_SHADER_LEVEL_S				[DXLevel::DirectX_9_1]		= "ps_4_0_level_9_1"; //"ps_2_0";

	// Vertex shader model level - string representation
	LT_VERTEX_SHADER_LEVEL_S			[DXLevel::DirectX_11_0]		= "vs_5_0";
	LT_VERTEX_SHADER_LEVEL_S			[DXLevel::DirectX_9_1]		= "vs_4_0_level_9_1"; //"vs_2_0";
	
	// DX rendering device; potentially using reference device for highest DX levels if unsupported by dev hardware
	LT_DEVICE_DRIVER_TYPE				[DXLevel::DirectX_11_0]		= D3D_DRIVER_TYPE_HARDWARE; // D3D_DRIVER_TYPE_REFERENCE;
	LT_DEVICE_DRIVER_TYPE				[DXLevel::DirectX_9_1]		= D3D_DRIVER_TYPE_HARDWARE;
	
	/* Return success following build of all tables */
	return ErrorCodes::NoError;
}

Result DXLocaliser::ApplyDXLocalisation(DXLevel DirectXLevel)
{
	// Perform validation on the DX level parameter provided
	if (DirectXLevel == DXLocaliser::DX_LEVEL_UNINITIALISED)
		return ErrorCodes::DXLocaliserNotInitialised;
	if ((int)DirectXLevel < 0 || (int)DirectXLevel >= DXLevel::Z__COUNT)
		return ErrorCodes::InvalidDXLevelPassedToLocaliser;

	// Otherwise, we want to copy the relevant value from each localisation table into the relevant parameter
	DXL_D3D_FEATURE_LEVEL		= LT_D3D_FEATURE_LEVEL			[(int)DirectXLevel];
	DXL_SM_LEVEL				= LT_SM_LEVEL					[(int)DirectXLevel];
	DXL_PIXEL_SHADER_LEVEL_S	= LT_PIXEL_SHADER_LEVEL_S		[(int)DirectXLevel].c_str();
	DXL_VERTEX_SHADER_LEVEL_S	= LT_VERTEX_SHADER_LEVEL_S		[(int)DirectXLevel].c_str();
	DXL_DEVICE_DRIVER_TYPE		= LT_DEVICE_DRIVER_TYPE			[(int)DirectXLevel];

	// Return success once all parameters are set
	return ErrorCodes::NoError;
}


DXLocaliser::~DXLocaliser(void)
{
}
