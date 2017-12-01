#pragma once

#include <vector>
#include "DX11_Core.h"
#include "Shader.h"
#include "ShaderParameter.h"
#include "StructuredBuffer.h"
#include "CPUGraphicsResourceAccess.h"

class StructuredBufferDX11 : public StructuredBuffer
{
public:

	// Construct a new structured buffer resource
	StructuredBufferDX11(UINT bindFlags, const void* data, size_t element_count, UINT stride, CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool isUAV = false);

	// Update data within the buffer
	void						SetData(void* data, size_t elementSize, size_t offset, size_t numElements);

	// Bind this resource to the given shader target
	void						Bind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype);

	// Remove this (or any) binding from the given shader target
	void						Unbind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype);

	// Return the UAV (if applicable)
	ID3D11UnorderedAccessView*	GetUnorderedAccessView() const;

	// Default destructor
	StructuredBufferDX11(void);


private:

	// GPU resources
	ID3D11Buffer * 						m_buffer;
	ID3D11ShaderResourceView *			m_srv[1];
	ID3D11UnorderedAccessView *			m_uav[1];

	// The system data buffer
	typedef uint8_t								BufferElementType;
	typedef std::vector<BufferElementType>		BufferType;
	BufferType									m_data;

	// The stride of the vertex buffer in bytes
	UINT								m_stride;

	// Flags indicating how this buffer should be bound
	UINT								m_bindflags;

	// The last slot the UAV was bound to
	UINT								m_last_slot;

	bool								m_isUAV;			// UAV, requiring GPU-write access?
	CPUGraphicsResourceAccess			m_cpu_access;		// Level of CPU access required
	bool								m_is_dynamic;		// Requires CPU read or write access?
		
	// Static null buffer resources, used for more efficient unbinding
	static ID3D11ShaderResourceView * const	m_null_srv[1];
	static ID3D11UnorderedAccessView * const m_null_uav[1];

};