#pragma once

#include "CompilerSettings.h"


class Material
{
public:

	// Custom type for material ID
	typedef unsigned int		MaterialID;				

	// Texture indices.  Texture resources will be bound to these registers in all shader programs, if they exist
	enum class TextureType
	{
		Diffuse = 0,
		Normal = 1,
		Ambient = 2,
		Emissive = 3,
		Specular = 4,
		SpecularPower = 5,
		Bump = 6,
		Opacity = 7,

		TEXTURE_TYPE_COUNT = 8
	};

	Material(void);
	~Material(void);

	CMPINLINE MaterialID		GetID(void) const { return m_id; }
	void						AssignNewUniqueID(void);

protected:

private:

	// Unique material ID
	MaterialID					m_id; 
	static MaterialID			GlobalMaterialIDCount;	// Monotonically-increasing ID counter for material objects
	CMPINLINE static MaterialID NewID(void) { return ++GlobalMaterialIDCount; }

};