#pragma once

#include "IndexBuffer.h"

class IndexBufferDX11 : public IndexBuffer
{
public:

	// Default constructor; will not allocate any resources
	CMPINLINE IndexBufferDX11(void) : IndexBuffer() { }

	// Construct an index buffer with the specified data
	IndexBufferDX11(const void *data, UINT count, UINT stride) noexcept;

	// Construct an index buffer based on the given model data
	IndexBufferDX11(const ModelData & model_data) noexcept;

	// Construct a sequential index buffer of the specified length, using the default index format
	IndexBufferDX11(UINT count) noexcept;

	// Copy construction and assignment must be disallowed, since this IndexBuffer manages a single COM resource
	CMPINLINE			IndexBufferDX11(const IndexBufferDX11 & other) = delete;
	CMPINLINE			IndexBufferDX11 & operator=(const IndexBufferDX11 & other) = delete;

	// Move constructor; simply delegate
	CMPINLINE			IndexBufferDX11(IndexBufferDX11 && other) noexcept : IndexBuffer(std::move(other)) { }

	// Move assignment; simply delegate
	CMPINLINE IndexBufferDX11 & operator=(IndexBufferDX11 && other) noexcept { return static_cast<IndexBufferDX11&>(IndexBuffer::operator=(std::move(other))); }


	void Bind(UINT slot_id);
	void Unbind(UINT slot_id);


	// Destructor
	~IndexBufferDX11(void) noexcept;

private:

};