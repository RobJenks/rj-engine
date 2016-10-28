#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include "DX11_Core.h"

#include "ErrorCodes.h"
#include "Model.h"
#include "GameDataExtern.h"
#include "GameVarsExtern.h"
#include "FastMath.h"
#include "Utility.h"
#include "CoreEngine.h"
#include "iObject.h"
#include "iSpaceObject.h"
#include "iSpaceObjectEnvironment.h"
#include "Actor.h"
#include "Order.h"
#include "Order_ActorTravelToPosition.h"
#include "OrientedBoundingBox.h"
#include "EnvironmentTree.h"
class iShader;

#include "OverlayRenderer.h"

// Initialisation of static constant values
const float OverlayRenderer::DEFAULT_LINE_THICKNESS = 2.0f;
const AXMVECTOR OverlayRenderer::PATH_NODE_RENDER_SIZE = XMVectorSet(3.0f, 1.5f, 3.0f, 0.0f);
const AXMVECTOR OverlayRenderer::PATH_NODE_RENDER_ORIGIN = XMVectorSet(XMVectorGetX(OverlayRenderer::PATH_NODE_RENDER_SIZE) / 2.0f, 0.0f, 
																	   XMVectorGetZ(OverlayRenderer::PATH_NODE_RENDER_SIZE) / 2.0f, 0.0f);
const int OverlayRenderer::m_box_dimensions[3][4] = { { 1, 3, 5, 7 }, { 8, 9, 10, 11 }, { 0, 2, 4, 6 } };

// Default constructor, initialise all values to defaults
OverlayRenderer::OverlayRenderer(void)
{
	// Set default values for certain parameters
	m_nodespinspeed = 1.0f;

	// Set initial standard model storage to null
	for (int i = 0; i < (int)OverlayRenderer::RenderColour::RC_COUNT; i++)
	{
		m_models[i] = NULL;
	}

	// Also set other models to NULL before initialisation
	m_blueprintoverlay = NULL;
	m_blueprintcubeoverlay = NULL;
	m_matrix_boxtransforms = NULL;
}

// Initialises the overlay rendering component; includes loading of all required models
Result OverlayRenderer::Initialise(void)
{
	Result result;

	// Line models will all use the same underlying geometry, but with the following different textures
	std::string line_models[] = { "overlay_none.dds", "overlay_green.dds", "overlay_red.dds", "overlay_lblue.dds" };

	// Create all models required for overlay rendering
	for (int i = 0; i < (int)OverlayRenderer::RenderColour::RC_COUNT; ++i)
	{
		m_models[i] = new Model();
		m_models[i]->SetCentredAboutOrigin(false);
		m_models[i]->SetCode(concat("OverlayModel")(i).str());
		result = m_models[i]->Initialise(	concat(D::DATA)("\\Models\\Misc\\unit_line.rjm").str(),
											concat(D::IMAGE_DATA)("\\Models\\Misc\\")(line_models[i]).str());
		if (result != ErrorCodes::NoError) return ErrorCodes::CannotLoadOverlayRendererModelComponents;
	}

	// Initialise blueprint overlay model
	m_blueprintoverlay = new Model();
	m_blueprintoverlay->SetCode("OverlayBlueprintModel");
	result = m_blueprintoverlay->Initialise(concat(D::DATA)("\\Models\\Misc\\unit_facing_square.rjm").str(),
											concat(D::IMAGE_DATA)("\\Models\\Misc\\overlay_blueprint.dds").str());
	if (result != ErrorCodes::NoError) return ErrorCodes::CannotLoadOverlayRendererModelComponents;

	// Initialise blueprint element-sized model
	m_blueprintcubeoverlay = new Model();
	m_blueprintcubeoverlay->SetCode("OverlayBlueprintCubeModel");
	result = m_blueprintcubeoverlay->Initialise(concat(D::DATA)("\\Models\\Misc\\unit_line.rjm").str(),
		concat(D::IMAGE_DATA)("\\Models\\Misc\\overlay_blueprint.dds").str());
	//m_blueprintcubeoverlay->SetActualModelSize(Game::ElementLocationToPhysicalPositionF(INTVECTOR3(1, 1, 1)));
	if (result != ErrorCodes::NoError) return ErrorCodes::CannotLoadOverlayRendererModelComponents;

	// Initialise all pre-cached transform matrices for render efficiency
	InitialiseCachedMatrices();

	// Return the overall initialisation result
	return ErrorCodes::NoError;
}

// Initialise all pre-cached transform matrices for render efficiency
void OverlayRenderer::InitialiseCachedMatrices()
{
	XMMATRIX mtmp;

	/* Basic matrices used for intermediate transforms */
	m_matrix_yrot = XMMatrixRotationY(PIOVER2);			// 90 degree rotation about the y axis
	m_matrix_xrotneg = XMMatrixRotationX(-PIOVER2);		// -90 degree rotation about the x axis

	/* Standard element-length, default thickness, line scaling transform */
	m_matrix_stdelementscale = XMMatrixScaling(DEFAULT_LINE_THICKNESS, DEFAULT_LINE_THICKNESS, Game::C_CS_ELEMENT_SCALE);

	/* Array of matrices for a default element-sized box */
	m_matrix_boxtransforms = new AXMMATRIX_P[12];												// 12 edges to a box
	
	// Bottom edges
	m_matrix_boxtransforms[0].value = ID_MATRIX;												// [0] = fwd from origin
	mtmp = XMMatrixRotationY(PIOVER2);
	m_matrix_boxtransforms[1].value = XMMatrixMultiply(mtmp, m_matrix_boxtransforms[0].value);	// [1] = right from origin
	mtmp = XMMatrixTranslation(Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f);
	m_matrix_boxtransforms[2].value = XMMatrixMultiply(mtmp, m_matrix_boxtransforms[0].value);	// [2] = fwd, to right of origin
	mtmp = XMMatrixTranslation(-Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f);
	m_matrix_boxtransforms[3].value = XMMatrixMultiply(m_matrix_boxtransforms[1].value, mtmp);	// [3] = right, above origin

	// Top edges
	mtmp = XMMatrixTranslation(0.0f, Game::C_CS_ELEMENT_SCALE, 0.0f);
	m_matrix_boxtransforms[4].value = XMMatrixMultiply(mtmp, m_matrix_boxtransforms[0].value);	// [4] = fwd, above [0]
	m_matrix_boxtransforms[5].value = XMMatrixMultiply(m_matrix_boxtransforms[1].value, mtmp);	// [5] = right, above [1]
	m_matrix_boxtransforms[6].value = XMMatrixMultiply(mtmp, m_matrix_boxtransforms[2].value);	// [6] = fwd, above [2]
	m_matrix_boxtransforms[7].value = XMMatrixMultiply(m_matrix_boxtransforms[3].value, mtmp);	// [7] = right, above [3]
	
	// Vertical edges
	mtmp = XMMatrixRotationX(-PI / 4.0f);
	m_matrix_boxtransforms[8].value = XMMatrixMultiply(m_matrix_boxtransforms[0].value, mtmp);	// [8] = up, from origin
	mtmp = XMMatrixTranslation(-Game::C_CS_ELEMENT_SCALE, 0.0f, 0.0f);
	m_matrix_boxtransforms[9].value = XMMatrixMultiply(m_matrix_boxtransforms[8].value, mtmp);	// [9] = up, right of origin
	mtmp = XMMatrixTranslation(0.0f, -Game::C_CS_ELEMENT_SCALE, 0.0f);
	m_matrix_boxtransforms[10].value = XMMatrixMultiply(m_matrix_boxtransforms[8].value, mtmp);	// [10] = up, in front of origin
	m_matrix_boxtransforms[11].value = XMMatrixMultiply(m_matrix_boxtransforms[9].value, mtmp);	// [11] = up, right & in front of origin

	/* Node-related cached matrices.  Also set the spin rate here so that derived fields are calculated */
	m_matrix_nodescale = XMMatrixScalingFromVector(OverlayRenderer::PATH_NODE_RENDER_SIZE);
	m_matrix_nodeorigin = XMMatrixTranslationFromVector(XMVectorNegate(OverlayRenderer::PATH_NODE_RENDER_ORIGIN));
	m_matrix_nodescale_and_origin = XMMatrixMultiply(m_matrix_nodescale, m_matrix_nodeorigin);
	SetNodeSpinSpeed(0.5f);
}

void OverlayRenderer::SetNodeSpinSpeed(float speed)
{
	// Store the new spin speed
	m_nodespinspeed = speed;

	// Recalculate the radians/millisecond rotation value based on this new speed
	m_nodespinradians = (speed * TWOPI) * (1.0f / 1000.0f);
}

// Method to add a line for rendering.  Accepts a world matrix for the line
void XM_CALLCONV OverlayRenderer::RenderLine(const FXMMATRIX world, OverlayRenderer::RenderColour colour)
{
	// Add a request to the core engine to render this line
	Game::Engine->RenderModel(m_models[(int)colour], world);
}

// Method to add a line for rendering.  Accepts a world matrix for the line
void XM_CALLCONV OverlayRenderer::RenderLine(const FXMMATRIX world, const FXMVECTOR position, const XMFLOAT3 & colour, float alpha)
{
	// Add a request to the core engine to render this line
	Game::Engine->RenderModel(m_models[RenderColour::RC_None], position, colour, alpha, world);
}

// Method to add a line for rendering.  Accepts a world matrix for the line, plus scaling length & thickness parameters
void XM_CALLCONV OverlayRenderer::RenderLine(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float length, float thickness)
{
	// Scale the current world matrix and pass it to the engine rendering method
	XMMATRIX mworld = XMMatrixMultiply(XMMatrixScaling(thickness, thickness, length), world);
	Game::Engine->RenderModel(m_models[(int)colour], mworld);
}

// Method to add a line for rendering.  Does all calculation of required world matrix to generate the line between two points
void OverlayRenderer::RenderLine(const FXMVECTOR pt1, const FXMVECTOR pt2, OverlayRenderer::RenderColour colour, float thickness, float length)
{
	// Derive the world matrix required to create a line from pt1 to pt2
	XMMATRIX world;
	DetermineLineWorldMatrix(world, pt1, pt2, thickness, length);

	// Now render the line based on this world matrix
	RenderLine(world, colour);
}

// Determines the world matrix required to transform a line model into the correct position, given only
// the coordinates of each line end point.  Also optionally accepts a line length parameter to avoid 
// the SQRT otherwise required for calculating the line length.
void OverlayRenderer::DetermineLineWorldMatrix(XMMATRIX & outMatrix, const FXMVECTOR pt1, const FXMVECTOR pt2, float thickness, float length)
{
	// Take the vector difference of the two points
	XMVECTOR vdiff = XMVectorSubtract(pt2, pt1);

	// If the length is not specified, calculate it now via aa * bb = cc
	if (length <= 0.0f) 
	{
		length = XMVectorGetX(XMVector3Length(vdiff));
	}

	// Start with a scaling matrix that will set both the line length and thickness in the same operation
	XMMATRIX scale = XMMatrixScaling(thickness, thickness, length);

	// Now determine the quaternion rotation from the basis vector to the desired heading difference, and apply via rotation matrix
	XMMATRIX rot = XMMatrixRotationQuaternion(QuaternionBetweenVectors(BASIS_VECTOR, vdiff));
	
	// Finally apply a translation matrix to move the transformed model into position, with pt1 at the origin and pt2
	// successfully transformed to its desired position (world = scale * rot * trans)
	outMatrix = XMMatrixMultiply(XMMatrixMultiply(scale, rot), XMMatrixTranslationFromVector(pt1));
}

// Method to render a box at the specified location.  World matrix specifies transforming to the target location/orientation.  Size/thickness
// are used to derive the scaling matrix
void XM_CALLCONV OverlayRenderer::RenderBox(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float thickness, float xSize, float ySize, float zSize)
{
	XMMATRIX mfinal, scale, trans, scale_x_rot;

	// First handle edges in the forward (Z) direction
	trans = ID_MATRIX; 
	scale = XMMatrixScaling(thickness, thickness, zSize);

	mfinal = XMMatrixMultiply(scale, world);							// > Edge 1: fwd from origin
	RenderLine(mfinal, colour);

	trans.r[3] = XMVectorSetX(trans.r[3], xSize - thickness);			// x-translation
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);	// > Edge 2: fwd, to right of origin
	RenderLine(mfinal, colour);
	
	trans.r[3] = XMVectorSetY(trans.r[3], ySize - thickness);			// now has x- & y-translation
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);	// > Edge 3: fwd, above and to the right of origin
	RenderLine(mfinal, colour);

	trans.r[3] = XMVectorSetX(trans.r[3], 0.0f);						// now has only y-translation
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);	// > Edge 4: fwd, above origin
	RenderLine(mfinal, colour);
	
	// Now handle edges in the right (X) direction
	//D3DXMatrixScaling(&scale, thickness, thickness, xSize); | scale._33 = xSize;					
	scale.r[2] = XMVectorSetZ(scale.r[2], xSize);							// update z-scaling factor to the length of an X side
	scale_x_rot = XMMatrixMultiply(scale, m_matrix_yrot);

	trans.r[3] = XMVectorSetZ(trans.r[3], thickness);						// translate forwards so in line
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 5: right, above origin
	RenderLine(mfinal, colour);
	
	trans.r[3] = XMVectorSetZ(trans.r[3], zSize);							// x-translation (in fwd direction)
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 6: right, above & in front of origin
	RenderLine(mfinal, colour);

	trans.r[3] = XMVectorSetY(trans.r[3], 0.0f);							// remove y translation, now at fwd-Z
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 7: right, ahead of origin
	RenderLine(mfinal, colour);

	trans.r[3] = XMVectorSetZ(trans.r[3], thickness);						// translate z-back to origin, minus thickness again
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 8: right, from origin
	RenderLine(mfinal, colour);

	// Finally handle edges in the up (Y) direction
	//D3DXMatrixScaling(&scale, thickness, thickness, ySize);
	scale.r[2] = XMVectorSetZ(scale.r[2], ySize);							// update z-scaling factor to the length of a Y side
	scale_x_rot = XMMatrixMultiply(scale, m_matrix_xrotneg);

	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 9: up, from origin
	RenderLine(mfinal, colour);

	trans.r[3] = XMVectorSetX(trans.r[3], xSize - thickness);				// translate to the right, minus thickness
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 10: up, to right of origin
	RenderLine(mfinal, colour);

	trans.r[3] = XMVectorSetZ(trans.r[3], zSize);							// translate fwd, so at fwd+right
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 11: up, ahead & right of origin
	RenderLine(mfinal, colour);

	trans.r[3] = XMVectorSetX(trans.r[3], 0.0f);
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 12: up, ahead of origin
	RenderLine(mfinal, colour);
}


// Method to render a box at the specified location
void XM_CALLCONV OverlayRenderer::RenderBox(const FXMMATRIX world, const XMFLOAT3 & size, const XMFLOAT3 & colour, float alpha, float thickness)
{
	XMMATRIX mfinal, scale, trans, scale_x_rot;


	// First handle edges in the forward (Z) direction
	trans = ID_MATRIX;
	scale = XMMatrixScaling(thickness, thickness, size.z);
	mfinal = XMMatrixMultiply(scale, world);							// > Edge 1: fwd from origin

	// Note: determine an approximate position for the box to aid in alpha testing
	XMVECTOR pos = XMVector3TransformCoord(NULL_VECTOR, mfinal);

	RenderLine(mfinal, pos, colour, alpha);

	trans.r[3] = XMVectorSetX(trans.r[3], size.x - thickness);			// x-translation
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);	// > Edge 2: fwd, to right of origin
	RenderLine(mfinal, pos, colour, alpha);

	trans.r[3] = XMVectorSetY(trans.r[3], size.y - thickness);			// now has x- & y-translation
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);	// > Edge 3: fwd, above and to the right of origin
	RenderLine(mfinal, pos, colour, alpha);

	trans.r[3] = XMVectorSetX(trans.r[3], 0.0f);						// now has only y-translation
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);	// > Edge 4: fwd, above origin
	RenderLine(mfinal, pos, colour, alpha);

	// Now handle edges in the right (X) direction
	//D3DXMatrixScaling(&scale, thickness, thickness, size.x); | scale._33 = size.x;					
	scale.r[2] = XMVectorSetZ(scale.r[2], size.x);							// update z-scaling factor to the length of an X side
	scale_x_rot = XMMatrixMultiply(scale, m_matrix_yrot);

	trans.r[3] = XMVectorSetZ(trans.r[3], thickness);						// translate forwards so in line
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 5: right, above origin
	RenderLine(mfinal, pos, colour, alpha);

	trans.r[3] = XMVectorSetZ(trans.r[3], size.z);							// x-translation (in fwd direction)
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 6: right, above & in front of origin
	RenderLine(mfinal, pos, colour, alpha);

	trans.r[3] = XMVectorSetY(trans.r[3], 0.0f);							// remove y translation, now at fwd-Z
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 7: right, ahead of origin
	RenderLine(mfinal, pos, colour, alpha);

	trans.r[3] = XMVectorSetZ(trans.r[3], thickness);						// translate z-back to origin, minus thickness again
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 8: right, from origin
	RenderLine(mfinal, pos, colour, alpha);

	// Finally handle edges in the up (Y) direction
	//D3DXMatrixScaling(&scale, thickness, thickness, size.y);
	scale.r[2] = XMVectorSetZ(scale.r[2], size.y);							// update z-scaling factor to the length of a Y side
	scale_x_rot = XMMatrixMultiply(scale, m_matrix_xrotneg);

	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 9: up, from origin
	RenderLine(mfinal, pos, colour, alpha);

	trans.r[3] = XMVectorSetX(trans.r[3], size.x - thickness);				// translate to the right, minus thickness
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 10: up, to right of origin
	RenderLine(mfinal, pos, colour, alpha);

	trans.r[3] = XMVectorSetZ(trans.r[3], size.z);							// translate fwd, so at fwd+right
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 11: up, ahead & right of origin
	RenderLine(mfinal, pos, colour, alpha);

	trans.r[3] = XMVectorSetX(trans.r[3], 0.0f);
	mfinal = XMMatrixMultiply(XMMatrixMultiply(scale_x_rot, trans), world);	// > Edge 12: up, ahead of origin
	RenderLine(mfinal, pos, colour, alpha);
}

// Method to render a box around the specified element of a complex ship
void OverlayRenderer::RenderElementBox(iSpaceObjectEnvironment *ship, const INTVECTOR3 & element, OverlayRenderer::RenderColour colour, float thickness)
{
	// Parameter check
	if (ship)
	{
		// Create a transform matrix to translate to the desired element.  Switch y & z since going from element > world space
		XMMATRIX trans = XMMatrixTranslationFromVector(
			XMVectorMultiply(VectorFromIntVector3SwizzleYZ(element), Game::C_CS_ELEMENT_SCALE_V));

		// Render a box at the desired location by multiplying the ship zero-point world matrix by this transform
		RenderBox( XMMatrixMultiply(trans, ship->GetZeroPointWorldMatrix()), colour, thickness, Game::C_CS_ELEMENT_SCALE );
	}
}

// Method to render a box around the specified element(s) of a complex ship
void OverlayRenderer::RenderElementBox(	iSpaceObjectEnvironment *ship, const INTVECTOR3 & element_location, const INTVECTOR3 & element_size, 
										const XMFLOAT3 & colour, float alpha, float thickness)
{
	// Parameter check
	if (ship)
	{
		// Create a transform matrix to translate to the desired element.  Switches y & z since going from element > world space
		XMMATRIX trans = XMMatrixTranslationFromVector(Game::ElementLocationToPhysicalPosition(element_location));

		// Determine the size of the box in world coordinates.  Switch y & z since going from element > world space
		XMFLOAT3 size = XMFLOAT3(	(float)element_size.x * Game::C_CS_ELEMENT_SCALE, (float)element_size.z * Game::C_CS_ELEMENT_SCALE,
									(float)element_size.y * Game::C_CS_ELEMENT_SCALE);

		// Render a box at the desired location by multiplying the ship zero-point world matrix by this transform
		RenderBox(XMMatrixMultiply(trans, ship->GetZeroPointWorldMatrix()), size, colour, alpha, thickness);
	}
}

// Method to render a box at the specified element *position* (i.e. not element index, but actual coord in element space)
void OverlayRenderer::RenderElementBoxAtRelativeElementLocation(iSpaceObjectEnvironment* ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour, float thickness)
{
	// Parameter check
	if (ship)
	{
		// Create a transform matrix to translate to the desired element.  Switch y & z since going from element > world space.  No need 
		// to scale translation by element size since this method accepts the coordinate value itself, not the element index
		XMMATRIX trans = XMMatrixTranslationFromVector(VectorFromIntVector3(elementpos));

		// Render a box at the desired location by multiplying the ship world matrix by this transform
		RenderBox( XMMatrixMultiply(trans, ship->GetZeroPointWorldMatrix()), colour, thickness, Game::C_CS_ELEMENT_SCALE );
	}
}

// Method to render a box at the specified element *position* (i.e. not element index, but actual coord in element space).  Allows any size
void OverlayRenderer::RenderBoxAtRelativeElementLocation(iSpaceObjectEnvironment* ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour,
														 float xSize, float ySize, float zSize, float thickness)
{
	// Parameter check
	if (ship)
	{
		// Create a transform matrix to translate to the desired element.  Switch y & z since going from element > world space.  No need 
		// to scale translation by element size since this method accepts the coordinate value itself, not the element index
		XMMATRIX trans = XMMatrixTranslationFromVector(VectorFromIntVector3(elementpos));

		// Render a box at the desired location by multiplying the ship world matrix by this transform
		RenderBox( XMMatrixMultiply(trans, ship->GetZeroPointWorldMatrix()), colour, thickness, xSize, ySize, zSize);
	}
}

// Method to add a cuboid for rendering.  Accepts a world matrix for the cuboid position, plus size parameters.  Uses line model.
void XM_CALLCONV OverlayRenderer::RenderCuboid(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float xSize, float ySize, float zSize)
{
	// Generate a scaling matrix to account for the length & thickness, plus a translation to offset to the cuboid centre
	// D3DXMatrixScaling(&mscale, xSize, ySize, zSize);
	// D3DXMatrixTranslation(&mtrans, xSize * -0.5f, ySize * -0.5f, zSize * -0.5f);
	XMVECTOR size = XMVectorSet(xSize, ySize, zSize, 0.0f);
	XMMATRIX scale = XMMatrixScalingFromVector(size);
	XMMATRIX trans = XMMatrixTranslationFromVector(XMVectorMultiply(size, HALF_VECTOR_N));

	// Scale & translate the current world matrix before passing it to the engine rendering method
	XMMATRIX mworld = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);
	Game::Engine->RenderModel(m_models[(int)colour], mworld);
}

// Method to add a cuboid for rendering.  Accepts a world matrix for the cuboid position, plus size parameters.  Uses line model.
void XM_CALLCONV OverlayRenderer::RenderCuboid(const FXMMATRIX world, OverlayRenderer::RenderColour colour, const CXMVECTOR size)
{
	// Generate a scaling matrix to account for the length & thickness, plus a translation to offset to the cuboid centre
	// D3DXMatrixScaling(&mscale, xSize, ySize, zSize);
	// D3DXMatrixTranslation(&mtrans, xSize * -0.5f, ySize * -0.5f, zSize * -0.5f);
	XMMATRIX scale = XMMatrixScalingFromVector(size);
	XMMATRIX trans = XMMatrixTranslationFromVector(XMVectorMultiply(size, HALF_VECTOR_N));

	// Scale & translate the current world matrix before passing it to the engine rendering method
	XMMATRIX mworld = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);
	Game::Engine->RenderModel(m_models[(int)colour], mworld);
}

// Method to add a cuboid for rendering.  Accepts a world matrix for the cuboid position, plus size parameters.  Uses line model.  Includes alpha blending.
// Therefore also requires the position to be provided for efficient depth-sorting
void XM_CALLCONV OverlayRenderer::RenderCuboid(const FXMMATRIX world, OverlayRenderer::RenderColour colour, float xSize, float ySize, float zSize, float alpha, const CXMVECTOR position)
{
	// Generate a scaling matrix to account for the length & thickness, plus a translation to offset to the cuboid centre
	// D3DXMatrixScaling(&mscale, xSize, ySize, zSize);
	// D3DXMatrixTranslation(&mtrans, xSize * -0.5f, ySize * -0.5f, zSize * -0.5f);
	XMVECTOR size = XMVectorSet(xSize, ySize, zSize, 0.0f);
	XMMATRIX scale = XMMatrixScalingFromVector(size);
	XMMATRIX trans = XMMatrixTranslationFromVector(XMVectorMultiply(size, HALF_VECTOR_N));

	// Scale & translate the current world matrix before passing it to the engine rendering method
	XMMATRIX mworld = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);
	Game::Engine->RenderModel(m_models[(int)colour], position, alpha, mworld);
}

// Method to add a cuboid for rendering.  Accepts a world matrix for the cuboid position, plus size parameters.  Uses line model.  Includes 
// custom colour.  No alpha blending
void XM_CALLCONV OverlayRenderer::RenderCuboid(const FXMMATRIX world, float xSize, float ySize, float zSize, const XMFLOAT4 & colour)
{
	// Generate a scaling matrix to account for the length & thickness, plus a translation to offset to the cuboid centre
	XMVECTOR size = XMVectorSet(xSize, ySize, zSize, 0.0f);
	XMMATRIX scale = XMMatrixScalingFromVector(size);
	XMMATRIX trans = XMMatrixTranslationFromVector(XMVectorMultiply(size, HALF_VECTOR_N));

	// Scale & translate the current world matrix before passing it to the engine rendering method
	XMMATRIX mworld = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);
	Game::Engine->RenderModel(m_models[RenderColour::RC_None], colour, mworld);
}

// Method to add a cuboid for rendering.  Accepts a world matrix for the cuboid position, plus size parameters.  Uses line model.  Includes 
// custom colour and alpha blending.  Requires the position to be provided for efficient depth-sorting
void XM_CALLCONV OverlayRenderer::RenderCuboid(const FXMMATRIX world, float xSize, float ySize, float zSize, const XMFLOAT3 & colour, float alpha, const CXMVECTOR position)
{
	// Generate a scaling matrix to account for the length & thickness, plus a translation to offset to the cuboid centre
	XMVECTOR size = XMVectorSet(xSize, ySize, zSize, 0.0f);
	XMMATRIX scale = XMMatrixScalingFromVector(size);
	XMMATRIX trans = XMMatrixTranslationFromVector(XMVectorMultiply(size, HALF_VECTOR_N));

	// Scale & translate the current world matrix before passing it to the engine rendering method
	XMMATRIX mworld = XMMatrixMultiply(XMMatrixMultiply(scale, trans), world);
	Game::Engine->RenderModel(m_models[RenderColour::RC_None], position, colour, alpha, mworld);
}


// Method to add a cuboid for rendering in a ship-relative element space location.
void OverlayRenderer::RenderCuboidAtRelativeElementLocation(iSpaceObjectEnvironment *ship, const INTVECTOR3 & elementpos, OverlayRenderer::RenderColour colour, 
														    float xSize, float ySize, float zSize)
{
	// Parameter check
	if (ship)
	{
		// Generate a scaling matrix to account for the length & thickness, plus a translation to both centre the item & move to the right element
		// Swap y & z coords since we are moving from element to world space
		// D3DXMatrixTranslation(&mtrans, elementpos.x - (xSize * 0.5f), elementpos.z - (ySize * 0.5f), elementpos.y - (zSize * 0.5f));
		XMVECTOR size = XMVectorSet(xSize, ySize, zSize, 0.0f);
		XMMATRIX scale = XMMatrixScalingFromVector(size);
		XMMATRIX trans = XMMatrixTranslationFromVector(XMVectorSubtract(
			VectorFromIntVector3(elementpos),
			XMVectorMultiply(size, HALF_VECTOR)));

		// Scale & translate the current world matrix before passing it to the engine rendering method
		XMMATRIX mworld = XMMatrixMultiply(XMMatrixMultiply(scale, trans), ship->GetZeroPointWorldMatrix());
		Game::Engine->RenderModel(m_models[(int)colour], mworld);
	}
}

// Method to add a cuboid for rendering in a ship-relative element space location.
void OverlayRenderer::RenderCuboidAtRelativeElementLocation(iSpaceObjectEnvironment *ship, const INTVECTOR3 & element_pos, const INTVECTOR3 & element_size, 
															const XMFLOAT3 & colour, float alpha)
{
	// Parameter check
	if (ship)
	{
		// Generate a scaling matrix to account for the length & thickness, plus a translation to both centre the item & move to the right element
		// Swap y & z coords since we are moving from element to world space
		// D3DXMatrixTranslation(&mtrans, elementpos.x - (xSize * 0.5f), elementpos.z - (ySize * 0.5f), elementpos.y - (zSize * 0.5f));
		XMVECTOR size = Game::ElementLocationToPhysicalPosition(element_size);
		XMMATRIX scale = XMMatrixScalingFromVector(size);
		XMMATRIX trans = XMMatrixTranslationFromVector(XMVectorMultiplyAdd(
			size, NULL_VECTOR, Game::ElementLocationToPhysicalPosition(element_pos)));

		// Scale & translate the current world matrix before passing it to the engine rendering method
		// Use the light highlight/fade shader to add the custom colour and alpha values
		XMMATRIX mworld = XMMatrixMultiply(XMMatrixMultiply(scale, trans), ship->GetZeroPointWorldMatrix());
		Game::Engine->RenderModel(m_models[RenderColour::RC_None], ship->GetPosition(), colour, alpha, mworld);

		//OutputDebugString(concat("El: ")(element_pos.ToString())(" = ")(Vector3ToString(XMVector3TransformCoord(NULL_VECTOR, mworld)))("\n").str().c_str());
	}
}

// Renders a cuboid based on the location of its vertices
void OverlayRenderer::RenderCuboid(AXMVECTOR_P(&pVertices)[8], OverlayRenderer::RenderColour colour, float thickness)
{
	// Render lines connecting the vertices.  [0-3] represent one end face in ccw order, [4-7] represent
	// the second end face in ccw order.  v[x] and v[x+4] are opposite vertices in the two end faces

	RenderLine(pVertices[0].value, pVertices[1].value, colour, thickness, -1.0f);		// These four lines connect face #1
	RenderLine(pVertices[1].value, pVertices[2].value, colour, thickness, -1.0f);
	RenderLine(pVertices[2].value, pVertices[3].value, colour, thickness, -1.0f);
	RenderLine(pVertices[3].value, pVertices[0].value, colour, thickness, -1.0f);
	RenderLine(pVertices[4].value, pVertices[5].value, colour, thickness, -1.0f);		// These four lines connect face #2
	RenderLine(pVertices[5].value, pVertices[6].value, colour, thickness, -1.0f);
	RenderLine(pVertices[6].value, pVertices[7].value, RenderColour::RC_Green, thickness*2.0f, -1.0f); // TODO: DEBUG
	RenderLine(pVertices[7].value, pVertices[4].value, colour, thickness, -1.0f);
	RenderLine(pVertices[0].value, pVertices[4].value, colour, thickness, -1.0f);		// These four lines connect the two parallel faces
	RenderLine(pVertices[1].value, pVertices[5].value, colour, thickness, -1.0f);
	RenderLine(pVertices[2].value, pVertices[6].value, colour, thickness, -1.0f);
	RenderLine(pVertices[3].value, pVertices[7].value, colour, thickness, -1.0f);
}

// Renders a cuboid based on the location of its vertices.  Also supplies the known cuboid size (edge to edge, i.e. 2*extent) to save calculation time
void OverlayRenderer::RenderCuboid(AXMVECTOR_P(&pVertices)[8], OverlayRenderer::RenderColour colour, float thickness, const CXMVECTOR size)
{
	// Render lines connecting the vertices.  [0-3] represent one end face in ccw order, [4-7] represent
	// the second end face in ccw order.  v[x] and v[x+4] are opposite vertices in the two end faces
	XMFLOAT3 sizef; XMStoreFloat3(&sizef, size);

	RenderLine(pVertices[0].value, pVertices[1].value, colour, thickness, sizef.x);		// These four lines connect face #1
	RenderLine(pVertices[1].value, pVertices[2].value, colour, thickness, sizef.y);
	RenderLine(pVertices[2].value, pVertices[3].value, colour, thickness, sizef.x);
	RenderLine(pVertices[3].value, pVertices[0].value, colour, thickness, sizef.y);
	RenderLine(pVertices[4].value, pVertices[5].value, colour, thickness, sizef.x);		// These four lines connect face #2
	RenderLine(pVertices[5].value, pVertices[6].value, colour, thickness, sizef.y);
	RenderLine(pVertices[6].value, pVertices[7].value, colour, thickness, sizef.x);
	RenderLine(pVertices[7].value, pVertices[4].value, colour, thickness, sizef.y);
	RenderLine(pVertices[0].value, pVertices[4].value, colour, thickness, sizef.z);		// These four lines connect the two parallel faces
	RenderLine(pVertices[1].value, pVertices[5].value, colour, thickness, sizef.z);
	RenderLine(pVertices[2].value, pVertices[6].value, colour, thickness, sizef.z);
	RenderLine(pVertices[3].value, pVertices[7].value, colour, thickness, sizef.z);
}

// Renders a semi-transparent ovelay above the specified element
void OverlayRenderer::RenderElementOverlay(iSpaceObjectEnvironment & ship, const INTVECTOR3 & element, const XMFLOAT4 & colour_alpha)
{
	// Create a transform matrix to translate to the desired element, and also to the centre of that element.  
	// Switch y & z since going from element > world space.  
	// D3DXMatrixTranslation(&mtrans,	(element.x * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT,
	//									((element.z + 1) * Game::C_CS_ELEMENT_SCALE),							// Render at base of element layer above
	// 									(element.y * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT);
	XMVECTOR add = XMVectorSetY(XMVectorReplicate(Game::C_CS_ELEMENT_MIDPOINT), Game::C_CS_ELEMENT_SCALE);		// { MidP, Scale, MidP }
	XMMATRIX trans = XMMatrixTranslationFromVector(XMVectorAdd(													// (El*Scale) + { MidP, Scale, MidP }
		XMVectorMultiply(VectorFromIntVector3SwizzleYZ(element), Game::C_CS_ELEMENT_SCALE_V), add));

	// Scale to element size, then translate to the relevant place for rendering
	Game::Engine->RenderModelFlat(m_blueprintoverlay, ship.GetPosition(), colour_alpha,
		XMMatrixMultiply(XMMatrixMultiply(ELEMENT_SCALE_MATRIX, trans), ship.GetZeroPointWorldMatrix()));
}

// Renders a semi-transparent ovelay 3D overlay around the specified element
void OverlayRenderer::RenderElement3DOverlay(iSpaceObjectEnvironment & ship, const INTVECTOR3 & element, const XMFLOAT4 & colour_alpha)
{
	// Create a transform matrix to translate to the desired element, and also to the centre of that element.  
	// Switch y & z since going from element > world space.  
	// D3DXMatrixTranslation(&mtrans,	(element.x * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT,
	//									(element.z * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT,
	// 									(element.y * Game::C_CS_ELEMENT_SCALE) + Game::C_CS_ELEMENT_MIDPOINT);
	XMMATRIX trans = XMMatrixTranslationFromVector(XMVectorAdd(													// (El*Scale) 
		XMVectorMultiply(VectorFromIntVector3SwizzleYZ(element), Game::C_CS_ELEMENT_SCALE_V),					// + { MidP, MidP, MidP }
		Game::C_CS_ELEMENT_MIDPOINT_V));

	// Scale to element size, then translate to the relevant place for rendering
	Game::Engine->RenderModelFlat(m_blueprintcubeoverlay, ship.GetPosition(), colour_alpha,
		XMMatrixMultiply(XMMatrixMultiply(ELEMENT_SCALE_MATRIX, trans), ship.GetZeroPointWorldMatrix()));
}

// Renders an OBB to world space.  Base thickness is the width of the bounding lines that will be drawn for branch OBBs.  Leaf OBBs
// will be rendered at a multiple of this thickness so it is clear which OBBs are actually colliding objects
void OverlayRenderer::RenderOBB(const OrientedBoundingBox & obb, bool recursive, OverlayRenderer::RenderColour colour, float basethickness)
{
	// Make sure the OBB is valid
	if (IsZeroFloat3(obb.ConstData().ExtentF)) return;

	// Get the vertices that make up this OBB
	AXMVECTOR_P v[8];
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

// Overloaded method to render a node in world space.  Accepts a node position and constructs the required world matrix
void XM_CALLCONV OverlayRenderer::RenderNode(const FXMVECTOR pos, OverlayRenderer::RenderColour colour)
{
	RenderNode(XMMatrixTranslationFromVector(pos), colour);
}


// Method to add a node for rendering.  Accepts a world matrix for the cuboid position, plus size parameters.  Uses line model.  Spins in place.
void XM_CALLCONV OverlayRenderer::RenderNode(const FXMMATRIX world, OverlayRenderer::RenderColour colour)
{
	// Determine the rotation matrix to be used based on current clock time
	XMMATRIX mrot = XMMatrixRotationY((Game::ClockMs % 1000) * m_nodespinradians);

	// Scale, translate * rotate the node before moving it into world space and calling the engine rendering method
	XMMATRIX mworld = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(
		m_matrix_nodescale,
		m_matrix_nodeorigin),
		mrot),
		world);
	Game::Engine->RenderModel(m_models[(int)colour], mworld);
}

// Method to add a node for rendering in a ship-relative element space location.  Spins in place.
void OverlayRenderer::RenderNodeAtRelativeElementLocation(iSpaceObjectEnvironment *ship, INTVECTOR3 elementpos, OverlayRenderer::RenderColour colour)
{
	// Parameter check
	if (ship)
	{
		// Determine the rotation matrix to be used based on current clock time
		XMMATRIX mrot = XMMatrixRotationY((Game::ClockMs % 1000) * m_nodespinradians);

		// Generate translation matrix.  We will be translating from the -ve origin point by the offset amount, so will end up just -ve from
		// the offset point by the required amount to centre the node; swap y & z coords since we are moving from element to world space
		XMMATRIX mtrans = XMMatrixTranslationFromVector(VectorFromIntVector3SwizzleYZ(elementpos));

		// Translate to origin, scale, rotate the node, translate to position, before moving it into world space and calling the engine rendering method
		XMMATRIX mworld = XMMatrixMultiply(XMMatrixMultiply(XMMatrixMultiply(
			m_matrix_nodescale_and_origin,
			mrot),
			mtrans),
			ship->GetZeroPointWorldMatrix());
		Game::Engine->RenderModel(m_models[(int)colour], mworld);
	}
}


// Methods to render the path being taken by an actor through a complex ship environment
void OverlayRenderer::RenderActorPath(Actor *actor, float thickness)
{
	// Parameter check
	if (actor)
	{
		INTVECTOR3 pos, el;

		// Get a reference to the actor environment, and make sure it is valid
		iSpaceObjectEnvironment *env = actor->GetParentEnvironment(); if (!env) return;

		// Iterate through the actor order queue
		Actor::OrderQueue::const_iterator it_end = actor->Orders.end();
		for (Actor::OrderQueue::const_iterator it = actor->Orders.begin(); it != it_end; ++it)
		{
			// If this is a 'travel to position' order then we want to render it
			if ((*it)->GetType() == Order::OrderType::ActorTravelToPosition)
			{
				const Order_ActorTravelToPosition *ao = (const Order_ActorTravelToPosition*)(*it);
				for (int i = 0; i < ao->PathLength; ++i)
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

// Renders an overlay over the specified environment.  Accepts a function that determines the overlay at each element
// The function parameter has signature "XMFLOAT4 func(environment, element_id)" and returns the colour/alpha for the 
// overlay.  It is applied for each element in turn
void OverlayRenderer::RenderEnvironmentOverlay(iSpaceObjectEnvironment & env, XMFLOAT4(*func)(iSpaceObjectEnvironment&, int))
{
	// Iterate over each element in turn
	int n = env.GetElementCount();
	for (int i = 0; i < n; ++i)
	{
		// Apply the function to this element and get the resulting colour/alpha, then
		// render an overlay on the corresponding element
		RenderElementOverlay(env, env.GetElementDirect(i).GetLocation(), func(env, i));
	}
}

// Renders an overlay over the specified environment, for the specific deck of the environment.  Accepts a function
// that determines the overlay at each element.  The function parameter has signature "XMFLOAT4 func(environment, element_id)" 
// and returns the colour/alpha for the overlay.  It is applied for each element in turn
void OverlayRenderer::RenderEnvironmentOverlay(iSpaceObjectEnvironment & env, int deck, XMFLOAT4(*func)(iSpaceObjectEnvironment&, int))
{
	// Retrieve data on the deck that we want to render
	const iSpaceObjectEnvironment::DeckInfo & deck_info = env.GetDeckInformation(deck);

	// Iterate over each element in this deck in turn
	for (int i = deck_info.ElementStart; i <= deck_info.ElementEnd; ++i)
	{
		// Apply the function to this element and get the resulting colour/alpha, then
		// render an overlay on the corresponding element
		RenderElementOverlay(env, env.GetElementDirect(i).GetLocation(), func(env, i));
	}
}


// Renders an overlay over the specified environment.  Accepts a function that determines the overlay at each element
// The function parameter has signature "XMFLOAT4 func(environment, element_id)" and returns the colour/alpha for the 
// overlay.  It is applied for each element in turn and generates a 3D overlay around each element
void OverlayRenderer::RenderEnvironment3DOverlay(iSpaceObjectEnvironment & env, XMFLOAT4(*func)(iSpaceObjectEnvironment&, int))
{
	// Iterate over each element in turn
	int n = env.GetElementCount();
	for (int i = 0; i < n; ++i)
	{
		// Apply the function to this element and get the resulting colour/alpha, then
		// render an overlay on the corresponding element
		RenderElement3DOverlay(env, env.GetElementDirect(i).GetLocation(), func(env, i));
	}
}

// Renders an overlay over the specified environment, for the specific deck of the environment.  Accepts a function
// that determines the overlay at each element.  The function parameter has signature "XMFLOAT4 func(environment, element_id)" 
// and returns the colour/alpha for the overlay.  It is applied for each element in turn and generates a 3D overlay around each element
void OverlayRenderer::RenderEnvironment3DOverlay(iSpaceObjectEnvironment & env, int deck, XMFLOAT4(*func)(iSpaceObjectEnvironment&, int))
{
	// Retrieve data on the deck that we want to render
	const iSpaceObjectEnvironment::DeckInfo & deck_info = env.GetDeckInformation(deck);

	// Iterate over each element in this deck in turn
	for (int i = deck_info.ElementStart; i <= deck_info.ElementEnd; ++i)
	{
		// Apply the function to this element and get the resulting colour/alpha, then
		// render an overlay on the corresponding element
		RenderElement3DOverlay(env, env.GetElementDirect(i).GetLocation(), func(env, i));
	}
}

void OverlayRenderer::DebugRenderEnvironmentTree(const EnvironmentTree *tree, bool include_children)
{
	// Parameter check
	if (!tree) return;
	const iSpaceObjectEnvironment *env = tree->GetEnvironment();
	if (!env) return;

	// Create a world matrix that will translate the rendered box into position, then render it
	XMFLOAT3 sizef = tree->GetSizeF();
	XMMATRIX world = XMMatrixMultiply(XMMatrixTranslationFromVector(tree->GetMin()), env->GetZeroPointWorldMatrix());
	RenderBox(world, (tree->GetTotalItemCount() == 0 ? RenderColour::RC_Red : RenderColour::RC_Green),
		(tree->GetTotalItemCount() == 0 ? 0.25f : 0.5f), sizef.x, sizef.y, sizef.z);

	// Now also render children if required
	if (include_children)
	{
		int children = tree->GetChildCount();
		for (int i = 0; i < children; ++i)
			DebugRenderEnvironmentTree(tree->GetActiveChildNode(i), true);
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
	if (m_matrix_boxtransforms) { SafeDeleteArray(m_matrix_boxtransforms); }
	if (m_blueprintoverlay) { m_blueprintoverlay->Shutdown(); SafeDelete(m_blueprintoverlay); }
	if (m_blueprintcubeoverlay) { m_blueprintcubeoverlay->Shutdown(); SafeDelete(m_blueprintcubeoverlay); }
}


OverlayRenderer::~OverlayRenderer(void)
{
}



