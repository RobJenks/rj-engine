#pragma once

#include "../Definitions/VertexDefinitions.hlsl.h"
#include "Buffer.h"

class IndexBuffer : public Buffer 
{
public:

	// Index format in use globally by the application; use 32- rather than 16-bit for more complex mesh support
	typedef INDEX_BUFFER_TYPE	INDEX_TYPE;
	static const DXGI_FORMAT	INDEX_FORMAT = DXGI_FORMAT_R32_UINT;

	
	// Default constructor; will not allocate any resources
	CMPINLINE IndexBuffer(void) : Buffer() { }

	// Construct an index buffer with the specified data
	IndexBuffer(const void *data, UINT count, UINT stride) noexcept;
	
	// Copy construction and assignment must be disallowed, since this IndexBuffer manages a single COM resource
	CMPINLINE			IndexBuffer(const IndexBuffer & other) = delete;
	CMPINLINE			IndexBuffer & operator=(const IndexBuffer & other) = delete;

	// Move constructor; simply delegate
	CMPINLINE			IndexBuffer(IndexBuffer && other) noexcept : Buffer(std::move(other)) { }

	// Move assignment; simply delegate
	CMPINLINE IndexBuffer & operator=(IndexBuffer && other) noexcept { return static_cast<IndexBuffer&>(Buffer::operator=(std::move(other))); }



	CMPINLINE auto		GetIndexCount(void) const	{ return m_buffer_elementcount[0]; }
	CMPINLINE auto		GetIndexSize(void) const	{ return m_stride[0]; }


	// Creates a sequential index buffer of the given length, using the default index format
	static IndexBuffer	CreateSequentialIndexBuffer(UINT count);


	// Destructor
	~IndexBuffer(void) noexcept;


private:


};