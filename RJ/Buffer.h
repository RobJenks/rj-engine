#pragma once

#include "CompilerSettings.h"
#include "DX11_Core.h"

class Buffer
{
public:

	enum class BufferType
	{
		Unknown = 0,
		VertexBuffer,
		IndexBuffer,
		StructuredBuffer,
		ConstantBuffer
	};

	// Default constructor
	Buffer(void) noexcept;

	// Construct a new buffer of the given type, and with the given parameters.  Will use a default buffer
	// descriptor based upon the given buffer type
	Buffer(Buffer::BufferType buffertype, const void *data, UINT count, UINT stride) noexcept;

	// Construct a new buffer of the given type, with the specific buffer descriptor provided, and with 
	// the given parameters for buffer content
	Buffer::Buffer(Buffer::BufferType buffertype, const D3D11_BUFFER_DESC & buffer_desc, const void *data, UINT count, UINT stride) noexcept;

	// Copy construction and assignment must be disallowed, since this buffer manages a single COM resource
	CMPINLINE					Buffer(const Buffer & other) = delete;
	CMPINLINE					Buffer & operator=(const Buffer & other) = delete;

	// Move constructor
	Buffer(Buffer && other) noexcept;

	// Move assignment
	Buffer & operator=(Buffer && other) noexcept;


	CMPINLINE BufferType		GetBufferType(void) const { return m_buffertype; }
	CMPINLINE ID3D11Buffer *	GetCompiledBuffer(void) const { return m_buffer[0]; }
	CMPINLINE UINT				GetElementCount(void) const { return m_buffer_elementcount[0]; }
	CMPINLINE UINT				GetStride(void) const { return m_stride[0]; }
	

	// Static method to construct a buffer descriptor based on the given parameters
	static D3D11_BUFFER_DESC	CreateBufferDescriptor(UINT bind_flags, D3D11_USAGE usage_type, UINT cpu_access, UINT misc_flags, UINT bytewidth = 0U, UINT structure_stride = 0U);

	// Static methods to construct default buffer descriptors for a given type of buffer
	static D3D11_BUFFER_DESC	DetermineDefaultBufferDescriptor(Buffer::BufferType buffertype);
	static UINT					DetermineDefaultBindFlags(Buffer::BufferType buffertype);
	static D3D11_USAGE			DetermineDefaultUsageType(Buffer::BufferType buffertype);
	static UINT					DetermineDefaultCpuAccessFlags(Buffer::BufferType buffertype);
	static UINT					DetermineDefaultMiscFlags(Buffer::BufferType buffertype);


	// Map data into this buffer
	template <class T>
	void						Set(const T *data);
	void						Set(const void *data, UINT data_size);


	// Destructor
	~Buffer(void) noexcept;

protected:

	// Initialise a new D3D buffer with the given data
	void						InitialiseBuffer(Buffer::BufferType buffer_type, D3D11_BUFFER_DESC buffer_desc, const void *data, UINT bytewidth, ID3D11Buffer **ppOutBuffer);



	BufferType					m_buffertype;
	
	ID3D11Buffer *				m_buffer[1];
	UINT						m_buffer_elementcount[1];
	UINT						m_stride[1];
	UINT						m_buffersize;		// (ElementCount * Stride == Total buffer size)
	
	static UINT					m_offset[1];		// Always constant, so maintain as static field
	static ID3D11Buffer *		null_buffer[1];		// For more efficient unbinding
};



// Static method to update the contents of the buffer with the given object data
// Maps a single instance of T into the buffer, from the location pointed to by 'data'
template <class T>
void Buffer::Set(const T *data)
{
	Set((const void*)data, sizeof(T));
}

