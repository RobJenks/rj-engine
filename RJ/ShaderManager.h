#pragma once

#ifndef __ShaderManagerH__
#define __ShaderManagerH__

#include <string>
#include <vector>
#include "ErrorCodes.h"
#include "DX11_Core.h"
class InputLayoutDesc;

// This class has no special alignment requirements
class ShaderManager
{
public:

	// Enumeration of defined sampler states
	enum DefinedSamplerState
	{
		StandardLinearSampler = 0,
		StandardLinearClampSampler
	};

	// Loads a compiled shader object (*.cso) and returns the byte data
	static Result		LoadCompiledShader(const std::string & filename, byte **ppOutShader, SIZE_T *pOutBufferSize);

	// Creates a new shader from the specified CSO
	static Result		CreateVertexShader(	ID3D11Device *device, const std::string & filename, InputLayoutDesc *layout_desc,
										ID3D11VertexShader **ppOutShader, ID3D11InputLayout **ppOutInputLayout);
	static Result		CreatePixelShader(ID3D11Device *device, const std::string & filename, ID3D11PixelShader **ppOutShader);
	static Result		CreateGeometryShader(ID3D11Device *device, const std::string & filename, ID3D11GeometryShader **ppOutShader);

	// Return a standard defined sampler description for use in shader initialisation
	static Result		GetStandardSamplerDescription(DefinedSamplerState type, D3D11_SAMPLER_DESC & outSamplerDesc);

	// Return a standard defined sampler state for use in shader initialisation
	static Result		CreateStandardSamplerState(DefinedSamplerState type, ID3D11Device *device, ID3D11SamplerState **ppOutSamplerState);

	// Create a standard dynamic constant buffer of the specified size (Usage=Dynamic, BindFlags=ConstantBuffer, CPUAccessFlags=Write, 
	// MiscFlags = 0, StructureByteStride = 0, ByteWidth = @bytewidth)
	static Result		CreateStandardDynamicConstantBuffer(UINT bytewidth, ID3D11Device *device, ID3D11Buffer **ppOutConstantBuffer);

	// Create a buffer based on the specified parameters
	static Result		CreateBuffer(D3D11_USAGE usage, UINT bytewidth, UINT bindflags, UINT cpuaccessflags, UINT miscflags, UINT structurebytestride,
									 ID3D11Device *device, ID3D11Buffer **ppOutBuffer);


protected:

	// Limit the buffer size that can be allocated to prevent overflows etc. causing errors
	static const UINT	MAX_BUFFER_ALLOCATION = 32768U;

};




#endif






