#pragma once

#ifndef __GameObjectsH__
#define __GameObjectsH__

#include <vector>
#include <unordered_map>
#include "GameVarsExtern.h"
#include "Logging.h"
#include "iObject.h"

// Flag which determines whether object register interactions will be logged in debug mode
// #define	DEBUG_LOG_OBJECT_REGISTER_OPERATIONS

// Logs an object register interaction to the debug output, in debug mode only
#	if defined(_DEBUG) && defined(DEBUG_LOG_OBJECT_REGISTER_OPERATIONS)
#		define OBJ_REGISTER_LOG(str) Game::Log << LOG_DEBUG << str << "\n"
#	else 
#		define OBJ_REGISTER_LOG(str)
#	endif



// This file contains no objects with special alignment requirements
namespace Game
{
	// Forward declarations
	CMPINLINE void RemoveObjectRegisterEntry(Game::ID_TYPE id);

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
		
		// Constructor accepting all parameters; used only for internal operations
		ObjectRegisterEntry(Game::ID_TYPE id, iObject *object, bool active, int refcount) : ID(id), Object(object), Active(active), RefCount(refcount) { }

		// Record a new reference to this entry
		CMPINLINE void			ReferenceAdded(void)		
		{ 
			++RefCount; 
			//OBJ_REGISTER_LOG(concat("New reference to object ")(ID)(" (\"")(Object ? Object->GetInstanceCode() : "<null>")("\"), refcount is now ")(RefCount)(", Active = ")(Active ? "true" : "false").str());
		}

		// Record the removal of a reference to this entry
		CMPINLINE void			ReferenceRemoved(void)		
		{ 
			if (--RefCount <= 0 && !Active)		// NOTE: Order is important to avoid --Refcount being skipped via short-circuiting
			{
				//OBJ_REGISTER_LOG(concat("Removing reference to object ")(ID)(" (\"")(Object ? Object->GetInstanceCode() : "<null>")("\"), refcount is now ")(RefCount)(", Active = ")(Active ? "true" : "false").str());
				//OBJ_REGISTER_LOG(concat("REMOVING OBJECT REGISTER ENTRY FOR OBJECT ")(ID).str());
				Game::RemoveObjectRegisterEntry(ID);
			}
			else
			{
				//OBJ_REGISTER_LOG(concat("Removing reference to object ")(ID)(" (\"")(Object ? Object->GetInstanceCode() : "<null>")("\"), refcount is now ")(RefCount)(", Active = ")(Active ? "true" : "false").str());
			}
		}
	};

	// Type definitions for primary object register collection
	typedef std::unordered_map<Game::ID_TYPE, ObjectRegisterEntry>	ObjectRegister;					

	// Type definition for secondary registers; these hold pointers to the primary collection entries, to ensure one single record of each object
	typedef std::unordered_map<std::string, ObjectRegisterEntry*>	ObjectRegisterByInstanceCode;	// Secondary register; allows lookup of objects by instance code

	// Primary game object collections
	extern Game::ObjectRegister					Objects;
	extern Game::ObjectRegisterByInstanceCode	ObjectsByCode;

	// Collection of all visible objects, recreated each frame
	extern std::vector<iObject*>				VisibleObjects;

	// Short-term lists of objects waiting to be registered or unregistered with the global collection; actioned each frame
	extern std::vector<iObject*>				RegisterList;			// Store the object so it can be inserted into the global game objects list
	extern std::vector<Game::ID_TYPE>			UnregisterList;			// Stored as ID in case object is being unregistered while it is being shut down
	extern std::vector<Game::ID_TYPE>			RegisterRemovalList;	// Objects where the entire entry can now be removed from the register (i.e. inactive & refcount == 0)

	// Determines whether object registers are locked, e.g. when the main logic cycle is iterating through the collection
	// If registers are unlocked we can add/remove objects as normal; if locked, objects are instead added to the relevant 
	// lists to be actioned at the end of the frame
	extern bool									m_registers_locked;
	CMPINLINE bool								ObjectRegistersLocked(void)					{ return m_registers_locked; }
	CMPINLINE void								LockObjectRegisters(void)					{ m_registers_locked = true; }
	CMPINLINE void								UnlockObjectRegisters(void)					{ m_registers_locked = false; }

	// Performs the requested action on an object.  Protected and should only be called by the higher-level management methods
	void										PerformObjectRegistration(iObject *object);
	void										PerformObjectUnregistration(Game::ID_TYPE id);
	void										PerformRegisterEntryRemoval(Game::ID_TYPE id);

	// Register an object with the global collection
	CMPINLINE void								RegisterObject(iObject *obj)
	{
		if (m_registers_locked)
		{
			// If registers are locked, add to the registration list for addition to the global collection in the next cycle
			OBJ_REGISTER_LOG(concat("Registering object ")(obj ? obj->GetID() : 0)(" (\"")(obj ? obj->GetInstanceCode() : "<null>")("\")...Adding to registration list").str());
			Game::RegisterList.push_back(obj);
		}
		else
		{
			// If registers are unlocked we can add the object immediately
			OBJ_REGISTER_LOG(concat("Registering object ")(obj ? obj->GetID() : 0)(" (\"")(obj ? obj->GetInstanceCode() : "<null>")("\")...Registering immediately").str());
			PerformObjectRegistration(obj);
		}
	}

	// Unregister an object from the global collection
	void										UnregisterObject(iObject *obj);

	// Remove the entire entry from the object register; performed when active == false and refcount == 0
	CMPINLINE void								RemoveObjectRegisterEntry(Game::ID_TYPE id)
	{
		if (m_registers_locked)
		{
			// If registers are locked, add to the removal vector which is processed at the end of each frame
			OBJ_REGISTER_LOG(concat("Removing object ")(id)(" register entry...Adding to register removal list").str());
			Game::RegisterRemovalList.push_back(id);
		}
		else
		{
			// If registers are unlocked we can remove the entry immediately
			OBJ_REGISTER_LOG(concat("Removing object ")(id)(" register entry...Removing immediately").str());
			PerformRegisterEntryRemoval(id);
		}
	}

	// Perform an in-place swap of two object register entries.  Only executed if both IDs are valid
	// and refer to active objects in the register.  This is a debug method which bypasses the standard
	// object register controls and should be used infrequently, if at all
	void										SwapObjectRegisterEntries(Game::ID_TYPE object0, Game::ID_TYPE object1);
	
	// Method which processes all pending register/unregister requests to update the global collection.  Executed once per frame
	void										UpdateGlobalObjectCollection(void);

	// Resets the null object reference every frame so that its reference count never goes out of bounds
	void										ResetNullObjectReference(void);

	// Initialises all object register data on application startup
	void										InitialiseObjectRegisters(void);

	// Deallocates all object register data on application shutdown
	void										ShutdownObjectRegisters(void);
	
	// Returns the null object register entry
	extern ObjectRegisterEntry *				NullObjectReference;

	// Marks an object as visible.  No parameter checking; calling function must ensure the object is non-null
	// or an exception will be thrown
	CMPINLINE void								MarkObjectAsVisible(iObject *obj)
	{
		obj->MarkAsVisible();
		Game::VisibleObjects.push_back(obj);
	}

	// Clears the visible object collection ready for the next frame
	CMPINLINE void								ClearVisibleObjectCollection(void)	{ Game::VisibleObjects.clear(); }


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
		return (it != ObjectsByCode.end() && it->second->Active);
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
		Game::ObjectRegisterEntry & entry = *(it->second);
		return (entry.Active ? entry.Object : NULL);
	}

	// Attempts to locate an object that matches either the instance code (priority) or ID provided
	CMPINLINE iObject *							FindObjectByIdentifier(const std::string & identifier)
	{
		if (identifier == NullString) return NULL;

		iObject *o = GetObjectByInstanceCode(identifier);
		if (o) return o;

		const char *c = identifier.c_str();
		int id = atoi(c);
		return GetObjectByID(id);
	}


	// Notifies the central object collection that the code of the specified object has changed.  Ensures that
	// correct references are maintained in the central object collection
	void										NotifyChangeOfObjectInstanceCode(iObject *object, const std::string & old_code);

	// Verifies the integrity of each object register to ensure no data gets out of sync.  Enabled during debug only
#	ifdef _DEBUG
		void									_DebugVerifyObjectRegisterIntegrity(void);
#		define									VERIFY_OBJECT_REGISTER_INTEGRITY() _DebugVerifyObjectRegisterIntegrity()
#	else
#		define									VERIFY_OBJECT_REGISTER_INTEGRITY()
#	endif


};



#endif









