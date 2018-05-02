#pragma once

#ifndef __AlignedAllocatorH__
#define __AlignedAllocatorH__

#include <malloc.h>
#include <memory>


// Enable or disable aligned allocations
//#define ALIGNED_ALLOCATORS_ENABLED

#ifndef ALIGNED_ALLOCATORS_ENABLED

	/*** DISABLED ***/

	template <class T, size_t Alignment> struct AlignedAllocator : public std::allocator<T> { };

#else

	/*** ENABLED ***/


/**
* STL-compliant allocator that allocates aligned memory.
* @tparam T Type of the element to allocate.
* @tparam Alignment Alignment of the allocation, e.g. 16.
* @ingroup AlignedAllocator
*/
template <class T, size_t Alignment>
struct AlignedAllocator
	: public std::allocator<T> // Inherit construct(), destruct() etc.
{
#if 0
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T*        pointer;
	typedef const T*  const_pointer;
	typedef T&        reference;
	typedef const T&  const_reference;
	typedef T         value_type;
#endif
	typedef typename std::allocator<T>::size_type size_type;
	typedef typename std::allocator<T>::pointer pointer;
	typedef typename std::allocator<T>::const_pointer const_pointer;

	/// Defines an aligned allocator suitable for allocating elements of type
	/// @c U.
	template <class U>
	struct rebind { typedef AlignedAllocator<U, Alignment> other; };

	/// Default-constructs an allocator.
	AlignedAllocator() throw() { }

	/// Copy-constructs an allocator.
	AlignedAllocator(const AlignedAllocator& other) throw()
		: std::allocator<T>(other) { }

	/// Convert-constructs an allocator.
	template <class U>
	AlignedAllocator(const AlignedAllocator<U, Alignment>&) throw() { }

	/// Destroys an allocator.
	~AlignedAllocator() throw() { }

	/// Allocates @c n elements of type @c T, aligned to a multiple of
	/// @c Alignment.
	pointer allocate(size_type n)
	{
		return allocate(n, const_pointer(0));
	}

	/// Allocates @c n elements of type @c T, aligned to a multiple of
	/// @c Alignment.
	pointer allocate(size_type n, const_pointer /* hint */)
	{
		void *p;
#ifndef _WIN32
		if (posix_memalign(&p, Alignment, n*sizeof(T)) != 0)
			p = NULL;
#else
		p = _aligned_malloc(n*sizeof(T), Alignment);
#endif
		if (!p)
			throw std::bad_alloc();
		return static_cast<pointer>(p);
	}

	/// Frees the memory previously allocated by an aligned allocator.
	void deallocate(pointer p, size_type /* n */)
	{
#ifndef _WIN32
		free(p);
#else
		_aligned_free(p);
#endif
	}
};

/**
* Checks whether two aligned allocators are equal. Two allocators are equal
* if the memory allocated using one allocator can be deallocated by the other.
* @returns Always @c true.
* @ingroup AlignedAllocator
*/
template <class T1, size_t A1, class T2, size_t A2>
bool operator == (const AlignedAllocator<T1, A1> &, const AlignedAllocator<T2, A2> &)
{
	return true;
}

/**
* Checks whether two aligned allocators are not equal. Two allocators are equal
* if the memory allocated using one allocator can be deallocated by the other.
* @returns Always @c false.
* @ingroup AlignedAllocator
*/
template <class T1, size_t A1, class T2, size_t A2>
bool operator != (const AlignedAllocator<T1, A1> &, const AlignedAllocator<T2, A2> &)
{
	return false;
}


#endif 


#endif