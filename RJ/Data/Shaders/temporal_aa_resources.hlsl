#ifndef __TemporalAAResourcesHLSL__
#define __TemporalAAResourcesHLSL__


// Texture resources bound to the PS
TEXTURE2D TAAColourBufferInput REGISTER(t0);
TEXTURE2D TAAHistoryBufferInput REGISTER(t0);
TEXTURE2D TAADepthBufferInput REGISTER(t2);
TEXTURE2D TAAVelocityBufferInput REGISTER(t3);
TEXTURE2D TAAVelocityNeighbourhoodInput REGISTER(t4);

// Input texture names
#define TAAColourBufferInputName BUFFER_NAME(TAAColourBufferInput);
#define TAADepthBufferInputName BUFFER_NAME(TAADepthBufferInput);
#define TAAVelocityBufferInputName BUFFER_NAME(TAAVelocityBufferInput);
#define TAAVelocityNeighbourhoodInputName BUFFER_NAME(TAAVelocityNeighbourhoodInput);




// Constant buffer used for temporal AA rendering
CBUFFER TemporalAABuffer REGISTER(b4)
{
	float4 C_Jitter;						// xy = current frame, zw = prior frame
	float C_NearClip;						// Near plane distance
	float C_FarClip;						// Far plane distance
	float C_FeedbackMin;					// Minimum feedback contribution from history during reprojection
	float C_FeedbackMax;					// Maximum feedback contribution from history during reprojection

	// Padding
	//float2 _padding_taa;					// CB size must be a multiple of 16 bytes
};


// String references to each buffer binding
#define TemporalAABufferName BUFFER_NAME(TemporalAABuffer)










#endif