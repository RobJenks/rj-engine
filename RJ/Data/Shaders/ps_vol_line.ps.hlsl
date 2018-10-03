#include "../../VolumetricLineRenderingCommonData.hlsl.h"


/////////////
// GLOBALS //
/////////////
static const float BigPFloat = 9999.0f;
static const float BigMFloat = -9999.0f;


// Geometric resolution of ray-sphere intersection http://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld013.htm
//   P: ray origin
//   D: normalized ray direction
//   C: sphere center position
//   radius_sq: taken from geometry shader input
float2 intersectSphere(float3 P, float3 D, float3 C, float radius_sq)
{
	// Compute data needed in the geometric transformation
	float3 L = C - P;
	float tca = dot(L, D);
	float d2 = dot(L, L) - tca*tca;

	// First and second intersection point offset
	float thc = sqrt(radius_sq - d2);

	// Return the two solutions: P+ret.x*D and P+ret.y*D
	//return float2(tca) + float2(-thc, thc);
	return float2(tca - thc, tca + thc);
}

// Quadratic equation resolution for ray-cylinder intersection (general version) http://www.cs.jhu.edu/%7Ecohen/RenderingTechniques/assignment3.html
//   P: ray origin
//   D: normalized ray direction
//   radius_sq: taken from geometry shader input
//   linelength: taken from geometry shader input
float2 intersectCylinder(float3 P, float3 D, float radius_sq, float linelength)	
{
	// Resolving quadratic equation for not capped cylinder intersection
	float a = dot(D.yz, D.yz);
	float b = 2.0f * dot(P.yz, D.yz);
	float c = dot(P.yz, P.yz) - radius_sq;
	float b2m4ac = b*b - (4.0f*a*c);

	// Compute the two solutions
	b2m4ac = sqrt(b2m4ac);
	//float2 ret = float2(-b - b2m4ac, -b + b2m4ac) * float2(1.0f / (2.0f * a));
	float mult_vec = (1.0f / (2.0f * a));
	float2 ret = float2(-b - b2m4ac, -b + b2m4ac) * float2(mult_vec, mult_vec);

	// Test cylinder boundary (to avoid infinite cylinder)
	float2 test = P.xx + D.xx*ret;
	if (test.x < 0.0f || test.x > linelength)
		ret.x = BigPFloat;
	if (test.y < 0.0f || test.y > linelength)
		ret.y = BigMFloat;

	return ret;
}


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS_Vol_Line(VLPixelInputType input) : SV_TARGET
{ 
	// Determine back and front intersection with cylinder and spheres in line space
	float3 norm_linespace_ViewDir = normalize(input.line_viewdir);
	float2 t1 = intersectCylinder(input.line_viewpos, norm_linespace_ViewDir, input.radius_sq, input.linelength);
	float2 t2 = intersectSphere(input.line_viewpos, norm_linespace_ViewDir, float3(0.0f, 0.0f, 0.0f), input.radius_sq);
	float2 t3 = intersectSphere(input.line_viewpos, norm_linespace_ViewDir, float3(input.linelength, 0.0f, 0.0f), input.radius_sq);

	// Near and far intersection points
	float front = min(t2.x, t3.x);
	float back = max(t2.y, t3.y);
	front = min(front, t1.x);
	back = max(back, t1.y);

	// Intersection with environment
	float envLinearDepth = C_FarClipDistance * VLDepthBufferInput.Sample(LinearRepeatSampler, input.position.xy / float2(C_ViewportSize.x, C_ViewportSize.y)).r;
	front = min(front, envLinearDepth);
	back = min(back, envLinearDepth);

	// Interesection with front plane and sanity check for back and front values
	front = max(front, C_NearClipDistance);
	back = max(back, front);

	// Compute thickness of the volumetric line
	float thickness = back - front;
	float inVolumeLine = 1.0f - step(thickness, 0.0f);

	// Apply blending
	float lineFacing = abs(dot(normalize(input.view_pos), (input.p2 - input.p1) / input.linelength));
	float viewRadius = lerp(2.0f*input.radius, input.linelength, lineFacing*lineFacing*lineFacing*lineFacing);
	float intens = ((thickness) / viewRadius);
	
	// Return the final pixel colour
	return float4(intens * input.colour.x, intens * input.colour.y, intens * input.colour.z, input.colour.w) * inVolumeLine;
}
