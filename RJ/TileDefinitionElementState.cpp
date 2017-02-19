#include "TileDefinitionElementState.h"

// Internal method which returns a debug string output representing the set of element states, given 
// the specified tile orientation(s) to be returned. Shown as a 2D x/y representation, with z 
// values represented within an array at each element
std::string TileDefinitionElementState::DebugStringOutput_Internal(Rotation90Degree start_rotation, Rotation90Degree end_rotation)
{
	if (start_rotation > end_rotation) return "";
	concat result = concat("");

	std::string blank = "";
	for (int i = 0; i < m_size.z; ++i) { blank = concat("    ")((i != 0 ? " " : "")).str(); }

	INTVECTOR3 max_size = IntVector3Max(m_size, m_size_transposed);
	for (int y = (max_size.y - 1); y >= 0; --y)			// Note: decremeny y from max>min, since elements are arranged with the origin at bottom-bottom-left 
	{
		for (int rot = (int)start_rotation; rot <= (int)end_rotation; ++rot)
		{
			INTVECTOR3 size = GetSize((Rotation90Degree)rot);
			bool render = (y < size.y);
			for (int x = 0; x < size.x; ++x)
			{
				if (render) result("[");
				for (int z = 0; z < size.z; ++z)
				{
					if (!render) { result(blank);  continue; }
					result((z != 0 ? "," : ""))(GetDefaultElementState(INTVECTOR3(x, y, z), (Rotation90Degree)rot).Properties);
				}
				if (render) result("] ");
			}
			result("    ");
		}
		result("\n");
	}
	return result.str();
}
