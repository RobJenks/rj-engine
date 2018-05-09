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
		Ambient = 0, 
		Emissive = 1, 
		Diffuse = 2,
		Specular = 3,
		SpecularPower = 4,
		Normal = 5, 
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