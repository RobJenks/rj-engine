#pragma once

#include <string>
#include "CompilerSettings.h"

// Enumeration of possible 90-degree directions
enum Direction { Left = 0, Up, Right, Down, UpLeft, UpRight, DownRight, DownLeft, ZUp, ZDown, _Count };
const int DirectionCount = 10;

// Structure holding the basic directions, for iteration over the possible movement directions
const int Directions[DirectionCount] = { Direction::Left, Direction::Up, Direction::Right, Direction::Down, Direction::UpLeft,
Direction::UpRight, Direction::DownRight, Direction::DownLeft, Direction::ZUp, Direction::ZDown };

// Bitstring direction values
enum DirectionBS
{
	Left_BS = (1 << Direction::Left),
	Up_BS = (1 << Direction::Up),
	Right_BS = (1 << Direction::Right),
	Down_BS = (1 << Direction::Down),
	UpLeft_BS = (1 << Direction::UpLeft),
	UpRight_BS = (1 << Direction::UpRight),
	DownRight_BS = (1 << Direction::DownRight),
	DownLeft_BS = (1 << Direction::DownLeft),
	ZUp_BS = (1 << Direction::ZUp),
	ZDown_BS = (1 << Direction::ZDown),
	None_BS = (1 << Direction::_Count)
};

const DirectionBS DirectionBS_All = (DirectionBS)(DirectionBS::Left_BS | DirectionBS::Up_BS | DirectionBS::Right_BS | DirectionBS::Down_BS | DirectionBS::UpLeft_BS |
	DirectionBS::UpRight_BS | DirectionBS::DownRight_BS | DirectionBS::DownLeft_BS | DirectionBS::ZUp_BS | DirectionBS::ZDown_BS);

// Convert a direction into a bitstring direction
CMPINLINE DirectionBS		DirectionToBS(Direction direction) { return (DirectionBS)(1 << direction); }

// Convert a bitstring direction into a direction
CMPINLINE Direction			BSToDirection(DirectionBS direction)
{
	switch (direction)
	{
		case DirectionBS::Left_BS:		return Direction::Left;
		case DirectionBS::Up_BS:		return Direction::Up;
		case DirectionBS::Right_BS:		return Direction::Right;
		case DirectionBS::Down_BS:		return Direction::Down;
		case DirectionBS::UpLeft_BS:	return Direction::UpLeft;
		case DirectionBS::UpRight_BS:	return Direction::UpRight;
		case DirectionBS::DownRight_BS:	return Direction::DownRight;
		case DirectionBS::DownLeft_BS:	return Direction::DownLeft;
		case DirectionBS::ZUp_BS:		return Direction::ZUp;
		case DirectionBS::ZDown_BS:		return Direction::ZDown;
		default:						return Direction::_Count;
	}
}

// Structure that holds the effect of applying a rotation to each direction value (in 2D only)
const int RotatedDirections[Direction::_Count + 1][4] = {
	{ Direction::Left, Direction::Up, Direction::Right, Direction::Down },
	{ Direction::Up, Direction::Right, Direction::Down, Direction::Left },
	{ Direction::Right, Direction::Down, Direction::Left, Direction::Up },
	{ Direction::Down, Direction::Left, Direction::Up, Direction::Right },
	{ Direction::UpLeft, Direction::UpRight, Direction::DownRight, Direction::DownLeft },
	{ Direction::UpRight, Direction::DownRight, Direction::DownLeft, Direction::UpLeft },
	{ Direction::DownRight, Direction::DownLeft, Direction::UpLeft, Direction::UpRight },
	{ Direction::DownLeft, Direction::UpLeft, Direction::UpRight, Direction::DownRight },
	{ Direction::ZUp, Direction::ZUp, Direction::ZUp, Direction::ZUp },
	{ Direction::ZDown, Direction::ZDown, Direction::ZDown, Direction::ZDown },
	{ Direction::_Count, Direction::_Count, Direction::_Count, Direction::_Count },
};

// Structure that holds the effect of applying a rotation to each direction value (in 2D only)
const int RotatedBSDirections[Direction::_Count + 1][4] = {
	{ DirectionBS::Left_BS, DirectionBS::Up_BS, DirectionBS::Right_BS, DirectionBS::Down_BS },
	{ DirectionBS::Up_BS, DirectionBS::Right_BS, DirectionBS::Down_BS, DirectionBS::Left_BS },
	{ DirectionBS::Right_BS, DirectionBS::Down_BS, DirectionBS::Left_BS, DirectionBS::Up_BS },
	{ DirectionBS::Down_BS, DirectionBS::Left_BS, DirectionBS::Up_BS, DirectionBS::Right_BS },
	{ DirectionBS::UpLeft_BS, DirectionBS::UpRight_BS, DirectionBS::DownRight_BS, DirectionBS::DownLeft_BS },
	{ DirectionBS::UpRight_BS, DirectionBS::DownRight_BS, DirectionBS::DownLeft_BS, DirectionBS::UpLeft_BS },
	{ DirectionBS::DownRight_BS, DirectionBS::DownLeft_BS, DirectionBS::UpLeft_BS, DirectionBS::UpRight_BS },
	{ DirectionBS::DownLeft_BS, DirectionBS::UpLeft_BS, DirectionBS::UpRight_BS, DirectionBS::DownRight_BS },
	{ DirectionBS::ZUp_BS, DirectionBS::ZUp_BS, DirectionBS::ZUp_BS, DirectionBS::ZUp_BS },
	{ DirectionBS::ZDown_BS, DirectionBS::ZDown_BS, DirectionBS::ZDown_BS, DirectionBS::ZDown_BS },
	{ DirectionBS::None_BS, DirectionBS::None_BS, DirectionBS::None_BS, DirectionBS::None_BS },
};

// Enumeration of possible 90-degree rotations
enum Rotation90Degree { Rotate0 = 0, Rotate90 = 1, Rotate180 = 2, Rotate270 = 3 };
extern const Rotation90Degree ROT90_VALUES[4];

// Parses the string representation of a 90-degree rotation value
Rotation90Degree TranslateRotation90Degree(std::string rotvalue);

// Returns the result of rotating one 90-degree value by another
Rotation90Degree Compose90DegreeRotations(Rotation90Degree current_rotation, Rotation90Degree rotate_by);

// Returns the 90-degree rotation required to transform between two rotation values
Rotation90Degree Rotation90BetweenValues(Rotation90Degree start_rotation, Rotation90Degree end_rotation);

// Indicates whether the specified 90-degree rotation value is valid
CMPINLINE bool Rotation90DegreeIsValid(Rotation90Degree rotation) { return (int)rotation >= (int)Rotate0 && (int)rotation <= (int)Rotate270; }


// Method to return the effect of a rotation on a 2D direction, using the above predefined structure
CMPINLINE Direction GetRotatedDirection(Direction direction, Rotation90Degree rotation)
{
	if ((int)direction >= 0 && (int)direction < Direction::_Count && (int)rotation >= 0 && (int)rotation < 4)
		return (Direction)RotatedDirections[direction][rotation];
	else
		return direction;
}

// Method to return the effect of a rotation on a 2D direction, using the above predefined structure
CMPINLINE DirectionBS GetRotatedBSDirection(DirectionBS direction, Rotation90Degree rotation)
{
	if (rotation < 0 || rotation >= 4) return direction;
	for (int i = 0; i < Direction::_Count; ++i)
	{
		if (direction == RotatedBSDirections[i][0])	return (DirectionBS)RotatedBSDirections[0][(int)rotation];
	}
	return direction;
}

// Returns a value indicating whether this is a diagonal direction or not
CMPINLINE bool IsDiagonalDirection(Direction direction) {
	return (direction == Direction::UpLeft || direction == Direction::UpRight ||
		direction == Direction::DownLeft || direction == Direction::DownRight);
}

// Methods to manipulate direction values and translate to/from their string representations
std::string DirectionToString(Direction direction);
Direction DirectionFromString(std::string direction);
Direction GetOppositeDirection(Direction dir);
DirectionBS GetOppositeDirectionBS(DirectionBS dir);

