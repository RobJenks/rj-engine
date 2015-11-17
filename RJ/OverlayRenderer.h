#pragma once

#ifndef __OverlayRendererH__
#define __OverlayRendererH__

#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "Utility.h"
class iShader;
class iSpaceObject;
class Model;
class Actor;
class OrientedBoundingBox;


// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class OverlayRenderer : public ALIGN16<OverlayRenderer>
{
public:

	// Default thickness for lines, if not specified
	static const float	DEFAULT_LINE_THICKNESS;
	static const XMVECTOR PATH_NODE_RENDER_SIZE;
	static const XMVECTOR PATH_NODE_RENDER_ORIGIN;

	// Enumeration of the possible colours/textures for overlay rendering
	enum RenderColour { RC_Green = 0, RC_Red, RC_LightBlue, RC_COUNT };

	// Initialises the overlay rendering component; includes loading of all required models
	Result				Initialise(void);

	// Initialise all pre-cached transform matrices for render efficiency
	void				InitialiseCachedMatrices(void);

	// Methods to get and set the spin rate for nodes rendered by the overlay renderer
	CMPINLINE float		GetNodeSpinSpeed(void) { return m_nodespinspeed; }
	void				SetNodeSpinSpeed(float speed);

	// Method to add a line for rendering.  Accepts a world matrix for the line, plus scaling length & thickness parameters
	void				RenderLine(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float length, float thickness);

	// Method to add a line for rendering.  Accepts a world matrix for the line
	void				RenderLine(const FXMMATRIX world, OverlayRenderer::RenderColour colour);

	// Determines the world matrix required to transform a line model into the correct position, given only
	// the coordinates of each line end point.  Also optionally accepts a line length parameter to avoid 
	// the SQRT otherwise required for calculating the line length.
	void				DetermineLineWorldMatrix(XMMATRIX & outMatrix, const FXMVECTOR pt1, const FXMVECTOR pt2, float thickness, float length);
	CMPINLINE void		DetermineLineWorldMatrix(XMMATRIX & outMatrix, const FXMVECTOR pt1, const FXMVECTOR pt2, float thickness)
	{
		// If we don't know the line length, pass -1.0f and the method will derive it via a SQRT on the line vertices
		DetermineLineWorldMatrix(outMatrix, pt1, pt2, thickness, -1.0f);
	}

	// Method to add a line for rendering.  Does all calculation of required world matrix to generate the line between two points.  If 
	// 'length' is given as <= 0 then the length will be calculated automatically (via a sqrt, so avoid if possible)
	void				RenderLine(FXMVECTOR pt1, FXMVECTOR pt2, OverlayRenderer::RenderColour colour, float thickness, float length);

	// Method to render a box at the specified location.  World matrix specifies transforming to the target location/orientation.  Size/thickness
	// are used to derive the scaling matrix
	void				RenderBox(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float thickness, float xSize, float ySize, float zSize);

	// Method to render a box at the specified location.  Overloaded method that renders a cube rather than cuboid
	CMPINLINE void		RenderBox(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float thickness, float size)
	{
		RenderBox(world, colour, thickness, size, size, size);
	}

	// Methods to render a box around a specified element / element position / arbitrary position within a complex ship
	void				RenderElementBox(iSpaceObject *ship, const INTVECTOR3 & element, OverlayRenderer::RenderColour colour, float thickness);
	void				RenderElementBoxAtRelativeElementLocation(iSpaceObject *ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour, float thickness);
	void				RenderBoxAtRelativeElementLocation(iSpaceObject *ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour, float thickness, float xSize, float ySize, float zSize);

	// Methods to add a cuboid for rendering.  Uses line model scaled to the size of the cuboid.
	void				RenderCuboid(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float xSize, float ySize, float zSize);
	void				RenderCuboid(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float xSize, float ySize, float zSize, float alpha, const CXMVECTOR position);
	void				RenderCuboid(AXMVECTOR_P(&pVertices)[8], OverlayRenderer::RenderColour colour, float thickness);
	void				RenderCuboid(AXMVECTOR_P(&pVertices)[8], OverlayRenderer::RenderColour colour, float thickness, const CXMVECTOR size);
	void				RenderCuboidAtRelativeElementLocation(iSpaceObject *ship, INTVECTOR3 elementpos, OverlayRenderer::RenderColour colour, 
															  float xSize, float ySize, float zSize);

	// Methods to render semi-transparent overlay components
	void				RenderElementOverlay(iSpaceObject *ship, const INTVECTOR3 & element, const FXMVECTOR colour, float alpha);

	// Renders an OBB to world space.  Base thickness is the width of the bounding lines that will be drawn for branch OBBs.  Leaf OBBs
	// will be rendered at a multiple of this thickness so it is clear which OBBs are actually colliding objects
	void				RenderOBB(const OrientedBoundingBox & obb, bool recursive, OverlayRenderer::RenderColour colour, float basethickness);

	// Methods to add a node for rendering.  Uses line model.  Spins in place.
	void				RenderNode(const FXMMATRIX world, OverlayRenderer::RenderColour colour);
	void				RenderNodeAtRelativeElementLocation(iSpaceObject *ship, INTVECTOR3 elementpos, OverlayRenderer::RenderColour colour);

	// Methods to render the path being taken by an actor through a complex ship environment
	void				RenderActorPath(Actor *actor, float thickness);

	// Shutdown method to deallocate all resources maintained by the renderer
	void Shutdown(void);

	OverlayRenderer(void);
	~OverlayRenderer(void);


private:
	
	// Maintain a counter of the next unique ID to be assigned 
	//static int										m_nextid;
	
	// Array of vectors, each holding the render data for a type of line being rendered
	//std::vector<D3DXMATRIX>							m_lines[RC_COUNT];

	// Array of model filenames, and the models themselves, corresponding to each type of line that can be rendered
	const char *									m_modelfilenames[(int)RC_COUNT];
	Model *											m_models[(int)RC_COUNT];

	// Other, more complex overlay models
	Model *											m_blueprintoverlay;	

	// Cached simple transforms for use during transformation calculations
	AXMMATRIX										m_matrix_yrot;							// 90 degree rotation about the y axis
	AXMMATRIX										m_matrix_xrotneg;						// -90 degree rotation about the x axis
	
	// Cached matrix to transform a line to one-element length, with standard thickness
	AXMMATRIX										m_matrix_stdelementscale;

	// Cached trans/rot world matrices for boxes, with variants also defined for those with standard properties
	AXMMATRIX_P *									m_matrix_boxtransforms;					// Array of transform matrices, unscaled
	
	// Array of indices, indicating which 4 are relevant to each of the three dimensions
	static const int								m_box_dimensions[3][4];

	// Fields used for node rendering functionality
	float											m_nodespinspeed;		// Speed multiplier for node spin rate
	float											m_nodespinradians;		// Calculated; number of radians to spin per millisecond
	AXMMATRIX										m_matrix_nodescale, m_matrix_nodeorigin, m_matrix_nodescale_and_origin;
																							

};



#endif