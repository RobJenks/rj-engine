#pragma once

#ifndef __OverlayRendererH__
#define __OverlayRendererH__

#include <functional>
#include "DX11_Core.h"
#include "ErrorCodes.h"
#include "Utility.h"
class iShader;
class iSpaceObject;
class iSpaceObjectEnvironment;
class Model;
class Actor;
class OrientedBoundingBox;
class EnvironmentTree;


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
	enum RenderColour { RC_None = 0, RC_Green, RC_Red, RC_LightBlue, RC_COUNT };

	// Initialises the overlay rendering component; includes loading of all required models
	Result				Initialise(void);

	// Initialise all pre-cached transform matrices for render efficiency
	void				InitialiseCachedMatrices(void);

	// Methods to get and set the spin rate for nodes rendered by the overlay renderer
	CMPINLINE float		GetNodeSpinSpeed(void) { return m_nodespinspeed; }
	void				SetNodeSpinSpeed(float speed);

	// Method to add a line for rendering.  Accepts a world matrix for the line, plus scaling length & thickness parameters
	void XM_CALLCONV	RenderLine(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float length, float thickness);

	// Method to add a line for rendering.  Accepts a world matrix for the line
	void XM_CALLCONV	RenderLine(const FXMMATRIX world, OverlayRenderer::RenderColour colour);
	void XM_CALLCONV	RenderLine(const FXMMATRIX world, const FXMVECTOR position, const XMFLOAT3 & colour, float alpha);

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
	void XM_CALLCONV	RenderBox(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float thickness, float xSize, float ySize, float zSize);

	// Method to render a box at the specified location.  Overloaded method that renders a cube rather than cuboid
	CMPINLINE void XM_CALLCONV	RenderBox(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float thickness, float size)
	{
		RenderBox(world, colour, thickness, size, size, size);
	}

	// Method to render a box at the specified location
	void XM_CALLCONV	RenderBox(const FXMMATRIX world, const XMFLOAT3 & size, const XMFLOAT3 & colour, float alpha, float thickness);

	// Methods to render a box around a specified element / element position / arbitrary position within a complex ship
	void				RenderElementBox(iSpaceObjectEnvironment *ship, const INTVECTOR3 & element, OverlayRenderer::RenderColour colour, float thickness);
	void				RenderElementBox(iSpaceObjectEnvironment *ship, const INTVECTOR3 & element_location, const INTVECTOR3 & element_size,
										 const XMFLOAT3 & colour, float alpha, float thickness);
	void				RenderElementBoxAtRelativeElementLocation(iSpaceObjectEnvironment *ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour, float thickness);
	void				RenderBoxAtRelativeElementLocation(iSpaceObjectEnvironment *ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour, float thickness, float xSize, float ySize, float zSize);

	// Methods to add a cuboid for rendering.  Uses line model scaled to the size of the cuboid.
	void XM_CALLCONV	RenderCuboid(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float xSize, float ySize, float zSize);
	void XM_CALLCONV	RenderCuboid(const FXMMATRIX world, OverlayRenderer::RenderColour colour, const CXMVECTOR size);
	void XM_CALLCONV	RenderCuboid(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float xSize, float ySize, float zSize, float alpha, const CXMVECTOR position);
	void XM_CALLCONV	RenderCuboid(const FXMMATRIX world, float xSize, float ySize, float zSize, const XMFLOAT3 & colour, float alpha, const CXMVECTOR position);
	void XM_CALLCONV	RenderCuboid(const FXMMATRIX world, float xSize, float ySize, float zSize, const XMFLOAT4 & colour);
	void				RenderCuboid(AXMVECTOR_P(&pVertices)[8], OverlayRenderer::RenderColour colour, float thickness);
	void				RenderCuboid(AXMVECTOR_P(&pVertices)[8], OverlayRenderer::RenderColour colour, float thickness, const CXMVECTOR size);
	void				RenderCuboidAtRelativeElementLocation(	iSpaceObjectEnvironment *ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour, 
																float xSize, float ySize, float zSize);
	void				RenderCuboidAtRelativeElementLocation(	iSpaceObjectEnvironment *ship, const INTVECTOR3 & element_pos, const INTVECTOR3 & element_size,
																const XMFLOAT3 & colour, float alpha);
	
	// Methods to render a semi-transparent 2D overlay over the top of an element
	void				RenderElementOverlay(iSpaceObjectEnvironment & ship, const INTVECTOR3 & element, const XMFLOAT4 & colour_alpha);
	CMPINLINE void		RenderElementOverlay(iSpaceObjectEnvironment *ship, const INTVECTOR3 & element, const XMFLOAT4 & colour_alpha)
	{
		if (ship) RenderElementOverlay(*ship, element, colour_alpha);
	}

	// Methods to render a semi-transparent 3D overlay around an element
	void				RenderElement3DOverlay(iSpaceObjectEnvironment & ship, const INTVECTOR3 & element, const XMFLOAT4 & colour_alpha);
	CMPINLINE void		RenderElement3DOverlay(iSpaceObjectEnvironment *ship, const INTVECTOR3 & element, const XMFLOAT4 & colour_alpha)
	{
		if (ship) RenderElement3DOverlay(*ship, element, colour_alpha);
	}

	// Renders an OBB to world space.  Base thickness is the width of the bounding lines that will be drawn for branch OBBs.  Leaf OBBs
	// will be rendered at a multiple of this thickness so it is clear which OBBs are actually colliding objects
	void				RenderOBB(	const OrientedBoundingBox & obb, bool recursive, bool leaf_nodes_only, 
									OverlayRenderer::RenderColour colour, float basethickness);
	CMPINLINE void		RenderOBB(const OrientedBoundingBox & obb, bool recursive, OverlayRenderer::RenderColour colour, float basethickness)
	{
		RenderOBB(obb, recursive, false, colour, basethickness);
	}

	// Method to add a node for rendering.  Uses line model.  Spins in place.
	void XM_CALLCONV	RenderNode(const FXMMATRIX world, OverlayRenderer::RenderColour colour);
	
	// Overloaded method to render a node in world space.  Accepts a node position and constructs the required world matrix
	void XM_CALLCONV	RenderNode(const FXMVECTOR pos, OverlayRenderer::RenderColour colour);

	// Render a node at the specified element within the given environment
	void				RenderNodeAtRelativeElementLocation(iSpaceObjectEnvironment *ship, INTVECTOR3 elementpos, OverlayRenderer::RenderColour colour);

	// Methods to render the path being taken by an actor through a complex ship environment
	void				RenderActorPath(Actor *actor, float thickness);

	// Renders an overlay over the specified environment.  Accepts a function that determines the overlay at each element
	// The function parameter has signature "XMFLOAT4 func(environment, element_id)" and returns the colour/alpha for the 
	// overlay.  It is applied for each element in turn
	void				RenderEnvironmentOverlay(iSpaceObjectEnvironment & env, XMFLOAT4(*func)(iSpaceObjectEnvironment&, int));
	void				RenderEnvironmentOverlay(iSpaceObjectEnvironment & env, int deck, XMFLOAT4(*func)(iSpaceObjectEnvironment&, int));

	// Renders an overlay over the specified environment.  Accepts a function that determines the overlay at each element
	// The function parameter has signature "XMFLOAT4 func(environment, element_id)" and returns the colour/alpha for the 
	// overlay.  It is applied for each element in turn and generates a 3D overlay around each element
	void				RenderEnvironment3DOverlay(iSpaceObjectEnvironment & env, XMFLOAT4(*func)(iSpaceObjectEnvironment&, int));
	void				RenderEnvironment3DOverlay(iSpaceObjectEnvironment & env, int deck, XMFLOAT4(*func)(iSpaceObjectEnvironment&, int));

	
	// Performs debug rendering of an octree node, and optionally all the way down the subtree as well
	template <typename T>
	void				DebugRenderSpatialPartitioningTree(const Octree<T> *tree, bool include_children)
	{
		// Create a world matrix that will translate the rendered box into position, then render it
		if (!tree) return;
		XMMATRIX world = XMMatrixTranslationFromVector(tree->m_min);
		RenderBox(world, (tree->m_itemcount == 0 ? RenderColour::RC_Red : RenderColour::RC_Green),
						 (tree->m_itemcount == 0 ? 1.0f : 3.0f), (tree->m_xmax - tree->m_xmin), (tree->m_ymax - tree->m_ymin), (tree->m_zmax - tree->m_zmin));

		// Now also render children if required
		if (include_children)
			for (int i = 0; i < 8; ++i)
				if (tree->m_children[i]) DebugRenderSpatialPartitioningTree(tree->m_children[i], true);
	}

	// Performs debug rendering of an environment tree node, and optionally all the way down the subtree as well
	void				DebugRenderEnvironmentTree(const EnvironmentTree *tree, bool include_children);

	// Shutdown method to deallocate all resources maintained by the renderer
	void Shutdown(void);

	OverlayRenderer(void);
	~OverlayRenderer(void);


private:
	
	// Maintain a counter of the next unique ID to be assigned 
	//static int										m_nextid;
	
	// Array of vectors, each holding the render data for a type of line being rendered
	//std::vector<D3DXMATRIX>							m_lines[RC_COUNT];

	// Array of models corresponding to each type of line that can be rendered
	Model *											m_models[(int)RC_COUNT];

	// Other, more complex overlay models
	Model *											m_blueprintoverlay;	
	Model *											m_blueprintcubeoverlay;

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