#pragma once

#include "CompilerSettings.h"
#include "Buffer.h"
#include "../Definitions/ModelData.h"

class VertexBuffer : public Buffer
{
public:

	VertexBuffer(void);
	VertexBuffer(const void *data, UINT count, UINT stride);
	VertexBuffer(const ModelData & model_data);

	CMPINLINE auto		GetVertexCount(void) const { return m_buffer_elementcount[0]; }
	CMPINLINE auto		GetVertexSize(void) const { return m_stride[0]; }


	~VertexBuffer(void);


private:


};