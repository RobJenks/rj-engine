#pragma once

#ifndef __ActorAttributesH__
#define __ActorAttributesH__

#include <map>
#include "CompilerSettings.h"
#include "FastMath.h"
#include "ErrorCodes.h"
#include "GameDataExtern.h"
class TiXmlElement;

// Enumeration of actor attributes
enum ActorAttr
{
	// Basic attributes
	A_Accuracy = 0,			// Primary attribute
	A_Agility,				// Primary attribute
	A_Constitution,			// Primary attribute
	A_Will,					// Primary attribute

	// Basic properties
	A_BaseHealth,			// Affected by: Constitution

	// Movement properties
	A_RunSpeed,				// Affected by: Agility
	A_WalkSpeed,			// Affected by: RunSpeed
	A_StrafeSpeed,			// Affected by: RunSpeed
	
	// (Count of actor attributes)
	A_COUNT
};

// Methods and data relating to actor attribute generation
namespace ActorAttributeGeneration
{
	// Basic attribute generation data
	struct ActorAttributeGenerationParams 
	{ 
		float min, base, max;
		ActorAttributeGenerationParams(void) { min = 0.0f; base = 50.0f; max = 100.0f; }
	};
	extern ActorAttributeGenerationParams ActorAttributeParams[ActorAttr::A_COUNT];

	// Attribute effects on one another.  NOTE effects are applied in the order that they are loaded, important for (attr1 > effect on attr2) & (attr2 > effect on attr3)
	struct ActorAttributeEffect
	{
		ActorAttr source, target;			// Source and target attributes for this effect; source will have the calculated modifier effect on target.
		float atmin, atbase, atmax;			// Percentage of base modifier at each of the min, base and max.  Lerp between these points.

		ActorAttributeEffect(void) { atmin = 0.0f; atbase = 0.0f; atmax = 0.0f; }
		ActorAttributeEffect(ActorAttr _src, ActorAttr _tgt, float _atmin, float _atbase, float _atmax)
		{
			source = _src; target = _tgt;
			atmin = _atmin; atbase = _atbase; atmax = _atmax;
		}
	};
	extern std::vector<ActorAttributeEffect> ActorAttributeEffects;

	// Method to set up and load all attribute-related data
	extern void InitialiseActorAttributeData(void);
	extern Result LoadAttributeGenerationData(TiXmlElement *node);

	// String representation of each attribute, for lookup purposes in both directions
	static std::tr1::unordered_map<std::string, ActorAttr> _ActorAttr_lookup;
	static std::string _ActorAttr_names[(int)ActorAttr::A_COUNT];
}

// Methods to translate between attribute values and their string representations
extern ActorAttr TranslateActorAttributeFromString(const std::string & name);
extern std::string TranslateActorAttributeToStringName(ActorAttr attr);

// The methods that can be used to derive attribute values from the base values
enum AttributeDerivationType
{
	Uniform = 0,				// Default
	NormalDistribution
};

extern AttributeDerivationType TranslateAttributeDerivationTypeFromString(const std::string & name);

// Struct used to store base actor attributes.  Becomes the template type for the ActorAttributes collection
struct ActorBaseAttributeData
{
	float						BaseMinValue;		// Low end of the range that the base value can take
	float						BaseMaxValue;		// high end of the range that the base value can take

	float						MinBound;			// Value can go no lower than this, regardless of any modifiers etc.
	float						MaxBound;			// Value can go no higher than this, regardless of any modifiers etc.

	AttributeDerivationType		DerivationType;		// Method used to derive a value from the base values

	// Default constructor to set default values
	ActorBaseAttributeData(void) {	BaseMinValue = 1.0f; BaseMaxValue = 1.0f; 
									MinBound = 1.0f; MaxBound = 1.0f; DerivationType = AttributeDerivationType::Uniform; }

	// Method to generate a value based on our base values and derivation methods
	float Generate(void)
	{
		float result;

		// Take different action depending on derivation type
		if (DerivationType == AttributeDerivationType::NormalDistribution)
		{
			// Generate based on a normal distribution between the min & max base values
			result = NormalDistRange(BaseMinValue, BaseMaxValue);
		}
		else
		{
			// Default: generate a uniform random value between the min & max
			result = frand_lh(BaseMinValue, BaseMaxValue);
		}

		// Constrain to be within the min & max bounds, then return the value
		return (min(max(result, MinBound), MaxBound));
	}

};

// Struct used to store actor instance attributes.  Becomes the template type for the ActorAttributes collection
struct ActorInstanceAttributeData
{
	float						BaseValue;			// Base value inherited from the ActorBase object
	float						Value;				// Final value, once modifiers etc. have been applied for this actor instance
};

// Struct holding all the key actor attributes.  Used in multiple places to ensure consistency
template <typename T>
struct ActorAttributes
{
	// Array of attribute data
	T							_data[ActorAttr::A_COUNT];

	// Define the [] operator to return a member from the data array based on its index.  Compiler should optimise to constant memory offsets
	CMPINLINE T&				operator[] (ActorAttr attr) { return _data[(int)attr]; }
	CMPINLINE T&				operator[] (int attr)		{ return _data[attr]; }

	// Method to set the values of an attribute based on the string representation of its name
	void SetData(const std::string & name, const T & data)
	{
		ActorAttr attr = TranslateActorAttributeFromString(name);
		if (attr == ActorAttr::A_COUNT) return;
		
		_data[(int)attr] = data;
	}
};

// Define types for each templated version of the attribute collection
typedef			ActorAttributes<ActorBaseAttributeData>			ActorBaseAttributes;
typedef			ActorAttributes<ActorInstanceAttributeData>		ActorInstanceAttributes;


#endif