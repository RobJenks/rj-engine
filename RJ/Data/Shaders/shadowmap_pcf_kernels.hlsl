#ifndef __ShadowMapPCFKernelsHLSL__
#define __ShadowMapPCFKernelsHLSL__


/* Null kernel */
static const int PCF_KERNEL_NULL_TAPS = 1;
static const int2 PCF_KERNEL_NULL_OFFSETS[PCF_KERNEL_NULL_TAPS] = { int2(0, 0) };
static const float PCF_KERNEL_NULL_WEIGHTS[PCF_KERNEL_NULL_TAPS] = { 1.0f };
static const float PCF_KERNEL_NULL_WEIGHT_SUM = 1.0f;



/* 3x3 box filter for 9-tap PCF kernel */
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



/* 5x5 disc filter for X-tap PCF kernel */
/*
{ 0.0,0.5,1.0,0.5,0.0 },
{ 0.5,1.0,1.0,1.0,0.5 },
{ 1.0,1.0,1.0,1.0,1.0 },
{ 0.5,1.0,1.0,1.0,0.5 },
{ 0.0,0.5,1.0,0.5,0.0 }
*/
static const int PCF_KERNEL_DISC5_TAPS = 21;
static const int2 PCF_KERNEL_DISC5_OFFSETS[PCF_KERNEL_DISC5_TAPS] = 
{
	/*    N    */ int2(-1, -2), int2(0, -2), int2(+1, -2), /*    N    */
	int2(-2, -1), int2(-1, -1), int2(0, -1), int2(+1, -1), int2(+2, -1), 
	int2(-2, 0),  int2(-1, 0),  int2(0, 0),  int2(+1, 0),  int2(+2, 0),
	int2(-2, +1), int2(-1, +1), int2(0, +1), int2(+1, +1), int2(+2, +1),
	/*    N    */ int2(-1, +2), int2(0, +2), int2(+1, +2), /*    N    */
};
static const float PCF_KERNEL_DISC5_WEIGHTS[PCF_KERNEL_DISC5_TAPS] = 
{
	/* N */ 0.5f, 1.0f, 0.5f, /* N */
	0.5f, 1.0f, 1.0f, 1.0f, 0.5f, 
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 
	0.5f, 1.0f, 1.0f, 1.0f, 0.5f,
	/* N */ 0.5f, 1.0f, 0.5f, /* N */
};
static const float PCF_KERNEL_DISC5_WEIGHT_SUM = (0.5f * 8) + (1.0f * 13);




#endif