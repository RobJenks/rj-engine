#pragma once

#ifndef __ObjectReferenceH__
#define __ObjectReferenceH__

#include <unordered_map>
#include "GameVarsExtern.h"
#include "GameObjects.h"

template <class T>
struct ObjectReference
{
public:

	// Default constructor; establishes a link to the null object register entry
	CMPINLINE ObjectReference(void) noexcept: m_entry(Game::NullObjectReference) { }

	// Constructor; establishes a reference to the object with the specified ID
	CMPINLINE ObjectReference(Game::ID_TYPE id) noexcept
	{
		// Get a reference to the relevant object register entry
		Game::ObjectRegister::iterator entry = Game::Objects.find(id);
		if (entry == Game::Objects.end())
		{
			m_entry = Game::NullObjectReference;		// Link to the null object reference, which simply wraps NULL
			m_entry->ReferenceAdded();					// Add a reference (even to the null entry) to prevent it being refcounted out later
		}
		else
		{
			m_entry = &(entry->second);					// Store a reference to the relevant entry
			m_entry->ReferenceAdded();					// Record this reference to the entry, to maintain an accurate reference count
		}
	}

	// Constructor; establishes a reference to the specified object
	CMPINLINE ObjectReference(iObject *object) : ObjectReference(object ? object->GetID() : 0) { }

	// Custom copy assignment operator; assigns one reference to another, incrementing reference counts accordingly
	CMPINLINE ObjectReference & operator=(const ObjectReference & rhs) noexcept
	{
		// Remove our current reference
		m_entry->ReferenceRemoved();

		// Copy the reference in the source object
		m_entry = rhs.m_entry;

		// Increment the reference count since we are now also referencing this object
		m_entry->ReferenceAdded();
		return *this;
	}
	
	// Custom copy assignment operator; creates a new reference to wrap the specified object 
	CMPINLINE ObjectReference & operator=(iObject *object) noexcept 
	{  
		// Remove our current reference
		m_entry->ReferenceRemoved();

		// Parameter check; if target is null, point at the null object reference
		if (!object) { m_entry = Game::NullObjectReference; m_entry->ReferenceAdded(); return *this; }

		// Attempt to locate the new object in the register; if not found, point at the null object reference
		Game::ObjectRegister::iterator entry = Game::Objects.find(object->GetID());
		if (entry == Game::Objects.end()) { m_entry = Game::NullObjectReference; m_entry->ReferenceAdded(); return *this; }

		// Store the new reference and increment its reference count
		m_entry = &(entry->second);
		m_entry->ReferenceAdded();

		return *this;
	}

	// Custom copy constructor
	CMPINLINE ObjectReference(const ObjectReference & source) noexcept 
	{
		// Copy the reference from our source object
		m_entry = source.m_entry;

		// Increment the reference count since we are now also pointing at this object
		m_entry->ReferenceAdded();
	}

	// Custom move constructor
	CMPINLINE ObjectReference(ObjectReference && other) noexcept : m_entry(std::move(other.m_entry))
	{
		// Assign the null object reference to the source object, since it no longer owns this reference
		other.m_entry = Game::NullObjectReference;
	}

	// Custom move assignment operator
	CMPINLINE ObjectReference & operator=(ObjectReference && other) noexcept
	{
		// Release any existing reference that we have
		m_entry->ReferenceRemoved();
		
		// Take the new object reference
		m_entry = std::move(other.m_entry);

		// Assign the null object reference to the source object, since it no longer owns this reference
		other.m_entry = Game::NullObjectReference;

		return *this;
	}

	// Destructor; remove reference to the object, triggering deallocation of the object register entry if necessary
	CMPINLINE ~ObjectReference(void) noexcept
	{
		// Decrement entry reference count.  This will cause a deallocation of the register entry if appropriate
		m_entry->ReferenceRemoved();
	}

	// Operator (); returns the object being wrapped by this reference
	// No longer need to test (entry.Active ? entry.object : NULL) since object is set to NULL on unregistering anyway
	CMPINLINE T * operator()(void) noexcept			{ return (T*)(m_entry->Object); }
	CMPINLINE T * operator()(void) const noexcept	{ return (T*)(m_entry->Object); }


protected:

	// Internal reference to the object register entry for this object
	Game::ObjectRegisterEntry *							m_entry;

};


#endif






