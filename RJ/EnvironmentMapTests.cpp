#include "TestError.h"
#include "EnvironmentMap.h"
#include "EnvironmentMapFalloffMethod.h"
#include "EnvironmentMapBlendMode.h"
#include "ComplexShip.h"
#include "ComplexShipSection.h"
#include "ComplexShipElement.h"

#include "EnvironmentMapTests.h"


TestResult EnvironmentMapTests::BasicInitialisationTests()
{
	TestResult result = NewResult();

	EnvironmentMap<int, EnvironmentMapBlendMode::BlendAdditive> intmap = EnvironmentMap<int, EnvironmentMapBlendMode::BlendAdditive>(INTVECTOR3(5, 5, 5));
	EnvironmentMap<float, EnvironmentMapBlendMode::BlendAdditive> floatmap = EnvironmentMap<float, EnvironmentMapBlendMode::BlendAdditive>(INTVECTOR3(5, 5, 5));
	result.AssertEqual((int)intmap.Data.size(), (5 * 5 * 5), ERR("Incorrect allocation of integer map"));
	result.AssertEqual((int)floatmap.Data.size(), (5 * 5 * 5), ERR("Incorrect allocation of float map"));

	intmap.SetBlockingProperties(0U);
	floatmap.SetTransmissionProperties(1U);
	
	intmap.SetFalloffMethod(EnvironmentMapFalloffMethod<int>()
		.WithAbsoluteFalloff(1)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<int>::FalloffTransmissionType::Distance)
	);
	floatmap.SetFalloffMethod(EnvironmentMapFalloffMethod<float>()
		.WithRelativeFalloff(0.5f)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<float>::FalloffTransmissionType::Square)
	);

	return result;
}

TestResult EnvironmentMapTests::FalloffMethodTests()
{
	TestResult result = NewResult();

	EnvironmentMapFalloffMethod<int> intfalloff = EnvironmentMapFalloffMethod<int>()
		.WithAbsoluteFalloff(2)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<int>::FalloffTransmissionType::Square);

	result.AssertEqual(intfalloff.ApplyFalloff(10, Direction::Right), (10 - 2), ERR("Absolute integer square falloff method incorrectly applied to adjacent"));
	result.AssertEqual(intfalloff.ApplyFalloff(10, Direction::UpLeft), (10 - 2), ERR("Absolute integer square falloff method incorrectly applied to diagonal"));
	intfalloff.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<int>::FalloffTransmissionType::Distance);
	result.AssertEqual(intfalloff.ApplyFalloff(10, Direction::UpLeft), (10 - (int)(2 * ROOT2)), ERR("Absolute integer distance falloff method incorrectly applied to diagonal"));
	

	EnvironmentMapFalloffMethod<float> floatfalloff = EnvironmentMapFalloffMethod<float>()
		.WithRelativeFalloff(0.5f)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<float>::FalloffTransmissionType::Square);

	result.AssertEqual(floatfalloff.ApplyFalloff(10.0f, Direction::Right), (10.0f * 0.5f), ERR("Relative float square falloff method incorrectly applied to adjacent"));
	result.AssertEqual(floatfalloff.ApplyFalloff(10.0f, Direction::UpLeft), (10.0f * 0.5f), ERR("Relative float square falloff method incorrectly applied to diagonal"));
	floatfalloff.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<float>::FalloffTransmissionType::Distance);
	result.AssertEqual(floatfalloff.ApplyFalloff(10.0f, Direction::UpLeft), (10.0f * (1.0f - (0.5f * ROOT2))), ERR("Relative float distance falloff method incorrectly applied to diagonal"));
	
	return result;
}

TestResult EnvironmentMapTests::BlendModeTests()
{
	TestResult result = NewResult();

	EnvironmentMapBlendMode::BlendAdditive<int> intblend;
	intblend.SetInitialValue(10);
	result.AssertEqual(intblend.Apply(10, 5), 15, ERR("Additive integer blending failed"));
	result.AssertEqual(intblend.Apply(15, -5), 10, ERR("Additive integer blending failed"));

	EnvironmentMapBlendMode::BlendAdditive<float> float_add;
	EnvironmentMapBlendMode::BlendMultiplicative<float> float_mult;
	EnvironmentMapBlendMode::BlendMinimumValue<float> float_minv;
	EnvironmentMapBlendMode::BlendMaximumValue<float> float_maxv;
	EnvironmentMapBlendMode::BlendReplaceDestination<float> float_repl;
	EnvironmentMapBlendMode::BlendIgnoreNewValue<float> float_ignore;
	float_ignore.SetInitialValue(2.0f);

	result.AssertEqual(float_add.Apply(2.0f, 4.0f), (2.0f + 4.0f), ERR("Additive float blending failed"));
	result.AssertEqual(float_mult.Apply(2.0f, 4.0f), (2.0f * 4.0f), ERR("Mutliplicative float blending failed"));
	result.AssertEqual(float_minv.Apply(2.0f, 4.0f), 2.0f, ERR("MinValue float blending failed"));
	result.AssertEqual(float_maxv.Apply(2.0f, 4.0f), 4.0f, ERR("MaxValue float blending failed"));
	result.AssertEqual(float_repl.Apply(2.0f, 4.0f), 4.0f, ERR("Replace destination float blending failed"));
	result.AssertEqual(float_repl.Apply(4.0f, 2.0f), 2.0f, ERR("Replace destination float blending failed"));
	result.AssertEqual(float_ignore.Apply(2.0f, 4.0f), 4.0f, ERR("Ignore new float blending failed")); 
	result.AssertEqual(float_ignore.Apply(3.0f, 4.0f), 3.0f, ERR("Ignore new float blending failed"));
	result.AssertEqual(float_ignore.Apply(4.0f, 2.0f), 4.0f, ERR("Ignore new float blending failed"));

	return result;
}

TestResult EnvironmentMapTests::BasicAdditivePropogationTests()
{
	TestResult result = NewResult();

	INTVECTOR3 size = INTVECTOR3(5, 5, 5);
	EnvironmentMap<int, EnvironmentMapBlendMode::BlendAdditive> map = EnvironmentMap<int, EnvironmentMapBlendMode::BlendAdditive>(size);
	map.SetZeroThreshold(0);
	map.SetBlockingProperties(0U);
	map.SetFalloffMethod(EnvironmentMapFalloffMethod<int>()
		.WithAbsoluteFalloff(2)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<int>::FalloffTransmissionType::Distance));
	
	auto env = GenerateTestElementEnvironment(size);
	
	map
		.BeginUpdate()
		.WithInitialValues(1)
		.WithSourceCell(EnvironmentMap<int, EnvironmentMapBlendMode::BlendAdditive>::MapCell(env->GetElementIndex(INTVECTOR3(3, 3, 3)), 10))
		.Execute(env->GetElements());

	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 3, 3))), 10, ERR("Source cell not initialised correctly in additive propogation test"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 3, 3))), (1 + (10 - 2)), ERR("Single-cell additive propogation failed"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 2, 3))), (1 + (10 - (int)(2 * ROOT2))), ERR("Single-cell diagonal additive propogation failed"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 1, 3))), (1 + ((1 + (10 - 2)) - 2)), ERR("Multi-cell additive propogation failed"));

	OutputDebugString(concat("\Environment map additive propogation tests:\n")(map.DebugStringOutput("%d"))("\n").str().c_str());
	return result;
}

TestResult EnvironmentMapTests::BasicMultiplicativePropogationTests()
{
	TestResult result = NewResult();

	INTVECTOR3 size = INTVECTOR3(5, 5, 5);
	EnvironmentMap<float, EnvironmentMapBlendMode::BlendMultiplicative> map = EnvironmentMap<float, EnvironmentMapBlendMode::BlendMultiplicative>(size);
	map.SetZeroThreshold(0);
	map.SetBlockingProperties(0U);
	map.SetFalloffMethod(EnvironmentMapFalloffMethod<float>()
		.WithAbsoluteFalloff(2.0f)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<float>::FalloffTransmissionType::Distance));

	auto env = GenerateTestElementEnvironment(size);

	map
		.BeginUpdate()
		.WithInitialValues(2.0f)
		.WithSourceCell(EnvironmentMap<float, EnvironmentMapBlendMode::BlendMultiplicative>::MapCell(env->GetElementIndex(INTVECTOR3(3, 3, 3)), 10.0f))
		.Execute(env->GetElements());

	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 3, 3))), 10.0f, ERR("Source cell not initialised correctly in multiplictive propogation test"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 3, 3))), (2.0f * (10.0f - 2.0f)), ERR("Single-cell multiplictive propogation failed"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 2, 3))), (2.0f * (10.0f - (2.0f * ROOT2))), ERR("Single-cell diagonal multiplictive propogation failed"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 1, 3))), (2.0f * ((2.0f * (10.0f - 2.0f)) - 2.0f)), ERR("Multi-cell multiplictive propogation failed"));

	OutputDebugString(concat("\Environment map multiplicative propogation tests:\n")(map.DebugStringOutput("%.1f"))("\n").str().c_str());
	return result;
}

TestResult EnvironmentMapTests::BasicAveragedPropogationTests()
{
	TestResult result = NewResult();

	INTVECTOR3 size = INTVECTOR3(5, 5, 5);
	EnvironmentMap<float, EnvironmentMapBlendMode::BlendAveraged> map = EnvironmentMap<float, EnvironmentMapBlendMode::BlendAveraged>(size);
	map.SetZeroThreshold(0);
	map.SetBlockingProperties(0U);
	map.SetFalloffMethod(EnvironmentMapFalloffMethod<float>()
		.WithAbsoluteFalloff(2.0f)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<float>::FalloffTransmissionType::Distance));

	auto env = GenerateTestElementEnvironment(size);

	map
		.BeginUpdate()
		.WithInitialValues(2.0f)
		.WithSourceCell(EnvironmentMap<float, EnvironmentMapBlendMode::BlendAveraged>::MapCell(env->GetElementIndex(INTVECTOR3(3, 3, 3)), 10.0f))
		.Execute(env->GetElements());

	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 3, 3))), 10.0f, ERR("Source cell not initialised correctly in averaged propogation test"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 3, 3))), 0.5f * (2.0f + (10.0f - 2.0f)), ERR("Single-cell averaged propogation failed"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 2, 3))), 0.5f * (2.0f + (10.0f - (2.0f * ROOT2))), ERR("Single-cell diagonal averaged propogation failed"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 1, 3))), 0.5f * (2.0f + ((0.5f * (2.0f + (10.0f - 2.0f)))-2.0f)), ERR("Multi-cell averaged propogation failed"));

	OutputDebugString(concat("\Environment map averaged propogation tests:\n")(map.DebugStringOutput("%.1f"))("\n").str().c_str());
	return result;
}

TestResult EnvironmentMapTests::BasicRelativeFalloffPropogationTests()
{
	TestResult result = NewResult();

	INTVECTOR3 size = INTVECTOR3(5, 5, 5);
	EnvironmentMap<float, EnvironmentMapBlendMode::BlendAdditive> map = EnvironmentMap<float, EnvironmentMapBlendMode::BlendAdditive>(size);
	map.SetZeroThreshold(0);
	map.SetBlockingProperties(0U);
	map.SetFalloffMethod(EnvironmentMapFalloffMethod<float>()
		.WithRelativeFalloff(0.25f)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<float>::FalloffTransmissionType::Distance));

	auto env = GenerateTestElementEnvironment(size);

	map
		.BeginUpdate()
		.WithInitialValues(2.0f)
		.WithSourceCell(EnvironmentMap<float, EnvironmentMapBlendMode::BlendAdditive>::MapCell(env->GetElementIndex(INTVECTOR3(3, 3, 3)), 10.0f))
		.Execute(env->GetElements());

	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 3, 3))), 10.0f, ERR("Source cell not initialised correctly in relative falloff propogation test"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 3, 3))), 2.0f + (10.0f * (1.0f - 0.25f)), ERR("Single-cell relative falloff propogation failed"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 2, 3))), 2.0f + (10.0f * (1.0f - (0.25f * ROOT2))), ERR("Single-cell diagonal relative falloff propogation failed"));
	result.AssertEqual(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 1, 3))), 2.0f + ((1.0f-0.25f) * (2.0f + (10.0f * (1.0f-0.25f)))), ERR("Multi-cell relative falloff propogation failed"));

	OutputDebugString(concat("\nEnvironment map relative falloff propogation tests:\n")(map.DebugStringOutput("%.1f"))("\n").str().c_str());
	return result;
}

std::unique_ptr<ComplexShip> EnvironmentMapTests::GenerateTestElementEnvironment(const INTVECTOR3 & size)
{
	ComplexShipSection *sec = new ComplexShipSection();
	sec->ResizeSection(size);
	sec->DefaultElementState.ApplyDefaultElementState(ElementStateDefinition::ElementState(ComplexShipElement::PROPERTY::PROP_ACTIVE));

	ComplexShip *env = new ComplexShip();
	env->InitialiseElements(size);
	env->AddShipSection(sec);
	env->UpdateEnvironment();

	int n = env->GetElementCount();
	for (int i = 0; i < n; ++i)
		env->GetElementDirect(i).SetProperty(ComplexShipElement::PROPERTY::PROP_ACTIVE);
	env->UpdateEnvironment();

	return std::unique_ptr<ComplexShip>(env);
}


