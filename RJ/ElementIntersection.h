#pragma once

#ifndef __ElementIntersectionH__
#define __ElementIntersectionH__


#include <vector>
#include "CompilerSettings.h"

// Structure holding data on the intersection of an element
struct ElementIntersection
{
	int							ID;				// The element that was intersected
	float						StartTime;		// The clock time (secs) at which the intersection begins
	float						EndTime;		// The clock time (secs) at which the intersection ends
	float						Degree;			// The degree of intersection (in range [0.0-1.0]) with this element

	CMPINLINE ElementIntersection(void) : ID(0), StartTime(0U), EndTime(0U), Degree(0.0f) { }
	CMPINLINE ElementIntersection(int id, float startTime, float endTime, float degree) : ID(id), StartTime(startTime), EndTime(endTime), Degree(degree) { }
	CMPINLINE ElementIntersection(const ElementIntersection & other) : ID(other.ID), StartTime(other.StartTime), EndTime(other.EndTime), Degree(other.Degree) { }
	CMPINLINE ElementIntersection & operator=(const ElementIntersection & other) { ID = other.ID; StartTime = other.StartTime; EndTime = other.EndTime; Degree = other.Degree; }
};

// Custom collection of element intersection data
class ElementIntersectionData : public std::vector<ElementIntersection>
{
public:
	// Equality test for two sets of intersection data; compares only certain fields
	CMPINLINE bool EqualSequence(const ElementIntersectionData & other)
	{
		// Early-exit opportunity
		if (size() != other.size()) return false;

		// Make sure each element is identical
		ElementIntersectionData::const_iterator it, other_it;
		ElementIntersectionData::const_iterator it_end = end();
		ElementIntersectionData::const_iterator other_end = other.end();

		// Note: we don't get or compare b.end() since we know both vectors are of identical length
		for (it = begin(), other_it = other.begin(); it != it_end; ++it, ++other_it)
		{
			// We don't test full equality here (since the event times are always likely to differ slightly).  Instead
			// just test that the same IDs occur in the same sequence
			if ((*it).ID != (*other_it).ID) return false;
		}

		// All elements were identical so return success
		return true;
	}

	CMPINLINE ElementIntersectionData & operator=(const ElementIntersectionData & other)
	{
		// Clear all existing data
		clear();

		// Add all elements from the source collection
		ElementIntersectionData::const_iterator it_end = other.end();
		for (ElementIntersectionData::const_iterator it = other.begin(); it != it_end; ++it)
		{
			push_back(*it);
		}

		return *this;
	}
};




#endif


