#pragma once

#include "DX11_Core.h"


class CameraView
{
public:

	static XMMATRIX InverseView(const FXMVECTOR position, const FXMVECTOR orientation);
	static XMMATRIX InverseView(const CXMMATRIX position, const FXMVECTOR orientation);
	static XMMATRIX InverseView(const FXMVECTOR position, const CXMMATRIX orientation);
	static XMMATRIX InverseView(const CXMMATRIX position, const CXMMATRIX orientation);

	static void ViewAndInverse(const FXMVECTOR position, const FXMVECTOR orientation, XMMATRIX & outView, XMMATRIX & outInverseView);
	static void ViewAndInverse(const CXMMATRIX position, const FXMVECTOR orientation, XMMATRIX & outView, XMMATRIX & outInverseView);
	static void ViewAndInverse(const FXMVECTOR position, const CXMMATRIX orientation, XMMATRIX & outView, XMMATRIX & outInverseView);
	static void ViewAndInverse(const CXMMATRIX position, const CXMMATRIX orientation, XMMATRIX & outView, XMMATRIX & outInverseView);

	static XMMATRIX View(const FXMVECTOR position, const FXMVECTOR orientation);
	static XMMATRIX View(const CXMMATRIX position, const FXMVECTOR orientation);
	static XMMATRIX View(const FXMVECTOR position, const CXMMATRIX orientation);
	static XMMATRIX View(const CXMMATRIX position, const CXMMATRIX orientation);

	static void ViewAndInverseWithOffset(const FXMVECTOR position, const FXMVECTOR orientation, const CXMMATRIX offset, XMMATRIX & outView, XMMATRIX & outInverseView);
	static void ViewAndInverseWithOffset(const CXMMATRIX position, const FXMVECTOR orientation, const CXMMATRIX offset, XMMATRIX & outView, XMMATRIX & outInverseView);
	static void ViewAndInverseWithOffset(const FXMVECTOR position, const CXMMATRIX orientation, const CXMMATRIX offset, XMMATRIX & outView, XMMATRIX & outInverseView);
	static void ViewAndInverseWithOffset(const CXMMATRIX position, const CXMMATRIX orientation, const CXMMATRIX offset, XMMATRIX & outView, XMMATRIX & outInverseView);



private:


};