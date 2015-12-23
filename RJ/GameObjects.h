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
		Game::ID_TYPE			ID;				// ID of the object wrapped by this register entry
		iObject *				Object;			// Pointer to the object itself
		bool					Active;			// Flag indicating whether the object still exists
		int						RefCount;		// Number of external references to this entry.  Entry will only be deleted once
												// it is inactive, and once there are no further external references to the object
		// Constructors
		ObjectRegisterEntry(void) : ID(0), Object(NULL), Active(true), RefCount(0) { }
		ObjectRegisterEntry(iObject *object) : ID(object ? object->GetID() : 0), Object(object), Active(true), RefCount(0) { }

		// Record a new reference to this entry
		CMPINLINE void			ReferenceAdded(void)		{ ++RefCount; }

		// Record the removal of a reference to this entry
		CMPINLINE void			ReferenceRemoved(void)		{ if (--RefCount <= 0) Game::RemoveObjectRegisterEntry(ID); }
	};

	// Type definitions for game object collections
	typedef std::unordered_map<Game::ID_TYPE, ObjectRegisterEntry>	ObjectRegister;					// Primary register maintaining the primary register entry
	typedef std::unordered_map<std::string, ObjectRegisterEntry>	ObjectRegisterByInstanceCode;	// Secondary register; allows lookup of objects by instance code

	// Primary game object collections
	extern Game::ObjectRegister					Objects;
	extern Game::ObjectRegisterByInstanceCode	ObjectsByCode;

	// Short-term lists of objects waiting to be registered or unregistered with the global collection; actioned each frame
	extern std::vector<iObject*>				RegisterList;			// Store the object so it can be inserted into the global game objects list
	extern std::vector<Game::ID_TYPE>			UnregisterList;			// Stored as ID in case object is being unregistered while it is being shut down
	extern std::vector<Game::ID_TYPE>			RegisterRemovalList;	// Objects where the entire entry can now be removed from the register (i.e. inactive & refcount == 0)

	// Register an object with the global collection
	CMPINLINE void								RegisterObject(iObject *obj)
	{
		// Add to the registration list for addition to the global collection in the next cycle
		Game::RegisterList.push_back(obj);
	}

	// Unregister an object from the global collection
	void										UnregisterObject(iObject *obj);

	// Remove the entire entry from the object register; performed when active == false and refcount == 0
	CMPINLINE void								RemoveObjectRegisterEntry(Game::ID_TYPE id)
	{
		// Add to the removal vector which is processed at the end of each frame
		Game::RegisterRemovalList.push_back(id);
	}
	
	// Method which processes all pending register/unregister requests to update the global collection.  Executed once per frame
	void										UpdateGlobalObjectCollection(void);

	// Initialises all object register data on application startup
	void										InitialiseObjectRegisters(void);

	// Deallocates all object register data on application shutdown
	void										ShutdownObjectRegisters(void);
	
	// Returns the null object register entry
	static										ObjectRegisterEntry *NullObjectReference;

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

	// Notifies the central object collection that the code of the specified object has changed.  Ensures that
	// correct references are maintained in the central object collection
	void										NotifyChangeOfObjectInstanceCode(iObject *object, const std::string & old_code);
};



#endif









