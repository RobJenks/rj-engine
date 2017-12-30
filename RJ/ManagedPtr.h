#pragma once

#include <memory>
#include "CompilerSettings.h"


// Wraps a unique_ptr and allows direct access to both managed & raw ptr
template <typename T>
struct ManagedPtr
{
	std::unique_ptr<T>			UniquePtr;
	T *							RawPtr;


	// Constructor
	CMPINLINE ManagedPtr(void)
		:
		UniquePtr(std::make_unique<T>())
	{
		RawPtr = UniquePtr.get();
	}

	// Constructor
	CMPINLINE ManagedPtr(T *ptr)
		:
		UniquePtr(std::unique_ptr<T>(ptr))
	{
		RawPtr = UniquePtr.get();
	}

	// Constructor
	CMPINLINE ManagedPtr(std::unique_ptr<T> uptr)
		:
		UniquePtr(std::move(uptr))
	{
		RawPtr = UniquePtr.get();
	}

	// Copy construction and assignment are disabled
	CMPINLINE ManagedPtr(const ManagedPtr<T> & other) = 0;
	CMPINLINE ManagedPtr & operator=(const ManagedPtr<T> & other) = 0;

	// Move constructor
	CMPINLINE ManagedPtr(ManagedPtr<T> && other)
		:
		UniquePtr(std::move(other.UniquePtr))
	{
		RawPtr = UniquePtr.get();
	}

	// Move assignment
	CMPINLINE ManagedPtr & operator=(ManagedPtr<T> && other)
		:
		UniquePtr(std::move(other.UniquePtr))
	{
		RawPtr = UniquePtr.get();
	}

	// Destructor
	CMPINLINE ~ManagedPtr(void)
	{
		// All resources deallocated by the unique_ptr
	}


};







