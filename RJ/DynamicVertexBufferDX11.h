#pragma once

#include "VertexBufferDX11.h"


// Specialised subclass of the vertex buffer which allows dynamic CPU access to buffer contents (as
// opposed to regular vertex buffers which are immutable once created)
class DynamicVertexBufferDX11 : public VertexBufferDX11
{
public:

	// Constructors
	DynamicVertexBufferDX11(const void *data, UINT count, UINT stride) noexcept;
	
	// Copy construction and assignment must be disallowed, since this VertexBuffer manages a single COM resource
	CMPINLINE			DynamicVertexBufferDX11(const DynamicVertexBufferDX11 & other) = delete;
	CMPINLINE			DynamicVertexBufferDX11 & operator=(const DynamicVertexBufferDX11 & other) = delete;

	// Move constructor; simply delegate
	CMPINLINE			DynamicVertexBufferDX11(DynamicVertexBufferDX11 && other) noexcept : VertexBufferDX11(std::move(other)) { }

	// Move assignment; simply delegate
	CMPINLINE DynamicVertexBufferDX11 & operator=(DynamicVertexBufferDX11 && other) noexcept { return static_cast<DynamicVertexBufferDX11&>(VertexBufferDX11::operator=(std::move(other))); }

	
	// Destructor
	~DynamicVertexBufferDX11(void) noexcept;



private:

};