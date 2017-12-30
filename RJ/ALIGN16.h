#pragma once

#ifndef __ALIGN16H__
#define __ALIGN16H__

#include <new>
#include <malloc.h>

// Debug option to log all allocation and deallocations performed through this interface
//#define RJ_LOG_ALIGN_ALLOCS

// Define an include block to force the use of aligned allocation functions in multiple-inheritance
// situations, where the compiler cannot otherwise determine which allocation/deallocation functions to use
#define USE_ALIGN16_ALLOCATORS(T) \
		using ALIGN16<T>::operator new; \
		using ALIGN16<T>::operator new[]; \
		using ALIGN16<T>::operator delete; \
		using ALIGN16<T>::operator delete[]; \

// Additional debug imports if required
#ifdef RJ_LOG_ALIGN_ALLOCS
#	include <Windows.h>
#	include <crtdefs.h>
#	define FN_ID __FUNCTION__
#endif

// Define debug output functions if required
#ifdef RJ_LOG_ALIGN_ALLOCS
#	define RJ_ALIGN_ALLOC_OUTPUT(msg) OutputDebugString(msg)
#else
#	define RJ_ALIGN_ALLOC_OUTPUT(msg) 
#endif

template <typename T>
class ALIGN16
{
public:

	// Overridden "new" operator to ensure 16-bit alignment for all allocations of this class
	void* operator new (size_t size)
	{ 
		RJ_ALIGN_ALLOC_OUTPUT(concat("> New aligned allocation [")(FN_ID)("], size = ")(size).str().c_str());

		void *p = _aligned_malloc(size, 16U);
		if (!p)
		{
			RJ_ALIGN_ALLOC_OUTPUT(", ALLOCATION FAILED\n");
			throw std::bad_alloc();
		}

		RJ_ALIGN_ALLOC_OUTPUT(concat(", addr = ")(p)("\n").str().c_str());

		return p;
	}

	// Overridden "delete" operator to deallocate 16-bit-aligned heap-allocated instances of this class
	void operator delete (void *p)
	{
		RJ_ALIGN_ALLOC_OUTPUT(concat("> Deleting aligned allocation [")(FN_ID)("] at ")(p)("\n").str().c_str());

		_aligned_free(static_cast<T*>(p));
	}

	// Overridden "new[]" operator to ensure 16-bit alignment for all allocations of this class
	void* operator new[] (size_t size)
	{
		RJ_ALIGN_ALLOC_OUTPUT(concat("> New aligned array allocation [")(FN_ID)("], size = ")(size).str().c_str());

		void *p = _aligned_malloc(size, 16U);
		if (!p)
		{
			RJ_ALIGN_ALLOC_OUTPUT(", ALLOCATION FAILED\n");
			throw std::bad_alloc();
		}

		RJ_ALIGN_ALLOC_OUTPUT(concat(", addr = ")(p)("\n").str().c_str());

		return p;
	}

	// Overridden "delete[]" operator to deallocate 16-bit-aligned heap-allocated instances of this class
	void operator delete[] (void *p)
	{
		RJ_ALIGN_ALLOC_OUTPUT(concat("> Deleting aligned array allocation [")(FN_ID)("] at ")(p)("\n").str().c_str());

		_aligned_free(static_cast<T*>(p));
	}

	// Overridden "placement new" operator to initialise an object within a pre-allocated block of memory
	void* operator new (size_t size, void *buffer)
	{
		RJ_ALIGN_ALLOC_OUTPUT(concat("> Aligned placement allocation [")(FN_ID)("] of size ")(size)(" at ")(buffer)("\n").str().c_str());

		return buffer;
	}

	// Overridden "new" operator with "nothrow" to ensure 16-bit alignment for all allocations of this class
	// Will return NULL rather than throwing an exception if the memory allocation fails
	void* operator new (size_t size, const std::nothrow_t &t) throw()
	{
		RJ_ALIGN_ALLOC_OUTPUT(concat("> Aligned nothrow allocation [")(FN_ID)("], size = ")(size).str().c_str());

		// Attempt to perform the allocation
		void *p = _aligned_malloc(size, 16U);
		
#		ifdef RJ_LOG_ALIGN_ALLOCS
			if (p)		RJ_ALIGN_ALLOC_OUTPUT(concat(", addr = ")(p)("\n").str().c_str());
			else		RJ_ALIGN_ALLOC_OUTPUT(", COULD NOT ALLOCATE\n");
#		endif

		// Return p without testing it; if the allocation failed, p will be NULL.  Replaces "if (!p) throw std::bad_alloc();"
		return p;
	}


	// Overridden "new[]" operator with "nothrow" to ensure 16-bit alignment for all allocations of this class
	// Will return NULL rather than throwing an exception if the memory allocation fails
	void* operator new[](size_t size, const std::nothrow_t &t) throw()
	{
		RJ_ALIGN_ALLOC_OUTPUT(concat("> Aligned nothrow array allocation [")(FN_ID)("] , size = ")(size).str().c_str());

		// Attempt to perform the allocation
		void *p = _aligned_malloc(size, 16U);

#		ifdef RJ_LOG_ALIGN_ALLOCS
			if (p)		RJ_ALIGN_ALLOC_OUTPUT(concat(", addr = ")(p)("\n").str().c_str());
			else		RJ_ALIGN_ALLOC_OUTPUT(", COULD NOT ALLOCATE\n");
#		endif

		// Return p without testing it; if the allocation failed, p will be NULL.  Replaces "if (!p) throw std::bad_alloc();"
		return p;
	}

	// Overridden "delete[]" operator to deallocate 16-bit-aligned heap-allocated instances of this class
	// Required counterpart to new[](nothrow) so that memory can be automatically freed if initialisation throws an exception
	void operator delete[](void *p, const std::nothrow_t &t) throw()
	{
		RJ_ALIGN_ALLOC_OUTPUT(concat("> Deleting aligned nothrow array allocation [")(FN_ID)("] at ")(p)("\n").str().c_str());

		_aligned_free(static_cast<T*>(p));
	}


	// Static allocation/deallocation methods along 16-byte word boundaries
	static T * New(void);
	static void Delete(T *alloc);
	static void SafeDelete(T *alloc);

};




#endif