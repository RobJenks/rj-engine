#pragma once

#include "IndexBuffer.h"

class IndexBufferDX11 : public IndexBuffer
{
public:

	IndexBufferDX11(void);
	IndexBufferDX11(const void *data, UINT count, UINT stride);

	void Bind(UINT slot_id);
	void Unbind(UINT slot_id);


private:

};