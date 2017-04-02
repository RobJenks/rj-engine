#include "TestError.h"
#include "EnvironmentMap.h"
#include "EnvironmentMapFalloffMethod.h"
#include "EnvironmentMapBlendMode.h"
#include "ComplexShip.h"
#include "ComplexShipElement.h"

#include "EnvironmentMapTests.h"


TestResult EnvironmentMapTests::BasicInitialisationTests()
{
	TestResult result;

	EnvironmentMap<int, EnvironmentMapBlendMode::BlendAdditive> intmap = EnvironmentMap<int, EnvironmentMapBlendMode::BlendAdditive>(INTVECTOR3(5, 5, 5));
	EnvironmentMap<float, EnvironmentMapBlendMode::BlendAdditive> floatmap = EnvironmentMap<float, EnvironmentMapBlendMode::BlendAdditive>(INTVECTOR3(5, 5, 5));
	result.Assert(intmap.Data.size() == (5 * 5 * 5), ERR("Incorrect allocation of integer map"));
	result.Assert(floatmap.Data.size() == (5 * 5 * 5), ERR("Incorrect allocation of float map"));

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

	result.TestPassed();
	return result;
}

TestResult EnvironmentMapTests::FalloffMethodTests()
{
	TestResult result;

	EnvironmentMapFalloffMethod<int> intfalloff = EnvironmentMapFalloffMethod<int>()
		.WithAbsoluteFalloff(2)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<int>::FalloffTransmissionType::Square);

	result.Assert(intfalloff.ApplyFalloff(10, Direction::Right) == (10 - 2), ERR("Absolute integer square falloff method incorrectly applied to adjacent"));
	result.Assert(intfalloff.ApplyFalloff(10, Direction::UpLeft) == (10 - 2), ERR("Absolute integer square falloff method incorrectly applied to diagonal"));
	intfalloff.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<int>::FalloffTransmissionType::Distance);
	result.Assert(intfalloff.ApplyFalloff(10, Direction::UpLeft) == (10 - (2 * ONE_BY_ROOT2)), ERR("Absolute integer distance falloff method incorrectly applied to diagonal"));
	

	EnvironmentMapFalloffMethod<float> floatfalloff = EnvironmentMapFalloffMethod<float>()
		.WithRelativeFalloff(0.5f)
		.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<float>::FalloffTransmissionType::Square);

	result.Assert(floatfalloff.ApplyFalloff(10.0f, Direction::Right) == (10.0f * 0.5f), ERR("Relative float square falloff method incorrectly applied to adjacent"));
	result.Assert(floatfalloff.ApplyFalloff(10.0f, Direction::UpLeft) == (10.0f * 0.5f), ERR("Relative float square falloff method incorrectly applied to diagonal"));
	floatfalloff.WithFalloffTransmissionType(EnvironmentMapFalloffMethod<float>::FalloffTransmissionType::Distance);
	result.Assert(floatfalloff.ApplyFalloff(10.0f, Direction::UpLeft) == (10.0f - (2.0f * ONE_BY_ROOT2)), ERR("Relative float distance falloff method incorrectly applied to diagonal"));
	
	return result;
}

TestResult EnvironmentMapTests::BlendModeTests()
{
	TestResult result;

	EnvironmentMapBlendMode::BlendAdditive<int> intblend;
	intblend.SetInitialValue(10);
	result.Assert(intblend.Apply(10, 5) == 15, ERR("Additive integer blending failed"));
	result.Assert(intblend.Apply(15, -5) == 10, ERR("Additive integer blending failed"));

	EnvironmentMapBlendMode::BlendAdditive<float> float_add;
	EnvironmentMapBlendMode::BlendMultiplicative<float> float_mult;
	EnvironmentMapBlendMode::BlendMinimumValue<float> float_minv;
	EnvironmentMapBlendMode::BlendMaximumValue<float> float_maxv;
	EnvironmentMapBlendMode::BlendReplaceDestination<float> float_repl;
	EnvironmentMapBlendMode::BlendIgnoreNewValue<float> float_ignore;
	float_ignore.SetInitialValue(2.0f);

	result.Assert(float_add.Apply(2.0f, 4.0f) == (2.0f + 4.0f), ERR("Additive float blending failed"));
	result.Assert(float_mult.Apply(2.0f, 4.0f) == (2.0f * 4.0f), ERR("Mutliplicative float blending failed"));
	result.Assert(float_minv.Apply(2.0f, 4.0f) == 2.0f, ERR("MinValue float blending failed"));
	result.Assert(float_maxv.Apply(2.0f, 4.0f) == 4.0f, ERR("MaxValue float blending failed"));
	result.Assert(float_repl.Apply(2.0f, 4.0f) == 4.0f, ERR("Replace destination float blending failed"));
	result.Assert(float_repl.Apply(4.0f, 2.0f) == 2.0f, ERR("Replace destination float blending failed"));
	result.Assert(float_ignore.Apply(2.0f, 4.0f) == 2.0f, ERR("Ignore new float blending failed"));
	result.Assert(float_ignore.Apply(4.0f, 2.0f) == 4.0f, ERR("Ignore new float blending failed"));

	return result;
}

TestResult EnvironmentMapTests::BasicAdditivePropogationTests()
{
	TestResult result;

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

	result.Assert(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 3, 3))) == 10);
	result.Assert(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 3, 3))) == (1 + (10-2)));
	result.Assert(map.Data.at(env->GetElementIndex(INTVECTOR3(2, 2, 3))) == (1 + (10 - (2 * ONE_BY_ROOT2))));
	result.Assert(map.Data.at(env->GetElementIndex(INTVECTOR3(3, 1, 3))) == (1 + ((1 + (10 - 2)) - 2)));

	return result;
}


std::unique_ptr<ComplexShip> EnvironmentMapTests::GenerateTestElementEnvironment(const INTVECTOR3 & size)
{
	ComplexShip *env = new ComplexShip();
	env->InitialiseElements(size);
	env->UpdateEnvironment();

	return std::unique_ptr<ComplexShip>(env);
}


