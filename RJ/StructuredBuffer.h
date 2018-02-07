#pragma once

#include "Buffer.h"

class StructuredBuffer : public Buffer
{
public:

	StructuredBuffer(const D3D11_BUFFER_DESC & buffer_desc, const void *data, UINT count, UINT stride);


private:


};