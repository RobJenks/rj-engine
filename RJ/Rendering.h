#pragma once

class ID3D11Device2;
class ID3D11DeviceContext2;


class Rendering
{
public:

	// The device and context type in use for rendering.  Keep decoupled from core engine or renderer 
	// to simplify component dependencies
	typedef ID3D11Device			RenderDeviceType;
	typedef ID3D11DeviceContext		RenderDeviceContextType;


private:

};

