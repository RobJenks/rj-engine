#pragma once

#include "Utility.h"
#include "FastMath.h"
#include "GameVarsExtern.h"
#include "LogManager.h"

template <typename T>
class PrecalculatedRandomSequence
{
public:

	// Static constants
	static const unsigned long				MAX_SEQUENCE_ALLOCATION = (unsigned long)(1e6 * 1e3);		// Max of 1m sequences, each of 1000 entries, as an approx sensible limit

	// Builds and returns a set of 'count' random integral sequences, with values in the range [low high).  All sequences will be of length (high-low).
	// If 'distinct' is set the sequences will contain each value in the range exactly once, otherwise this is not guaranteed
	PrecalculatedRandomSequence(T low, T high, unsigned int length, unsigned int count, bool distinct);

	// Builds and returns a set of 'count' random sequences, with values taken from the source array provided.  All sequences will be 
	// of length 'length'.  All sequences will contain each value in the source set exactly once
	PrecalculatedRandomSequence(const T *values, unsigned int length, unsigned int count);

	// Default constructor.  Will create an uninitialized sequence generator.  Implemented to allow lazy instantiation by consuming classes
	PrecalculatedRandomSequence(void);
	
	// Copy constructor
	PrecalculatedRandomSequence(const PrecalculatedRandomSequence & other);

	// Copy assignment operator
	PrecalculatedRandomSequence & operator=(const PrecalculatedRandomSequence & other);

	// Move assignment operator
	PrecalculatedRandomSequence & operator=(PrecalculatedRandomSequence && other);
	

	// Returns a flag indicating whether the sequence data was successfully initialised
	CMPINLINE bool							IsInitialised(void) const { return (m_data != NULL); }

	// Returns a pointer to the start of a randomly-selected sequence.  Guaranteed to return a non-null
	// pointer into the data array as long as the sequence object was successfully initialised
	const T *								RandomSequence(void) const;

	// Returns a pointer to the start of the sequence with the given index, or NULL if this is not a valid sequence
	const T *								GetSequence(unsigned int index) const;

	CMPINLINE unsigned int					GetSequenceLength(void) const					{ return m_length; }
	CMPINLINE unsigned int					GetSequenceCount(void) const					{ return m_count; }
	CMPINLINE unsigned long					GetTotalAllocatedElements(void) const			{ return m_total_alloc; }
	CMPINLINE T *							DataPtr(void) const								{ return m_data; }

	// Deallocates the sequence object and all associated memory
	~PrecalculatedRandomSequence();

protected:

	// Data is stored in one contiguous block for efficiency
	T *										m_data;

	// Allocation properties
	unsigned int							m_length;				// Length of each generated sequence
	unsigned int							m_count;				// The number of sequences generated
	unsigned long							m_total_alloc;			// Total number of elements in the allocated data

	// Sets the content of this object to null, in case of error or other issues
	void									NullData();

	// Allocates a contiguous block of space for 'count' sequences of size 'length'
	void									AllocateSequenceMemory(unsigned int length, unsigned int count);

	// Deallocates all owned memory
	void									DeallocateMemory(void);

	// Populates the dataset with randomly-generated sequences.  Sequences will contain a random selection of values
	// with each in the range [low high), and potentially with duplicate entries
	CMPINLINE void							PopulateSequenceData(T low, T high);

	// Populates the dataset with randomly-generated sequences.  Sequences will contain a random selection of values
	// with each in the range [low high), where every value is guaranteed to be unique.  In the case of integral 
	// types where (length == (high - low)), this will result in a set of randomly shuffled |length| sequences.  For
	// types (e.g. float) where the distinct property does not make sense this method regresses to the non-distinct version
	CMPINLINE void							PopulateSequenceDataDistinct(T low, T high);

	// Populates the dataset with random sequences of values in a given source set.  |source| == 'length', 
	// and each generated sequence will accordingly be of size 'length'
	CMPINLINE void							PopulateSequenceDataFromSource(const T *values, unsigned int length);


};


// Builds and returns a set of 'count' random integral sequences, with values in the range [low high).  All sequences will be of length (high-low).
// If 'distinct' is set the sequences will contain each value in the range exactly once, otherwise this is not guaranteed
template <typename T>
PrecalculatedRandomSequence<T>::PrecalculatedRandomSequence(T low, T high, unsigned int length, unsigned int count, bool distinct)
{
	// Set dataset to null in advance of any initialisation
	m_data = NULL;

	// Parameter check
	if (low >= high)
	{ 
		NullData(); return; 
	}

	// Allocate a contiguous block of space for the sequences
	AllocateSequenceMemory(length, count);
	if (m_data == NULL) { NullData(); return; }

	// Populate the sequences based on the provided parameters
	if (distinct)
	{
		PopulateSequenceDataDistinct(low, high);
	}
	else
	{
		PopulateSequenceData(low, high);
	}
}

// Builds and returns a set of 'count' random sequences, with values taken from the source array provided.  All sequences will be 
// of length 'length'.  All sequences will contain each value in the source set exactly once
template <typename T>
PrecalculatedRandomSequence<T>::PrecalculatedRandomSequence(const T *values, unsigned int length, unsigned int count)
{
	// Set dataset to null in advance of any initialisation
	m_data = NULL;

	// Parameter check
	if (values == NULL || length <= 0 || count <= 0)
	{
		NullData(); return;
	}

	// Allocate a contiguous block of space for the sequences
	AllocateSequenceMemory(length, count);
	if (m_data == NULL) { NullData(); return; }

	// Populate the sequences based on the provided parameters
	PopulateSequenceDataFromSource(values, length);
}

// Default constructor.  Will create an uninitialized sequence generator.  Implemented to allow lazy instantiation by consuming classes
template <typename T>
PrecalculatedRandomSequence<T>::PrecalculatedRandomSequence(void)
{
	m_data = NULL;
	NullData();
}

// Copy constructor
template <typename T>
PrecalculatedRandomSequence<T>::PrecalculatedRandomSequence(const PrecalculatedRandomSequence & other)
{
	AllocateSequenceMemory(other.GetSequenceLength(), other.GetSequenceCount());
	std::copy(other.DataPtr(), other.DataPtr() + m_total_alloc, m_data);
}

// Copy assignment operator
template <typename T>
PrecalculatedRandomSequence<T> & PrecalculatedRandomSequence<T>::operator=(const PrecalculatedRandomSequence<T> & other)
{
	if (this != &other)
	{
		if (m_data != NULL) DeallocateMemory();

		AllocateSequenceMemory(other.GetSequenceLength(), other.GetSequenceCount());
		std::copy(other.DataPtr(), other.DataPtr() + m_total_alloc, m_data);
	}
	return *this;
}

// Move assignment operator
template <typename T>
PrecalculatedRandomSequence<T> & PrecalculatedRandomSequence<T>::operator=(PrecalculatedRandomSequence<T> && other)
{
	if (this != &other)
	{
		if (m_data != NULL) DeallocateMemory();

		m_length = other.GetSequenceLength();
		m_count = other.GetSequenceCount();
		m_total_alloc = other.GetTotalAllocatedElements();
		m_data = other.DataPtr();

		other.m_data = NULL;		// To prevent deallocation by the other object during "NullData()"
		other.NullData();
	}
	return *this;
}


// Sets the content of this object to null, in case of error or other issues
template <typename T>
void PrecalculatedRandomSequence<T>::NullData()
{
	DeallocateMemory();

	m_data = NULL;
	m_length = m_count = 0U;
	m_total_alloc = 0UL;
}

// Deallocates all owned memory
template <typename T>
void PrecalculatedRandomSequence<T>::DeallocateMemory(void)
{
	if (m_data != NULL) SafeDeleteArray(m_data);
}
		

// Allocates a contiguous block of space for 'count' sequences of size 'length'
template <typename T>
void PrecalculatedRandomSequence<T>::AllocateSequenceMemory(unsigned int length, unsigned int count)
{
	// Validate that we are within the acceptable sequence size bounds
	unsigned long total_alloc = (length * count);
	if (total_alloc > PrecalculatedRandomSequence<T>::MAX_SEQUENCE_ALLOCATION)
	{
		NullData(); 
		return;
	}

	// Allocate the block of memory and store associated data
	m_data = new T[total_alloc];
	m_length = length;
	m_count = count;
	m_total_alloc = total_alloc;
}


// Populates the dataset with randomly-generated sequences.  Sequences will contain a random selection of values
// with each in the range [low high), and potentially with duplicate entries
template <typename T>
void PrecalculatedRandomSequence<T>::PopulateSequenceData(T low, T high)
{
	// This method MUST be specialised per type
	Game::Log << LOG_ERROR << "Attempted to precalculate random sequence of unsupported value type \"" << typeid(T).name() << "\"";
}

// Populates the dataset with randomly-generated sequences.  Sequences will contain a random selection of values
// with each in the range [low high), and potentially with duplicate entries
template <>
void PrecalculatedRandomSequence<int>::PopulateSequenceData(int low, int high)
{
	// We can actually populate the entire array independently of sequence size/count, since the only 
	// criteria here is to have non-unique random elements within each sequence
	int range = (high - low);
	for (unsigned long i = 0U; i < m_total_alloc; ++i)
	{
		// Instead of having irand_lh calculate (high - low) in every iteration, precalculate and then 
		// make the adjustment here ourselves for efficiency
		m_data[i] = (low + irand_h(range));
	}
}

// Populates the dataset with randomly-generated sequences.  Sequences will contain a random selection of values
// with each in the range [low high), and potentially with duplicate entries
template <>
void PrecalculatedRandomSequence<float>::PopulateSequenceData(float low, float high)
{
	// We can actually populate the entire array independently of sequence size/count, since the only 
	// criteria here is to have non-unique random elements within each sequence
	float range = (high - low);
	for (unsigned long i = 0U; i < m_total_alloc; ++i)
	{
		// Instead of having irand_lh calculate (high - low) in every iteration, precalculate and then 
		// make the adjustment here ourselves for efficiency
		m_data[i] = (low + frand_h(range));
	}
}



// Populates the dataset with randomly-generated sequences.  Sequences will contain a random selection of values
// with each in the range [low high), where every value is guaranteed to be unique.  In the case of integral 
// types where (length == (high - low)), this will result in a set of randomly shuffled |length| sequences.  For
// types (e.g. float) where the distinct property does not make sense this method regresses to the non-distinct version
template <typename T>
void PrecalculatedRandomSequence<T>::PopulateSequenceDataDistinct(T low, T high)
{
	// This method MUST be specialised per type
	Game::Log << LOG_ERROR << "Attempted to precalculate distinct random sequence of unsupported value type \"" << typeid(T).name() << "\"";
}


// Populates the dataset with randomly-generated sequences.  Sequences will contain a random selection of values
// with each in the range [low high), where every value is guaranteed to be unique.  In the case of integral 
// types where (length == (high - low)), this will result in a set of randomly shuffled |length| sequences.  For
// types (e.g. float) where the distinct property does not make sense this method regresses to the non-distinct version
template <>
void PrecalculatedRandomSequence<int>::PopulateSequenceDataDistinct(int low, int high)
{
	// Parameter check; make sure that we have a sufficient value range to fill the requested sequence length
	int src_range = (high - low);
	if (m_length > (unsigned int)src_range)
	{
		NullData(); return;
	}

	// Prepopulate one source array with the values that will be populated in each sequence
	int *source = new int[src_range];
	for (int i = 0; i < src_range; ++i) source[i] = (low + i);

	// Populate each sequence in turn.  Seq_ptr will point to the index of the start of each new sequence
	for (unsigned long seq_ptr = 0UL; seq_ptr < m_total_alloc; seq_ptr += m_length)
	{
		int src_ptr = src_range;
		for (unsigned int i = 0U; i < m_length; ++i)
		{
			// Select a random entry from the remaning source vector and assign it to the data array
			int index = irand_h(src_ptr);
			m_data[seq_ptr + i] = source[index];

			// Swap & (effectively) pop this entry off the end of the source vector so it is not selected again
			// We swap with source[src_ptr - 1] and then want to decrement the src ptr, so combine both 
			// as a reference to source[--src_ptr] for efficiency
			std::swap(source[index], source[--src_ptr]);
		}
	}

	// Delete any temporary allocated memory
	SafeDeleteArray(source);
}


// Populates the dataset with randomly-generated sequences.  Sequences will contain a random selection of values
// with each in the range [low high), where every value is guaranteed to be unique.  In the case of integral 
// types where (length == (high - low)), this will result in a set of randomly shuffled |length| sequences.  For
// types (e.g. float) where the distinct property does not make sense this method regresses to the non-distinct version
template <>
void PrecalculatedRandomSequence<float>::PopulateSequenceDataDistinct(float low, float high)
{
	// No reasonable use case for maintaining 'uniqueness' of floating point entries, so revert to 
	// non-distinct version 
	PopulateSequenceData(low, high);
}

// Populates the dataset with random sequences of values in a given source set.  |source| == 'length', 
// and each generated sequence will accordingly be of size 'length'
template <typename T>
void PrecalculatedRandomSequence<T>::PopulateSequenceDataFromSource(const T *values, unsigned int length)
{
	// We will first populate the data array with distinct integers from 0 to (length-1)
	PopulateSequenceDataDistinct((T)0, (T)length);

	// These values will then be used as indices into our source array
	for (unsigned long i = 0; i < m_total_alloc; ++i)
	{
		m_data[i] = values[m_data[i]];			// Replace current element with source[element]
	}
}


// Returns a pointer to the start of a randomly-selected sequence.  Guaranteed to return a non-null
// pointer into the data array as long as the sequence object was successfully initialised
template <typename T>
const T * PrecalculatedRandomSequence<T>::RandomSequence(void) const
{
	return (m_data + (irand_h(m_count) * m_length));
}

// Returns a pointer to the start of the sequence with the given index, or NULL if this is not a valid sequence
template <typename T>
const T * PrecalculatedRandomSequence<T>::GetSequence(unsigned int index) const
{
	if (index >= m_count) return NULL;
	return (m_data + (index * m_length));
}


// Deallocates the sequence object and all associated memory
template <typename T>
PrecalculatedRandomSequence<T>::~PrecalculatedRandomSequence()
{
	// Deallcoate the primary data array
	DeallocateMemory();
}