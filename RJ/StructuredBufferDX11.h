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
	static StructuredBufferDX11	* Create(	UINT bindFlags, const void* data, UINT element_count, UINT stride, 
											CPUGraphicsResourceAccess cpuAccess = CPUGraphicsResourceAccess::None, bool isUAV = false);

	// Update data within the buffer
	void						SetData(void* data, size_t elementSize, size_t offset, size_t numElements);

	// Bind this resource to the given shader target
	void						Bind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype);

	// Remove this (or any) binding from the given shader target
	void						Unbind(Shader::Type shadertype, Shader::SlotID slot_id, ShaderParameter::Type parametertype);

	// Return the resources encapsulated by this buffer (where applicable)
	ID3D11ShaderResourceView *	GetShaderResourceView(void) const;
	ID3D11UnorderedAccessView *	GetUnorderedAccessView(void) const;

	// Default destructor
	~StructuredBufferDX11(void);


private:

	// Private constructor; new structured buffers should be instantiated through StructuredBufferDX11::Create()
	StructuredBufferDX11(const D3D11_BUFFER_DESC & buffer_desc, const void* data, UINT element_count, UINT stride);


	// GPU resources
	ID3D11ShaderResourceView *			m_srv[1];
	ID3D11UnorderedAccessView *			m_uav[1];

	// The system data buffer
	typedef uint8_t								BufferElementType;
	typedef std::vector<BufferElementType>		BufferType;
	BufferType									m_data;

	// The last slot the UAV was bound to
	UINT								m_last_slot;

		
	// Static null buffer resources, used for more efficient unbinding
	static ID3D11ShaderResourceView * const	m_null_srv[1];
	static ID3D11UnorderedAccessView * const m_null_uav[1];

};