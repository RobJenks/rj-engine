#include "Utility.h"
#include "ErrorCodes.h"
#include "LogManager.h"
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
	m_factioncount = (int)m_factions.size();

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
Faction::F_ID FactionManagerObject::GetFactionIDByCode(const std::string & code)
{
	if (m_factioncodemap.count(code) == 0)			return Faction::NullFaction;
	else											return m_factioncodemap[code];
}

// Returns a faction ID based upon the faction's name.  Returns the null faction
// (0) if the code cannot be found
Faction::F_ID FactionManagerObject::GetFactionIDByName(const std::string & name)
{
	std::vector<Faction*>::const_iterator it = std::find_if(m_factions.begin(), m_factions.end(),
		[&name](const Faction *faction) { return (faction && faction->GetName() == name); });

	return (it == m_factions.end() ? 0 : (*it)->GetFactionID());
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
Result FactionManagerObject::Initialise(void)
{
	Result result, overallresult = ErrorCodes::NoError;

	// Initialise the faction disposition matrix
	result = InitialiseDispositionMatrix();
	if (result != ErrorCodes::NoError)
	{
		overallresult = result;
		Game::Log << LOG_ERROR << "Error initialising faction disposition matrix\n";
	}

	// Return the overall result
	return overallresult;
}

// Initialises the faction disposition matrix, holding the disposition of every faction to every other.  Should be called
// once all factions have been loaded at the start of the game. 
Result FactionManagerObject::InitialiseDispositionMatrix(void)
{
	// Make sure we have a valid number of factions
	if (m_factioncount < 1) { m_dispmatrix = NULL; return ErrorCodes::NoFactionsToBuildFactionDispositionMatrix; }

	// We want to allocate an nxn matrix for n factions
	m_dispmatrix = (Faction::FactionDisposition**)malloc(sizeof(Faction::FactionDisposition*) * m_factioncount);
	if (!m_dispmatrix) return ErrorCodes::CouldNotAllocateMemoryForFactionDispMatrix;
	for (int i = 0; i < m_factioncount; ++i)
	{
		// Allocate memory for all factions
		m_dispmatrix[i] = (Faction::FactionDisposition*)malloc(sizeof(Faction::FactionDisposition) * m_factioncount);

		// Initialise all relations to neutral
		for (int j = 0; j < m_factioncount; ++j) m_dispmatrix[i][j] = Faction::FactionDisposition::Neutral;
	}

	// Overwrite the diagonal (i == j) relations to friendly, since this is the faction's disposition to itself
	for (int i = 0; i < m_factioncount; ++i) m_dispmatrix[i][i] = Faction::FactionDisposition::Friendly;

	// Return success
	return ErrorCodes::NoError;
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


// Virtual inherited method to accept a command from the console
bool FactionManagerObject::ProcessConsoleCommand(GameConsoleCommand & command)
{
	if (command.InputCommand == "GetFaction")
	{
		if (!command.HasParameter(0)) {
			command.SetOutput(GameConsoleCommand::CommandResult::Failure,
				ErrorCodes::RequiredCommandParametersNotProvided, "Faction name / code not provided");
		}
		else {
			// First attempt to get the function based on faction ID, then based on code and name
			bool found = true;
			Faction::F_ID id = command.ParameterAsInt(0);
			if (id == Faction::NullFaction && command.Parameter(0) != "0") {			// 0 signifies no conversion to int, unless we actually specified "0"
				id = GetFactionIDByCode(command.Parameter(0));
				if (id == Faction::NullFaction) id = GetFactionIDByName(command.Parameter(0));
				if (id == Faction::NullFaction) found = false;
			}

			if (found && IsValidFactionID(id) && GetFaction(id) != NULL) {
				command.SetSuccessOutput(GetFaction(id)->DebugString());
			}
			else {
				command.SetOutput(GameConsoleCommand::CommandResult::Failure,
					ErrorCodes::CommandParameterIsNotValid, "No faction exists with the specified name / code");
			}
		}
		return true;
	}
	else if (command.InputCommand == "GetFactionDisposition")
	{
		if (command.ParameterCount() < 2) {
			command.SetOutput(GameConsoleCommand::CommandResult::Failure,
				ErrorCodes::RequiredCommandParametersNotProvided, "Expected format: GetFactionDisposition [FactionID] [FactionID]");
		}
		else {
			Faction::F_ID f0 = command.ParameterAsInt(0);
			Faction::F_ID f1 = command.ParameterAsInt(1);
			if ( !IsValidFactionID(f0) || !IsValidFactionID(f1) || GetFaction(f0) == NULL || GetFaction(f1) == NULL || 
				(f0 == Faction::NullFaction && command.Parameter(0) != "0") ||
				(f1 == Faction::NullFaction && command.Parameter(1) != "0")) 
			{
				command.SetOutput(GameConsoleCommand::CommandResult::Failure,
					ErrorCodes::CommandParameterIsNotValid, "Valid faction IDs not provided");
			}
			else {
				Faction::FactionDisposition disp = GetDisposition(f0, f1);
				command.SetSuccessOutput(concat("Faction \"")(GetFaction(f0)->GetName())("\" (")(f0)(") is ")
					(StrLower(Faction::TranslateFactionDispositionToString(disp)))(" towards faction \"")
					(GetFaction(f1)->GetName())("\" (")(f1)(")").str());
			}
		}
		return true;
	}
	else if (command.InputCommand == "GetFactionRelations")
	{
		if (!command.HasParameter(0))
			command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::RequiredCommandParametersNotProvided, "No faction ID provided");
		else
		{
			Faction::F_ID id = command.ParameterAsInt(0);
			if (!IsValidFactionID(id) || GetFaction(id) == NULL || (id == Faction::NullFaction && command.Parameter(0) != "0"))
				command.SetOutput(GameConsoleCommand::CommandResult::Failure, ErrorCodes::CommandParameterIsNotValid, "Faction ID is not valid");
			else
			{
				std::string friends = NullString, enemies = NullString;
				for (int i = 0; i < m_factioncount; ++i)
				{
					if (i == id) continue;	// Ignore the self-relation

					Faction::FactionDisposition disp = GetDisposition(id, i);
					if (disp == Faction::FactionDisposition::Friendly)
						friends = concat((friends == NullString ? "{ \"" : ", \""))(GetFaction(i)->GetName())("\" (")(id)(")").str();
					else if (disp == Faction::FactionDisposition::Hostile)
						enemies = concat((enemies == NullString ? "{ \"" : ", \""))(GetFaction(i)->GetName())("\" (")(id)(")").str();
				}
				friends += (friends == NullString ? "no-one" : "}");
				enemies += (enemies == NullString ? "no-one" : "}");
				command.SetSuccessOutput(concat("Faction \"")(GetFaction(id)->GetName())("\" (")(id)(") is friendly towards ")
					(friends)(" and hostile towards ")(enemies).str());
			}
		}
		return true;
	}

	// We did not recognise the command
	return false;
}