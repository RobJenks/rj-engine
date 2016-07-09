#include "Utility.h"
#include "BasicRay.h"


// Generates a string representation of the ray
std::string BasicRay::ToString(void) const
{
	return concat("BasicRay { ")(Vector3ToString(Origin))(" + k")(Vector3ToString(Direction))(" }").str();
}