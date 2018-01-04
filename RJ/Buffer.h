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
	Buffer(void);

	// Construct a new buffer of the given type, and with the given parameters
	Buffer(Buffer::BufferType buffertype, const void *data, UINT count, UINT stride);

	// Initialise a new D3D buffer with the given data
	void InitialiseBuffer(UINT bindflags, const void *data, UINT count, UINT stride, ID3D11Buffer **ppOutBuffer);

	CMPINLINE BufferType		GetBufferType(void) const { return m_buffertype; }
	CMPINLINE const ID3D11Buffer *	GetCompiledBuffer(void) const { return m_buffer[0]; }
	CMPINLINE UINT				GetElementCount(void) const { return m_buffer_elementcount[0]; }
	CMPINLINE UINT				GetStride(void) const { return m_stride[0]; }
	CMPINLINE UINT				GetBindFlags(void) const { return m_bindflags; }
	
	static UINT					DetermineBindFlags(Buffer::BufferType buffertype);


	// Map data into this buffer
	template <class T>
	void						Set(const T *data);
	void						Set(const void *data, UINT data_size);


	// Destructor
	~Buffer(void);

protected:

	BufferType					m_buffertype;
	
	ID3D11Buffer *				m_buffer[1];
	UINT						m_buffer_elementcount[1];
	UINT						m_stride[1]; 
	UINT						m_bindflags;

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

