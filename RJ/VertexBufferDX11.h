#pragma once

#include "CompilerSettings.h"
#include "VertexBuffer.h"
#include "Shader.h"
#include "ShaderParameter.h"
#include "../Definitions/ModelData.h"


class VertexBufferDX11 : public VertexBuffer 
{
public:

	// Default constructor; will not allocate any resources
	CMPINLINE VertexBufferDX11(void) : VertexBuffer() { }

	// Constructors
	VertexBufferDX11(const void *data, UINT count, UINT stride) noexcept;
	VertexBufferDX11(const ModelData & model_data) noexcept;

	// Copy construction and assignment must be disallowed, since this VertexBuffer manages a single COM resource
	CMPINLINE			VertexBufferDX11(const VertexBufferDX11 & other) = delete;
	CMPINLINE			VertexBufferDX11 & operator=(const VertexBufferDX11 & other) = delete;

	// Move constructor; simply delegate
	CMPINLINE			VertexBufferDX11(VertexBufferDX11 && other) noexcept : VertexBuffer(std::move(other)) { }

	// Move assignment; simply delegate
	CMPINLINE VertexBufferDX11 & operator=(VertexBufferDX11 && other) noexcept { return static_cast<VertexBufferDX11&>(VertexBuffer::operator=(std::move(other))); }

	void Bind(UINT slot_id);
	void Unbind(UINT slot_id);

	// Destructor
	~VertexBufferDX11(void) noexcept;

private:


};