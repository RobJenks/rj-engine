#include "Utility.h"
#include "Faction.h"

#include "FactionManagerObject.h"


// Default constructor
FactionManagerObject::FactionManagerObject(void)
{
	// Initialise fields to default values
	m_initialised = false;
	m_factioncount = 0;
	m_dispmatrix = NULL;

	// A default NULL faction will exist in slot 0 upon startup, for e.g. objects that have no affiliation
	Faction *nullfac = new Faction();
	nullfac->SetCode("NULL_FACTION");
	nullfac->SetName("No Affiliation");
	AddFaction(nullfac);
}

// Adds a new faction to the game.  Returns the ID assigned to the new faction.  Faction will begin neutral to all others
Faction::F_ID FactionManagerObject::AddFaction(Faction *faction)
{
	// Make sure this is a valid faction
	if (!faction) return Faction::NullFaction;

	// The ID of this new faction will be assigned sequentially based on the number of factions; assign it to the faction now
	Faction::F_ID id = (Faction::F_ID)m_factioncount;
	faction->SetFactionID(id);

	// Add the new faction and update the faction count
	m_factions.push_back(faction);
	m_factioncount = m_factions.size();

	// Also update the reverse maps back to faction IDs
	m_factioncodemap[faction->GetCode()] = id;

	// If we have already initialised the faction manager data (i.e. we are no longer in the pre-game initialisation)
	// then we need to perform a delta update of the various internal matrices
	if (m_initialised)
	{
		ExpandDispositionMatrixForNewFaction();
	}

	// Return the ID assigned to the new faction
	return id;
}

// Returns a faction ID based upon the faction's unique string code.  Returns the null 
// faction (0) if the code cannot be found
Faction::F_ID FactionManagerObject::GetFaction(const std::string & code)
{
	if (m_factioncodemap.count(code) == 0)			return Faction::NullFaction;
	else											return m_factioncodemap[code];
}

// Changes the disposition of one faction towards another.  Triggered by the faction objects as they analyse all contributing 
// factors and then notify the faction manager if a major change in their relations has now occured
void FactionManagerObject::FactionDispositionChanged(Faction::F_ID FactionA, Faction::F_ID FactionB, Faction::FactionDisposition disposition)
{
	// Parameter check
	if (!IsValidFactionID(FactionA) || !IsValidFactionID(FactionB)) return;

	// Update the disposition matrix accordingly
	m_dispmatrix[FactionA][FactionB] = disposition;
}

// Initialises the faction manager once all faction data has been loaded
void FactionManagerObject::Initialise(void)
{
	// Initialise the faction disposition matrix
	InitialiseDispositionMatrix();
}

// Initialises the faction disposition matrix, holding the disposition of every faction to every other.  Should be called
// once all factions have been loaded at the start of the game. 
void FactionManagerObject::InitialiseDispositionMatrix(void)
{
	// Make sure we have a valid number of factions
	if (m_factioncount < 1) { m_dispmatrix = NULL; return; }

	// We want to allocate an nxn matrix for n factions
	m_dispmatrix = (Faction::FactionDisposition**)malloc(sizeof(Faction::FactionDisposition*) * m_factioncount);
	for (int i = 0; i < m_factioncount; ++i)
	{
		// Allocate memory for all factions
		m_dispmatrix[i] = (Faction::FactionDisposition*)malloc(sizeof(Faction::FactionDisposition) * m_factioncount);

		// Initialise all relations to neutral
		for (int j = 0; j < m_factioncount; ++j) m_dispmatrix[i][j] = Faction::FactionDisposition::Neutral;
	}

	// Overwrite the diagonal (i == j) relations to friendly, since this is the faction's disposition to itself
	for (int i = 0; i < m_factioncount; ++i) m_dispmatrix[i][i] = Faction::FactionDisposition::Friendly;
}

// Expands the disposition matrix to account for a new faction being added.  Unlikely to be required
// but can be used in case e.g. a new faction forms from an existing one during a game
void FactionManagerObject::ExpandDispositionMatrixForNewFaction(void)
{
	// We are expanding the number of faction by one; append a new row & column to the matrix
	int oldcount = (m_factioncount - 1);
	for (int i = 0; i < oldcount; ++i)
	{
		// Reallocate memory to add a new column on this row, and set it to neutral by default
		m_dispmatrix[i] = (Faction::FactionDisposition*)realloc(m_dispmatrix, sizeof(Faction::FactionDisposition) * m_factioncount);
		m_dispmatrix[i][m_factioncount - 1] = Faction::FactionDisposition::Neutral;
	}

	// We want to add an entirely new row on the end for the new faction
	m_dispmatrix = (Faction::FactionDisposition**)realloc(m_dispmatrix, sizeof(Faction::FactionDisposition*) * m_factioncount);
	m_dispmatrix[m_factioncount - 1] = (Faction::FactionDisposition*)malloc(sizeof(Faction::FactionDisposition) * m_factioncount);

	// Default the new row to neutral, except the last element which is on the diagonal & friendly
	for (int i = 0; i < oldcount; ++i) m_dispmatrix[oldcount][i] = Faction::FactionDisposition::Neutral;
	m_dispmatrix[oldcount][oldcount] = Faction::FactionDisposition::Friendly;
}

// Deallocates the faction disposition matrix, which holds the disposition of every faction to every other
void FactionManagerObject::ShutdownDispositionMatrix(void)
{
	if (m_dispmatrix)
	{
		for (int i = 0; i < m_factioncount; ++i)
		{
			SafeFree(m_dispmatrix[i]);
		}

		m_dispmatrix = NULL;
	}
}

// Default destructor
FactionManagerObject::~FactionManagerObject(void)
{
	// Release all allocated memory; first deallocate the dispositon matrix
	ShutdownDispositionMatrix();
}
