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
#include <xnamath.h>
#include "CompilerSettings.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Light
////////////////////////////////////////////////////////////////////////////////
class Light
{
public:
	Light();
	Light(const Light&);
	~Light();

	void SetAmbientColor(float, float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetDirection(float, float, float);

	CMPINLINE D3DXVECTOR4 GetAmbientColor() { return m_ambientColor; }
	CMPINLINE D3DXVECTOR4 GetDiffuseColor() { return m_diffuseColor; }
	CMPINLINE D3DXVECTOR3 GetDirection() { return m_direction; }

private:
	D3DXVECTOR4 m_ambientColor;
	D3DXVECTOR4 m_diffuseColor;
	D3DXVECTOR3 m_direction;
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
	DirectionalLight(XMFLOAT3 direction, XMFLOAT4 ambient, XMFLOAT4 diffuse, XMFLOAT4 specular)
					{ Direction = direction; Ambient = ambient; Diffuse = diffuse; Specular = specular; }
};

#endif