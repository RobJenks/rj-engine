#include "TileDefinitionElementState.h"

// Store a default element state, and apply it to all elements in the area
void TileDefinitionElementState::ApplyDefaultElementState(TileDefinitionElementState::ElementState default_state)
{
	m_defaultstate = default_state;
	for (int i = 0; i < 4; ++i)
	{
		for (int el = 0; el < m_count; ++el) m_state[i][el] = default_state;
	}
}

// Retuns the default state of an element at the specified location, given the specified tile orientation
TileDefinitionElementState::ElementState TileDefinitionElementState::GetElementState(const INTVECTOR3 & location, Rotation90Degree tile_rotation)
{
	int index = ELEMENT_INDEX_EX(location.x, location.y, location.z, GetSize(tile_rotation));
	if (index < 0 || index >= m_count || !Rotation90DegreeIsValid(tile_rotation)) return ElementState();

	return m_state[(int)tile_rotation][index];
}

// Set the default state of an element within the tile.  Properties are replicated to each copy of 
// the ElementState set (once per orientation)
void TileDefinitionElementState::SetElementState(TileDefinitionElementState::ElementState element_state, const INTVECTOR3 & location, Rotation90Degree rotation)
{
	for (int rot = (int)Rotation90Degree::Rotate0; rot <= (int)Rotation90Degree::Rotate270; ++rot)
	{
		Rotation90Degree delta = Rotation90BetweenValues(rotation, (Rotation90Degree)rot);
		INTVECTOR3 loc = GetRotatedElementLocation(location, delta, GetSize((Rotation90Degree)rot));
		int index = ELEMENT_INDEX_EX(loc.x, loc.y, loc.z, GetSize((Rotation90Degree)rot));
		if (index >= 0 && index < m_count)
		{
			m_state[rot][index] = element_state;
		}
	}
}



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
	for (int y = (max_size.y - 1); y >= 0; --y)			// Note: decrement y from max>min, since elements are arranged with the origin at bottom-bottom-left 
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
					result((z != 0 ? "," : ""))(GetElementState(INTVECTOR3(x, y, z), (Rotation90Degree)rot).Properties);
				}
				if (render) result("] ");
			}
			result("    ");
		}
		result("\n");
	}
	return result.str();
}
