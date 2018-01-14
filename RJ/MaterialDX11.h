#pragma once

#ifndef __MaterialH__
#define __MaterialH__

#include <array>
#include "Material.h"
#include "../Definitions/MaterialData.hlsl.h"
#include "CompilerSettings.h"
class ShaderDX11;
class TextureDX11;
class ConstantBufferDX11;

class MaterialDX11 : public Material
{
public:

	typedef std::array<TextureDX11*, (size_t)TextureType::TEXTURE_TYPE_COUNT> MaterialTextureSet;
	static const MaterialTextureSet::size_type MaterialTextureTypeCount = (MaterialTextureSet::size_type)Material::TextureType::TEXTURE_TYPE_COUNT;

	// Default constructor
	MaterialDX11(void);

	// Default copy constructor
	MaterialDX11(const MaterialDX11 &other);

	// Material data; held in structure common to both cpp and hlsl builds
	// TODO: This data needs to be 16-bit aligned.  This is therefore currently only guaranteed to work
	// in 64-bit where alignment is always to 16-bit word boundaries.  If we want to support 32-bit this
	// data will need to be heap-allocated using an aligned allocator
	MaterialData				Data;

	// Access material data
	CMPINLINE auto				GetGlobalAmbient(void) const		{ return Data.GlobalAmbient; }
	CMPINLINE auto				GetAmbientColour(void) const		{ return Data.AmbientColor; }
	CMPINLINE auto				GetEmissiveColour(void) const		{ return Data.EmissiveColor; }
	CMPINLINE auto				GetDiffuseColour(void) const		{ return Data.DiffuseColor; }
	CMPINLINE auto				GetSpecularColour(void) const		{ return Data.SpecularColor; }
	CMPINLINE auto				GetReflectance(void) const			{ return Data.Reflectance; }
	CMPINLINE auto				GetOpacity(void) const				{ return Data.Opacity; }
	CMPINLINE auto				GetSpecularPower(void) const		{ return Data.SpecularPower; }
	CMPINLINE auto				GetSpecularScale(void) const		{ return Data.SpecularScale; }
	CMPINLINE auto				GetIndexOfRefraction(void) const	{ return Data.IndexOfRefraction; }
	CMPINLINE auto				HasAmbientTexture(void) const		{ return Data.HasAmbientTexture; }
	CMPINLINE auto				HasEmissiveTexture(void) const		{ return Data.HasEmissiveTexture; }
	CMPINLINE auto				HasDiffuseTexture(void) const		{ return Data.HasDiffuseTexture; }
	CMPINLINE auto				HasSpecularTexture(void) const		{ return Data.HasSpecularTexture; }
	CMPINLINE auto				HasSpecularPowerTexture(void) const { return Data.HasSpecularPowerTexture; }
	CMPINLINE auto				HasNormalTexture(void) const		{ return Data.HasNormalTexture; }
	CMPINLINE auto				HasBumpTexture(void) const			{ return Data.HasBumpTexture; }
	CMPINLINE auto				HasOpacityTexture(void) const		{ return Data.HasOpacityTexture; }
	CMPINLINE auto				GetBumpIntensity(void) const		{ return Data.BumpIntensity; }
	CMPINLINE auto				GetAlphaThreshold(void) const		{ return Data.AlphaThreshold; }

	// Store material data
	CMPINLINE void				SetGlobalAmbient(float4 value)			{ Data.GlobalAmbient = value; CompileMaterial(); }
	CMPINLINE void				SetAmbientColour(float4 value)			{ Data.AmbientColor = value; CompileMaterial(); }
	CMPINLINE void				SetEmissiveColour(float4 value)			{ Data.EmissiveColor = value; CompileMaterial(); }
	CMPINLINE void				SetDiffuseColour(float4 value)			{ Data.DiffuseColor = value; CompileMaterial(); }
	CMPINLINE void				SetSpecularColour(float4 value)			{ Data.SpecularColor = value; CompileMaterial(); }
	CMPINLINE void				SetReflectance(float4 value)			{ Data.Reflectance = value; CompileMaterial(); }
	CMPINLINE void				SetOpacity(float value)					{ Data.Opacity = value; CompileMaterial(); }
	CMPINLINE void				SetSpecularPower(float value)			{ Data.SpecularPower = value; CompileMaterial(); }
	CMPINLINE void				SetSpecularScale(float value)			{ Data.SpecularScale = value; CompileMaterial(); }
	CMPINLINE void				SetIndexOfRefraction(float value)		{ Data.IndexOfRefraction = value; CompileMaterial(); }
	CMPINLINE void				SetHasAmbientTexture(bool value)		{ Data.HasAmbientTexture = value; CompileMaterial(); }
	CMPINLINE void				SetHasEmissiveTexture(bool value)		{ Data.HasEmissiveTexture = value; CompileMaterial(); }
	CMPINLINE void				SetHasDiffuseTexture(bool value)		{ Data.HasDiffuseTexture = value; CompileMaterial(); }
	CMPINLINE void				SetHasSpecularTexture(bool value)		{ Data.HasSpecularTexture = value; CompileMaterial(); }
	CMPINLINE void				SetHasSpecularPowerTexture(bool value)	{ Data.HasSpecularPowerTexture = value; CompileMaterial(); }
	CMPINLINE void				SetHasNormalTexture(bool value)			{ Data.HasNormalTexture = value; CompileMaterial(); }
	CMPINLINE void				SetHasBumpTexture(bool value)			{ Data.HasBumpTexture = value; CompileMaterial(); }
	CMPINLINE void				SetHasOpacityTexture(bool value)		{ Data.HasOpacityTexture = value; CompileMaterial(); }
	CMPINLINE void				SetBumpIntensity(float value)			{ Data.BumpIntensity = value; CompileMaterial(); }
	CMPINLINE void				SetAlphaThreshold(float value)			{ Data.AlphaThreshold = value; CompileMaterial(); }

	bool						IsTransparent(void) const;

	// Texture collection for this material
	const TextureDX11 *			GetTexture(TextureType type) const;
	const MaterialTextureSet &	GetTextures(void) const;
	void						SetTexture(TextureType type, TextureDX11 *texture);
	void						SetTextures(const MaterialTextureSet & textures);


	// Bind this material to the current rendering pipeline
	void						Bind(ShaderDX11 *shader) const;

	// We can suspend recompilation of the material while setting multiple properties sequentially
	CMPINLINE void				SuspendUpdates(void) { m_updates_suspended = true; }
	void						ResumeUpdates(void);

	// Default destructor
	~MaterialDX11(void);


private:

	struct TextureBinding
	{
		TextureDX11 * texture;
		unsigned int slot;

		TextureBinding(void) : texture(NULL), slot(0U) { }
		TextureBinding(TextureDX11 * _texture, unsigned int _slot) : texture(_texture), slot(_slot) { }
	};


private:

	// Pointer to each texture that may be assigned to the material
	MaterialTextureSet			m_textures;

	// Material data is compiled into a constant buffer following any change
	ConstantBufferDX11 *		m_cbuffer;

	// Smaller collapsed set of only textures which are attached to this material, for binding efficiency
	std::array<TextureBinding, MaterialTextureTypeCount> m_texture_bindings;
	std::array<TextureBinding, MaterialTextureTypeCount>::size_type m_texture_binding_count;

	// Other material properties
	bool						m_updates_suspended;

	// Update the material state and recompile following a change to the object resources
	void						UpdateMaterialState(void);

	// Compile the material into its constant buffer representation
	void						CompileMaterial(void);
};



#endif


