#ifndef __AdjustableParameterH__
#define __AdjustableParameterH__

#include "GameVarsExtern.h"
class TiXmlElement;

// Adjustable parameters are templated classes, that can work on parameter which have defined standard (e.g. <, >) parameters
// This class has no special alignment requirements (as long as it is not used for alignment-requiring data)
template <typename T>
struct AdjustableParameter
{
	T					Value;				// The current value of this parameter
	T					Max;				// Maximum value of the parameter
	T					Min;				// Minimum value of the parameter
	T					Target;				// Current target value of the parameter
	T					ChangeRate;			// The amount that this parameter can change per second (should be an ABSOLUTE value)

	// Default constructor; does nothing
	AdjustableParameter(void) { }

	// Constructor allowing all initial parameters to be specified
	AdjustableParameter(T _min, T _max, T _currentvalue, T _currenttargetvalue, T _changerate)
	{
		Min = _min; Max = _max;
		Value = _currentvalue; Target = _currenttargetvalue;
		ChangeRate = _changerate;
	}

	// Performs an update of the parameter, moving the parameter value towards its target if applicable
	void Update(unsigned int delta_ms)
	{
		if (Target > Value)		Value = (T)min(Value + (float)min((Target - Value), (ChangeRate * ((float)delta_ms * 0.001f))), Max);
		else					Value = (T)max(Value - (float)min((Value - Target), (ChangeRate * ((float)delta_ms * 0.001f))), Min);
	}

	// Methods to determine whether the current value has reached its target.  Specialised as required.
	CMPINLINE bool				IsAtTarget(void) const;

	// Method to read the parameter data from an XML node.  Method must be implemented
	// per type being read, so the default templated method does nothing
	void						ReadDataFromXML(TiXmlElement *node) { }

	// Allow assignment of adjustable parameter values to one another
	CMPINLINE void operator=(const AdjustableParameter<T> & rhs)
	{
		Min = rhs.Min; Max = rhs.Max;
		Value = rhs.Value; Target = rhs.Target;
		ChangeRate = rhs.ChangeRate;
	}
};

// Methods to determine whether the current value has reached its target.  Specialised as required.
template<typename T>		
CMPINLINE bool AdjustableParameter<T>::IsAtTarget(void) const
{
	// By default, do a simple equality check
	return (Value == Target);
}

// Specialised method to compare floats, with margin for floating point innaccuracy
template<> 
CMPINLINE bool AdjustableParameter<float>::IsAtTarget(void) const
{
	// Allow margin for floating-point innaccuracy
	return (fabs(Value - Target) < Game::C_EPSILON);
}





#endif





