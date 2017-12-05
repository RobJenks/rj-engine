#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer 
{
public:

	// Index format in use globally by the application; use 32- rather than 16-bit for more complex mesh support
	static const DXGI_FORMAT INDEX_FORMAT = DXGI_FORMAT_R32_UINT;

	IndexBuffer(void);
	IndexBuffer(const void *data, UINT count, UINT stride);

	CMPINLINE auto		GetIndexCount(void) const	{ return m_buffer_elementcount[0]; }
	CMPINLINE auto		GetIndexSize(void) const	{ return m_stride[0]; }


	~IndexBuffer(void);


private:


};