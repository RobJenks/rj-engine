#pragma once

#ifndef __DataRegisterH__
#define __DataRegisterH__

#include <string>
#include <unordered_map>
#include "CompilerSettings.h"

template <typename T>
class DataRegister
{
public:

	// Underlying collection of data, indexed by unique string code
	typedef std::unordered_map<std::string, T*>		RegisterType;
	RegisterType									Data;

	// Constructor 
	DataRegister(void)
	{
		// By default we do not shutdown objects on deallocation
		m_shutdown_on_deallocation = false;
	}

	// Constructor; flag indicates whether the collection will Shutdown(void) objects on deallocation
	DataRegister(bool shutdown_on_deallocation)
	{
		// Store the parameter indicating whether we shutdown objects on deallocation
		m_shutdown_on_deallocation = shutdown_on_deallocation;
	}

	// Flag indicating whether the collection should call a generic Shutdown(void) method on data
	// objects when deallocating the data register.  Default = false
	CMPINLINE bool									WillShutdownObjectsOnDeallocation(void) const	{ return m_shutdown_on_deallocation; }
	CMPINLINE void									SetShutdownOnDeallocation(bool shutdown)		{ m_shutdown_on_deallocation = shutdown; }

	// Retrieve a data object from the register based upon its unique string code
	CMPINLINE T *									Get(const std::string & code)
	{
		if (Data.count(code) == 0)					return NULL;
		else										return Data[code];		
	}

	// Store an object in the register based upon its unique string code.  If an object already exists 
	// with that code it will not be replaced - code must be unique
	CMPINLINE void									Store(T *obj)
	{
		if (!obj) return;
		const std::string & code = obj->GetCode();
		if (code != NullString && Data.count(code) == 0)
		{
			Data[code] = obj;
		}
	}

	// Returns a value indicating whether the specified object exists
	CMPINLINE bool									Exists(const std::string & code) const
	{
		return (Data.count(code) != 0);
	}

	// Destructor; deallocates all data held within the data register
	DataRegister<T>::~DataRegister(void)
	{
		T *obj;

		// Iterate through the data collection
		RegisterType::iterator it_end = Data.end();
		for (RegisterType::iterator it = Data.begin(); it != it_end; ++it)
		{
			obj = (T*)it->second;
			if (obj)
			{
				// Calll the object shutdown method if required
				if (m_shutdown_on_deallocation) obj->Shutdown();

				// Deallocate the object
				delete obj;
			}
		}

		// Finally, clear the register of all data
		Data.clear();
	}

protected:

	// Flag indicating whether the collection should call a generic Shutdown(void) method on data
	// objects when deallocating the data register.  Default = false
	bool					m_shutdown_on_deallocation;

};




#endif