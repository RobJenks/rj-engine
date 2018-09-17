#include "ShadowSettings.h"
#include "Model.h"


// Objects smaller than this threshold will not cast shadows, by default
const float ShadowSettings::MIN_SHADOW_CASTER_RADIUS = 0.05f;



// Determines whether an object with the given properties should cast shadows
bool ShadowSettings::ShouldCastShadows(Model *model, float bounding_radius)
{
	// Must have geometry in order to cast shadows
	if (!model) return false;

	// Evaluate common criteria
	return VerifyCommonCriteria(bounding_radius);
}

// Determines whether an object with the given properties should cast shadows
bool ShadowSettings::ShouldCastShadows(ArticulatedModel *model, float bounding_radius)
{
	// Must have geometry in order to cast shadows
	if (!model) return false;

	// Evaluate common criteria
	return VerifyCommonCriteria(bounding_radius);
}





// Evaluate the common criteria for shadow rendering
bool ShadowSettings::VerifyCommonCriteria(float bounding_radius)
{
	// Validate against minimum allowable size
	if (bounding_radius < MIN_SHADOW_CASTER_RADIUS) return false;

	// Passed all checks
	return true;
}
