#include "StructuredBufferDX11.h"
#include "CoreEngine.h"


// Static null buffer resources, used for more efficient unbinding
ID3D11ShaderResourceView * const StructuredBufferDX11::m_null_srv[1] = { nullptr };
ID3D11UnorderedAccessView * const StructuredBufferDX11::m_null_uav[1] = { nullptr };


// Construct a new structured buffer resource
StructuredBufferDX11::StructuredBufferDX11(UINT bindFlags, const void* data, UINT element_count, UINT stride, CPUGraphicsResourceAccess cpuAccess, bool isUAV)
	:
	m_buffer(NULL), 
	m_bindflags(bindFlags),
	m_stride(stride), 
	m_cpu_access(cpuAccess)
{
	m_srv[0] = NULL;
	m_uav[0] = NULL;
	m_buffer_elementcount[0] = element_count;							// Member of base class; assign here rather than in initialiser-list
	m_is_dynamic = (m_cpu_access != CPUGraphicsResourceAccess::None);	// Dynamic if any CPU memory access required
	m_isUAV = (isUAV && !m_is_dynamic);									// UAV resources cannot be dynamic
	auto device = Game::Engine->GetDevice();

	// Assign data to the system buffer
	size_t bytecount = (m_buffer_elementcount[0] * m_stride);
	if (data)
	{
		m_data.assign((BufferElementType*)data, (BufferElementType*)data + bytecount);
	}
	else
	{
		m_data.reserve(bytecount);
	}

	// Create an appropriate GPU buffer resource for data up to this capacity
	D3D11_BUFFER_DESC bufferdesc = {};
	bufferdesc.ByteWidth = (UINT)bytecount;

	if (((int)m_cpu_access & (int)CPUGraphicsResourceAccess::Read) != 0)
	{
		bufferdesc.Usage = D3D11_USAGE_STAGING;
		bufferdesc.CPUAccessFlags = (D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ);
	}
	else if (((int)m_cpu_access & (int)CPUGraphicsResourceAccess::Write) != 0)
	{
		bufferdesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	}
	else
	{
		bufferdesc.Usage = D3D11_USAGE_DEFAULT;
		bufferdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (m_isUAV)
		{
			bufferdesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}
	}

	bufferdesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferdesc.StructureByteStride = m_stride;

	D3D11_SUBRESOURCE_DATA subresourceData;
	subresourceData.pSysMem = (void*)m_data.data();
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;


	HRESULT result = device->CreateBuffer(&bufferdesc, &subresourceData, &m_buffer);
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Failed to initialise structured buffer (hr= " << result << ", bindflag=" << m_bindflags << ", bytewidth=" << bytecount << ", cpuaccess=" << m_cpu_access << ")\n";
		return;
	}

	if ((bufferdesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = m_buffer_elementcount[0];

		result = device->CreateShaderResourceView(m_buffer, &srvDesc, &(m_srv[0]));
		if (FAILED(result))
		{
			Game::Log << LOG_ERROR << "Failed to initialise structured buffer SRV resource (hr= " << result << ", elementcount=" << m_buffer_elementcount << ")\n";
			return;
		}
	}

	if ((bufferdesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) != 0)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = m_buffer_elementcount[0];
		uavDesc.Buffer.Flags = 0;

		result = device->CreateUnorderedAccessView(m_buffer, &uavDesc, &(m_uav[0]));
		if (FAILED(result))
		{
			Game::Log << LOG_ERROR << "Failed to initialise structured buffer UAV resource (hr=" << result << ", elementcount=" << m_buffer_elementcount << ")\n";
			return;
		}
	}
}

// Update data within the buffer
void StructuredBufferDX11::SetData(void* data, size_t elementSize, size_t offset, size_t numElements)
{
	// Make the assumption that sizeof(char) == 1
	unsigned char *first = (unsigned char*)data + (offset * elementSize);
	unsigned char *last = first + (numElements * elementSize);
	m_data.assign(first, last);
}

// Bind this resource to the given shader target
void StructuredBufferDX11::Bind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype)
{
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

// Return the UAV (if applicable)
ID3D11UnorderedAccessView *	StructuredBufferDX11::GetUnorderedAccessView() const
{
	return m_uav[0];
}


// Default destructor
StructuredBufferDX11::~StructuredBufferDX11(void)
{
	ReleaseIfExists(m_buffer);
	ReleaseIfExists(m_srv[0]);
	ReleaseIfExists(m_uav[0]);
}


















