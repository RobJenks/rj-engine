#include "CameraView.h"


/* Calculate inverse view matrix */

XMMATRIX CameraView::InverseView(const FXMVECTOR position, const FXMVECTOR orientation)
{
	return InverseView(XMMatrixTranslationFromVector(position), XMMatrixRotationQuaternion(orientation));
}


XMMATRIX CameraView::InverseView(const CXMMATRIX position, const FXMVECTOR orientation)
{
	return InverseView(position, XMMatrixRotationQuaternion(orientation));
}


XMMATRIX CameraView::InverseView(const FXMVECTOR position, const CXMMATRIX orientation)
{
	return InverseView(XMMatrixTranslationFromVector(position), orientation);
}


XMMATRIX CameraView::InverseView(const CXMMATRIX position, const CXMMATRIX orientation)
{
	return XMMatrixMultiply(orientation, position);
}



/* Calculate both the view and inverse view matrices */

void CameraView::ViewAndInverse(const FXMVECTOR position, const FXMVECTOR orientation, XMMATRIX & outView, XMMATRIX & outInverseView)
{
	ViewAndInverse(XMMatrixTranslationFromVector(position), XMMatrixRotationQuaternion(orientation), outView, outInverseView);
}


void CameraView::ViewAndInverse(const CXMMATRIX position, const FXMVECTOR orientation, XMMATRIX & outView, XMMATRIX & outInverseView)
{
	ViewAndInverse(position, XMMatrixRotationQuaternion(orientation), outView, outInverseView);
}


void CameraView::ViewAndInverse(const FXMVECTOR position, const CXMMATRIX orientation, XMMATRIX & outView, XMMATRIX & outInverseView)
{
	ViewAndInverse(XMMatrixTranslationFromVector(position), orientation, outView, outInverseView);
}


void CameraView::ViewAndInverse(const CXMMATRIX position, const CXMMATRIX orientation, XMMATRIX & outView, XMMATRIX & outInverseView)
{
	outInverseView = InverseView(position, orientation);
	outView = XMMatrixInverse(NULL, outInverseView);
}



/* Calculate a view matrix (note: does the work to calculate view + inverse, since inverse is required during calculation */

XMMATRIX CameraView::View(const FXMVECTOR position, const FXMVECTOR orientation)
{
	return View(XMMatrixTranslationFromVector(position), XMMatrixRotationQuaternion(orientation));
}


XMMATRIX CameraView::View(const CXMMATRIX position, const FXMVECTOR orientation)
{
	return View(position, XMMatrixRotationQuaternion(orientation));
}


XMMATRIX CameraView::View(const FXMVECTOR position, const CXMMATRIX orientation)
{
	return View(XMMatrixTranslationFromVector(position), orientation);
}


XMMATRIX CameraView::View(const CXMMATRIX position, const CXMMATRIX orientation)
{
	XMMATRIX view, inv;
	ViewAndInverse(position, orientation, view, inv);

	return view;
}


/* Calculate a view and inverse which incorporate the given offset matrix */


void CameraView::ViewAndInverseWithOffset(const FXMVECTOR position, const FXMVECTOR orientation, const CXMMATRIX offset, XMMATRIX & outView, XMMATRIX & outInverseView)
{
	ViewAndInverseWithOffset(XMMatrixTranslationFromVector(position), XMMatrixRotationQuaternion(orientation), offset, outView, outInverseView);
}

void CameraView::ViewAndInverseWithOffset(const CXMMATRIX position, const FXMVECTOR orientation, const CXMMATRIX offset, XMMATRIX & outView, XMMATRIX & outInverseView)
{
	ViewAndInverseWithOffset(position, XMMatrixRotationQuaternion(orientation), offset, outView, outInverseView);
}

void CameraView::ViewAndInverseWithOffset(const FXMVECTOR position, const CXMMATRIX orientation, const CXMMATRIX offset, XMMATRIX & outView, XMMATRIX & outInverseView)
{
	ViewAndInverseWithOffset(XMMatrixTranslationFromVector(position), orientation, offset, outView, outInverseView);
}

void CameraView::ViewAndInverseWithOffset(const CXMMATRIX position, const CXMMATRIX orientation, const CXMMATRIX offset, XMMATRIX & outView, XMMATRIX & outInverseView)
{
	// Offset matrix = (offset * rotation * translation)
	// Take a shortcut and call the primary View/Inverse method with the rotation parameter equal to (offset * rotation)
	ViewAndInverse(position, XMMatrixMultiply(offset, orientation), outView, outInverseView);
}
