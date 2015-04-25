#include "iUIControl.h"

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