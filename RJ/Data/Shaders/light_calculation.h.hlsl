#if !defined(__light_calculationH__)
#define __light_calculationH__

#include "material_definition.h"
#include "light_definition.h"

// Base method to calculate the light value for a basic (e.g. directional) light
// 'index' is index into the light data array
// 'material' is the index of the material being illuminated
// 'world_position' is the world position of this pixel
// 'light_direction' is the direction of light intersecting the pixel, in world space
// 'normal' is the normalised vertex normal, calculated once and passed in for each method call here
float4 CalculateLightBase(unsigned int index, MATERIAL_ID material, float3 world_position, float3 light_direction, float3 normal)
{
	float4 ambient_colour = float4(Lights[index].Colour, 1.0f) * Lights[index].AmbientIntensity;
	float4 diffuse_colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular_colour = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float diffusefactor = dot(normal, -light_direction);
	if (diffusefactor > 0.0f)
	{
		diffuse_colour = float4(Lights[index].Colour * Lights[index].DiffuseIntensity * diffusefactor, 1.0f);

		float3 vertex_to_eye = normalize(EyeWorldPos - world_position);
		float3 light_reflect = normalize(reflect(Lights[index].Direction, normal));
		float specular_factor = dot(vertex_to_eye, light_reflect);
		if (specular_factor > 0.0f)
		{
#			pragma warning (disable: 3571)									// Disable warning for "x must be >=0 in pow(x,y)" since we have accounted for it here
			specular_factor = pow(specular_factor, Lights[index].SpecularPower);
			specular_colour = float4(Lights[index].Colour * Materials[material].Specular.w * specular_factor, 1.0f);
		}
	}

	return (ambient_colour + diffuse_colour + specular_colour);
}


// Method to calculate the light value for a directional light
// 'index' is index into the light data array
// 'material' is the index of the material being illuminated
// 'world_position' is the world position of this pixel
// 'normal' is the normalised vertex normal, calculated once and passed in for each method call here
float4 CalculateDirectionalLight(unsigned int index, MATERIAL_ID material, float3 world_position, float3 normal)
{
	return CalculateLightBase(index, material, world_position, Lights[index].Direction, normal);
}


// Method to calculate the light value for a point light
// 'index' is index into the light data array
// 'material' is the index of the material being illuminated
// 'world_position' is the world position of this pixel
// 'normal' is the normalised vertex normal, calculated once and passed in for each method call here
float4 CalculatePointLight(unsigned int index, MATERIAL_ID material, float3 world_position, float3 normal)
{
	// Determine light vector and distance to this pixel in world space
	float3 light_direction = (world_position - Lights[index].Position);
	float distance = length(light_direction);
	light_direction = normalize(light_direction);

	// Perform base light calculations
	float4 colour = CalculateLightBase(index, material, world_position, light_direction, normal);

	// Calculate attenuation modifier
	float attenuation = Lights[index].Attenuation.Constant +
						Lights[index].Attenuation.Linear * distance +
						Lights[index].Attenuation.Exp * (distance * distance);

	// Return attenuated light value
	return (colour / attenuation);
}


#endif