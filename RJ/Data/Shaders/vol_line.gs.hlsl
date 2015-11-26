////////////////////////////////////////////////////////////////////////////////
// Filename: vol_line.gs.hlsl
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer ConstantBuffer
{
	matrix projectionmatrix;
	float radius;
	float3 PADDING;
};


//////////////
// TYPEDEFS //
//////////////

static const float3 UP_AXIS = float3(0.0f, 1.0f, 0.0f);
static const float3 RIGHT_AXIS = float3(1.0f, 0.0f, 0.0f);


struct GeomInputType
{
	float4 P0 : POSITION0;
	float4 P1 : POSITION1;
};

struct PixelInputType
{
	float4 position : SV_POSITION;		// Position (in projection space)
	float3 p1 : P1;						// P1 in view space
	float linelength : LENGTH;			// Length between P1 and P2
	float3 p2 : P2;						// P2 in view space
	float radius_sq : RADIUS_SQ;		// Squared radius of the line
	float3 view_pos : VIEWPOS;			// Position in view space
	float3 line_viewpos : LINE_VIEWPOS;	// View position in line space
	float3 line_viewdir : LINE_VIEWDIR;	// View direction in line space
};


////////////////////////////////////////////////////////////////////////////////
// Geometry Shader
////////////////////////////////////////////////////////////////////////////////
[maxvertexcount(16)]
void main(point GeomInputType linedata[1], inout TriangleStream<PixelInputType> triStream)
{
	PixelInputType output;

	// linedata[0/1] = p1/p2.  Both in view space
	output.p1 = linedata[0].P0.xyz;
	output.p2 = linedata[0].P1.xyz;

	// Get the difference vector between endpoints and determine line length
	float3 linedir = (output.p2 - output.p1);
	output.linelength = length(linedir);
	float3 normLineDir = (linedir / output.linelength);
	output.radius_sq = (radius * radius);

	// Construct orthonormal basis of the line
	float3 d2 = cross(normLineDir, UP_AXIS);
	if (abs(normLineDir.y) > 0.999f)
		d2 = cross(normLineDir, RIGHT_AXIS);
	d2 = normalize(d2);
	float3 d3 = cross(normLineDir, d2);	// Normalisation is not required for d3

	float3 d2norm = d2;
	float3 d3norm = d3;
	d2 *= radius; d3 *= radius;

	float3 lineDirOffsetM = radius * normLineDir;
	float3 lineDirOffsetP = linedir + lineDirOffsetM;

	// Compute view position in line space (used by pixel shader)
	output.line_viewpos = float3(dot(normLineDir, -output.p1), dot(d2norm, -output.p1), dot(d3norm, -output.p1));

	// Precompute all vertex positions
	float4 viewPos000 = float4(output.p1 - d2 - d3 - lineDirOffsetM, 1.0f);
	float4 viewPos001 = float4(output.p1 - d2 + d3 - lineDirOffsetM, 1.0f);
	float4 viewPos010 = float4(output.p1 + d2 - d3 - lineDirOffsetM, 1.0f);
	float4 viewPos011 = float4(output.p1 + d2 + d3 - lineDirOffsetM, 1.0f);
	float4 viewPos110 = float4(output.p1 + d2 - d3 + lineDirOffsetP, 1.0f);
	float4 viewPos111 = float4(output.p1 + d2 + d3 + lineDirOffsetP, 1.0f);
	float4 viewPos100 = float4(output.p1 - d2 - d3 + lineDirOffsetP, 1.0f);
	float4 viewPos101 = float4(output.p1 - d2 + d3 + lineDirOffsetP, 1.0f);
	
	// Also compute the projection of all vertex positions
	float4 viewPos000proj = mul(viewPos000, projectionmatrix);
	float4 viewPos001proj = mul(viewPos001, projectionmatrix);
	float4 viewPos011proj = mul(viewPos011, projectionmatrix);
	float4 viewPos010proj = mul(viewPos010, projectionmatrix);
	float4 viewPos100proj = mul(viewPos100, projectionmatrix);
	float4 viewPos101proj = mul(viewPos101, projectionmatrix);
	float4 viewPos111proj = mul(viewPos111, projectionmatrix);
	float4 viewPos110proj = mul(viewPos110, projectionmatrix);

	// Precompute the view direction of each vertex; information that will be used by the pixel shader
	//precompute view direction for each vertex (interpolate to FP)
	float3 linespace_ViewDir000 = float3(dot(normLineDir, viewPos000.xyz), dot(d2norm, viewPos000.xyz), dot(d3norm, viewPos000.xyz));
	float3 linespace_ViewDir001 = float3(dot(normLineDir, viewPos001.xyz), dot(d2norm, viewPos001.xyz), dot(d3norm, viewPos001.xyz));
	float3 linespace_ViewDir011 = float3(dot(normLineDir, viewPos011.xyz), dot(d2norm, viewPos011.xyz), dot(d3norm, viewPos011.xyz));
	float3 linespace_ViewDir010 = float3(dot(normLineDir, viewPos010.xyz), dot(d2norm, viewPos010.xyz), dot(d3norm, viewPos010.xyz));
	float3 linespace_ViewDir100 = float3(dot(normLineDir, viewPos100.xyz), dot(d2norm, viewPos100.xyz), dot(d3norm, viewPos100.xyz));
	float3 linespace_ViewDir101 = float3(dot(normLineDir, viewPos101.xyz), dot(d2norm, viewPos101.xyz), dot(d3norm, viewPos101.xyz));
	float3 linespace_ViewDir111 = float3(dot(normLineDir, viewPos111.xyz), dot(d2norm, viewPos111.xyz), dot(d3norm, viewPos111.xyz));
	float3 linespace_ViewDir110 = float3(dot(normLineDir, viewPos110.xyz), dot(d2norm, viewPos110.xyz), dot(d3norm, viewPos110.xyz));

	// Emit vertex data
	output.view_pos = viewPos001.rgb;
	output.line_viewdir = linespace_ViewDir001;
	output.position = viewPos001proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos000.rgb;
	output.line_viewdir = linespace_ViewDir000;
	output.position = viewPos000proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos101.rgb;
	output.line_viewdir = linespace_ViewDir101;
	output.position = viewPos101proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos100.rgb;
	output.line_viewdir = linespace_ViewDir100;
	output.position = viewPos100proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos111.rgb;
	output.line_viewdir = linespace_ViewDir111;
	output.position = viewPos111proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos110.rgb;
	output.line_viewdir = linespace_ViewDir110;
	output.position = viewPos110proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos011.rgb;
	output.line_viewdir = linespace_ViewDir011;
	output.position = viewPos011proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos010.rgb;
	output.line_viewdir = linespace_ViewDir010;
	output.position = viewPos010proj;
	triStream.Append(output); /////////////////////////////////////
	
	triStream.RestartStrip(); //////////////////////////////////////////////////////////////////////////

	output.view_pos = viewPos101.rgb;
	output.line_viewdir = linespace_ViewDir101;
	output.position = viewPos101proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos111.rgb;
	output.line_viewdir = linespace_ViewDir111;
	output.position = viewPos111proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos001.rgb;
	output.line_viewdir = linespace_ViewDir001;
	output.position = viewPos001proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos011.rgb;
	output.line_viewdir = linespace_ViewDir011;
	output.position = viewPos011proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos000.rgb;
	output.line_viewdir = linespace_ViewDir000;
	output.position = viewPos000proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos010.rgb;
	output.line_viewdir = linespace_ViewDir010;
	output.position = viewPos010proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos100.rgb;
	output.line_viewdir = linespace_ViewDir100;
	output.position = viewPos100proj;
	triStream.Append(output); /////////////////////////////////////
	output.view_pos = viewPos110.rgb;
	output.line_viewdir = linespace_ViewDir110;
	output.position = viewPos110proj;
	triStream.Append(output); /////////////////////////////////////
	
	triStream.RestartStrip(); //////////////////////////////////////////////////////////////////////////

}



