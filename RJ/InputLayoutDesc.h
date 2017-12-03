#pragma once

#ifndef __InputLayoutDescH__
#define __InputLayoutDescH__

#include <vector>
#include <unordered_map>
#include "CompilerSettings.h"
#include "DX11_Core.h"

// This class has no special alignment requirements
class InputLayoutDesc
{
public:

	// Adds a new component to the input layout.  Returns a pointer to this object to allow chaining
	CMPINLINE InputLayoutDesc &							Add(LPCSTR SemanticName, UINT SemanticIndex, DXGI_FORMAT Format, UINT InputSlot,
															UINT AlignedByteOffset, D3D11_INPUT_CLASSIFICATION InputSlotClass, UINT InstanceDataStepRate)
	{
		D3D11_INPUT_ELEMENT_DESC element = { SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, InstanceDataStepRate };
		m_elements.push_back(element);

		return (*this);
	}

	// Returns a pointer to the input layout data
	CMPINLINE D3D11_INPUT_ELEMENT_DESC *				Data(void)					{ return (m_elements.empty() ? NULL : &(m_elements[0])); }
	CMPINLINE const D3D11_INPUT_ELEMENT_DESC *			Data(void) const			{ return (m_elements.empty() ? NULL : &(m_elements[0])); }

	// Returns the number of elements in the layout
	CMPINLINE UINT										ElementCount(void) const	{ return (UINT)m_elements.size(); }

	// Returns the total size of the data
	CMPINLINE SIZE_T									BufferSize(void) const		{ return (sizeof(D3D11_INPUT_ELEMENT_DESC) * m_elements.size()); }

	// Initialises the standard input layouts that are reusable across multiple shader scenarios
	static void											InitialiseStaticData(void);

	// Store a standard input layout in the static central collection
	static void											AddStandardLayout(const std::string & name, const InputLayoutDesc & layout);

	// Returns a standard input layout.  Returns a flag indicating whether the layout could be retrieved
	static bool											GetStandardLayout(const std::string & name, InputLayoutDesc & outLayout);

protected:

	// Collection of input layout elements in this description
	std::vector<D3D11_INPUT_ELEMENT_DESC>						m_elements;

	// Static collection of standard and reusable input layouts
	static std::unordered_map<std::string, InputLayoutDesc>		StandardLayouts;

};



#endif





