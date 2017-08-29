#pragma once

#include <vector>
#include "RM_ModelDataCollection.h"

class RenderQueue : public std::vector<RM_ModelDataCollection>
{
public:

	// Default constructor
	CMPINLINE RenderQueue(void) { }

	// Constructor that will pre-initialise the given number of elements
	RenderQueue(const size_type count);

	// Retrieve an element from the queue; no bounds checking
	CMPINLINE RM_ModelDataCollection &			Get(size_type index) { return operator[](index); }

	// Assign this instance to the given model buffer
	void										RegisterModelBuffer(size_t shader, size_t slot, ModelBuffer *model);

	// Clears the specified model data following a successful render, unregistering any 
	// relevant model buffer data and returning to a state ready for next frame
	void										UnregisterModelBuffer(size_t shader, size_t slot);




};

