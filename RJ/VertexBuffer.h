#pragma once

#include "CompilerSettings.h"
#include "Buffer.h"
#include "../Definitions/ModelData.h"

class VertexBuffer : public Buffer
{
public:

	// Default constructor; will not allocate any resources
	CMPINLINE VertexBuffer(void) : Buffer() { }

	// Constructors
	VertexBuffer(const void *data, UINT count, UINT stride) noexcept;
	VertexBuffer(const D3D11_BUFFER_DESC & buffer_desc, const void *data, UINT count, UINT stride) noexcept;
	VertexBuffer(const ModelData & model_data) noexcept;

	// Copy construction and assignment must be disallowed, since this VertexBuffer manages a single COM resource
	CMPINLINE			VertexBuffer(const VertexBuffer & other) = delete;
	CMPINLINE			VertexBuffer & operator=(const VertexBuffer & other) = delete;

	// Move constructor; simply delegate
	CMPINLINE			VertexBuffer(VertexBuffer && other) noexcept : Buffer(std::move(other)) { }

	// Move assignment; simply delegate
	CMPINLINE VertexBuffer & operator=(VertexBuffer && other) noexcept { return static_cast<VertexBuffer&>(Buffer::operator=(std::move(other))); }


	CMPINLINE auto		GetVertexCount(void) const { return m_buffer_elementcount[0]; }
	CMPINLINE auto		GetVertexSize(void) const { return m_stride[0]; }


	// Destructor
	~VertexBuffer(void) noexcept;


private:


};