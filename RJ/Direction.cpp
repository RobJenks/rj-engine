#include <string>
#include "Direction.h"


// Parses the string representation of a 90-degree rotation value
Rotation90Degree TranslateRotation90Degree(std::string rotvalue)
{
	if (rotvalue == "0")				return Rotation90Degree::Rotate0;
	else if (rotvalue == "90")				return Rotation90Degree::Rotate90;
	else if (rotvalue == "180")				return Rotation90Degree::Rotate180;
	else if (rotvalue == "270")				return Rotation90Degree::Rotate270;
	else									return Rotation90Degree::Rotate0;
}

// Returns the result of rotating one 90-degree value by another
Rotation90Degree Compose90DegreeRotations(Rotation90Degree current_rotation, Rotation90Degree rotate_by)
{
	// The rotated direction will be the sum of the rotation indices, modulo 4.  E.g. if the current 
	// rotation is 90d and we are rotating by 270d, the new rotation value will be
	//    ROT90_VALUES[((int)Rotate90 + (int)Rotate270) % 4] = ROT90_VALUES[(1+3) % 4] = ROT90_VALUES[0] = Rotate0
	return ROT90_VALUES[((int)current_rotation + (int)rotate_by) % 4];
}

// Returns the 90-degree rotation required to transform between two rotation values
Rotation90Degree Rotation90BetweenValues(Rotation90Degree start_rotation, Rotation90Degree end_rotation)
{
	// Subtract one rotation from the other, making sure both are within the valid range of values.  rot_value will always be in the range [-3, +3]
	int rot_value = (((int)end_rotation % 4) - ((int)start_rotation % 4));

	// The value could be negative (in the range -1 to -3) in case of counter-clockwise rotation, so account for that here
	if (rot_value < 0) rot_value += 4;

	// Convert back to a 90-degree rotation value and return
	return (Rotation90Degree)rot_value;
}
