#pragma once

#include "DX11_Core.h"
#include "CompilerSettings.h"


class VariableSizeValue
{
public:

	enum class SizeType { VectorSize = 0, VectorSizeExact, MaxDimension };

	CMPINLINE VariableSizeValue(void) : m_type(VariableSizeValue::SizeType::VectorSize), m_data(0.0f, 0.0f, 0.0f) { }
	CMPINLINE VariableSizeValue(SizeType type) : m_type(type), m_data(0.0f, 0.0f, 0.0f) { }
	CMPINLINE VariableSizeValue(float max_dimension) : m_type(VariableSizeValue::SizeType::MaxDimension), m_data(max_dimension, 0.0f, 0.0f) { }
	CMPINLINE VariableSizeValue(const XMFLOAT3 & size, bool exact = false) 
		: 
		m_type(exact ? VariableSizeValue::SizeType::VectorSizeExact : VariableSizeValue::SizeType::VectorSize), 
		m_data(size) 
	{ 
	}

	CMPINLINE VariableSizeValue(const FXMVECTOR size, bool exact = false)
		:
		m_type(exact ? VariableSizeValue::SizeType::VectorSizeExact : VariableSizeValue::SizeType::VectorSize)
	{
		XMStoreFloat3(&m_data, size);
	}

	CMPINLINE SizeType		GetType(void) const { return m_type; }
	CMPINLINE XMFLOAT3		GetVectorSize(void) const { return m_data; }
	CMPINLINE float			GetMaxDimension(void) const { return m_data.x; }

	CMPINLINE bool			IsExactSize(void) const { return (m_type == VariableSizeValue::SizeType::VectorSizeExact); }
	CMPINLINE bool			IsProportionPreserving(void) const { return (!IsExactSize()); }

	template <class T>
	CMPINLINE void			ApplyToObject(T * object) const
	{
		if (!object) return;
		switch (m_type)
		{
			case SizeType::VectorSize:		
				object->SetSize(m_data);
				break;

			case SizeType::VectorSizeExact:
				object->SetSize(m_data, false);		// (Size, PreserveProportions?)
				break;

			case SizeType::MaxDimension:
				object->SetSize(m_data.x);
				break;
		}
	}

private:

	SizeType				m_type;
	XMFLOAT3				m_data;
		


};
