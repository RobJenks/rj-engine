#pragma once

#ifndef __StandardModifiersH__
#define __StandardModifiersH__

#include "CompilerSettings.h"
#include "Modifiers.h"

namespace StandardModifiers
{

	static unsigned int Count = 0;
#	define DMod(id) static const unsigned int id = Count++;
#	include "StandardModifiers.list"
#	undef DMod


	static void InitialiseStandardModifiers(void)
	{
		if (!Modifiers::AllModifiers.Empty()) return;
		
#		define DMod(id) Modifiers::AddWithID(StandardModifiers::id, #id, #id);
#		include "StandardModifiers.list"
#		undef DMod
	}


};






#endif