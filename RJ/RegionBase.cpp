#include "RegionBase.h"

RegionBase::RegionBase(void)
{
}

RegionBase::~RegionBase(void)
{
}

// Determines whether an update is required, based on the threshold & current/previous centres
bool RegionBase::DetermineThresholdUpdateRequired(D3DXVECTOR3 & boundary)
{
	// Determine difference between the current and previous centre points
	D3DXVECTOR3 diff = D3DXVECTOR3( m_centre.x - m_prevcentre.x, 
									m_centre.y - m_prevcentre.y,
									m_centre.z - m_prevcentre.z );

	// Check whether threshold is exceeded, and return difference vector if it is in any dimension
	if (diff.x > m_threshold.x || diff.y > m_threshold.y || diff.z > m_threshold.z) {
		boundary = D3DXVECTOR3(diff);
		return true;
	}
	else
		return false;
}

