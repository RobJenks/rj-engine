#pragma once

#include "IndexBuffer.h"

class IndexBufferDX11 : public IndexBuffer
{
public:

	// Construct an empty buffer
	IndexBufferDX11(void);

	// Construct an index buffer with the specified data
	IndexBufferDX11(const void *data, UINT count, UINT stride);

	// Construct a sequential index buffer of the specified length, using the default index format
	IndexBufferDX11(UINT count);

	void Bind(UINT slot_id);
	void Unbind(UINT slot_id);


private:

};