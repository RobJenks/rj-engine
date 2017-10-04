#pragma once

#include <vector>
#include <algorithm>

class Collections
{

public:

	// 
	// Erase all elements in the collection which satisfy the given predicate.  Does not guarantee
	// that order of remaining elements will be preserved
	// 
	template <template <typename, typename> class TCollection,
		typename TElement,
		typename TAllocator = std::allocator<TElement>,
		typename _Pred>

	static void Erase(typename TCollection<TElement, TAllocator> & collection, _Pred pred)
	{
		typename TCollection<TElement, TAllocator>::iterator it = std::partition(collection.begin(), collection.end(),
			[pred](const auto & el) { return !pred(el); });
		collection.erase(it, collection.end());
	}

	// 
	// Delete and erase all elements in the collection which satisfy the given predicate.  Does 
	// not guarantee that order of remaining elements will be preserved
	// 
	template <template <typename, typename> class TCollection,
		typename TElement,
		typename TAllocator = std::allocator<TElement>,
		typename _Pred>

	static void DeleteErase(typename TCollection<TElement, TAllocator> & collection, _Pred pred)
	{
		typename TCollection<TElement, TAllocator>::iterator it = std::partition(collection.begin(), collection.end(),
			[pred](auto & el) { return !pred(el); });
		DeleteErase(collection, it, collection.end());

	}

	//
	// Delete and erase all elements in the collection in the given iterator range.  Does not guarantee that
	// order of remaining elements will be preserved
	// 
	template <template <typename, typename> class TCollection,
		typename TElement,
		typename TAllocator = std::allocator<TElement>>

	static void DeleteErase(
			typename TCollection<TElement, TAllocator> & collection,
			typename TCollection<TElement, TAllocator>::iterator begin,
			typename TCollection<TElement, TAllocator>::iterator end)
	{
		std::for_each(begin, end, [](auto *el) { if (el) delete el; });
		collection.erase(begin, end);
	}

	//
	// Delete and erase the specified elements in the collection.  Does not guarantee that
	// order of remaining elements will be preserved
	// 
	template <template <typename, typename> class TCollection,
		typename TElement,
		typename TAllocator = std::allocator<TElement>>

	static void DeleteEraseElement(
		typename TCollection<TElement, TAllocator> & collection,
		typename TCollection<TElement, TAllocator>::iterator element)
	{
		if (*element) delete (*element);
		collection.erase(element);
	}

	// 
	// Delete and erase all value-wrapped elements in the collection which satisfy the 
	// given predicate.  Does not guarantee that order of remaining elements will be preserved
	// 
	template <template <typename, typename> class TCollection,
		typename TElement,
		typename TAllocator = std::allocator<TElement>,
		typename _Pred>

	static void DeleteEraseValue(typename TCollection<TElement, TAllocator> & collection, _Pred pred)
	{
		typename TCollection<TElement, TAllocator>::iterator it = std::partition(collection.begin(), collection.end(),
			[pred](auto & el) { return !pred(el); });
		DeleteEraseValue(collection, it, collection.end());

	}

	//
	// Delete and erase all value-wrapped elements in the collection in the given iterator 
	// range.  Does not guarantee that order of remaining elements will be preserved
	// 
	template <template <typename, typename> class TCollection,
		typename TElement,
		typename TAllocator = std::allocator<TElement>>

	static void DeleteEraseValue(
		typename TCollection<TElement, TAllocator> & collection,
		typename TCollection<TElement, TAllocator>::iterator begin,
		typename TCollection<TElement, TAllocator>::iterator end)
	{
		std::for_each(begin, end, [](auto & el) { if (el.value) delete el.value; });
		collection.erase(begin, end);
	}


};