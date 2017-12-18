////////////////////////////////////////////////////////////////////////////////
// Filename: vol_line.ps.hlsl
////////////////////////////////////////////////////////////////////////////////

/////////////
// GLOBALS //
/////////////
Texture2D linearDepthTexture : register(t0);
Texture2D shaderTexture : register(t1);

SamplerState SampleType;

cbuffer ConstantBuffer
{
	float clipdistance_far;		// Distance to the far clip plane
	float clipdistance_front;	// Distance to the near clip plane
	float viewport_width;		// Size of the viewport
	float viewport_height;		// Size of the viewport
};


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
	float4 position : SV_POSITION;		// Position (in projection space)
	float3 p1 : P1;						// P1 in view space
	float linelength : LENGTH;			// Length between P1 and P2
	float3 p2 : P2;						// P2 in view space
	float radius : RADIUS;				// Radius of the line
	float radius_sq : RADIUS_SQ;		// Squared radius of the line
	float3 view_pos : VIEWPOS;			// Position in view space
	float3 line_viewpos : LINE_VIEWPOS;	// View position in line space
	float3 line_viewdir : LINE_VIEWDIR;	// View direction in line space
	float4 colour : COLOUR;				// Colour (including alpha) of the line
};

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

// Determines the common factor for the intersection point of two lines
float2 LineLineIntersect(float3 p1, float3 p2, float3 p3, float3 p4)
{
	float3 p13, p43, p21;
	float d1343, d4321, d1321, d4343, d2121;
	float numer, denom;
	float mua;

	p13 = p1 - p3;
	p43 = p4 - p3;
	p21 = p2 - p1;

	d1343 = dot(p13, p43);
	d4321 = dot(p43, p21);
	d1321 = dot(p13, p21);
	d4343 = dot(p43, p43);
	d2121 = dot(p21, p21);

	denom = d2121 * d4343 - d4321 * d4321;
	//if (abs(denom) < 0.0000001)//almost impossible: view ray perfectly aligned with line direction (and it would only affect one pixel)
	//	return vec2(0.0,0.0);
	numer = d1343 * d4321 - d1321 * d4343;

	mua = numer / denom;
	mua = clamp(mua, 0.0, 1.0);
	return float2(mua, (d1343 + d4321*mua) / d4343); //return (mua,mub)
}


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 main(PixelInputType input) : SV_TARGET
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
	
	// Compute the two closest points on the volumetric line and current view ray
	float3 p4 = input.view_pos + input.view_pos;									// Note that p3 = input.view_pos, no need to duplicate
	float2 muab = LineLineIntersect(input.p1, input.p2, input.view_pos, p4);

	// Dervive pa and pb, the two closest points
	float3 pa = input.p1 + muab.x*(input.p2 - input.p1);
	float3 pb = input.view_pos + muab.y*(p4 - input.view_pos);

	// Use this to determine the texture sample coordinate
	float fsample = length(pa - pb) / input.radius;
	
	// Intersection with environment
	float envLinearDepth = clipdistance_far * linearDepthTexture.Sample(SampleType, input.position.xy / float2(viewport_width, viewport_height)).r;
	front = min(front, envLinearDepth);
	back = min(back, envLinearDepth);

	// Interesection with front plane and sanity check for back and front values
	front = max(front, clipdistance_front);
	back = max(back, front);

	// Compute thickness of the volumetric line
	float thickness = back - front;
	float inVolumeLine = 1.0f - step(thickness, 0.0f);

	// Apply blending
	float lineFacing = abs(dot(normalize(input.view_pos), (input.p2 - input.p1) / input.linelength));
	float viewRadius = lerp(2.0f*input.radius, input.linelength, lineFacing*lineFacing*lineFacing*lineFacing);
	float intens = ((thickness) / viewRadius);

	// Determine attentuation and shading values
	float attenuation = thickness / (2.0f * input.radius);
	//float3 shading = shaderTexture.Sample(SampleType, float2(fsample, 0.5)).rgb;
	float4 shading = shaderTexture.Sample(SampleType, float2(fsample, 0.5));

	// Return the final pixel colour
	//return float4(intens * input.colour.x, intens * input.colour.y, intens * input.colour.z, input.colour.w) * inVolumeLine;
	//return (float4(shading, attenuation) /* input.colour*/ /* float4(intens,intens,intens,1.0f)*/) /* inVolumeLine*/;
	//return (shading * input.colour * float4(intens, intens, intens, attenuation) * inVolumeLine);
	return float4(shading.xyz, shading.a * attenuation) * input.colour;
}
