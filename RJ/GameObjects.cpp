#include "iObject.h"

#include "GameObjects.h"

namespace Game
{
	// Global object collection (TODO: in future, maintain only local objects in a collection so we don't run unnecessary simulation)
	Game::ObjectRegister					Objects(0);

	// Secondary object collections
	Game::ObjectRegisterByInstanceCode		ObjectsByCode(0);

	// Short-term lists of objects waiting to be registered or unregistered with the global collection; actioned each frame
	std::vector<Game::ID_TYPE>				UnregisterList(0);		// Stored as ID in case object is being unregistered while it is being shut down
	std::vector<iObject*>					RegisterList(0);		// Store the object so it can be inserted into the global game objects list

	// Unregister an object from the global collection
	void UnregisterObject(iObject *obj)
	{
		// Make sure this is a valid object
		if (!obj) return;

		// Check whether this object is in fact registered with the global collection
		Game::ObjectRegister::iterator entry = Game::Objects.find(obj->GetID());
		if (entry != Game::Objects.end())
		{
			// If it is, add it's ID to the shutdown list for removal in the next cycle.  Add the ID rather than
			// the object since this unregistering could be requested as part of object shutdown, and when the
			// unregister list is next processed the object may no longer be valid.
			Game::UnregisterList.push_back(obj->GetID());

			// Flag the object as inactive in all global collections to avoid processing it in the upcoming frame
			entry->second.Active = false;

			// Also remove from secondary registers.  Should always exist here as well since the collections should always
			// remain in sync, but still perform a check to ensure the object exists in each register
			Game::ObjectRegisterByInstanceCode::iterator code_entry = Game::ObjectsByCode.find(obj->GetInstanceCode());
			if (code_entry != Game::ObjectsByCode.end()) code_entry->second.Active = false;
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
			iObject *obj; Game::ID_TYPE id;
			for (std::vector<Game::ID_TYPE>::size_type i = 0; i < nU; ++i)
			{
				// Get a reference to the object being removed
				id = Game::UnregisterList[i];
				obj = Game::Objects[id].Object;

				// Remove this object from the global collection.  It should definitely exist, but check to be safe
				Game::ObjectRegister::iterator entry = Game::Objects.find(id);
				if (entry != Game::Objects.end())
				{
					entry->second.Object = NULL;		// Remove the pointer first; ensures we are deregistered, and also prevents ".erase()" from calling its destructor
					Game::Objects.erase(id);			// Erasing from the map will remove the key and element, reducing search space for the future
				}

				// Also remove from all secondary registers
				Game::ObjectRegisterByInstanceCode::iterator code_entry = Game::ObjectsByCode.find(obj->GetInstanceCode());
				if (code_entry != Game::ObjectsByCode.end())
				{
					code_entry->second.Object = NULL;
					Game::ObjectsByCode.erase(obj->GetInstanceCode());
				}

				// Finally call the destructor for this object to deallocate all resources
				delete obj;
			}

			// Clear the pending unregister list
			Game::UnregisterList.clear();
		}

		// Now process any register requests
		std::vector<iObject*>::size_type nR = Game::RegisterList.size();
		if (nR != 0)
		{
			// Process each request in turn
			iObject *obj;
			for (std::vector<iObject*>::size_type i = 0; i < nR; ++i)
			{
				// Check the object exists
				obj = Game::RegisterList[i]; if (!obj) continue;

				// Make sure the object meets all criteria for e.g. uniqueness
				if (Game::ObjectsByCode.count(obj->GetInstanceCode()) != 0) continue;		// Ignore this object if it does not have a unique instance code

				// Register with the global collection
				Game::Objects[obj->GetID()] = Game::ObjectRegisterEntry(obj);

				// Register with secondary collections
				Game::ObjectsByCode[obj->GetInstanceCode()] = Game::ObjectRegisterEntry(obj);	// We know the code is unique after the test above
			}

			// Clear the pending registration list
			Game::RegisterList.clear();
		}
	}

};