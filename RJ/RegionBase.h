#pragma once
#ifndef __RegionBaseH__
#define __RegionBaseH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "ErrorCodes.h"

class RegionBase
{
public:
	RegionBase(void);
	~RegionBase(void);

	// Initialisation method - not currently specified as part of the interface, since each region will require very different data

	/*virtual Result					Initialise(const D3DXVECTOR3 & centre, 
										const D3DXVECTOR3 & minbounds, 
										const D3DXVECTOR3 & maxbounds, 
										const D3DXVECTOR3 & updatethreshold)		= 0;*/

	// Method to move the region to a new centre point; performs the logic to determine whether any updates are necessary
	virtual void					MoveRegion(D3DXVECTOR3 centre)					= 0;

	// Method to update the region boundaries
	virtual void					UpdateRegionBoundaries(D3DXVECTOR3 boundary)	= 0;

	// Method to update the entire region
	virtual void					UpdateRegion(void)								= 0;

	// Retrieve coordinates of the centre of the region
	CMPINLINE D3DXVECTOR3*			GetRegionCentre(void) { return &m_centre; }

	// Minimum bounds of the region
	CMPINLINE D3DXVECTOR3*			GetMinBounds(void) { return &m_minbounds; }
	CMPINLINE void					SetMinBounds(D3DXVECTOR3 *min) { m_minbounds = *min; }

	// Maximum bounds of the region
	CMPINLINE D3DXVECTOR3*			GetMaxBounds(void) { return &m_maxbounds; }
	CMPINLINE void					SetMaxBounds(D3DXVECTOR3 *max) { m_maxbounds = *max; }

	// Region update threshold in each dimension
	CMPINLINE D3DXVECTOR3*			GetUpdateThreshold(void) { return &m_threshold; }
	CMPINLINE void					SetUpdateThreshold(D3DXVECTOR3 *threshold) { m_threshold = *threshold; }

	// Previous region centre, for use in calculating when to perform updates
	CMPINLINE D3DXVECTOR3*			GetPreviousCentre(void) { return &m_prevcentre; }
	CMPINLINE void					SetPreviousCentre(D3DXVECTOR3 *prevcentre) { m_prevcentre = *prevcentre; }

	// Method to determine whether an update is required, based on the threshold & current/previous centre
	bool							DetermineThresholdUpdateRequired(D3DXVECTOR3 & boundary);


protected:
	D3DXVECTOR3						m_centre;				// Coordinates of the centre of the region
	D3DXVECTOR3						m_minbounds;			// Min region bounds in each dimension
	D3DXVECTOR3						m_maxbounds;			// Max region bounds in each dimension

	D3DXVECTOR3						m_threshold;			// Update threshold in each dimension
	D3DXVECTOR3						m_prevcentre;			// Previous centre point; to calculate whether an update is required
	D3DXVECTOR3						m_boundary;				// The boundary region requiring update, following each calculation cycle


};



#endif