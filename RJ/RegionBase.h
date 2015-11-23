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

	// Method to move the region to a new centre point; performs the logic to determine whether any updates are necessary
	virtual void					MoveRegion(const FXMVECTOR centre)					= 0;

	// Method to update the region boundaries
	virtual void					UpdateRegionBoundaries(const FXMVECTOR boundary)	= 0;

	// Method to update the entire region
	virtual void					UpdateRegion(void)									= 0;

	// Retrieve coordinates of the centre of the region
	CMPINLINE XMVECTOR				GetRegionCentre(void) { return m_centre; }

	// Minimum bounds of the region
	CMPINLINE XMVECTOR				GetMinBounds(void) const { return m_minbounds; }
	CMPINLINE XMFLOAT3				GetMinBoundsF(void) const { return m_minboundsf; }
	CMPINLINE void					SetMinBounds(const FXMVECTOR min) 
	{ 
		m_minbounds = min; 
		XMStoreFloat3(&m_minboundsf, m_minbounds);
	}

	// Maximum bounds of the region
	CMPINLINE XMVECTOR				GetMaxBounds(void) const { return m_maxbounds; }
	CMPINLINE XMFLOAT3				GetMaxBoundsF(void) const { return m_maxboundsf; }
	CMPINLINE void					SetMaxBounds(const FXMVECTOR max) 
	{ 
		m_maxbounds = max; 
		XMStoreFloat3(&m_maxboundsf, m_maxbounds);
	}

	// Set the region bounds
	CMPINLINE void					SetRegionBounds(const FXMVECTOR bmin, const FXMVECTOR bmax)
	{
		m_minbounds = bmin; m_maxbounds = bmax;
		XMStoreFloat3(&m_minboundsf, m_minbounds);
		XMStoreFloat3(&m_maxboundsf, m_maxbounds);
	}


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

	XMFLOAT3						m_minboundsf;			// Local float representation of key field for convenience
	XMFLOAT3						m_maxboundsf;			// Local float representation of key field for convenience

};



#endif