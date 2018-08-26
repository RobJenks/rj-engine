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
static const float BOX3_WEIGHTS[BOX3_TAPS] =
{
	0.707f, 1.0f, 0.707f,
	1.0f, 1.5f, 1.0f,
	0.707f, 1.0f, 0.707f
};
static const float BOX3_WEIGHT_SUM = (0.707f * 4) + (1.0f * 4) + 1.5f;




#endif