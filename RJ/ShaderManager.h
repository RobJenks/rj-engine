#pragma once

#ifndef __ShaderManagerH__
#define __ShaderManagerH__

#include <string>
#include <vector>
#include "ErrorCodes.h"
#include "DX11_Core.h"
class InputLayoutDesc;

class ShaderManager
{
public:

	// Loads a compiled shader object (*.cso) and returns the byte data
	static Result LoadCompiledShader(const std::string & filename, std::vector<char> outShader);

	// Creates a new shader from the specified CSO
	static Result CreateVertexShader(ID3D11Device *device, const std::string & filename, InputLayoutDesc *layout_desc,
		ID3D11VertexShader **ppOutShader, ID3D11InputLayout **ppOutInputLayout);
	static Result CreatePixelShader(ID3D11Device *device, const std::string & filename, ID3D11PixelShader **ppOutShader);
	static Result CreateGeometryShader(ID3D11Device *device, const std::string & filename, ID3D11GeometryShader **ppOutShader);

};




#endif






