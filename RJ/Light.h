////////////////////////////////////////////////////////////////////////////////
// Filename: Light.h
////////////////////////////////////////////////////////////////////////////////
#ifndef __LightH__
#define __LightH__


//////////////
// INCLUDES //
//////////////
#include <windows.h>
#include "DX11_Core.h"
#include "CompilerSettings.h"

// This class has no special alignment requirements
class Light : public ALIGN16<Light>
{
public:
	Light();
	Light(const Light&);
	~Light();

	void SetAmbientColor(float, float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetDirection(float, float, float);

	CMPINLINE XMFLOAT4 GetAmbientColor()	{ return m_ambientColor; }
	CMPINLINE XMFLOAT4 GetDiffuseColor()	{ return m_diffuseColor; }
	CMPINLINE XMFLOAT3 GetDirection()		{ return m_direction; }

private:
	XMFLOAT4 m_ambientColor;
	XMFLOAT4 m_diffuseColor;
	XMFLOAT3 m_direction;
};



// Represents a directional light for use in shader calculations.  Elements are packed 
// into 4D vectors as per HLSL padding rules with the restriction that an element 
// cannot straddle a 4D vector boundary.
struct DirectionalLight
{
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT3 Direction;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.

	
	DirectionalLight(void) { ZeroMemory(this, sizeof(this)); }
	DirectionalLight(const XMFLOAT3 & direction, const XMFLOAT4 & ambient, const XMFLOAT4 & diffuse, const XMFLOAT4 & specular)
					{ Direction = direction; Ambient = ambient; Diffuse = diffuse; Specular = specular; }
};

#endif