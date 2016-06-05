#if !defined(__light_calculationH__)
#define __light_calculationH__

#include "material_definition.h"
#include "light_definition.h"

// Base method to calculate the light value for a basic (e.g. directional) light
// 'light' is the base light data
// 'material' is the index of the material being illuminated
// 'world_position' is the world position of this pixel
// 'light_direction' is the direction of light intersecting the pixel, in world space
// 'normal' is the normalised vertex normal, calculated once and passed in for each method call here
float4 CalculateLightBase(BaseLightData light, MATERIAL_ID material, float3 world_position, float3 light_direction, float3 normal)
{
	float4 ambient_colour = float4(light.Colour, 1.0f) * light.AmbientIntensity;
	float4 diffuse_colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular_colour = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float diffusefactor = dot(normal, -light_direction);
	if (diffusefactor > 0.0f)
	{
		diffuse_colour = float4(light.Colour * light.DiffuseIntensity * diffusefactor, 1.0f);

		float3 vertex_to_eye = normalize(EyeWorldPos - world_position);
		float3 light_reflect = normalize(reflect(light.Direction, normal));
		float specular_factor = dot(vertex_to_eye, light_reflect);
		if (specular_factor > 0.0f)
		{
#			pragma warning (disable: 3571)									// Disable warning for "x must be >=0 in pow(x,y)" since we have accounted for it here
			specular_factor = pow(specular_factor, light.SpecularPower);
			specular_colour = float4(light.Colour * Materials[material].Specular.w * specular_factor, 1.0f);
		}
	}

	return (ambient_colour + diffuse_colour + specular_colour);
}


// Method to calculate the light value for a directional light
// 'light' is the base light data
// 'material' is the index of the material being illuminated
// 'world_position' is the world position of this pixel
// 'normal' is the normalised vertex normal, calculated once and passed in for each method call here
float4 CalculateDirectionalLight(BaseLightData light, MATERIAL_ID material, float3 world_position, float3 normal)
{
	return CalculateLightBase(light, material, world_position, light.Direction, normal);
}


// Method to calculate the light value for a point light
// 'light' is the light data
// 'material' is the index of the material being illuminated
// 'world_position' is the world position of this pixel
// 'normal' is the normalised vertex normal, calculated once and passed in for each method call here
float4 CalculatePointLight(unsigned int light_index, MATERIAL_ID material, float3 world_position, float3 normal)
{
	// Determine light vector and distance to this pixel in world space
	float3 light_direction = (world_position - Lights[light_index].Position);
	float distance = length(light_direction);
	light_direction = normalize(light_direction);

	// Perform base light calculations
	float4 colour = CalculateLightBase((BaseLightData)Lights[light_index], material, world_position, light_direction, normal);

	// Calculate attenuation modifier
	float attenuation = Lights[light_index].Attenuation.Constant +
						Lights[light_index].Attenuation.Linear * distance +
						Lights[light_index].Attenuation.Exp * (distance * distance);

	// Return attenuated light value
	return (colour / attenuation);
}


#endif