#pragma once

#ifndef __Image2DH__
#define __Image2DH__

#include "DX11_Core.h"
#include "CompilerSettings.h"
#include "ErrorCodes.h"
#include "Rendering.h"
#include "Texture.h"
#include "iUIComponentRenderable.h"


// Class has no special alignment requirements
class Image2D : public iUIComponentRenderable
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:
	// The data format used to hold index buffer data.  Note that DX11 (feature level 11.0) appears to support
	// UINT32 sized indices, but feature level 9.1 only appears to support UINT16.  Using the latter for now
	// to maintain compatibility; likely too major a change to handle via the localiser
	typedef UINT16 INDEXFORMAT;		

	Image2D();
	Image2D(const Image2D&);
	~Image2D();

	Result Initialize(Rendering::RenderDeviceType * device, int screenWidth, int screenHeight, const char *textureFilename, int bitmapWidth, int bitmapHeight);
	void Shutdown();
	
	void Render(void);
	void Render(int xPosition, int yPosition, float zOrder);

	CMPINLINE bool GetRenderActive(void) const		{ return m_render; }
	CMPINLINE void SetRenderActive(bool render)		{ m_render = render; }

	CMPINLINE int GetIndexCount() { return m_indexCount; }
	CMPINLINE ID3D11ShaderResourceView* GetTexture() { return m_Texture->GetTexture(); }

	CMPINLINE int GetXPosition(void) { return m_x; }
	CMPINLINE int GetYPosition(void) { return m_y; }
	CMPINLINE void SetPosition(int x, int y) { m_x = x; m_y = y; }

	CMPINLINE float GetZOrder(void) { return m_z; }
	CMPINLINE void SetZOrder(float z) { m_z = z; }

	CMPINLINE int GetWidth(void) { return m_bitmapWidth; }
	CMPINLINE int GetHeight(void) { return m_bitmapHeight; }

private:
	Result InitializeBuffers(Rendering::RenderDeviceType *);
	void ShutdownBuffers();
	Result UpdateBuffers();
	void RenderBuffers();

	Result LoadTexture(Rendering::RenderDeviceType *, const char*);
	void ReleaseTexture();

private:

	int m_x, m_y;
	float m_z;

	Rendering::RenderDeviceType  *				m_device;
	Rendering::RenderDeviceContextType  *		m_devicecontext;

	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	VertexType *m_vertices;
	Texture* m_Texture;
	int m_screenWidth, m_screenHeight;
	float m_screenHalfWidth, m_screenHalfHeight;
	float m_screenLeft;
	int m_bitmapWidth, m_bitmapHeight;

	int m_previousPosX, m_previousPosY;
	float m_previousPosZ;
};


#endif