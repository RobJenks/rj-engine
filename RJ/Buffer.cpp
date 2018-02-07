#include "Logging.h"
#include "Buffer.h"
#include "CoreEngine.h"

// Initialise static data
UINT Buffer::m_offset[1] = { 0 };						// Always constant, so maintain as static field
ID3D11Buffer * Buffer::null_buffer[1] = { nullptr };	// For more efficient unbinding


// Default constructor; create a null buffer with all default values
Buffer::Buffer(void) noexcept 
	:
	m_buffertype(BufferType::Unknown)
{
	m_buffer[0] = NULL;
	m_buffer_elementcount[0] = 1U;
	m_stride[0] = 0U;
	m_buffersize = (m_buffer_elementcount[0] * m_stride[0]);
}

// Construct a new buffer of the given type, and with the given parameters.  Will use a default buffer
// descriptor based upon the given buffer type
Buffer::Buffer(Buffer::BufferType buffertype, const void *data, UINT count, UINT stride) noexcept 
	:
	m_buffertype(buffertype)
{
	m_buffer[0] = NULL;
	m_buffer_elementcount[0] = count;
	m_stride[0] = stride;
	m_buffersize = (m_buffer_elementcount[0] * m_stride[0]);

	InitialiseBuffer(buffertype, Buffer::DetermineDefaultBufferDescriptor(buffertype), data, m_buffersize, &(m_buffer[0]));
}

// Construct a new buffer of the given type, with the specific buffer descriptor provided, and with 
// the given parameters for buffer content
Buffer::Buffer(Buffer::BufferType buffertype, const D3D11_BUFFER_DESC & buffer_desc, const void *data, UINT count, UINT stride) noexcept
	:
m_buffertype(buffertype)
{
	m_buffer[0] = NULL;
	m_buffer_elementcount[0] = count;
	m_stride[0] = stride;
	m_buffersize = (m_buffer_elementcount[0] * m_stride[0]);

	InitialiseBuffer(buffertype, buffer_desc, data, m_buffersize, &(m_buffer[0]));
}

// Move constructor
Buffer::Buffer(Buffer && other) noexcept
	:
	m_buffertype(other.m_buffertype)
{
	m_buffer_elementcount[0] = other.m_buffer_elementcount[0];
	m_stride[0] = other.m_stride[0];
	m_buffersize = other.m_buffersize;

	// Buffer resource is MOVED to prevent it being deallocated from the source class later (double-release -> fatal error)
	m_buffer[0] = other.m_buffer[0];
	other.m_buffer[0] = NULL;
}

// Move assignment
Buffer & Buffer::operator=(Buffer && other) noexcept
{
	m_buffertype = other.m_buffertype;
	m_buffer_elementcount[0] = other.m_buffer_elementcount[0];
	m_stride[0] = other.m_stride[0];
	m_buffersize = other.m_buffersize;

	// Buffer resource is MOVED to prevent it being deallocated from the source class later (double-release -> fatal error)
	m_buffer[0] = other.m_buffer[0];
	other.m_buffer[0] = NULL;

	return *this;
}

// Initialise a new D3D buffer with the given data
void Buffer::InitialiseBuffer(Buffer::BufferType buffer_type, D3D11_BUFFER_DESC buffer_desc, const void *data, UINT bytewidth, ID3D11Buffer **ppOutBuffer)
{
	D3D11_SUBRESOURCE_DATA resourceData;

	resourceData.pSysMem = data;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	// Make sure the given bytewidth matches the descriptor data provided (bytewidth would not have been known at
	// the time the descriptor was created)
	buffer_desc.ByteWidth = bytewidth;

	// We should not be silently overwriting an existing allocated COM buffer
	assert(m_buffer[0] == NULL);	

	// We need an initial block of data for all [immutable] buffer resources, so create temporarily here if required
	bool allocated = false;
	if (!resourceData.pSysMem)
	{
		allocated = true;
		resourceData.pSysMem = new char[bytewidth];
	}

	// Attempt to create the buffer
	HRESULT result = Game::Engine->GetDevice()->CreateBuffer(&buffer_desc, &resourceData, ppOutBuffer);

	// Deallocate any initial block of source data that was allocated, if applicable, before even checking the result
	if (allocated) SafeDeleteArray(resourceData.pSysMem);

	// Check whether the buffer creation succeeded
	if (FAILED(result))
	{
		Game::Log << LOG_ERROR << "Failed to create D3D buffer (hr=" << result << ", type=" << (int)buffer_type << ", bindflags=" << buffer_desc.BindFlags << 
			", bytewidth=" << bytewidth << ", data=" << (data ? "non-null" : "NULL") << ", ppB=" << (ppOutBuffer ? "non-null" : "NULL") << 
			(allocated ? ", {Initial-Allocated}" : "") << ", *ppB=" << (ppOutBuffer ? (*ppOutBuffer ? "non-null" : "NULL") : "N/A") << ")\n";
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



// Static methods to construct default buffer descriptors for a given type of buffer.  Descriptor
// provided will have a zero bytewidth that must be completed by the calling function before submission to the device
D3D11_BUFFER_DESC Buffer::DetermineDefaultBufferDescriptor(Buffer::BufferType buffertype)
{
	D3D11_BUFFER_DESC desc;

	desc.BindFlags = Buffer::DetermineDefaultBindFlags(buffertype);
	desc.ByteWidth = 0U;				// To be completed by caller
	desc.CPUAccessFlags = Buffer::DetermineDefaultCpuAccessFlags(buffertype);
	desc.MiscFlags = Buffer::DetermineDefaultMiscFlags(buffertype);
	desc.StructureByteStride = 0U;		// To be completed by StructuredBuffer types directly
	desc.Usage = Buffer::DetermineDefaultUsageType(buffertype);

	return desc;
}

UINT Buffer::DetermineDefaultBindFlags(Buffer::BufferType buffertype)
{
	switch (buffertype)
	{
		case Buffer::BufferType::VertexBuffer:		return D3D11_BIND_VERTEX_BUFFER;
		case Buffer::BufferType::IndexBuffer:		return D3D11_BIND_INDEX_BUFFER;
		case Buffer::BufferType::ConstantBuffer:	return D3D11_BIND_CONSTANT_BUFFER;

		case Buffer::BufferType::StructuredBuffer:	return D3D11_BIND_SHADER_RESOURCE;	// SB can have multiple types; default selected here

		default:
			Game::Log << LOG_WARN << "Attempted to determine bind flags for unsupported buffer type " << (int)buffertype << "; failure likely\n";
			return 0U;
	}
}

D3D11_USAGE Buffer::DetermineDefaultUsageType(Buffer::BufferType buffertype)
{
	switch (buffertype)
	{
		case Buffer::BufferType::VertexBuffer:		
		case Buffer::BufferType::IndexBuffer:		
													return D3D11_USAGE_DEFAULT;

		case Buffer::BufferType::ConstantBuffer:	
		case Buffer::BufferType::StructuredBuffer:										// SB can have multiple types; default selected here
													return D3D11_USAGE_DYNAMIC;			

		default:
			Game::Log << LOG_WARN << "Attempted to determine usage mode for unsupported buffer type " << (int)buffertype << "; failure likely\n";
			return D3D11_USAGE_DEFAULT;
	}
}

UINT Buffer::DetermineDefaultCpuAccessFlags(Buffer::BufferType buffertype)
{
	switch (buffertype)
	{
		case Buffer::BufferType::VertexBuffer:
		case Buffer::BufferType::IndexBuffer:
													return 0U;

		case Buffer::BufferType::ConstantBuffer:
		case Buffer::BufferType::StructuredBuffer:										// SB can have multiple types; default selected here
													return D3D11_CPU_ACCESS_WRITE;			

		default:
			Game::Log << LOG_WARN << "Attempted to determine CPU access flags for unsupported buffer type " << (int)buffertype << "; failure likely\n";
			return 0U;
	}
}

UINT Buffer::DetermineDefaultMiscFlags(Buffer::BufferType buffertype)
{
	switch (buffertype)
	{
		case Buffer::BufferType::VertexBuffer:
		case Buffer::BufferType::IndexBuffer:
		case Buffer::BufferType::ConstantBuffer:
													return 0U;
		case Buffer::BufferType::StructuredBuffer:	
													return D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

		default:
			Game::Log << LOG_WARN << "Attempted to determine misc buffer flags for unsupported buffer type " << (int)buffertype << "; failure likely\n";
			return 0U;
	}
}






