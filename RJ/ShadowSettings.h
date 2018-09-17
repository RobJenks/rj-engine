#pragma once

class Model;
class ArticulatedModel;

class ShadowSettings
{
public:

	// Objects smaller than this threshold will not cast shadows, by default
	static const float					MIN_SHADOW_CASTER_RADIUS;

	// Determines whether an object with the given properties should cast shadows
	static bool							ShouldCastShadows(Model *model, float bounding_radius);
	static bool							ShouldCastShadows(ArticulatedModel *model, float bounding_radius);


private:

	// Evaluate the common criteria for shadow rendering
	static bool							VerifyCommonCriteria(float bounding_radius);


};
