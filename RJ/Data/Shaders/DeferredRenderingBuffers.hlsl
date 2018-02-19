#include "../../../Definitions/CppHLSLLocalisation.hlsl.h"


// Constant buffer holding current index into the light buffer (TODO: Can be combined with other cbuffer data?)
CBUFFER LightIndexBuffer REGISTER(b4)
{
	// The index of the light in the 'Lights' buffer
	_uint32 LightIndex;
};

// String references to each buffer binding
#define LightIndexBufferName BUFFER_NAME(LightIndexBuffer)


