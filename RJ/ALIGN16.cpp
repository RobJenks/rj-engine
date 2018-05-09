#include "ALIGN16.h"
#include <allocators>

#ifdef ALIGN16_ENABLED

// Static allocation method along 16-byte word boundaries
template <typename T>
T * ALIGN16<T>::New(void)
{
	_aligned_malloc(sizeof(T), 16U);
}

// Static deallocation method along 16-byte word boundaries
template <typename T>
void ALIGN16<T>::Delete(T *alloc)
{
	_aligned_free(alloc);
}

// Static deallocation method along 16-byte word boundaries
template <typename T>
void ALIGN16<T>::SafeDelete(T *alloc)
{
	_aligned_free(alloc);
	alloc = NULL;
}

#endif