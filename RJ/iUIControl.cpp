#include "GameVarsExtern.h"
#include "UIManagedControlDefinition.h"
#include "CoreEngine.h"
#include "Image2D.h"
#include "iUIControl.h"


// Method to initialise an image component for this control with default parameters
Image2D *iUIControl::InitialiseImageComponentDefault(UIManagedControlDefinition *def, const std::string & componentname, float zorder)
{
	// Create a new component object and set the unique code
	Image2D *component = new Image2D();
	component->SetCode(concat(m_code)(".")(componentname).str());

	// Set properties of the component
	component->SetMaterial(def->GetComponent(componentname));
	component->SetZOrder(zorder);
	component->SetAcceptsMouseInput(true);
	component->SetParentControl(this);

	// Return the new component
	return component;
}


iUIControl::Type iUIControl::DeriveType(std::string typestring)
{
	// Derive a control type based on a case-insensitive string comparison
	std::string s = StrLower(typestring);

	if (s == "button")						return iUIControl::Type::Button;
	else if (s == "textbox")				return iUIControl::Type::Textbox;
	else if (s == "combobox")				return iUIControl::Type::Combobox;

	else									return iUIControl::Type::Unknown;
}

// Generates a unique numeric control ID each time the method is called
int iUIControl::GenerateControlID(void)
{
	static int __CONTROL_ID_COUNTER = 0;
	return (__CONTROL_ID_COUNTER++);
}