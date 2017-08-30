#include <string>
#include <map>
#include "ErrorCodes.h"
#include "Utility.h"
#include "XML\\tinyxml.h"
#include "Modifiers.h"
#include "ModifierDetails.h"
#include "ActorAttributes.h"
using namespace ActorAttributeGeneration;

// Externally-defined collections
ActorAttributeGenerationParams ActorAttributeGeneration::ActorAttributeParams[ActorAttr::A_COUNT];
std::vector<ActorAttributeEffect> ActorAttributeGeneration::ActorAttributeEffects;

AttributeDerivationType TranslateAttributeDerivationTypeFromString(const std::string & name)
{
	std::string lcase = StrLower(name);
	if (name == "normal")				return AttributeDerivationType::NormalDistribution;
	else								return AttributeDerivationType::Uniform;
}

void ActorAttributeGeneration::InitialiseActorAttributeData(void)
{
	// Store the readable string representation of each attribute value 
	for (int i=0; i<(int)ActorAttr::A_COUNT; i++) _ActorAttr_names[i] = "";
	_ActorAttr_names[ActorAttr::A_Accuracy] = "Accuracy";
	_ActorAttr_names[ActorAttr::A_Agility] = "Agility";
	_ActorAttr_names[ActorAttr::A_BaseHealth] = "Base Health";
	_ActorAttr_names[ActorAttr::A_Constitution] = "Constitution";
	_ActorAttr_names[ActorAttr::A_RunSpeed] = "Run Speed";
	_ActorAttr_names[ActorAttr::A_StrafeSpeed] = "Strafe Speed";
	_ActorAttr_names[ActorAttr::A_WalkSpeed] = "Walk Speed";
	_ActorAttr_names[ActorAttr::A_Will] = "Will";

	// Create the inverse map from lookup string values to the numeric attribute index
	for (int i=0; i<(int)ActorAttr::A_COUNT; i++)
	{
		// Convert to lower case for lookup purposes, assuming we have a valid string
		if (_ActorAttr_names[i] == NullString) continue;
		std::string lookup = StrLower(_ActorAttr_names[i]);
		
		// Insert an entry in the lookup map for this attribute
		_ActorAttr_lookup[lookup] = (ActorAttr)i;
	}
}

// Loads attribute generation data from file
Result ActorAttributeGeneration::LoadAttributeGenerationData(TiXmlElement *node)
{
	std::string key, val;

	// Parameter check
	if (!node) return ErrorCodes::CannotLoadAttributeGenerationDataWithNullData;
	
	// Look at each child element in turn and pull data from them
	TiXmlElement *child = node->FirstChildElement();
	for (child; child; child=child->NextSiblingElement())
	{
		// Extract data from the node
		key = child->Value(); StrLowerC(key);

		if (key == "attributerange") {
			// Pull data from the relevant attributes
			const char *cname = child->Attribute("name");		
			const char *cmin = child->Attribute("min");			
			const char *cbase = child->Attribute("base");
			const char *cmax = child->Attribute("max");
			if (!cname || !cmin || !cbase || !cmax) continue;

			// Attempt to convert to the relevant in-game attribute
			std::string sname = cname;
			ActorAttr a = TranslateActorAttributeFromString(sname);
			if (a == ActorAttr::A_COUNT) continue;

			// Convert the range values for this attribute and then validate; make sure that min <= base <= max
			float vmin = (float)atof(cmin); float vbase = (float)atof(cbase); float vmax = (float)atof(cmax);
			if (vmin > vbase || vbase > vmax) continue;

			// Store the range values for this attribute
			ActorAttributeParams[(int)a].min = vmin;
			ActorAttributeParams[(int)a].base = vbase;
			ActorAttributeParams[(int)a].max = vmax;

		}
		else if (key == "attributeeffect") {
			// Pull data from the relevant attributes
			const char *csrc = child->Attribute("source");	
			const char *ctgt = child->Attribute("target");
			const char *atmin = child->Attribute("at_min");
			const char *atbase = child->Attribute("at_base");
			const char *atmax = child->Attribute("at_max");
			const char *cmodifier = child->Attribute("modifier");
			if (!csrc || !ctgt || !atmin || !atmax) continue;
			if (!atbase) atbase = "0.0";
			if (!cmodifier) cmodifier = "0";
			std::string modifier = cmodifier;

			// Attempt to translate both the source and target attributes to in-game values
			ActorAttr asrc = TranslateActorAttributeFromString(csrc);
			ActorAttr atgt = TranslateActorAttributeFromString(ctgt);
			if (asrc == ActorAttr::A_COUNT || atgt == ActorAttr::A_COUNT) continue;

			// Store this effect as a new entry in the attribute effects table
			ActorAttributeEffects.push_back(
				ActorAttributeEffect(asrc, atgt, (float)atof(atmin), (float)atof(atbase), (float)atof(atmax), 
				Modifiers::Get(modifier).GetID()));
		}
	}

	// Return success
	return ErrorCodes::NoError;
}

CMPINLINE ActorAttr TranslateActorAttributeFromString(const std::string & name)
{
	std::string lookup = StrLower(name);
	if (_ActorAttr_lookup.count(lookup) == 0) return ActorAttr::A_COUNT;

	return _ActorAttr_lookup[lookup];
}

CMPINLINE std::string TranslateActorAttributeToStringName(ActorAttr attr)
{
	return _ActorAttr_names[(int)attr];
}

