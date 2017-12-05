#pragma once

#include "VertexBuffer.h"
#include "Shader.h"
#include "ShaderParameter.h"


class VertexBufferDX11 : public VertexBuffer 
{
public:

	VertexBufferDX11(void);
	VertexBufferDX11(const void *data, UINT count, UINT stride);

	void Bind(UINT slot_id);
	void Unbind(UINT slot_id);

private:


};