#include "Image2D.h"
#include "Logging.h"
#include "Utility.h"
#include "CoreEngine.h"
#include "RenderAssetsDX11.h"

// Default constructor
Image2D::Image2D() noexcept
	:
	iUIComponentRenderable(), 
	m_material(NULL), 
	m_rotation(0.0f), 
	m_opacity(0.0f)
{
}

// Constructor providing all required information
Image2D::Image2D(const std::string & code, MaterialDX11 *material, XMFLOAT2 position, XMFLOAT2 size, float rotation, float opacity, float zorder) noexcept
	:
	iUIComponentRenderable(code, position, size, zorder, true, false, NULL), 
	m_material(material), 
	m_rotation(rotation), 
	m_opacity(opacity)
{
}

// Copy constructor
Image2D::Image2D(const Image2D & other) noexcept
	:
	iUIComponentRenderable(other), 
	m_material(other.m_material), 
	m_rotation(other.m_rotation), 
	m_opacity(other.m_opacity)
{
}

// Copy assignment
Image2D & Image2D::operator=(const Image2D & other) noexcept
{
	iUIComponentRenderable::operator=(other);
	m_material = other.m_material;
	m_rotation = other.m_rotation;
	m_opacity = other.m_opacity;

	return *this;
}

// Move constructor
Image2D::Image2D(Image2D && other) noexcept
	:
	iUIComponentRenderable(other), 
	m_material(other.m_material),
	m_rotation(other.m_rotation),
	m_opacity(other.m_opacity)
{
}

// Move assignment
Image2D & Image2D::operator=(Image2D && other) noexcept
{
	iUIComponentRenderable::operator=(other);
	m_material = other.m_material;
	m_rotation = other.m_rotation;
	m_opacity = other.m_opacity;

	return *this;
}

// Look up and set the material for this image
void Image2D::SetMaterial(const std::string & material)
{
	auto * mat = Game::Engine->GetAssets().GetMaterial(material);
	if (!mat)
	{
		Game::Log << LOG_ERROR << "Could not load material \"" << material << "\" for image component \"" << GetCode() << "\"\n";
	}

	SetMaterial(mat);
}


// Render image component to the primary RT
void Image2D::Render(void)
{
	// Early-exit validations
	if (!GetRenderActive()) return; 
	if (!m_material) return;
	
	// Submit request to the engine render queue
	Game::Engine->RenderMaterialToScreen(*m_material, m_position, m_size, m_rotation, m_opacity, m_zorder);
}


// Destructor
Image2D::~Image2D(void) noexcept
{
}

