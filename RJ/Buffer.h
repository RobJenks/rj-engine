#pragma once

#include "CompilerSettings.h"

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

	Buffer(void);

	CMPINLINE BufferType		GetBufferType(void) const { return m_buffertype; }
	CMPINLINE UINT				GetBufferElementCount(void) const { return m_buffer_elementcount; }
	

protected:

	BufferType					m_buffertype;
	UINT						m_buffer_elementcount;



};

