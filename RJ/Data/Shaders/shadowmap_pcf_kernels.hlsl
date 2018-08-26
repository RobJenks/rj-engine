#ifndef __ShadowMapPCFKernelsHLSL__
#define __ShadowMapPCFKernelsHLSL__


/* 3x3 box filter for 9-tap PCF kernel*/

static const int BOX3_TAPS = 9;
static const int2 BOX3_OFFSETS[BOX3_TAPS] =
{
	int2(-1, -1), int2(-1, 0), int2(-1, +1),
	int2(0, -1), int2(0, 0), int2(0, +1),
	int2(+1, -1), int2(+1, 0), int2(+1, +1)
};




#endif