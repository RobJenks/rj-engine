#include "FastMath.h"
#include "ShaderDX11.h"
#include "TextureDX11.h"
#include "MaterialDX11.h"
#include "ConstantBufferDX11.h"

// Initialise static variables
MaterialDX11::MaterialID MaterialDX11::GlobalMaterialIDCount = 0U;

// Default constructor
MaterialDX11::MaterialDX11(void)
	:
	m_updates_suspended(false), 
	m_texture_binding_count(0U)
{
	m_textures = { };
	m_texture_bindings = { };

	// Initialise the constant buffer
	m_cbuffer = ConstantBufferDX11::Create(&Data);
}

// Default copy constructor
MaterialDX11::MaterialDX11(const MaterialDX11 &other)
	:
	Data(other.Data), 
	m_textures(other.m_textures)
{
	AssignNewUniqueID();
}

bool MaterialDX11::IsTransparent(void) const
{
	return (Data.Opacity < 1.0f || Data.HasOpacityTexture);
}

void MaterialDX11::ResumeUpdates(void)
{
	m_updates_suspended = false;
	CompileMaterial();
}

// Compile the material into its constant buffer representation
void MaterialDX11::CompileMaterial(void)
{
	if (m_updates_suspended) return;

	m_cbuffer->Set(&Data);
}

// Texture collection for this material
const TextureDX11 *	MaterialDX11::GetTexture(TextureType type) const
{
	assert((unsigned int)type < MaterialDX11::MaterialTextureTypeCount);

	return m_textures[(int)type];
}

// Texture collection for this material
const MaterialDX11::MaterialTextureSet & MaterialDX11::GetTextures(void) const
{
	return m_textures;
}

// Texture collection for this material
void MaterialDX11::SetTexture(TextureType type, TextureDX11 *texture)
{
	assert((unsigned int)type < MaterialDX11::MaterialTextureTypeCount);

	m_textures[(int)type] = texture;
	UpdateTextureState();
}

// Texture collection for this material
void MaterialDX11::SetTextures(const MaterialTextureSet & textures)
{
	m_textures = textures;
	UpdateTextureState();
}

// Update the texture state flags following a change to the object texture resources
void MaterialDX11::UpdateTextureState(void)
{
	Data.HasDiffuseTexture = (m_textures[(int)TextureType::Diffuse] != NULL);
	Data.HasNormalTexture = (m_textures[(int)TextureType::Normal] != NULL);
	Data.HasAmbientTexture = (m_textures[(int)TextureType::Ambient] != NULL);
	Data.HasEmissiveTexture = (m_textures[(int)TextureType::Emissive] != NULL);
	Data.HasSpecularTexture = (m_textures[(int)TextureType::Specular] != NULL);
	Data.HasSpecularPowerTexture = (m_textures[(int)TextureType::SpecularPower] != NULL);
	Data.HasBumpTexture = (m_textures[(int)TextureType::Bump] != NULL);
	Data.HasOpacityTexture = (m_textures[(int)TextureType::Opacity] != NULL);

	// Update the smaller set of bindable textures whenever the underlying textures change
	m_texture_binding_count = 0U;
	for (size_t i = 0; i < (size_t)TextureType::TEXTURE_TYPE_COUNT; ++i)
	{
		if (m_textures[i] == NULL) continue;

		m_texture_bindings[m_texture_binding_count] = TextureBinding(m_textures[i], i);
		++m_texture_binding_count;
	}


	// Compiile the constant buffer holding all non-texture material data (incl. texture-related flags)
	CompileMaterial();
}

// Bind this material to the current rendering pipeline
void MaterialDX11::Bind(ShaderDX11 *shader) const
{
	if (!shader) return;

	// Bind each texture in turn
	Shader::Type shadertype = shader->GetType();
	for (size_t i = 0; i < m_texture_binding_count; ++i)
	{
		m_texture_bindings[i].texture->Bind(shadertype, i, ShaderParameter::Type::Texture);
	}

	// Also bind the material constant buffer to the relevant shader parameter, if the shader requires it
	Shader::SlotID material_slot = shader->GetMaterialSlot();
	if (material_slot != Shader::NO_SLOT_ID)
	{
		m_cbuffer->Bind(shadertype, material_slot);
	}
}


// Default destructor
MaterialDX11::~MaterialDX11(void)
{
	SafeDelete(m_cbuffer);
}


