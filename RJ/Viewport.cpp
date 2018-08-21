#include "Viewport.h"

void Viewport::CompileViewport(D3D11_VIEWPORT & outCompiledViewport) const
{
	outCompiledViewport.TopLeftX = X;
	outCompiledViewport.TopLeftY = Y;
	outCompiledViewport.Width = Width;
	outCompiledViewport.Height = Height;
	outCompiledViewport.MinDepth = MinDepth;
	outCompiledViewport.MaxDepth = MaxDepth;
}

D3D11_VIEWPORT Viewport::CompileViewport(void) const
{
	D3D11_VIEWPORT compiled;
	CompileViewport(compiled);

	return compiled;
}