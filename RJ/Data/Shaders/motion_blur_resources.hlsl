#ifndef __MotionBlurResourcesHLSL__
#define __MotionBlurResourcesHLSL__

#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"


// Input textures
TEXTURE2D MotionBlurColourBufferInput REGISTER(t0);
TEXTURE2D MotionBlurVelocityBufferInput REGISTER(t1);

// Input texture names
#define MotionBlurColourBufferInputName BUFFER_NAME(MotionBlurColourBufferInput)
#define MotionBlurVelocityBufferInputName BUFFER_NAME(MotionBlurVelocityBufferInput)




#endif