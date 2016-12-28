#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "ComplexShip.h"
#include "iSpaceObjectEnvironment.h"
#include "ComplexShipElement.h"

namespace UnitTests
{
	TEST_CLASS(EnvironmentDestructionTests)
	{
	private:

		static std::unique_ptr<ComplexShip> TestEnvironment(void) { return TestEnvironment(INTVECTOR3(40, 20, 3)); }
		static std::unique_ptr<ComplexShip> TestEnvironment(const INTVECTOR3 & size)
		{
			const INTVECTOR3 sz = IntVector3Clamp(size, ONE_INTVECTOR3, INTVECTOR3(100, 100, 100));

			ComplexShip *env = new ComplexShip();
			env->InitialiseElements(sz);
			env->UpdateEnvironment();

			return std::make_unique<ComplexShip>(*env);
		}

	public:

		TEST_METHOD(VerifyBuildOfOuterHullModel)
		{
			// Create the environment
			INTVECTOR3 size(40, 20, 3);
			std::unique_ptr<ComplexShip> env = TestEnvironment(size);

			// Test that only elements on the outer edges of the model are flagged as such
			int n = env->GetElementCount();
			ComplexShipElement *elements = env->GetElements();
			for (int i = 0; i < n; ++i)
			{
				const ComplexShipElement & el = elements[i];
				const INTVECTOR3 & loc = el.GetLocation();

				if (loc.x == 0 || loc.y == 0 || loc.z == 0 || loc.x == (size.x - 1) || loc.y == (size.y - 1) || loc.z == (size.z - 1))
					Assert::IsTrue(el.IsOuterHullElement(), L"Outer element is not correctly flagged as such");
				else
					Assert::IsFalse(el.IsOuterHullElement(), L"Inner element is incorrectly flagged as part of outer hull");
			}
		}

		TEST_METHOD(VerifyBuildOfDamagedOuterHullModel)
		{
			// Create the environment
			INTVECTOR3 size(40, 20, 3);
			std::unique_ptr<ComplexShip> env = TestEnvironment(size);
			
			// Destroy a corner element
			env->GetElement(0, 0, 0)->SetHealth(0.0f);
			env->UpdateEnvironment();
			Assert::IsTrue(env->GetElement(0, 0, 0)->IsDestroyed(), L"Element not correctly marked as destroyed");
			Assert::IsTrue(!env->GetElement(0, 0, 0)->IsOuterHullElement() && env->GetElement(1, 0, 0)->IsOuterHullElement() &&
				env->GetElement(0, 1, 0)->IsOuterHullElement() && env->GetElement(0, 0, 1)->IsOuterHullElement(), 
				L"Outer hull not rebuilt correctly around corner element");

			// Destroy an inner element
			env->GetElement(10, 10, 1)->SetHealth(0.0f);
			env->UpdateEnvironment();
			const INTVECTOR3 hull[] = { INTVECTOR3(9, 10, 1), INTVECTOR3(11, 10, 1), INTVECTOR3(10, 9, 1), INTVECTOR3(10, 11, 1), 
				INTVECTOR3(10, 10, 0), INTVECTOR3(10, 10, 2) };

			Assert::IsTrue(env->GetElement(10, 10, 1)->IsDestroyed(), L"Element not correctly marked as destroyed");
			Assert::IsFalse(env->GetElement(10, 10, 1)->IsOuterHullElement(), L"Destroyed element incorrectly marked as outer hull");
			for (INTVECTOR3 el : hull) Assert::IsTrue(env->GetElement(el)->IsOuterHullElement(),
				concat("Element ")(el.ToString())(" not correctly marked as outer hull").c_wstr());
		}

	};
}