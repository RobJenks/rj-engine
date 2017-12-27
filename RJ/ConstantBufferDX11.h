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

	// Map data into this constant buffer
	template <class T>
	void						Set(const T *data);
	void						Set(const void *data, UINT data_size);

	// Bind this resource to the given shader target
	void						Bind(Shader::Type shadertype, Shader::SlotID slot_id) const;

	// Remove this (or any) binding from the given shader target
	void						Unbind(Shader::Type shadertype, Shader::SlotID slot_id) const;

	// Default destructor
	~ConstantBufferDX11(void);

private:

	// Constant buffers should always be limited to one item.  Buffersize is the preallocated size on construction
	ID3D11Buffer *				m_buffer[1];
	UINT						m_buffersize;

	// Static null buffer resource, used for more efficient unbinding
	static ID3D11Buffer * const	m_null_buffer[1];

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

// Static method to update the contents of the buffer with the given object data
template <class T>
void ConstantBufferDX11::Set(const T *data)
{
	Set((const void*)data, sizeof(T));
}
