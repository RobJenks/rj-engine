#include "ElementStateDefinition.h"

// Default constructor	
ElementStateDefinition::ElementStateDefinition()
{
	m_size = m_size_transposed = NULL_INTVECTOR3;
	m_count = 0;
	m_filter = ElementStateFilters::ALL_PROPERTIES;
	for (int i = 0; i < 4; ++i) m_state[i] = std::vector<ElementState>();
}

// Constructor with specific state type specified
ElementStateDefinition::ElementStateDefinition(ElementStateFilters::ElementStateFilter state_filter)
{
	m_size = m_size_transposed = NULL_INTVECTOR3;
	m_count = 0;
	m_filter = state_filter;
	for (int i = 0; i < 4; ++i) m_state[i] = std::vector<ElementState>();
}

// Copy constructor
ElementStateDefinition::ElementStateDefinition(const ElementStateDefinition & other)
{
	// Initialise to the same dimensions as the source
	Initialise(other.GetSize(Rotation90Degree::Rotate0));
	
	// Copy the default element state 
	ApplyDefaultElementState(other.GetDefaultElementState());

	// Copy each state vector in turn
	for (int rot = (int)Rotation90Degree::Rotate0; rot <= (int)Rotation90Degree::Rotate270; ++rot)
	{
		m_state[rot] = other.GetStateVector((Rotation90Degree)rot);
	}
	
	// Copy (and apply) the state filter from the source object
	ChangeStateFilter(other.GetCurrentStateFilter());	
}

// Initialise the element state for a specific area size
void ElementStateDefinition::Initialise(const INTVECTOR3 & elementsize)
{
	m_size = elementsize;
	m_size_transposed = INTVECTOR3(m_size.y, m_size.x, m_size.z);
	m_count = (m_size.x * m_size.y * m_size.z);
	for (int i = 0; i < 4; ++i)
	{
		m_state[i] = std::vector<ElementState>(m_count);
	}
}

// Store a default element state, and apply it to all elements in the area
void ElementStateDefinition::ApplyDefaultElementState(ElementStateDefinition::ElementState default_state)
{
	// Default state is subject to the same state filter as all per-element states
	m_defaultstate = default_state;
	m_defaultstate.Properties &= m_filter;

	for (int rot = (int)Rotation90Degree::Rotate0; rot <= (int)Rotation90Degree::Rotate270; ++rot)
	{
		for (int el = 0; el < m_count; ++el) m_state[rot][el] = default_state;
	}
}

// Retuns the default state of an element at the specified location, given the specified tile orientation
ElementStateDefinition::ElementState ElementStateDefinition::GetElementState(const INTVECTOR3 & location, Rotation90Degree tile_rotation) const
{
	int index = ELEMENT_INDEX_EX(location.x, location.y, location.z, GetSize(tile_rotation));
	if (index < 0 || index >= m_count || !Rotation90DegreeIsValid(tile_rotation)) return ElementState();

	return m_state[(int)tile_rotation][index];
}

// Set the default state of an element within the tile.  Properties are replicated to each copy of 
// the ElementState set (once per orientation)
void ElementStateDefinition::SetElementState(ElementStateDefinition::ElementState element_state, const INTVECTOR3 & location, Rotation90Degree rotation)
{
	// Apply our element state filter first, if applicable
	element_state.Properties &= m_filter;

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

// Change the state filter in use by this definition.  Will re-evaluate all current state data to ensure 
// it complies with the new filter
void ElementStateDefinition::ChangeStateFilter(ElementStateFilters::ElementStateFilter filter)
{
	// Store the new filter
	m_filter = filter;

	// Re-evaluate all existing state data to ensure it complies with the new filter
	ApplyStateFilter();
}

// Applies the state filter to all stored element state data
void ElementStateDefinition::ApplyStateFilter(ElementStateFilters::ElementStateFilter filter)
{
	// Re-evaluate all existing state data to ensure it complies with the specified filter
	for (int r = (int)Rotation90Degree::Rotate0; r <= (int)Rotation90Degree::Rotate270; ++r)
	{
		for (int i = 0; i < m_count; ++i)
		{
			// Bitwise-AND every property with the new filter to remove any non-compliant values
			m_state[r][i].Properties &= filter;
		}
	}

	// Also apply the filter to our default element state
	m_defaultstate.Properties &= filter;
}


// Internal method which returns a debug string output representing the set of element states, given 
// the specified tile orientation(s) to be returned. Shown as a 2D x/y representation, with z 
// values represented within an array at each element
std::string ElementStateDefinition::DebugStringOutput_Internal(Rotation90Degree start_rotation, Rotation90Degree end_rotation)
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
