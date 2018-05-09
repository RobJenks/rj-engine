#include "StructuredBufferDX11.h"
#include "CoreEngine.h"


// Static null buffer resources, used for more efficient unbinding
ID3D11ShaderResourceView * const StructuredBufferDX11::m_null_srv[1] = { nullptr };
ID3D11UnorderedAccessView * const StructuredBufferDX11::m_null_uav[1] = { nullptr };


// Construct a new structured buffer resource
StructuredBufferDX11 * StructuredBufferDX11::Create(const void* data, UINT element_count, UINT stride, CPUGraphicsResourceAccess cpuAccess, bool isUAV)
{
	// Create an appropriate buffer descriptor based on the given parameters
	D3D11_BUFFER_DESC bufferdesc = {};
	bufferdesc.ByteWidth = (element_count * stride);

	if (((int)cpuAccess & (int)CPUGraphicsResourceAccess::Read) != 0)
	{
		bufferdesc.Usage = D3D11_USAGE_STAGING;
		bufferdesc.CPUAccessFlags = (D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ);
	}
	else if (((int)cpuAccess & (int)CPUGraphicsResourceAccess::Write) != 0)
	{
		bufferdesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	}
	else
	{
		bufferdesc.Usage = D3D11_USAGE_DEFAULT;
		bufferdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (isUAV)
		{
			bufferdesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}
	}

	bufferdesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferdesc.StructureByteStride = stride;

	// Now construct the buffer using this custom descriptor
	return new StructuredBufferDX11(bufferdesc, data, element_count, stride);
}

StructuredBufferDX11 * StructuredBufferDX11::Create(UINT element_count, UINT stride, CPUGraphicsResourceAccess cpuAccess, bool isUAV)
{
	return StructuredBufferDX11::Create(NULL, element_count, stride, cpuAccess, isUAV);
}


// Private constructor; new structured buffers should be instantiated through StructuredBufferDX11::Create()
StructuredBufferDX11::StructuredBufferDX11(const D3D11_BUFFER_DESC & buffer_desc, const void* data, UINT element_count, UINT stride)
	:
	StructuredBuffer(buffer_desc, data, element_count, stride), 
	m_sb_dirty(true)
{
	HRESULT result;
	m_srv[0] = NULL;
	m_uav[0] = NULL;
	
	// Assign data to the system buffer
	if (data)
	{
		m_data.assign((uint8_t*)data, (uint8_t*)data + m_buffersize);
	}
	else
	{
		m_data.reserve(m_buffersize);
	}

	// Now perform further initialisaton of the buffer - SRV, UAV etc - following initialisation of the base
	// ID3D11Buffer resource by the base Buffer class
	auto device = Game::Engine->GetDevice();
	if ((buffer_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = element_count;

		result = device->CreateShaderResourceView(m_buffer[0], &srvDesc, &(m_srv[0]));
		if (FAILED(result))
		{
			Game::Log << LOG_ERROR << "Failed to initialise structured buffer SRV resource (hr= " << result << ", elementcount=" << element_count << ")\n";
			return;
		}
	}

	if ((buffer_desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) != 0)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = element_count;
		uavDesc.Buffer.Flags = 0;

		result = device->CreateUnorderedAccessView(m_buffer[0], &uavDesc, &(m_uav[0]));
		if (FAILED(result))
		{
			Game::Log << LOG_ERROR << "Failed to initialise structured buffer UAV resource (hr=" << result << ", elementcount=" << element_count << ")\n";
			return;
		}
	}
}

// Update data within the buffer
void StructuredBufferDX11::SetData(void* data, size_t elementSize, size_t offset, size_t numElements)
{
	// Make the assumption that sizeof(char) == 1
	uint8_t *first = (uint8_t*)data + (offset * elementSize);
	uint8_t *last = first + (numElements * elementSize);
	m_data.assign(first, last);

	LightData * tmp = (LightData*)&(m_data[0]);

	// Data needs to be re-mapped into GPU memory
	m_sb_dirty = true;
}

// Commit buffer data to GPU memory
void StructuredBufferDX11::Commit(void)
{
	Set(&(m_data[0]), m_buffersize);

	m_sb_dirty = false;
}

// Bind this resource to the given shader target
void StructuredBufferDX11::Bind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype)
{
	// Re-commit data first if required
	if (m_sb_dirty)
	{
		Commit();
	}

	if (parametertype == ShaderParameter::Type::StructuredBuffer && m_srv[0])
	{
		switch (shadertype)
		{
			case Shader::Type::VertexShader:
				Game::Engine->GetDeviceContext()->VSSetShaderResources(slot_id, 1, m_srv);
				break;
			case Shader::Type::PixelShader:
				Game::Engine->GetDeviceContext()->PSSetShaderResources(slot_id, 1, m_srv);
				break; 
			case Shader::Type::HullShader:
				Game::Engine->GetDeviceContext()->HSSetShaderResources(slot_id, 1, m_srv);
				break;
			case Shader::Type::DomainShader:
				Game::Engine->GetDeviceContext()->DSSetShaderResources(slot_id, 1, m_srv);
				break;
			case Shader::Type::GeometryShader:
				Game::Engine->GetDeviceContext()->GSSetShaderResources(slot_id, 1, m_srv);
				break;
			case Shader::Type::ComputeShader:
				Game::Engine->GetDeviceContext()->CSSetShaderResources(slot_id, 1, m_srv);
				break;
		}
	}
	else if (parametertype == ShaderParameter::Type::RWBuffer && m_uav[0])
	{
		switch (shadertype)
		{
			case Shader::Type::ComputeShader:
				Game::Engine->GetDeviceContext()->CSSetUnorderedAccessViews(slot_id, 1, m_uav, nullptr);
				break;
		}
	}
}

// Remove this (or any) binding from the given shader target
void StructuredBufferDX11::Unbind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype)
{
	if (parametertype == ShaderParameter::Type::StructuredBuffer)
	{
		switch (shadertype)
		{
			case Shader::Type::VertexShader:
				Game::Engine->GetDeviceContext()->VSSetShaderResources(slot_id, 1, m_null_srv);
				break;
			case Shader::Type::PixelShader:
				Game::Engine->GetDeviceContext()->PSSetShaderResources(slot_id, 1, m_null_srv);
				break;
			case Shader::Type::HullShader:
				Game::Engine->GetDeviceContext()->HSSetShaderResources(slot_id, 1, m_null_srv);
				break;
			case Shader::Type::DomainShader:
				Game::Engine->GetDeviceContext()->DSSetShaderResources(slot_id, 1, m_null_srv);
				break;
			case Shader::Type::GeometryShader:
				Game::Engine->GetDeviceContext()->GSSetShaderResources(slot_id, 1, m_null_srv);
				break;
			case Shader::Type::ComputeShader:
				Game::Engine->GetDeviceContext()->CSSetShaderResources(slot_id, 1, m_null_srv);
				break;
		}
	}
	else if (parametertype == ShaderParameter::Type::RWBuffer)
	{
		switch (shadertype)
		{
			case Shader::Type::ComputeShader:
				Game::Engine->GetDeviceContext()->CSSetUnorderedAccessViews(slot_id, 1, m_null_uav, nullptr);
				break;
		}
	}
}

ID3D11ShaderResourceView * StructuredBufferDX11::GetShaderResourceView(void) const
{
	return m_srv[0];
}

ID3D11UnorderedAccessView *	StructuredBufferDX11::GetUnorderedAccessView() const
{
	return m_uav[0];
}



// Default destructor
StructuredBufferDX11::~StructuredBufferDX11(void)
{
	ReleaseIfExists(m_srv[0]);
	ReleaseIfExists(m_uav[0]);
}


















