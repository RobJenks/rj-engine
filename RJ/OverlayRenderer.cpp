#include <vector>
#include <string>
#include <cmath>
#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "GameDataExtern.h"
#include "GameVarsExtern.h"
#include "FastMath.h"
#include "Utility.h"
#include "CoreEngine.h"
#include "Model.h"
#include "iSpaceObject.h"
#include "Actor.h"
#include "Order.h"
#include "Order_ActorTravelToPosition.h"
#include "OrientedBoundingBox.h"
class iShader;

#include "OverlayRenderer.h"

// Initialisation of static constant values
const float OverlayRenderer::DEFAULT_LINE_THICKNESS = 2.0f;
const D3DXVECTOR3 OverlayRenderer::PATH_NODE_RENDER_SIZE = D3DXVECTOR3(3.0f, 1.5f, 3.0f);
const D3DXVECTOR3 OverlayRenderer::PATH_NODE_RENDER_ORIGIN = D3DXVECTOR3(OverlayRenderer::PATH_NODE_RENDER_SIZE.x / 2.0f, 0.0f, 
																		 OverlayRenderer::PATH_NODE_RENDER_SIZE.z / 2.0f);
const int OverlayRenderer::m_box_dimensions[3][4] = { { 1, 3, 5, 7 }, { 8, 9, 10, 11 }, { 0, 2, 4, 6 } };

// Default constructor, initialise all values to defaults
OverlayRenderer::OverlayRenderer(void)
{
	// Set default values for certain parameters
	m_nodespinspeed = 1.0f;

	// Set initial standard model storage to null
	for (int i = 0; i < (int)OverlayRenderer::RenderColour::RC_COUNT; i++)
	{
		m_modelfilenames[i] = NULL;
		m_models[i] = NULL;
	}

	// Also set other models to NULL before initialisation
	m_blueprintoverlay = NULL;
}

// Initialises the overlay rendering component; includes loading of all required models
Result OverlayRenderer::Initialise(void)
{
	Result result, overallresult;
	std::string s_filename;
	const char *filename;

	// All standard component instances will use the same underlying model
	std::string s_modelname = concat(D::DATA)("\\Models\\Misc\\unit_line.rjm").str();
	const char *modelname = s_modelname.c_str();

	// Specify the filename for each line type in turn
	m_modelfilenames[(int)OverlayRenderer::RenderColour::RC_Green] = "overlay_green.png";
	m_modelfilenames[(int)OverlayRenderer::RenderColour::RC_Red] = "overlay_red.png";
	m_modelfilenames[(int)OverlayRenderer::RenderColour::RC_LightBlue] = "overlay_lblue.png";

	// Now load each overlay texture in turn
	overallresult = ErrorCodes::NoError;
	for (int i = 0; i < (int)OverlayRenderer::RenderColour::RC_COUNT; i++)
	{
		// Load the model and texture from file
		s_filename = concat(D::DATA)("\\Models\\Misc\\")(m_modelfilenames[i]).str();
		filename = s_filename.c_str();
		m_models[i] = new Model();
		result = m_models[i]->Initialise(modelname, filename);

		// Record any failures that we encounter
		if (result != ErrorCodes::NoError) overallresult = result;
	}

	// Initialise blueprint overlay model
	m_blueprintoverlay = new Model();
	result = m_blueprintoverlay->Initialise(concat(D::DATA)("\\Models\\Misc\\unit_facing_square.rjm").str(),
											concat(D::DATA)("\\Models\\Misc\\overlay_blueprint.png").str());
	if (result != ErrorCodes::NoError) overallresult = result;

	// Initialise all pre-cached transform matrices for render efficiency
	InitialiseCachedMatrices();

	// Return the overall initialisation result
	return overallresult;
}

// Initialise all pre-cached transform matrices for render efficiency
void OverlayRenderer::InitialiseCachedMatrices()
{
	D3DXMATRIX mtmp;

	/* Basic matrices used for intermediate transforms */
	D3DXMatrixRotationY(&m_matrix_yrot, PI/2.0f);		// 90 degree rotation about the y axis
	D3DXMatrixRotationX(&m_matrix_xrotneg, -PI/2.0f);	// -90 degree rotation about the x axis

	/* Standard element-length, default thickness, line scaling transform */
	D3DXMatrixScaling(&m_matrix_stdelementscale, DEFAULT_LINE_THICKNESS, DEFAULT_LINE_THICKNESS, Game::C_CS_ELEMENT_SCALE);

	/* Array of matrices for a default element-sized box */
	m_matrix_boxtransforms = (D3DXMATRIX*)malloc(sizeof(D3DXMATRIX) * 12);	// 12 edges to a box
	
	// Bottom edges
	m_matrix_boxtransforms[0] = ID_MATRIX;														// [0] = fwd from origin
	D3DXMatrixRotationY(&mtmp, PI/2.0f);
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[1]), &mtmp, &(m_matrix_boxtransforms[0]));		// [1] = right from origin
	D3DXMatrixTranslation(&mtmp, Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f);
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[2]), &mtmp, &(m_matrix_boxtransforms[0]));		// [2] = fwd, to right of origin
	D3DXMatrixTranslation(&mtmp, -Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f);
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[3]), &(m_matrix_boxtransforms[1]), &mtmp);		// [3] = right, above origin

	// Top edges
	D3DXMatrixTranslation(&mtmp, 0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f);
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[4]), &mtmp, &(m_matrix_boxtransforms[0]));		// [4] = fwd, above [0]
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[5]), &(m_matrix_boxtransforms[1]), &mtmp);		// [5] = right, above [1]
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[6]), &mtmp, &(m_matrix_boxtransforms[2]));		// [6] = fwd, above [2]
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[7]), &(m_matrix_boxtransforms[3]), &mtmp);		// [7] = right, above [3]

	// Vertical edges
	D3DXMatrixRotationX(&mtmp, -PI/4.0f);
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[8]), &(m_matrix_boxtransforms[0]), &mtmp);	// [8] = up, from origin
	D3DXMatrixTranslation(&mtmp, -Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f);
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[9]), &(m_matrix_boxtransforms[8]), &mtmp);	// [9] = up, right of origin
	D3DXMatrixTranslation(&mtmp, 0.0f, -Game::C_CS_ELEMENT_SCALE, 0.0f);
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[10]), &(m_matrix_boxtransforms[8]), &mtmp);	// [10] = up, in front of origin
	D3DXMatrixMultiply(&(m_matrix_boxtransforms[11]), &(m_matrix_boxtransforms[9]), &mtmp);	// [11] = up, right & in front of origin

	/* Node-related cached matrices.  Also set the spin rate here so that derived fields are calculated */
	D3DXMatrixScaling(&m_matrix_nodescale, OverlayRenderer::PATH_NODE_RENDER_SIZE.x, OverlayRenderer::PATH_NODE_RENDER_SIZE.y, OverlayRenderer::PATH_NODE_RENDER_SIZE.z);
	D3DXMatrixTranslation(&m_matrix_nodeorigin, -OverlayRenderer::PATH_NODE_RENDER_ORIGIN.x, -OverlayRenderer::PATH_NODE_RENDER_ORIGIN.y, -OverlayRenderer::PATH_NODE_RENDER_ORIGIN.z);
	m_matrix_nodescale_and_origin = m_matrix_nodescale * m_matrix_nodeorigin;
	SetNodeSpinSpeed(0.5f);
}

void OverlayRenderer::SetNodeSpinSpeed(float speed)
{
	// Store the new spin speed
	m_nodespinspeed = speed;

	// Recalculate the radians/millisecond rotation value based on this new speed
	m_nodespinradians = (speed * TWOPI) / 1000.0f;
}

// Method to add a line for rendering.  Accepts a world matrix for the line
void OverlayRenderer::RenderLine(D3DXMATRIX *world, OverlayRenderer::RenderColour colour)
{
	// Add a request to the core engine to render this line
	Game::Engine->RenderModel(m_models[(int)colour], world);
}

// Method to add a line for rendering.  Accepts a world matrix for the line, plus scaling length & thickness parameters
void OverlayRenderer::RenderLine(D3DXMATRIX *world, OverlayRenderer::RenderColour colour, float length, float thickness)
{
	// Generate a scaling matrix to account for the length & thickness
	D3DXMATRIX mscale, mworld;
	D3DXMatrixScaling(&mscale, thickness, thickness, length);

	// Scale the current world matrix and pass it to the engine rendering method
	D3DXMatrixMultiply(&mworld, &mscale, world);
	Game::Engine->RenderModel(m_models[(int)colour], &mworld);
}

// Method to add a line for rendering.  Does all calculation of required world matrix to generate the line between two points
void OverlayRenderer::RenderLine(D3DXVECTOR3 & pt1, D3DXVECTOR3 & pt2, OverlayRenderer::RenderColour colour, float thickness, float length)
{
	// Derive the world matrix required to create a line from pt1 to pt2
	D3DXMATRIX world;
	DetermineLineWorldMatrix(&world, pt1, pt2, thickness, length);

	// Now render the line based on this world matrix
	RenderLine(&world, colour);
}

// Determines the world matrix required to transform a line model into the correct position, given only
// the coordinates of each line end point.  Also optionally accepts a line length parameter to avoid 
// the SQRT otherwise required for calculating the line length.
void OverlayRenderer::DetermineLineWorldMatrix(D3DXMATRIX *pOutMatrix, D3DXVECTOR3 & pt1, D3DXVECTOR3 & pt2, float thickness, float length)
{
	D3DXMATRIX m;
	D3DXQUATERNION q;

	// Take the vector difference of the two points
	D3DXVECTOR3 vdiff = (pt2 - pt1);

	// If the length is not specified, calculate it now via aa * bb = cc
	if (length <= 0.0f) 
	{
		length = sqrtf(vdiff.x * vdiff.x + vdiff.y * vdiff.y + vdiff.z * vdiff.z);
	}

	// Start with a scaling matrix that will set both the line length and thickness in the same operation
	D3DXMatrixScaling(pOutMatrix, thickness, thickness, length);

	// Now determine the quaternion rotation from the basis vector to the desired heading difference, and apply via rotation matrix
	QuaternionBetweenVectors(&q, &BASIS_VECTOR, &vdiff);
	D3DXMatrixRotationQuaternion(&m, &q);
	D3DXMatrixMultiply(pOutMatrix, pOutMatrix, &m);

	// Finally apply a translation matrix to move the transformed model into position, with pt1 at the origin and pt2
	// successfully transformed to its desired position
	D3DXMatrixTranslation(&m, pt1.x, pt1.y, pt1.z);
	D3DXMatrixMultiply(pOutMatrix, pOutMatrix, &m);
}

// Method to render a box at the specified location.  World matrix specifies transforming to the target location/orientation.  Size/thickness
// are used to derive the scaling matrix
void OverlayRenderer::RenderBox(const D3DXMATRIX & world, OverlayRenderer::RenderColour colour, float thickness, float xSize, float ySize, float zSize)
{
	D3DXMATRIX mfinal, scale, trans, scale_x_rot;
	trans = ID_MATRIX;

	// First handle edges in the forward (Z) direction
	D3DXMatrixScaling(&scale, thickness, thickness, zSize);

	mfinal = scale * world;												// > Edge 1: fwd from origin
	RenderLine(&mfinal, colour);

	trans._41 = xSize - thickness;		// x-translation
	mfinal = scale * trans * world;										// > Edge 2: fwd, to right of origin
	RenderLine(&mfinal, colour);
	
	trans._42 = ySize - thickness;		// now has x- & y-translation
	mfinal = scale * trans * world;										// > Edge 3: fwd, above and to the right of origin
	RenderLine(&mfinal, colour);

	trans._41 = 0.0f;					// now has only y-translation
	mfinal = scale * trans * world;										// > Edge 4: fwd, above origin
	RenderLine(&mfinal, colour);

	// Now handle edges in the right (X) direction
	//D3DXMatrixScaling(&scale, thickness, thickness, xSize);
	scale._33 = xSize;					// update z-scaling factor to the length of an X side
	scale_x_rot = scale * m_matrix_yrot;

	trans._43 = thickness;				// translate forwards so in line
	mfinal = scale_x_rot * trans * world;								// > Edge 5: right, above origin
	RenderLine(&mfinal, colour);

	trans._43 = zSize;					// x-translation (in fwd direction)
	mfinal = scale_x_rot * trans * world;								// > Edge 6: right, above & in front of origin
	RenderLine(&mfinal, colour);

	trans._42 = 0.0f;					// remove y translation, now at fwd-Z
	mfinal = scale_x_rot * trans * world;								// > Edge 7: right, ahead of origin
	RenderLine(&mfinal, colour);

	trans._43 = thickness;				// translate z-back to origin, minus thickness again
	mfinal = scale_x_rot * trans * world;								// > Edge 8: right, from origin
	RenderLine(&mfinal, colour);

	// Finally handle edges in the up (Y) direction
	//D3DXMatrixScaling(&scale, thickness, thickness, ySize);
	scale._33 = ySize;					// update z-scaling factor to the length of a Y side
	scale_x_rot = scale * m_matrix_xrotneg;

	mfinal = scale_x_rot * trans * world;								// > Edge 9: up, from origin
	RenderLine(&mfinal, colour);

	trans._41 = xSize - thickness;		// translate to the right, minus thickness
	mfinal = scale_x_rot * trans * world;								// > Edge 10: up, to right of origin
	RenderLine(&mfinal, colour);

	trans._43 = zSize;					// translate fwd, so at fwd+right
	mfinal = scale_x_rot * trans * world;								// > Edge 11: up, ahead & right of origin
	RenderLine(&mfinal, colour);

	trans._41 = 0.0f;
	mfinal = scale_x_rot * trans * world;								// > Edge 12: up, ahead of origin
	RenderLine(&mfinal, colour);
}

// Method to render a box around the specified element of a complex ship
void OverlayRenderer::RenderElementBox(iSpaceObject *ship, const INTVECTOR3 & element, OverlayRenderer::RenderColour colour, float thickness)
{
	// Parameter check
	if (ship)
	{
		// Create a transform matrix to translate to the desired element.  Switch y & z since going from element > world space
		D3DXMATRIX trans;
		D3DXMatrixTranslation(&trans, element.x * Game::C_CS_ELEMENT_SCALE, element.z * Game::C_CS_ELEMENT_SCALE, element.y * Game::C_CS_ELEMENT_SCALE);

		// Render a box at the desired location by multiplying the ship world matrix by this transform
		RenderBox( trans * (*(ship->GetWorldMatrix())), colour, thickness, Game::C_CS_ELEMENT_SCALE );
	}
}

// Method to render a box at the specified element *position* (i.e. not element index, but actual coord in element space)
void OverlayRenderer::RenderElementBoxAtRelativeElementLocation(iSpaceObject* ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour, float thickness)
{
	// Parameter check
	if (ship)
	{
		// Create a transform matrix to translate to the desired element.  Switch y & z since going from element > world space.  No need 
		// to scale translation by element size since this method accepts the coordinate value itself, not the element index
		D3DXMATRIX trans;
		D3DXMatrixTranslation(&trans, (FLOAT)elementpos.x, (FLOAT)elementpos.z, (FLOAT)elementpos.y);

		// Render a box at the desired location by multiplying the ship world matrix by this transform
		RenderBox( trans * (*(ship->GetWorldMatrix())), colour, thickness, Game::C_CS_ELEMENT_SCALE );
	}
}

// Method to render a box at the specified element *position* (i.e. not element index, but actual coord in element space).  Allows any size
void OverlayRenderer::RenderBoxAtRelativeElementLocation(iSpaceObject* ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour, 
														 float xSize, float ySize, float zSize, float thickness)
{
	// Parameter check
	if (ship)
	{
		// Create a transform matrix to translate to the desired element.  Switch y & z since going from element > world space.  No need 
		// to scale translation by element size since this method accepts the coordinate value itself, not the element index
		D3DXMATRIX trans;
		D3DXMatrixTranslation(&trans, (FLOAT)elementpos.x, (FLOAT)elementpos.z, (FLOAT)elementpos.y);

		// Render a box at the desired location by multiplying the ship world matrix by this transform
		RenderBox( trans * (*(ship->GetWorldMatrix())), colour, thickness, xSize, ySize, zSize );
	}
}

// Method to add a cuboid for rendering.  Accepts a world matrix for the cuboid position, plus size parameters.  Uses line model.
void OverlayRenderer::RenderCuboid(const D3DXMATRIX &world, OverlayRenderer::RenderColour colour, float xSize, float ySize, float zSize)
{
	// Generate a scaling matrix to account for the length & thickness, plus a translation to offset to the cuboid centre
	D3DXMATRIX mscale, mtrans, mworld;
	D3DXMatrixScaling(&mscale, xSize, ySize, zSize);
	D3DXMatrixTranslation(&mtrans, xSize * -0.5f, ySize * -0.5f, zSize * -0.5f);

	// Scale & translate the current world matrix before passing it to the engine rendering method
	mworld = mscale * mtrans * world;
	Game::Engine->RenderModel(m_models[(int)colour], &mworld);
}

// Method to add a cuboid for rendering.  Accepts a world matrix for the cuboid position, plus size parameters.  Uses line model.  Includes alpha blending.
// Therefore also requires the position to be provided for efficient depth-sorting
void OverlayRenderer::RenderCuboid(const D3DXMATRIX &world, OverlayRenderer::RenderColour colour, float xSize, float ySize, float zSize, float alpha, const D3DXVECTOR3 & position)
{
	// Generate a scaling matrix to account for the length & thickness, plus a translation to offset to the cuboid centre
	D3DXMATRIX mscale, mtrans, mworld;
	D3DXMatrixScaling(&mscale, xSize, ySize, zSize);
	D3DXMatrixTranslation(&mtrans, xSize * -0.5f, ySize * -0.5f, zSize * -0.5f);

	// Scale & translate the current world matrix before passing it to the engine rendering method
	mworld = mscale * mtrans * world;
	Game::Engine->RenderModel(m_models[(int)colour], &mworld, position, alpha);
}



// Method to add a cuboid for rendering in a ship-relative element space location.
void OverlayRenderer::RenderCuboidAtRelativeElementLocation(iSpaceObject *ship, INTVECTOR3 elementpos, OverlayRenderer::RenderColour colour, 
														    float xSize, float ySize, float zSize)
{
	// Parameter check
	if (ship)
	{
		// Generate a scaling matrix to account for the length & thickness, plus a translation to both centre the item & move to the right element
		// Swap y & z coords since we are moving from element to world space
		D3DXMATRIX mscale, mtrans, mworld;
		D3DXMatrixScaling(&mscale, xSize, ySize, zSize);
		D3DXMatrixTranslation(&mtrans, elementpos.x - (xSize * 0.5f), elementpos.z - (ySize * 0.5f), elementpos.y - (zSize * 0.5f));

		// Scale & translate the current world matrix before passing it to the engine rendering method
		mworld = mscale * mtrans * (*(ship->GetWorldMatrix()));
		Game::Engine->RenderModel(m_models[(int)colour], &mworld);
	}
}

// Renders a cuboid based on the location of its vertices
void OverlayRenderer::RenderCuboid(D3DXVECTOR3(&pVertices)[8], OverlayRenderer::RenderColour colour, float thickness)
{
	// Render lines connecting the vertices.  [0-3] represent one end face in ccw order, [4-7] represent
	// the second end face in ccw order.  v[x] and v[x+4] are opposite vertices in the two end faces

	RenderLine(pVertices[0], pVertices[1], colour, thickness, -1.0f);		// These four lines connect face #1
	RenderLine(pVertices[1], pVertices[2], colour, thickness, -1.0f);
	RenderLine(pVertices[2], pVertices[3], colour, thickness, -1.0f);
	RenderLine(pVertices[3], pVertices[0], colour, thickness, -1.0f);
	RenderLine(pVertices[4], pVertices[5], colour, thickness, -1.0f);		// These four lines connect face #2
	RenderLine(pVertices[5], pVertices[6], colour, thickness, -1.0f);
	RenderLine(pVertices[6], pVertices[7], colour, thickness, -1.0f);
	RenderLine(pVertices[7], pVertices[4], colour, thickness, -1.0f);
	RenderLine(pVertices[0], pVertices[4], colour, thickness, -1.0f);		// These four lines connect the two parallel faces
	RenderLine(pVertices[1], pVertices[5], colour, thickness, -1.0f);
	RenderLine(pVertices[2], pVertices[6], colour, thickness, -1.0f);
	RenderLine(pVertices[3], pVertices[7], colour, thickness, -1.0f);
}

// Renders a cuboid based on the location of its vertices.  Also supplies the known cuboid size (edge to edge, i.e. 2*extent) to save calculation time
void OverlayRenderer::RenderCuboid(D3DXVECTOR3(&pVertices)[8], OverlayRenderer::RenderColour colour, float thickness, const D3DXVECTOR3 & size)
{
	// Render lines connecting the vertices.  [0-3] represent one end face in ccw order, [4-7] represent
	// the second end face in ccw order.  v[x] and v[x+4] are opposite vertices in the two end faces

	RenderLine(pVertices[0], pVertices[1], colour, thickness, size.x);		// These four lines connect face #1
	RenderLine(pVertices[1], pVertices[2], colour, thickness, size.y);
	RenderLine(pVertices[2], pVertices[3], colour, thickness, size.x);
	RenderLine(pVertices[3], pVertices[0], colour, thickness, size.y);
	RenderLine(pVertices[4], pVertices[5], colour, thickness, size.x);		// These four lines connect face #2
	RenderLine(pVertices[5], pVertices[6], colour, thickness, size.y);
	RenderLine(pVertices[6], pVertices[7], colour, thickness, size.x);
	RenderLine(pVertices[7], pVertices[4], colour, thickness, size.y);
	RenderLine(pVertices[0], pVertices[4], colour, thickness, size.z);		// These four lines connect the two parallel faces
	RenderLine(pVertices[1], pVertices[5], colour, thickness, size.z);
	RenderLine(pVertices[2], pVertices[6], colour, thickness, size.z);
	RenderLine(pVertices[3], pVertices[7], colour, thickness, size.z);
}

// Renders a semi-transparent ovelay above the specified element
void OverlayRenderer::RenderElementOverlay(iSpaceObject *ship, const INTVECTOR3 & element, const D3DXVECTOR3 & colour, float alpha)
{
	// Parameter check
	if (ship)
	{
		// Create a transform matrix to translate to the desired element, and also to the centre of that element.  
		// Switch y & z since going from element > world space.  
		D3DXMATRIX mtrans, mscale;
		D3DXMatrixTranslation(&mtrans,	(element.x * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT,
										((element.z + 1) * Game::C_CS_ELEMENT_SCALE),							// Render at base of element layer above
										(element.y * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT);

		// Scale to element size, then translate to the relevant place for rendering
		Game::Engine->RenderModel(m_blueprintoverlay, &(ELEMENT_SCALE_MATRIX * mtrans * (*(ship->GetWorldMatrix()))), colour, alpha);
	}
}

// Renders an OBB to world space.  Base thickness is the width of the bounding lines that will be drawn for branch OBBs.  Leaf OBBs
// will be rendered at a multiple of this thickness so it is clear which OBBs are actually colliding objects
void OverlayRenderer::RenderOBB(const OrientedBoundingBox & obb, bool recursive, OverlayRenderer::RenderColour colour, float basethickness)
{
	// Make sure the OBB is valid
	if (IsZeroVector3(obb.Data.Extent)) return;

	// Get the vertices that make up this OBB
	D3DXVECTOR3 v[8];
	obb.DetermineVertices(v);

	// Render lines connecting the vertices.  [0-3] represent one end face in ccw order, [4-7] represent
	// the second end face in ccw order.  v[x] and v[x+4] are opposite vertices in the two end faces
	RenderCuboid(v, colour, (obb.HasChildren() ? basethickness : (basethickness * 4.0f)));

	// If we are rendering recursively, move down to the child OBBs of this object
	if (recursive)
	{
		for (int i = 0; i < obb.ChildCount; ++i)	
			RenderOBB(obb.Children[i], true, colour, basethickness);
	}
}

// Method to add a node for rendering.  Accepts a world matrix for the cuboid position, plus size parameters.  Uses line model.  Spins in place.
void OverlayRenderer::RenderNode(const D3DXMATRIX &world, OverlayRenderer::RenderColour colour)
{
	D3DXMATRIX mworld, mrot;
	
	// Determine the rotation matrix to be used based on current clock time
	D3DXMatrixRotationY(&mrot, ((Game::ClockMs % 1000) * m_nodespinradians) );

	// Scale, translate * rotate the node before moving it into world space and calling the engine rendering method
	mworld = (m_matrix_nodescale * m_matrix_nodeorigin * mrot) * world;
	Game::Engine->RenderModel(m_models[(int)colour], &mworld);
}

// Method to add a node for rendering in a ship-relative element space location.  Spins in place.
void OverlayRenderer::RenderNodeAtRelativeElementLocation(iSpaceObject *ship, INTVECTOR3 elementpos, OverlayRenderer::RenderColour colour)
{
	// Parameter check
	if (ship)
	{
		D3DXMATRIX mrot, mtrans, mworld;

		// Determine the rotation matrix to be used based on current clock time
		D3DXMatrixRotationY(&mrot, ((Game::ClockMs % 1000) * m_nodespinradians) );

		// Generate translation matrix.  We will be translating from the -ve origin point by the offset amount, so will end up just -ve from
		// the offset point by the required amount to centre the node; swap y & z coords since we are moving from element to world space
		D3DXMatrixTranslation(&mtrans, (FLOAT)elementpos.x, (FLOAT)elementpos.z, (FLOAT)elementpos.y);

		// Translate to origin, scale, rotate the node, translate to position, before moving it into world space and calling the engine rendering method
		mworld = (m_matrix_nodescale_and_origin * mrot * mtrans) * (*(ship->GetWorldMatrix()));
		Game::Engine->RenderModel(m_models[(int)colour], &mworld);
	}
}


// Methods to render the path being taken by an actor through a complex ship environment
void OverlayRenderer::RenderActorPath(Actor *actor, float thickness)
{
	// Parameter check
	if (actor)
	{
		INTVECTOR3 pos, el;
		Order_ActorTravelToPosition *ao;

		// Get a reference to the actor environment, and make sure it is valid
		iSpaceObject *env = actor->GetParentEnvironment(); if (!env) return;

		// Iterate through the actor order queue
		Actor::OrderQueue::const_iterator it_end = actor->Orders.end();
		for (Actor::OrderQueue::const_iterator it = actor->Orders.begin(); it != it_end; ++it)
		{
			// If this is a 'travel to position' order then we want to render it
			if ((*it)->GetType() == Order::OrderType::ActorTravelToPosition)
			{
				ao = (Order_ActorTravelToPosition*)(*it);
				for (int i = 0; i < ao->PathLength; i++)
				{
					// Render each node in turn; render nodes in green if they have already been traversed, otherwise render in red
					RenderNodeAtRelativeElementLocation(env, ao->PathNodes[i], 
						(i < ao->PathIndex ? OverlayRenderer::RenderColour::RC_Green : OverlayRenderer::RenderColour::RC_Red) );

					// Also determine the element that this node sits in and render the element outline
					el = Game::ElementPositionToElementIndex(ao->PathNodes[i]);
					RenderElementBox(env, el, OverlayRenderer::RenderColour::RC_LightBlue, 1.0f);
				}
			}
		}
	}
}


// Shutdown method to deallocate all resources maintained by the renderer
void OverlayRenderer::Shutdown(void)
{
	// Shutdown each model maintained by the renderer
	for (int i = 0; i < (int)OverlayRenderer::RenderColour::RC_COUNT; i++)
	{
		if (m_models[i]) { m_models[i]->Shutdown(); SafeDelete(m_models[i]); }
	}

	// Shutdown all cached/pre-calculated data
	if (m_matrix_boxtransforms ) { delete m_matrix_boxtransforms; m_matrix_boxtransforms = NULL; }
}


OverlayRenderer::~OverlayRenderer(void)
{
}



