#pragma once

#include "DX11_Core.h"
#include "Shader.h"


class ConstantBufferDX11
{
public:

	// Constructor; create a new constant buffer of the given size
	ConstantBufferDX11(UINT buffer_size);

	// Map data into this constant buffer
	void						Set(const void *data, UINT data_size);

	// Bind this resource to the given shader target
	void						Bind(Shader::Type shadertype, Shader::SlotID slot_id);

	// Remove this (or any) binding from the given shader target
	void						Unbind(Shader::Type shadertype, Shader::SlotID slot_id);

	// Default destructor
	~ConstantBufferDX11(void);

private:

	// Constant buffers should always be limited to one item.  Buffersize is the preallocated size on construction
	ID3D11Buffer *				m_buffer[1];
	UINT						m_buffersize;

	// Static null buffer resource, used for more efficient unbinding
	static ID3D11Buffer * const	m_null_buffer[1];

};