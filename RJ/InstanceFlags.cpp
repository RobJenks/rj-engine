#include "InstanceFlags.h"

/*
	Set flag state.  Also sets the IsSpecified flag to show that the value has been manually specified
*/
void InstanceFlags::SetFlag(InstanceFlags::Type flag)
{
	SetBit(Flags, flag);
	SetBit(Specified, flag);
}

void InstanceFlags::ClearFlag(InstanceFlags::Type flag)
{
	ClearBit(Flags, flag);
	SetBit(Specified, flag);
}

void InstanceFlags::SetFlagState(InstanceFlags::Type flag, bool value)
{
	SetBitState(Flags, flag, value);
	SetBit(Specified, flag);
}

/*
	Reset the flag to its default value, and clear the IsSpecified flag to show that it has not been manually specified
*/
void InstanceFlags::ResetFlag(InstanceFlags::Type flag)
{
	SetBitState(Flags, flag, CheckBit_Single(Flags, flag));	// TODO: can probably do this via bitwise ops directly
	ClearBit(Specified, flag);
}

/*
	Constructors
*/
InstanceFlags::InstanceFlags(void)
	:
	Flags(InstanceFlags::DEFAULT_INSTANCE_FLAGS),
	Specified(0U)
{ }

InstanceFlags::InstanceFlags(InstanceFlags::Type flags, InstanceFlags::Type is_specified)
	:
	Flags(flags),
	Specified(is_specified)
{ }

InstanceFlags::InstanceFlags(std::vector<std::tuple<InstanceFlags::Type, bool>> default_flags)
	:
	InstanceFlags()
{
	for (const auto & entry : default_flags)
	{
		SetFlagState(std::get<0>(entry), std::get<1>(entry));
	}
}


/*
	Static translation from flag name to actual bitwise value
*/
InstanceFlags::Type InstanceFlags::ParseFromName(HashVal hashed_name)
{
	if (hashed_name == HashedStrings::H_ShadowCaster)		return INSTANCE_FLAG_SHADOW_CASTER;
	/// else if ...

	return INSTANCE_FLAG_NONE;
}





