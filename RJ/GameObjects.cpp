#include "iObject.h"

#include "GameObjects.h"

namespace Game
{
	// Global object collection (TODO: in future, maintain only local objects in a collection so we don't run unnecessary simulation)
	Game::ObjectRegister					Objects(0);

	// Collection of all visible objects, recreated each frame
	std::vector<iObject*>					VisibleObjects(0);

	// Secondary object collections
	Game::ObjectRegisterByInstanceCode		ObjectsByCode(0);

	// Short-term lists of objects waiting to be registered or unregistered with the global collection; actioned each frame
	std::vector<Game::ID_TYPE>				UnregisterList(0);		// Stored as ID in case object is being unregistered while it is being shut down
	std::vector<iObject*>					RegisterList(0);		// Store the object so it can be inserted into the global game objects list
	std::vector<Game::ID_TYPE>				RegisterRemovalList(0);	// Objects where the entire entry can now be removed from the register (i.e. inactive & refcount == 0)

	// Returns the null object register entry
	ObjectRegisterEntry *					NullObjectReference = new ObjectRegisterEntry(0, NULL, true, 1000);

	// Determines whether object registers are locked, e.g. when the main logic cycle is iterating through the collection
	// If registers are unlocked we can add/remove objects as normal; if locked, objects are instead added to the relevant 
	// lists to be actioned at the end of the frame
	bool									m_registers_locked = false;

	// Initialises all object register data on application startup
	void InitialiseObjectRegisters(void)
	{
		// Create the null object register entry
		Game::NullObjectReference = new ObjectRegisterEntry();
		Game::NullObjectReference->Object = NULL;
		Game::NullObjectReference->Active = true;
		Game::NullObjectReference->RefCount = 1000U;				// Ref count should never hit zero so this is always active; use 1000 in case of unexpected error

		// Reserve space in the primary object vectors for efficiency
		Game::Objects.reserve(2048U);
		Game::VisibleObjects.reserve(2048U);
	}

	// Deallocates all object register data on application shutdown
	void ShutdownObjectRegisters(void)
	{
		// Traverse the object register one at a time
		Game::ObjectRegister::iterator it = Game::Objects.begin();
		Game::ObjectRegister::iterator it_end = Game::Objects.end();
		while (it != it_end)
		{
			// Make sure this object still exists
			if (it->second.Active && it->second.Object)
			{
				it->second.Active = false;				// Inactivate all entries so they now return null
				it->second.Object->Shutdown();			// Call virtual shutdown method on the object
				delete (it++)->second.Object;			// Delete the object, using post-increment to avoid invalidating the pointer
			}
			else
			{
				it->second.Active = false;				// Inactivate all entries so they now return null
				++it;									// If this object does not exist then simply increment immediately and move on
			}
		}

		// Call the unordered_map::clear method, which will call the destructor for every element in turn (now unnecessary) before clearing the collection
		// We no longer need to do this; leaving the entries present means that any shutdown logic beyond this that (incorrectly) tries to access
		// an object will retrieve NULL and therefore take no action
		//Game::Objects.clear();

		// Deallocate the null object register entry
		SafeDelete(Game::NullObjectReference);
	}

	// Unregister an object from the global collection
	void UnregisterObject(iObject *obj)
	{
		// Make sure this is a valid object
		if (!obj) return;

		// Check whether this object is in fact registered with the global collection
		Game::ObjectRegister::iterator entry = Game::Objects.find(obj->GetID());
		if (entry != Game::Objects.end())
		{
			// Flag the object as inactive in the global collection to avoid processing it in the upcoming frame
			entry->second.Active = false;

			// Remove based on object ID.  Use the ID rather than the object itself since this unregistering 
			// could be requested as part of object shutdown, and when the unregister list is next processed 
			// the object may no longer be valid.  Either remove immediately or add to the list, depending on lock state
			if (m_registers_locked)
			{
				// If registers are locked, add to the list for removal at the end of the frame
				Game::UnregisterList.push_back(obj->GetID());
			}
			else
			{
				// If registers are unlocked we can unregister the object immediately
				PerformObjectUnregistration(obj->GetID());
			}

		}
	}

	// Performs the requested action on an object.  Protected and should only be called by the higher-level management methods
	void PerformObjectRegistration(iObject *object)
	{
		// Make sure the object is valid and meets all criteria for e.g. uniqueness
		if (!object ||																// Ignore this object if it is NULL
			object->GetInstanceCode() == NullString ||								// Ignore this object if it has no instance code
			ObjectExists(object->GetInstanceCode())) return;						// Ignore this object if it does not have a unique instance code

		// Register with the global collection
		Game::Objects[object->GetID()] = Game::ObjectRegisterEntry(object);

		// Register with secondary collections
		Game::ObjectRegisterEntry *primary_entry = &(Game::Objects[object->GetID()]);
		Game::ObjectsByCode[object->GetInstanceCode()] = primary_entry;				// We know the code is unique after the test above
	}

	// Performs the requested action on an object.  Protected and should only be called by the higher-level management methods
	void PerformObjectUnregistration(Game::ID_TYPE id)
	{
		// Check the object exists.  It definitely should , but check to be safe
		Game::ObjectRegister::iterator entry = Game::Objects.find(id);
		if (entry != Game::Objects.end())
		{
			// Get a reference to the object
			iObject *obj = entry->second.Object;

			// Set the object to inactive and remove the pointer, so no further processes can access it
			entry->second.Active = false;
			entry->second.Object = NULL;

			// If there are no current references to this object we can also remove the entire entry from the register at the same time
			if (entry->second.RefCount <= 0) Game::RemoveObjectRegisterEntry(id);

			// Also remove from all secondary registers; these do not need to maintain a persistent register entry to
			// avoid null-references, since they are not maintaining any data directly.  They instead just reference entries
			// in the primary collection.  If we erase from a secondary collection the relevant retrieval methods will simply return null, correctly
			Game::ObjectRegisterByInstanceCode::iterator code_entry = Game::ObjectsByCode.find(obj->GetInstanceCode());
			if (code_entry != Game::ObjectsByCode.end()) Game::ObjectsByCode.erase(obj->GetInstanceCode());

			// Finally call the destructor for this object to deallocate all resources
			delete obj;
		}
	}

	// Performs the requested action on an object.  Protected and should only be called by the higher-level management methods
	void PerformRegisterEntryRemoval(Game::ID_TYPE id)
	{
		// The object should definitely exist, but make sure
		Game::ObjectRegister::iterator entry = Game::Objects.find(id);
		if (entry != Game::Objects.end())
		{
			// Assuming we found the entry, erase it now to remove all record from the register
			entry->second.Object = NULL;
			Game::Objects.erase(entry);
		}
	}


	// Method which processes all pending register/unregister requests to update the global collection.  Executed once per frame
	void UpdateGlobalObjectCollection(void)
	{
		// First process any unregister requests
		std::vector<Game::ID_TYPE>::size_type nU = Game::UnregisterList.size();
		if (nU != 0)
		{
			// Process each request in turn
			for (std::vector<Game::ID_TYPE>::size_type i = 0; i < nU; ++i)
			{
				// Unregister the object
				PerformObjectUnregistration(Game::UnregisterList[i]);
			}

			// Clear the pending unregister list
			Game::UnregisterList.clear();
		}

		// Process any entry-removal requests
		std::vector<ID_TYPE>::size_type nE = Game::RegisterRemovalList.size();
		if (nE != 0)
		{
			// Process each request in turn
			for (std::vector<Game::ID_TYPE>::size_type i = 0; i < nE; ++i)
			{
				// Remove the entry from the central register
				PerformRegisterEntryRemoval(Game::RegisterRemovalList[i]);
			}

			// Clear the pending removal list
			Game::RegisterRemovalList.clear();
		}

		// Now process any register requests
		std::vector<iObject*>::size_type nR = Game::RegisterList.size();
		if (nR != 0)
		{
			// Process each request in turn
			for (std::vector<iObject*>::size_type i = 0; i < nR; ++i)
			{
				// Register this object with the central register
				Game::PerformObjectRegistration(Game::RegisterList[i]); 
			}

			// Clear the pending registration list
			Game::RegisterList.clear();
		}
	}

	// Notifies the central object collection that the code of the specified object has changed.  Ensures that
	// correct references are maintained in the central object collection
	void NotifyChangeOfObjectInstanceCode(iObject *object, const std::string & old_code)
	{
		// Parameter check 
		if (!object || old_code == NullString || object->GetInstanceCode() == NullString ||
			ObjectExists(object->GetInstanceCode())) return;

		// Attempt to locate the object by its previous code
		Game::ObjectRegisterByInstanceCode::iterator entry = Game::ObjectsByCode.find(old_code);
		if (entry == Game::ObjectsByCode.end()) return;			// Error: object does not exist with this code (or it may be a standard, non-register object)
		if (!(entry->second)) return;							// Error: no link to primary entry (major error)
		if (entry->second->Object != object) return;			// Error: the object with this code is not the one we are attempting to change
		
		// Remove the secondary register entry for the old code
		ObjectRegisterEntry *primary_entry = (entry->second);
		entry->second = NULL;
		Game::ObjectsByCode.erase(entry);

		// Add a new secondary register entry for the new code
		Game::ObjectsByCode[object->GetInstanceCode()] = primary_entry;
	}


};




