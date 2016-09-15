#pragma once

#ifndef __SentenceTypeH__
#define __SentenceTypeH__

#include "DX11_Core.h"

struct SentenceType
{
	bool render;
	int x, y;
	XMFLOAT4 colour;
	ID3D11Buffer *vertexBuffer, *indexBuffer;
	int vertexCount, indexCount, maxLength;
	int fontID;
	float sentencewidth, sentenceheight;
};



#endif