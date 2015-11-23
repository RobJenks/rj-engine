#pragma once

#ifndef __InputLayoutDescH__
#define __InputLayoutDescH__

#include <vector>
#include "CompilerSettings.h"
#include "DX11_Core.h"

// This class has no special alignment requirements
class InputLayoutDesc
{
public:

	// Adds a new component to the input layout.  Returns a pointer to this object to allow chaining
	CMPINLINE InputLayoutDesc &						Add(LPCSTR SemanticName, UINT SemanticIndex, DXGI_FORMAT Format, UINT InputSlot,
														UINT AlignedByteOffset, D3D11_INPUT_CLASSIFICATION InputSlotClass, UINT InstanceDataStepRate)
	{
		D3D11_INPUT_ELEMENT_DESC element = { SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, InstanceDataStepRate };
		m_elements.push_back(element);

		return (*this);
	}

	// Returns a pointer to the input layout data
	CMPINLINE D3D11_INPUT_ELEMENT_DESC *			Data(void)					{ return (m_elements.empty() ? NULL : &(m_elements[0])); }

	// Returns the number of elements in the layout
	CMPINLINE UINT									ElementCount(void) const	{ return (UINT)m_elements.size(); }

	// Returns the total size of the data
	CMPINLINE SIZE_T								BufferSize(void) const		{ return (sizeof(D3D11_INPUT_ELEMENT_DESC) * m_elements.size()); }

protected:

	std::vector<D3D11_INPUT_ELEMENT_DESC>			m_elements;

};




#endif