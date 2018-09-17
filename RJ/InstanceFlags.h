#pragma once

#include "../Definitions/CppHLSLLocalisation.hlsl.h"
#include "Utility.h"
#include "HashFunctions.h"

class InstanceFlags
{
public:

	// 32-bit flag type.  Note this must match the RM_Instance definition
	typedef _uint32				Type;

	// Instance flags
	static const Type			INSTANCE_FLAG_NONE					= 0;
	static const Type			INSTANCE_FLAG_SHADOW_CASTER			= (1 << 0);

	
	// Default instance flags
	static const Type			DEFAULT_INSTANCE_FLAGS				= INSTANCE_FLAG_NONE;


	/* Instance data*/
	InstanceFlags::Type		Flags;				// Value of all instance flags
	InstanceFlags::Type		Specified;			// Indicates whether each flag was loaded, or is just defaulted

	/*
		Set flag state.  Also sets the IsSpecified flag to show that the value has been manually specified
	*/
	void			SetFlag(InstanceFlags::Type flag);
	void			ClearFlag(InstanceFlags::Type flag);
	void			SetFlagState(InstanceFlags::Type flag, bool value);

	/*
		Reset the flag to its default value, and clear the IsSpecified flag to show that it has not been manually specified
	*/
	void			ResetFlag(InstanceFlags::Type flag);


	/* 
		Constructors
	*/
	InstanceFlags(void);
	InstanceFlags(InstanceFlags::Type flags, InstanceFlags::Type is_specified);
	InstanceFlags(std::vector<std::tuple<InstanceFlags::Type, bool>> default_flags);

	/*
		Retrieve flag data
	*/
	CMPINLINE bool	GetFlag(InstanceFlags::Type flag) const
	{
		return CheckBit_Single(Flags, flag);
	}

	CMPINLINE bool	IsSpecified(InstanceFlags::Type flag) const
	{
		return CheckBit_Single(Specified, flag);
	}
	
	/*
		Static translation from flag name to actual bitwise value
	*/
	static Type ParseFromName(HashVal hashed_name);




};