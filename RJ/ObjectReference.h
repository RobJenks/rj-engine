#pragma once

#ifndef __ObjectReferenceH__
#define __ObjectReferenceH__

#include <unordered_map>
#include "GameVarsExtern.h"
#include "GameObjects.h"

struct ObjectReference
{
public:

	// Default constructor; establishes a link to the null object register entry
	CMPINLINE ObjectReference(void)
	{

	}

	// Constructor; establishes a reference to the object with the specified ID
	CMPINLINE ObjectReference(Game::ID_TYPE id)
	{
		// Get a reference to the relevant object register entry
		Game::ObjectRegister::iterator entry = Game::Objects.find(id);
		if (entry == Game::Objects.end())
		{
			m_entry = Game::NullObjectReference;		// Link to the null object reference, which simply wraps NULL
		}
		else
		{
			m_entry = &(entry->second);					// Store a reference to the relevant entry
			m_entry->ReferenceAdded();					// Record this reference to the entry, to maintain an accurate reference count
		}
	}

protected:

	// Internal reference to the object register entry for this object
	Game::ObjectRegisterEntry *							m_entry;

};


#endif






