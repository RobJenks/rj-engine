#include "RegionBase.h"

RegionBase::RegionBase(void)
{
}

RegionBase::~RegionBase(void)
{
}

// Determines whether an update is required, based on the threshold & current/previous centres
bool RegionBase::DetermineThresholdUpdateRequired(XMVECTOR & outBoundary)
{
	// Determine difference between the current and previous centre points
	XMVECTOR diff = XMVectorSubtract(m_centre, m_prevcentre);

	// Check whether threshold is exceeded, and return difference vector if it is in any dimension
	// if (diff.x > m_threshold.x || diff.y > m_threshold.y || diff.z > m_threshold.z) {
	if (!(XMVector3LessOrEqual(diff, m_threshold)))
	{
		outBoundary = diff;
		return true;
	}
	else
		return false;
}

