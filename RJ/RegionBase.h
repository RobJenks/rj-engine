#pragma once
#ifndef __RegionBaseH__
#define __RegionBaseH__

#include "DX11_Core.h"

#include "CompilerSettings.h"
#include "ErrorCodes.h"

// Class is 16-bit aligned to allow use of SIMD member variables
__declspec(align(16))
class RegionBase : public ALIGN16<RegionBase>
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
	virtual void					MoveRegion(const FXMVECTOR centre)					= 0;

	// Method to update the region boundaries
	virtual void					UpdateRegionBoundaries(const FXMVECTOR boundary)	= 0;

	// Method to update the entire region
	virtual void					UpdateRegion(void)									= 0;

	// Retrieve coordinates of the centre of the region
	CMPINLINE XMVECTOR				GetRegionCentre(void) { return m_centre; }

	// Minimum bounds of the region
	CMPINLINE XMVECTOR				GetMinBounds(void) { return m_minbounds; }
	CMPINLINE void					SetMinBounds(const FXMVECTOR min) { m_minbounds = min; }

	// Maximum bounds of the region
	CMPINLINE XMVECTOR				GetMaxBounds(void) { return m_maxbounds; }
	CMPINLINE void					SetMaxBounds(const FXMVECTOR max) { m_maxbounds = max; }

	// Region update threshold in each dimension
	CMPINLINE XMVECTOR				GetUpdateThreshold(void) { return m_threshold; }
	CMPINLINE void					SetUpdateThreshold(const FXMVECTOR threshold) { m_threshold = threshold; }

	// Previous region centre, for use in calculating when to perform updates
	CMPINLINE XMVECTOR				GetPreviousCentre(void) { return m_prevcentre; }
	CMPINLINE void					SetPreviousCentre(const FXMVECTOR prevcentre) { m_prevcentre = prevcentre; }

	// Method to determine whether an update is required, based on the threshold & current/previous centre
	bool							DetermineThresholdUpdateRequired(XMVECTOR & outBoundary);


protected:
	AXMVECTOR						m_centre;				// Coordinates of the centre of the region
	AXMVECTOR						m_minbounds;			// Min region bounds in each dimension
	AXMVECTOR						m_maxbounds;			// Max region bounds in each dimension

	AXMVECTOR						m_threshold;			// Update threshold in each dimension
	AXMVECTOR						m_prevcentre;			// Previous centre point; to calculate whether an update is required
	AXMVECTOR						m_boundary;				// The boundary region requiring update, following each calculation cycle


};



#endif