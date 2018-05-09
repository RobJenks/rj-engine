#pragma once

#include <string>
#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "iUIComponentRenderable.h"
class MaterialDX11;


// Class has no special alignment requirements
class Image2D : public iUIComponentRenderable
{
public:

	// Default constructor
	Image2D(void) noexcept;

	// Constructor providing all data
	Image2D(const std::string & code, MaterialDX11 *material, XMFLOAT2 position, XMFLOAT2 size, float rotation = 0.0f, float opacity = 1.0f, float zorder = 0.0f) noexcept;

	// Copy construction and assignment
	Image2D(const Image2D & other) noexcept;
	Image2D & operator=(const Image2D & other) noexcept;

	// Move construction and assignment
	Image2D(Image2D && other) noexcept;
	Image2D & operator=(Image2D && other) noexcept;

	// Return or set key parameters
	CMPINLINE MaterialDX11 *		GetMaterial(void) const { return m_material; }
	CMPINLINE float					GetRotation(void) const { return m_rotation; }
	CMPINLINE float					GetOpacity(void) const { return m_opacity; }
	CMPINLINE float					GetZOrder(void) const { return m_zorder; }

	void							SetMaterial(const std::string & material);
	CMPINLINE void					SetMaterial(MaterialDX11 *material) { m_material = material; }
	CMPINLINE void					SetRotation(float rotation) { m_rotation = rotation; }
	CMPINLINE void					SetOpacity(float opacity) { m_opacity = opacity; }
	CMPINLINE void					SetZOrder(float zorder) { m_zorder = zorder; }


	// Render image component to the primary RT
	void							Render(void);

	// Destructor
	~Image2D(void) noexcept;


private:

	void							Swap(const Image2D & other);

private:

	MaterialDX11 *					m_material;
	float							m_rotation;
	float							m_opacity;
};

