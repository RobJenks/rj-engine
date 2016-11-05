#include "GameVarsExtern.h"
#include "UIManagedControlDefinition.h"
#include "CoreEngine.h"
#include "iUIControl.h"


// Initialises an image component of this control using default parameters
Result iUIControl::InitialiseImageComponentDefault(UIManagedControlDefinition *def, Image2DRenderGroup **component,
	string componentname, int instancecount, float zorder)
{
	// Create a new component object and set the unique code
	(*component) = new Image2DRenderGroup();
	(*component)->SetCode(concat(m_code)(".")(componentname).str());

	// Initialise the control using default engine parameters
	Result result = (*component)->Initialize(Game::Engine->GetDevice(), Game::ScreenWidth, Game::ScreenHeight,
		def->GetComponent(componentname).c_str(), Texture::APPLY_MODE::Normal);
	if (result != ErrorCodes::NoError) return result;

	// Set other properties
	(*component)->SetZOrder(zorder);
	(*component)->SetAcceptsMouseInput(true);

	// Add as many instances as required.  All are given default values to begin with; expectation is that instances are updated later
	Image2DRenderGroup::Instance *inst = NULL;
	for (int i = 0; i<instancecount; i++)
	{
		inst = (*component)->AddInstance(INTVECTOR2(i, i), zorder, INTVECTOR2(10, 10), true, Rotation90Degree::Rotate0);
		if (inst) inst->control = this;
	}

	// Return success
	return ErrorCodes::NoError;
}


iUIControl::Type iUIControl::DeriveType(string typestring)
{
	// Derive a control type based on a case-insensitive string comparison
	string s = StrLower(typestring);

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