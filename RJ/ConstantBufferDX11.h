#pragma once

#include "DX11_Core.h"
#include "Shader.h"
#include "ConstantBuffer.h"


class ConstantBufferDX11 : public ConstantBuffer
{
public:

	// Static method to construct a new constant buffer for the given data
	template <class T>
	static ConstantBufferDX11 * Create(const T * data);

	// Constructor; create a new constant buffer of the given size
	ConstantBufferDX11(UINT buffer_size);

	// Bind this resource to the given shader target
	void						Bind(Shader::Type shadertype, Shader::SlotID slot_id) const;

	// Remove this (or any) binding from the given shader target
	void						Unbind(Shader::Type shadertype, Shader::SlotID slot_id) const;

	// Default destructor
	~ConstantBufferDX11(void);

private:

	// Buffersize is the preallocated size on construction
	UINT						m_buffersize;

	// Static null buffer resource, used for more efficient unbinding
	//static ID3D11Buffer * const	m_null_buffer[1];

};

// Static method to construct a new constant buffer for the given data
template <class T>
static ConstantBufferDX11 * ConstantBufferDX11::Create(const T *data)
{
	ConstantBufferDX11 *buffer = new ConstantBufferDX11(sizeof(T));
	if (data)
	{
		buffer->Set((const void*)data, sizeof(T));
	}

	return buffer;
}
