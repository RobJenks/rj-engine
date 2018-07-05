#ifndef __MotionBlurResourcesHLSL__
#define __MotionBlurResourcesHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"


// Input textures (wiil not all be bound at the same time, but SM5.0 does not yet support overlapping register IDs)
TEXTURE2D MotionBlurColourBufferInput REGISTER(t0);
TEXTURE2D MotionBlurDepthBufferInput REGISTER(t1);
TEXTURE2D MotionBlurVelocityBufferInput REGISTER(t2);
TEXTURE2D MotionBlurVelocityTileBufferInput REGISTER(t3);
TEXTURE2D MotionBlurVelocityNeighbourhoodInput REGISTER(t4);


// Input texture names
#define MotionBlurColourBufferInputName BUFFER_NAME(MotionBlurColourBufferInput)
#define MotionBlurDepthBufferInputName BUFFER_NAME(MotionBlurDepthBufferInput)
#define MotionBlurVelocityBufferInputName BUFFER_NAME(MotionBlurVelocityBufferInput)
#define MotionBlurVelocityTileBufferInputName BUFFER_NAME(MotionBlurVelocityTileBufferInput)
#define MotionBlurVelocityNeighbourhoodInputName BUFFER_NAME(MotionBlurVelocityNeighbourhoodInput)



#endif