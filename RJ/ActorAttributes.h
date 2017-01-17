#pragma once

#ifndef __ActorAttributesH__
#define __ActorAttributesH__

#include <map>
#include "CompilerSettings.h"
#include "FastMath.h"
#include "ErrorCodes.h"
#include "GameDataExtern.h"
#include "ModifiedValue.h"
#include "StandardModifiers.h"
class TiXmlElement;

// Enumeration of actor attributes
enum ActorAttr
{
	// Basic attributes
	A_Accuracy = 0,			// Primary attribute
	A_Agility,				// Primary attribute
	A_Constitution,			// Primary attribute
	A_Will,					// Primary attribute

	// Properties
	A_BaseHealth,			// Affected by: Constitution
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
	// This class has no special alignment requirements
	struct ActorAttributeGenerationParams 
	{ 
		float min, base, max;
		ActorAttributeGenerationParams(void) { min = 0.0f; base = 50.0f; max = 100.0f; }
	};
	extern ActorAttributeGenerationParams ActorAttributeParams[ActorAttr::A_COUNT];

	// Attribute effects on one another.  NOTE effects are applied in the order that they are loaded, important for (attr1 > effect on attr2) & (attr2 > effect on attr3)
	// This class has no special alignment requirements
	struct ActorAttributeEffect
	{
		ActorAttr source, target;				// Source and target attributes for this effect; source will have the calculated modifier effect on target.
		float atmin, atbase, atmax;				// Percentage of base modifier at each of the min, base and max.  Lerp between these points.
		ModifierDetails::ModifierID modifier;	// Reference to the modifier that this represents

		ActorAttributeEffect(void) : atmin(0.0f), atbase(0.0f), atmax(0.0f), modifier(StandardModifiers::NO_MODIFIER) { }
		ActorAttributeEffect(ActorAttr _src, ActorAttr _tgt, float _atmin, float _atbase, float _atmax, ModifierDetails::ModifierID _modifier)
			: source(_src), target(_tgt), atmin(_atmin), atbase(_atbase), atmax(_atmax), modifier(_modifier)
		{
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
	Fixed,					// No derivation; value is simply specified
	Uniform,				// Uniform random value within a range
	NormalDistribution		// Normally-distributed value within a range
};

extern AttributeDerivationType TranslateAttributeDerivationTypeFromString(const std::string & name);

// Struct used to store base actor attributes.  Becomes the template type for the ActorAttributes collection
// This class has no special alignment requirements
struct ActorBaseAttributeData
{
	// We either specify a value directly (FixedValue) or the parameters required in
	// order to generate the parameter (DerivationType, GenerateMin, GenerateMax)

	// This attribute is never permitted to be outside the bounds [MinBound MaxBound]
	float						MinBound;			// Value can go no lower than this, regardless of any modifiers etc.
	float						MaxBound;			// Value can go no higher than this, regardless of any modifiers etc.

	AttributeDerivationType		DerivationType;		// Method used to derive a value from the base values

	float						FixedBaseValue;		// Fixed value for this attribute if DerivationType == Fixed

	// We generate a new base attribute value in the range [BaseMinValue BaseMaxValue)
	float						GenerateMin;		// Low end of the range that the base value can take
	float						GenerateMax;		// high end of the range that the base value can take

	

	// Default constructor to set default values
	ActorBaseAttributeData(void) {	MinBound = 1.0f; MaxBound = 1.0f; DerivationType = AttributeDerivationType::NormalDistribution; }

	// Method to generate a value based on our base values and derivation methods
	float Generate(void)
	{
		float result;

		// Take different action depending on derivation type
		if (DerivationType == AttributeDerivationType::NormalDistribution)
		{
			// Generate based on a normal distribution between the min & max base values
			result = NormalDistRange(GenerateMin, GenerateMax);
		}
		else if (DerivationType == AttributeDerivationType::Uniform)
		{
			// Default: generate a uniform random value between the min & max
			result = frand_lh(GenerateMin, GenerateMax);
		}
		else
		{
			result = FixedBaseValue;
		}

		// Constrain to be within the min & max bounds, then return the value
		return clamp(result, MinBound, MaxBound);
	}

};

// Struct holding all the key actor attributes.  Used in multiple places to ensure consistency
// This class has no special alignment requirements
template <typename T>
struct ActorAttributes
{
	// Array of attribute data
	T							_data[ActorAttr::A_COUNT];

	// Define the [] operator to return a member from the data array based on its index.  Compiler should optimise to constant memory offsets
	CMPINLINE const T &			operator[] (ActorAttr attr) const	{ return _data[(int)attr]; }
	CMPINLINE T&				operator[] (ActorAttr attr)			{ return _data[(int)attr]; }
	CMPINLINE const T &			operator[] (int attr) const			{ return _data[attr]; }
	CMPINLINE T&				operator[] (int attr)				{ return _data[attr]; }

	// Method to set the values of an attribute based on the string representation of its name
	void SetData(const std::string & name, const T & data)
	{
		ActorAttr attr = TranslateActorAttributeFromString(name);
		if (attr == ActorAttr::A_COUNT) return;
		
		_data[(int)attr] = data;
	}
};

// Type used for generation of all base attributes
typedef			ActorAttributes<ActorBaseAttributeData>			ActorBaseAttributes;

// Type that holds all base & modified actor attributes
typedef			ActorAttributes<ModifiedValue<float>>			ActorInstanceAttributes;


#endif