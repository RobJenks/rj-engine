#include "InputLayoutDesc.h"

// Define static collection of standard input layouts
std::unordered_map<std::string, InputLayoutDesc> InputLayoutDesc::StandardLayouts;

// Store a standard input layout in the static central collection
void InputLayoutDesc::AddStandardLayout(const std::string & name, const InputLayoutDesc & layout)
{
	if (InputLayoutDesc::StandardLayouts.count(name) == 0)
	{
		InputLayoutDesc::StandardLayouts[name] = layout;
	}
}

// Returns a standard input layout.  Returns a flag indicating whether the layout could be retrieved
bool InputLayoutDesc::GetStandardLayout(const std::string & name, InputLayoutDesc & outLayout)
{
	if (InputLayoutDesc::StandardLayouts.count(name) != 0)
	{
		outLayout = InputLayoutDesc::StandardLayouts[name];
		return true;
	}
	else
	{
		return false;
	}
}

// Initialises the standard input layouts that are reusable across multiple shader scenarios
void InputLayoutDesc::InitialiseStaticData(void)
{
	// Standard input layout for instanced, textured and fully-lit models
	InputLayoutDesc::AddStandardLayout("Vertex_Inst_TexNormMatLit", InputLayoutDesc()
		.Add("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("MATERIAL", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("mTransform", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("mTransform", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("mTransform", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("mTransform", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("iParams", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("LightConfig", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1)
		.Add("padding", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1));

}

