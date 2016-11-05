#include "Utility.h"
#include "Faction.h"

// Default constructor
Faction::Faction(void)
{
	// Set all fields to default values
	m_id = (F_ID)-1;
	m_code = m_name = "";
}

// Default destructor
Faction::~Faction(void)
{

}

// Custom debug string function
std::string	Faction::DebugString(void) const
{
	return concat("Faction [ID=")(m_id)(", Code=\"")(m_code)("\", Name=\"")(m_name)("\"").str();
}


// Translates a faction disposition to/from its string representation
std::string Faction::TranslateFactionDispositionToString(Faction::FactionDisposition disp)
{
	switch (disp)
	{
		case Faction::FactionDisposition::Hostile:		return "Hostile";
		case Faction::FactionDisposition::Friendly:		return "Friendly";
		default:										return "Neutral";
	}
}

// Translates a faction disposition to/from its string representation
Faction::FactionDisposition	Faction::TranslateFactionDispositionFromString(std::string disp)
{
	StrLowerC(disp);
	if (disp == "Hostile")								return Faction::FactionDisposition::Hostile;
	else if (disp == "Friendly")						return Faction::FactionDisposition::Friendly;
	else												return Faction::FactionDisposition::Neutral;
}