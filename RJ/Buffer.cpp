#include "Logging.h"
#include "Buffer.h"
#include "CoreEngine.h"

// Initialise static data
UINT Buffer::m_offset[1] = { 0 };						// Always constant, so maintain as static field
ID3D11Buffer * Buffer::null_buffer[1] = { nullptr };	// For more efficient unbinding


// Default constructor; create a null buffer with all default values
Buffer::Buffer(void) noexcept 
	:
	m_buffertype(BufferType::Unknown), 
	m_bindflags(0U)
{
	m_buffer[0] = NULL;
	m_buffer_elementcount[0] = 1U;
	m_stride[0] = 0U;
}

// Construct a new buffer of the given type, and with the given parameters
Buffer::Buffer(Buffer::BufferType buffertype, const void *data, UINT count, UINT stride) noexcept 
	:
	m_buffertype(buffertype)
{
	m_bindflags = Buffer::DetermineBindFlags(buffertype);
	m_buffer[0] = NULL;
	m_buffer_elementcount[0] = count;
	m_stride[0] = stride;

	InitialiseBuffer(m_bindflags, data, count, stride, &(m_buffer[0]));
}

// Move constructor
Buffer::Buffer(Buffer && other) noexcept
	:
	m_bindflags(other.m_bindflags), 
	m_buffertype(other.m_buffertype)
{
	m_buffer_elementcount[0] = other.m_buffer_elementcount[0];
	m_stride[0] = other.m_stride[0];

	// Buffer resource is MOVED to prevent it being deallocated from the source class later (double-release -> fatal error)
	m_buffer[0] = other.m_buffer[0];
	other.m_buffer[0] = NULL;
}

// Move assignment
Buffer & Buffer::operator=(Buffer && other) noexcept
{
	m_bindflags = other.m_bindflags;
	m_buffertype = other.m_buffertype;
	m_buffer_elementcount[0] = other.m_buffer_elementcount[0];
	m_stride[0] = other.m_stride[0];

	// Buffer resource is MOVED to prevent it being deallocated from the source class later (double-release -> fatal error)
	m_buffer[0] = other.m_buffer[0];
	other.m_buffer[0] = NULL;

	return *this;
}

// Initialise a new D3D buffer with the given data
void Buffer::InitialiseBuffer(UINT bindflags, const void *data, UINT count, UINT stride, ID3D11Buffer **ppOutBuffer)
{
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA resourceData;

	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = bindflags;
	bufferDesc.ByteWidth = count * stride;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	resourceData.pSysMem = data;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	assert(m_buffer[0] == NULL);	// We should not be silently overwriting an existing allocated COM buffer

	HRESULT result = Game::Engine->GetDevice()->CreateBuffer(&bufferDesc, &resourceData, ppOutBuffer);
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Failed to create D3D buffer (hr=" << result << ", bindflags=" << bindflags << ", count=" << count << ", stride=" << stride <<
			", data=" << (data ? "non-null" : "NULL") << ", ppB=" << (ppOutBuffer ? "non-null" : "NULL") << 
			", *ppB=" << (ppOutBuffer ? (*ppOutBuffer ? "non-null" : "NULL") : "N/A") << ")\n";
	}
}

UINT Buffer::DetermineBindFlags(Buffer::BufferType buffertype)
{
	switch (buffertype)
	{
	case Buffer::BufferType::VertexBuffer:		return D3D11_BIND_VERTEX_BUFFER;
	case Buffer::BufferType::IndexBuffer:		return D3D11_BIND_INDEX_BUFFER;
	case Buffer::BufferType::ConstantBuffer:	return D3D11_BIND_CONSTANT_BUFFER;

	case Buffer::BufferType::StructuredBuffer:	/* Fallthrough: not relevant here */
	default:
		Game::Log << LOG_WARN << "Attempted to determine bind flags for unsupported buffer type " << (int)buffertype << "; failure likely\n";
		return 0U;
	}
}

// Map data into this buffer
void Buffer::Set(const void *data, UINT data_size)
{
	auto devicecontext = Game::Engine->GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mappedresource;
	if (FAILED(devicecontext->Map(m_buffer[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedresource)))
	{
		return;
	}

	memcpy(mappedresource.pData, data, data_size);
	devicecontext->Unmap(m_buffer[0], 0);
}


// Destructor
Buffer::~Buffer(void) noexcept
{
	ReleaseIfExists(m_buffer[0]);
}