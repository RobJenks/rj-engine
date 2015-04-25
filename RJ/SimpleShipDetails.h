#pragma once

#ifndef __SimpleShipDetailsH__
#define __SimpleShipDetailsH__

#include "DX11_Core.h"
#include "GameDataExtern.h"
#include "ShipDetails.h"

class SimpleShipDetails : public ShipDetails
{
public:
	D3DXVECTOR3		CameraPosition;			// Coordinates of the camera position
	D3DXVECTOR3		CameraRotation;			// Pitch/yaw/roll for the camera, relative to this ship
	D3DXMATRIX		CameraPositionMatrix;	// Efficiency measure; matrix derived from the camera position & orientation
	float			CameraElasticity;		// The amount by which the camera can deviate from centre when maneuvering

	//int					NumBoundingObjects;		// The number of bounding objects for this ship
	//BoundingObject		*Bounds;				// The bounding object(s) that encompass this ship

	void			RecalculateShipDetails(void);	// Recalculates the values of all derived fields

	SimpleShipDetails * Copy(void);

	CMPINLINE iHardpoints * GetHardpoints(void) { return (iHardpoints*)HP; }

	void			Shutdown(void);

	SimpleShipDetails(void);
	~SimpleShipDetails(void);

	static SimpleShipDetails *SimpleShipDetails::Get(const string &code);



};



#endif 







