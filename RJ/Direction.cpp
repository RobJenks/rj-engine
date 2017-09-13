#include <string>
#include "HashFunctions.h"
#include "FastMath.h"
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

std::string DirectionToString(Direction edge)
{
	switch (edge)
	{
		case Direction::Left:		return "left";
		case Direction::Up:			return "up";
		case Direction::Right:		return "right";
		case Direction::Down:		return "down";
		case Direction::UpLeft:		return "upleft";
		case Direction::UpRight:	return "upright";
		case Direction::DownRight:	return "downright";
		case Direction::DownLeft:	return "downleft";
		case Direction::ZUp:		return "zup";
		case Direction::ZDown:		return "zdown";
		default:					return NullString;
	}
}
Direction DirectionFromString(std::string edge)
{
	// All comparisons are case-insensitive
	StrLowerC(edge);
	
	if (edge == "left")				return Direction::Left;
	else if (edge == "up")			return Direction::Up;
	else if (edge == "right")		return Direction::Right;
	else if (edge == "down")		return Direction::Down;
	else if (edge == "upleft")		return Direction::UpLeft;
	else if (edge == "upright")		return Direction::UpRight;
	else if (edge == "downright")	return Direction::DownRight;
	else if (edge == "downleft")	return Direction::DownLeft;
	else if (edge == "zup")			return Direction::ZUp;
	else if (edge == "zdown")		return Direction::ZDown;
	else							return Direction::_Count;
}

Direction GetOppositeDirection(Direction dir)
{
	switch (dir)
	{
		case Direction::Left:		return Direction::Right;
		case Direction::Up:			return Direction::Down;
		case Direction::Right:		return Direction::Left;
		case Direction::Down:		return Direction::Up;
		case Direction::UpLeft:		return Direction::DownRight;
		case Direction::UpRight:	return Direction::DownLeft;
		case Direction::DownRight:	return Direction::UpLeft;
		case Direction::DownLeft:	return Direction::UpRight;
		case Direction::ZUp:		return Direction::ZDown;
		case Direction::ZDown:		return Direction::ZUp;
		default:					return Direction::_Count;
	}
}

DirectionBS GetOppositeDirectionBS(DirectionBS dir)
{
	switch (dir)
	{
		case DirectionBS::Left_BS:		return DirectionBS::Right_BS;
		case DirectionBS::Up_BS:		return DirectionBS::Down_BS;
		case DirectionBS::Right_BS:		return DirectionBS::Left_BS;
		case DirectionBS::Down_BS:		return DirectionBS::Up_BS;
		case DirectionBS::UpLeft_BS:	return DirectionBS::DownRight_BS;
		case DirectionBS::UpRight_BS:	return DirectionBS::DownLeft_BS;
		case DirectionBS::DownRight_BS:	return DirectionBS::UpLeft_BS;
		case DirectionBS::DownLeft_BS:	return DirectionBS::UpRight_BS;
		case DirectionBS::ZUp_BS:		return DirectionBS::ZDown_BS;
		case DirectionBS::ZDown_BS:		return DirectionBS::ZUp_BS;
		default:						return DirectionBS::None_BS;
	}
}

// Return the closest Direction value to the given heading vector
Direction DetermineClosestDirectionToVector(const FXMVECTOR v)
{
	// Take the dot product (v . unit-vectors[...]) and return the index (== Direction) closest to 1.0
	int best_index = 0;
	XMVECTOR dp = XMVector3Dot(v, DirectionVectors[0]);

	for (int i = 1; i < Direction::_Count; ++i)
	{
		// The dot product of two completely parallel vectors with same direction is 1.0, so 
		// we want to select the direction vector closest to this ideal
		XMVECTOR diff = XMVectorAbs(XMVectorSubtract(ONE_VECTOR, XMVector3Dot(v, DirectionVectors[i])));
		if (XMVector3Less(diff, dp))
		{
			dp = diff;
			best_index = i;
		}
	}

	return (Direction)best_index;
}



