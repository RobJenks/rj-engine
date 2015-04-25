#include "XML\tinyxml.h"
#include "AdjustableParameter.h"

// Explicit specialisation of the ReadDataFromXML method for each type that it will read; specialisations should be added here as required.
template void AdjustableParameter<int>::ReadDataFromXML(TiXmlElement *el);
template void AdjustableParameter<float>::ReadDataFromXML(TiXmlElement *el);


// Method to read the parameter data from an XML node.  Integer version.
template<>
void AdjustableParameter<int>::ReadDataFromXML(TiXmlElement *node)
{
	// Parameter check
	if (!node) return;

	// Min value
	const char *cmin = node->Attribute("min");
	if (cmin) this->Min = atoi(cmin);

	// Max value
	const char *cmax = node->Attribute("max");
	if (cmax) this->Max = atoi(cmax);

	// Current value
	const char *cval = node->Attribute("value");
	if (cval) this->Value = atoi(cval);

	// Target value
	const char *ctgt = node->Attribute("target");
	if (ctgt) this->Target = atoi(ctgt);

	// Change rate
	const char *cchg = node->Attribute("changerate");
	if (cchg) this->ChangeRate = atoi(cchg);
}

// Method to read the parameter data from an XML node.  Float version.
template<>
void AdjustableParameter<float>::ReadDataFromXML(TiXmlElement *node)
{
	// Parameter check
	if (!node) return;

	// Min value
	const char *cmin = node->Attribute("min");
	if (cmin) this->Min = (float)atof(cmin);

	// Max value
	const char *cmax = node->Attribute("max");
	if (cmax) this->Max = (float)atof(cmax);

	// Current value
	const char *cval = node->Attribute("value");
	if (cval) this->Value = (float)atof(cval);

	// Target value
	const char *ctgt = node->Attribute("target");
	if (ctgt) this->Target = (float)atof(ctgt);

	// Change rate
	const char *cchg = node->Attribute("changerate");
	if (cchg) this->ChangeRate = (float)atof(cchg);
}
