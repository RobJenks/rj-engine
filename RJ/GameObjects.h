#pragma once

#ifndef __GameObjectsH__
#define __GameObjectsH__

#include <vector>
#include <unordered_map>
#include "GameVarsExtern.h"
class iObject;

// This file contains no objects with special alignment requirements
namespace Game
{
	// Structure of one entry in the global object collection
	struct ObjectRegisterEntry
	{
		iObject *	Object;
		bool		Active;
		ObjectRegisterEntry(void) : Object(NULL), Active(true) { }
		ObjectRegisterEntry(iObject *object) : Object(object), Active(true) { }
	};

	// Type definitions for game object collections
	typedef std::unordered_map<Game::ID_TYPE, ObjectRegisterEntry>	ObjectRegister;					// Primary register maintaining the primary register entry
	typedef std::unordered_map<std::string, ObjectRegisterEntry>	ObjectRegisterByInstanceCode;	// Secondary register; allows lookup of objects by instance code

	// Primary game object collections
	extern Game::ObjectRegister					Objects;
	extern Game::ObjectRegisterByInstanceCode	ObjectsByCode;

	// Short-term lists of objects waiting to be registered or unregistered with the global collection; actioned each frame
	extern std::vector<iObject*>				RegisterList;		// Store the object so it can be inserted into the global game objects list
	extern std::vector<Game::ID_TYPE>			UnregisterList;		// Stored as ID in case object is being unregistered while it is being shut down

	// Register an object with the global collection
	CMPINLINE void								RegisterObject(iObject *obj)
	{
		// Add to the registration list for addition to the global collection in the next cycle
		Game::RegisterList.push_back(obj);
	}

	// Unregister an object from the global collection
	void										UnregisterObject(iObject *obj);
	
	// Method which processes all pending register/unregister requests to update the global collection.  Executed once per frame
	void										UpdateGlobalObjectCollection(void);


	// Test whether an object exists with the specified ID
	CMPINLINE bool								ObjectExists(Game::ID_TYPE id)
	{
		Game::ObjectRegister::iterator it = Objects.find(id);
		return (it != Objects.end() && it->second.Active);
	}

	// Test whether an object exists with the specified instance code
	CMPINLINE bool								ObjectExists(const std::string instance_code)
	{
		Game::ObjectRegisterByInstanceCode::iterator it = ObjectsByCode.find(instance_code);
		return (it != ObjectsByCode.end() && it->second.Active);
	}

	// Return the object with the specified ID, or NULL if no object exists with that ID
	CMPINLINE iObject *							GetObjectByID(Game::ID_TYPE id)
	{
		// Attempt to find the entry with this ID; if none exists, return NULL
		Game::ObjectRegister::iterator it = Objects.find(id);
		if (it == Objects.end()) return NULL;

		// An entry exists.  Make sure it is active, and if it is then return a pointer to the object
		Game::ObjectRegisterEntry & entry = it->second;
		return (entry.Active ? entry.Object : NULL);
	}

	// Return the object with the specified instance code, or NULL if no object exists with that code
	CMPINLINE iObject *							GetObjectByInstanceCode(const std::string & instance_code)
	{
		// Attempt to find the entry with this code; if none exists, return NULL
		Game::ObjectRegisterByInstanceCode::iterator it = ObjectsByCode.find(instance_code);
		if (it == ObjectsByCode.end()) return NULL;

		// An entry exists.  Make sure it is active, and if it is then return a pointer to the object
		Game::ObjectRegisterEntry & entry = it->second;
		return (entry.Active ? entry.Object : NULL);
	}
};



#endif









