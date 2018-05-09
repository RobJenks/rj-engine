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

// Standard render queue instancing layout
#define ADD_INSTANCING_LAYOUT \
	.Add("Transform", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1) \
	.Add("Transform", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1) \
	.Add("Transform", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1) \
	.Add("Transform", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1) \
	\
	.Add("Flags", 0, DXGI_FORMAT_R32_UINT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1) \
	.Add("SortKey", 0, DXGI_FORMAT_R32_UINT, 1, 68, D3D11_INPUT_PER_INSTANCE_DATA, 1) \
	.Add("padding", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 72, D3D11_INPUT_PER_INSTANCE_DATA, 1) \
	\
	.Add("Highlight", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1)


// Initialises the standard input layouts that are reusable across multiple shader scenarios
void InputLayoutDesc::InitialiseStaticData(void)
{
	/* Standard input layout for instanced, textured and fully-lit models */
	InputLayoutDesc::AddStandardLayout("Vertex_Inst_Standard_Layout", InputLayoutDesc()

		// Vertex input layout
		.Add("POSITION", 0,	DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("NORMAL", 0,	DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("TANGENT", 0,	DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0)
		.Add("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0)

		// Standard instancing input layout
		ADD_INSTANCING_LAYOUT
	);
}

