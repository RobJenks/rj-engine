#pragma once

#ifndef __DebugTestH__
#define __DebugTestH__

#include <sstream>
#include "CompilerSettings.h"
#include "Modifier.h"
#include "ModifiedValue.h"
#include "MValue.h"

CMPINLINE void TestModifiers(void)
{
	std::ostringstream sstrm;
	ModifiedValue<float> x = ModifiedValue<float>(10.0f);
	x.AddModifier(Modifier<float>::ModifierType::Additive, 1.0f);
	x.AddModifier(Modifier<float>::ModifierType::Multiplicative, 0.75f);
	x.AddModifier(Modifier<float>(Modifier<float>::ModifierType::Additive, 1.0f));
	x.AddModifier(Modifier<float>(Modifier<float>::ModifierType::Multiplicative, 0.75f));
	x.AddModifier(AdditiveModifier<float>(1.0f));
	x.AddModifier(MultiplicativeModifier<float>(0.75f));
	sstrm << "Result = " << x.Value << ", Actual = " << ((10.0f + 1.0f + 1.0f + 1.0f) * 0.75f * 0.75f * 0.75f) << "\n";
	x.RemoveModifierApprox(Modifier<float>(Modifier<float>::ModifierType::Additive, 1.0f));
	sstrm << "Result = " << x.Value << ", Actual = " << ((10.0f + 1.0f + 1.0f) * 0.75f * 0.75f * 0.75f) << "\n";
	x.RemoveAnyModifierApprox(Modifier<float>(Modifier<float>::ModifierType::Multiplicative, 0.75f));
	sstrm << "Result = " << x.Value << ", Actual = " << ((10.0f + 1.0f + 1.0f)) << "\n";
	x.AddModifier(MultiplicativeModifier<float>(0.5f));
	sstrm << "Result = " << x.Value << ", Actual = " << ((10.0f + 1.0f + 1.0f) * 0.5f) << "\n";
	x.RemoveModifiersOfType(Modifier<float>::ModifierType::Additive);
	sstrm << "Result = " << x.Value << ", Actual = " << ((10.0f) * 0.5f) << "\n";
	x.RemoveAllModifiers();
	sstrm << "Result = " << x.Value << ", Actual = " << ((10.0f)) << "\n";
	OutputDebugString(sstrm.str().c_str());
}

CMPINLINE void TestMValue(void)
{
	std::ostringstream sstrm;
	MValue<float>::New(10.0f);
	MValue<float>::AddModifier(Modifier<float>::ModifierType::Additive, 1.0f);
	MValue<float>::AddModifier(Modifier<float>::ModifierType::Multiplicative, 0.75f);
	MValue<float>::AddModifier(Modifier<float>(Modifier<float>::ModifierType::Additive, 1.0f));
	MValue<float>::AddModifier(Modifier<float>(Modifier<float>::ModifierType::Multiplicative, 0.75f));
	MValue<float>::AddModifier(AdditiveModifier<float>(1.0f));
	MValue<float>::AddModifier(MultiplicativeModifier<float>(0.75f));
	sstrm << "Result = " << MValue<float>::Value() << ", Actual = " << ((10.0f + 1.0f + 1.0f + 1.0f) * 0.75f * 0.75f * 0.75f) << "\n";
	MValue<float>::RemoveAdditiveModifier(1.0f);
	sstrm << "Result = " << MValue<float>::Value() << ", Actual = " << ((10.0f + 1.0f + 1.0f) * 0.75f * 0.75f * 0.75f) << "\n";
	MValue<float>::RemoveMultiplicativeModifier(0.75f);
	MValue<float>::RemoveMultiplicativeModifier(0.75f);
	MValue<float>::RemoveMultiplicativeModifier(0.75f);
	sstrm << "Result = " << MValue<float>::Value() << ", Actual = " << ((10.0f + 1.0f + 1.0f)) << "\n";
	MValue<float>::AddMultiplicativeModifier(0.5f);
	sstrm << "Result = " << MValue<float>::Value() << ", Actual = " << ((10.0f + 1.0f + 1.0f) * 0.5f) << "\n";
	MValue<float>::RemoveAdditiveModifier(1.0f);
	MValue<float>::RemoveAdditiveModifier(1.0f);
	sstrm << "Result = " << MValue<float>::Value() << ", Actual = " << ((10.0f) * 0.5f) << "\n";
	MValue<float>::RemoveMultiplicativeModifier(0.5f);
	sstrm << "Result = " << MValue<float>::Value() << ", Actual = " << ((10.0f)) << "\n";
	OutputDebugString(sstrm.str().c_str());
}




#endif





