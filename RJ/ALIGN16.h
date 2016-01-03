#pragma once

#ifndef __ALIGN16H__
#define __ALIGN16H__

#include <new>
#include <malloc.h>

// Define an include block to force the use of aligned allocation functions in multiple-inheritance
// situations, where the compiler cannot otherwise determine which allocation/deallocation functions to use
#define USE_ALIGN16_ALLOCATORS(T) \
		using ALIGN16<T>::operator new; \
		using ALIGN16<T>::operator new[]; \
		using ALIGN16<T>::operator delete; \
		using ALIGN16<T>::operator delete[]; \


template <typename T>
class ALIGN16
{
public:

	// Overridden "new" operator to ensure 16-bit alignment for all allocations of this class
	void* operator new (size_t size)
	{
		void *p = _aligned_malloc(size, 16U);
		if (!p) throw std::bad_alloc();
		return p;
	}

	// Overridden "delete" operator to deallocate 16-bit-aligned heap-allocated instances of this class
	void operator delete (void *p)
	{
		_aligned_free(static_cast<T*>(p));
	}

	// Overridden "new[]" operator to ensure 16-bit alignment for all allocations of this class
	void* operator new[] (size_t size)
	{
		void *p = _aligned_malloc(size, 16U);
		if (!p) throw std::bad_alloc();
		return p;
	}

	// Overridden "delete[]" operator to deallocate 16-bit-aligned heap-allocated instances of this class
	void operator delete[] (void *p)
	{
		_aligned_free(static_cast<T*>(p));
	}

	// Overridden "placement new" operator to initialise an object within a pre-allocated block of memory
	void* operator new (size_t size, void *buffer)
	{
		return buffer;
	}

	// Overridden "new" operator with "nothrow" to ensure 16-bit alignment for all allocations of this class
	// Will return NULL rather than throwing an exception if the memory allocation fails
	void* operator new (size_t size, const std::nothrow_t &t) throw()
	{
		void *p = _aligned_malloc(size, 16U);
		
		// Return p without testing it; if the allocation failed, p will be NULL.  Replaces "if (!p) throw std::bad_alloc();"
		return p;
	}


	// Overridden "new[]" operator with "nothrow" to ensure 16-bit alignment for all allocations of this class
	// Will return NULL rather than throwing an exception if the memory allocation fails
	void* operator new[](size_t size, const std::nothrow_t &t) throw()
	{
		void *p = _aligned_malloc(size, 16U);

		// Return p without testing it; if the allocation failed, p will be NULL.  Replaces "if (!p) throw std::bad_alloc();"
		return p;
	}

	// Overridden "delete[]" operator to deallocate 16-bit-aligned heap-allocated instances of this class
	// Required counterpart to new[](nothrow) so that memory can be automatically freed if initialisation throws an exception
	void operator delete[](void *p, const std::nothrow_t &t) throw()
	{
		_aligned_free(static_cast<T*>(p));
	}
};




#endif