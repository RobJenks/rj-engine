#pragma once

#include "Buffer.h"

class VertexBuffer : public Buffer
{
public:

	VertexBuffer(void);
	VertexBuffer(const void *data, UINT count, UINT stride);

	~VertexBuffer(void);


private:


};