#ifndef __CopyObjectH__
#define __CopyObjectH__

// Creates and returns a copy of the specified object.  Performs a shallow copy-constructor copy before
// passing control to the hierarchy of post-copy methods which will handle any deep-copying or defaulting 
// required at each level in the hierarchy.  Class being copied must implement the PerformCopy(src) method.
template <typename T>
T * CopyObject(T *source)
{
	// Parameter check
	if (!source) return NULL;

	// Create an object copy using the default copy-constructor
	T *obj = new T(*source);

	// Call the hierarchy of post-copy methods for any data that needs to be deep-copied
	// or defaulted, instead of a shallow copy via the constructor
	obj->InitialiseCopiedObject(source);

	// Return a pointer to the new object
	return obj;
}






#endif