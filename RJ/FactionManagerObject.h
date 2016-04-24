#pragma once

#ifndef __FactionManagerObjectH__
#define __FactionManagerObjectH__

#include <vector>
#include "CompilerSettings.h"
#include "Faction.h"


// This class has no special alignment requirements
class FactionManagerObject
{
public:

	// Default constructor
	FactionManagerObject(void);

	// Returns a reference to the faction with this ID
	CMPINLINE Faction *						GetFaction(Faction::F_ID ID)					{ return m_factions[ID]; }
	Faction::F_ID							GetFaction(const std::string & code);

	// Tests whether the supplied faction ID is valid
	CMPINLINE bool							IsValidFactionID(Faction::F_ID ID) const 		{ return (ID >= 0 && ID < m_factioncount); }

	// Adds a new faction to the game.  Returns the ID assigned to the new faction.  Faction will begin neutral to all others
	Faction::F_ID							AddFaction(Faction *faction);

	// Changes the disposition of one faction towards another.  Triggered by the faction objects as they analyse all contributing 
	// factors and then notify the faction manager if a major change in their relations has now occured
	void									FactionDispositionChanged(Faction::F_ID FactionA, Faction::F_ID FactionB, Faction::FactionDisposition disposition);

	// Initialises the faction manager once all faction data has been loaded
	Result									Initialise(void);

	// Returns the disposition of Faction A towards Faction B, i.e. whether Faction A likes Faction B
	CMPINLINE Faction::FactionDisposition	GetDisposition(Faction::F_ID FactionA, Faction::F_ID FactionB)	{ return m_dispmatrix[FactionA][FactionB]; }

	// Default destructor
	~FactionManagerObject(void);

protected:

	// Flag indicating whether the faction manager has been initialised
	bool									m_initialised;

	// The faction manager contains the master collection of each faction in the game
	std::vector<Faction*>					m_factions;
	int										m_factioncount;

	// Also maintain reverse maps from faction names or object to the faction ID, for lookup efficiency at runtime
	std::unordered_map<std::string, Faction::F_ID>	m_factioncodemap;

	// Maintain an nxn matrix for all n factions, recording the disposition of each faction towards each other
	Faction::FactionDisposition **			m_dispmatrix;

	// Initialises the faction disposition matrix, holding the disposition of every faction to every other.  Should be called
	// once all factions have been loaded at the start of the game. 
	Result									InitialiseDispositionMatrix(void);

	// Expands the disposition matrix to account for a new faction being added.  Unlikely to be required
	// but can be used in case e.g. a new faction forms from an existing one during a game
	void									ExpandDispositionMatrixForNewFaction(void);

	// Deallocates the faction disposition matrix, which holds the disposition of every faction to every other
	void									ShutdownDispositionMatrix(void);

};


#endif