#pragma once

#ifndef __MaterialH__
#define __MaterialH__

#include "MaterialData.hlsl.h"
#include "CompilerSettings.h"

class Material
{
public:

	typedef unsigned int		MaterialID;				// Custom type for material ID
	static MaterialID			GlobalMaterialIDCount;	// Monotonically-increasing ID counter for material objects
	static const MaterialID		MATERIAL_LIMIT;			// Maximum number of materials that can be rendered in one pass

	// Default constructor
	Material(void);
	
	// Unique material ID
	CMPINLINE MaterialID		GetID(void) const { return m_id; }

	// Material data; held in structure common to both cpp and hlsl builds
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
	CMPINLINE void				SetGlobalAmbient(float4 value) { Data.GlobalAmbient = value; }
	CMPINLINE void				SetAmbientColour(float4 value) { Data.AmbientColor = value; }
	CMPINLINE void				SetEmissiveColour(float4 value) { Data.EmissiveColor = value; }
	CMPINLINE void				SetDiffuseColour(float4 value) { Data.DiffuseColor = value; }
	CMPINLINE void				SetSpecularColour(float4 value) { Data.SpecularColor = value; }
	CMPINLINE void				SetReflectance(float4 value) { Data.Reflectance = value; }
	CMPINLINE void				SetOpacity(float value) { Data.Opacity = value; }
	CMPINLINE void				SetSpecularPower(float value) { Data.SpecularPower = value; }
	CMPINLINE void				SetSpecularScale(float value) { Data.SpecularScale = value; }
	CMPINLINE void				SetIndexOfRefraction(float value) { Data.IndexOfRefraction = value; }
	CMPINLINE void				SetHasAmbientTexture(bool value) { Data.HasAmbientTexture = value; }
	CMPINLINE void				SetHasEmissiveTexture(bool value) { Data.HasEmissiveTexture = value; }
	CMPINLINE void				SetHasDiffuseTexture(bool value) { Data.HasDiffuseTexture = value; }
	CMPINLINE void				SetHasSpecularTexture(bool value) { Data.HasSpecularTexture = value; }
	CMPINLINE void				SetHasSpecularPowerTexture(bool value) { Data.HasSpecularPowerTexture = value; }
	CMPINLINE void				SetHasNormalTexture(bool value) { Data.HasNormalTexture = value; }
	CMPINLINE void				SetHasBumpTexture(bool value) { Data.HasBumpTexture = value; }
	CMPINLINE void				SetHasOpacityTexture(bool value) { Data.HasOpacityTexture = value; }
	CMPINLINE void				SetBumpIntensity(float value) { Data.BumpIntensity = value; }
	CMPINLINE void				SetAlphaThreshold(float value) { Data.AlphaThreshold = value; }


	// Default destructor
	~Material(void);

private:

	// Unique material ID
	MaterialID					m_id;
	CMPINLINE void				AssignUniqueID(void) { m_id = ++GlobalMaterialIDCount; }


};



#endif


