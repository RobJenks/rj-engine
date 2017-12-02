#pragma once

struct ID3D11Device2;
struct ID3D11DeviceContext2;


class Rendering
{
public:

	// The device and context type in use for rendering.  Keep decoupled from core engine or renderer 
	// to simplify component dependencies
	typedef ID3D11Device2			RenderDeviceType;
	typedef ID3D11DeviceContext2	RenderDeviceContextType;
	

private:

};

