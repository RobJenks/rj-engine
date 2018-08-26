#ifndef __ShadowMapPCFKernelsHLSL__
#define __ShadowMapPCFKernelsHLSL__


/* Null kernel */
static const int PCF_KERNEL_NULL_TAPS = 1;
static const int2 PCF_KERNEL_NULL_OFFSETS[PCF_KERNEL_NULL_TAPS] = { int2(0, 0) };
static const float PCF_KERNEL_NULL_WEIGHTS[PCF_KERNEL_NULL_TAPS] = { 1.0f };
static const float PCF_KERNEL_NULL_WEIGHT_SUM = 1.0f;



/* 3x3 box filter for 9-tap PCF kernel*/
static const int PCF_KERNEL_BOX3_TAPS = 9;
static const int2 PCF_KERNEL_BOX3_OFFSETS[PCF_KERNEL_BOX3_TAPS] =
{
	int2(-1, -1), int2(-1, 0), int2(-1, +1),
	int2(0, -1), int2(0, 0), int2(0, +1),
	int2(+1, -1), int2(+1, 0), int2(+1, +1)
};
static const float PCF_KERNEL_BOX3_WEIGHTS[PCF_KERNEL_BOX3_TAPS] =
{
	0.707f, 1.0f, 0.707f,
	1.0f, 1.5f, 1.0f,
	0.707f, 1.0f, 0.707f
};
static const float PCF_KERNEL_BOX3_WEIGHT_SUM = (0.707f * 4) + (1.0f * 4) + 1.5f;







#endif