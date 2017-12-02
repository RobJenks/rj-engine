#pragma once

class RasterizerState
{
public:

	/**
	* FillMode determines how primitives are rendered.
	* <ul>
	*  <li>FillMode::Wireframe: Primitives are rendered as lines.</li>
	*  <li>FillMode::Solid: Primitives are rendered as solid objects.</li>
	* </ul>
	* DX11 does not distinguish between front-face and back-face fill modes. In this case,
	* only the front-face fill mode is considered.
	* OpenGL allows you to set the front and back face fill modes independently.
	*
	* @see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476131(v=vs.85).aspx
	* @see https://www.opengl.org/sdk/docs/man/html/glPolygonMode.xhtml
	*/
	enum class FillMode
	{
		Wireframe,
		Solid
	};

	/**
	* CullMode controls which polygons are rendered based on the direction they are
	* facing.
	* <ul>
	*  <li>None: Both back and front facing polygons are rendered (no culling is performed).
	*  <li>Front: Front-facing polygons are not rendered.
	*  <li>Back: Back-facing polygons are not rendered.
	*  <li>FrontAndBack: Both front and back-facing polygons are culled (OpenGL only)
	* </ul>
	* @see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476108(v=vs.85).aspx
	* @see https://www.opengl.org/sdk/docs/man/html/glCullFace.xhtml
	*/
	enum class CullMode
	{
		None,
		Front,
		Back,
		FrontAndBack    // OpenGL only
	};

	/**
	* FrontFace determines what polygons are considered "front facing" when applying clipping.
	* The order refers to the ordering of the vertices of a triangle when looking at the "front"
	* of the primitive.
	* <ul>
	*  <li>Clockwise: Vertices of a triangle that are arranged in a clockwise winding order are considered "front facing".
	*  This is the default value for DirectX (and in left-handed coordinate systems in general).</li>
	*  <li>CounterClockwise: Vertices of a triangle that are arranged in a counter-clockwise winding order are considered "front facing".
	*  This is the default value for OpenGL (and in right-handed coordinate systems in general).</li>
	* </ul>
	* @see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476198(v=vs.85).aspx
	* @see https://www.opengl.org/sdk/docs/man/html/glFrontFace.xhtml
	*/
	enum class FrontFace
	{
		Clockwise,
		CounterClockwise
	};

protected:


};