#include "PrecalculatedRandomSequence.h"
#include "SequenceGenerationTests.h"

TestResult SequenceGenerationTests::BasicInitialisationTests()
{
	TestResult result = NewResult();

	PrecalculatedRandomSequence<int> int_seq(0, 10, 5, 20, false);
	PrecalculatedRandomSequence<float> flt_seq(5.0f, 100.0f, 10, 5, false);

	PrecalculatedRandomSequence<int> int_seq_distinct(0, 10, 5, 20, true);
	PrecalculatedRandomSequence<float> flt_seq_distinct(5.0f, 100.0f, 10, 5, true);

	result.AssertTrue(int_seq.IsInitialised(), ERR("Failed initialisation test for basic integer sequence"));
	result.AssertTrue(flt_seq.IsInitialised(), ERR("Failed initialisation test for basic float sequence"));
	result.AssertTrue(int_seq_distinct.IsInitialised(), ERR("Failed initialisation test for integer distinct sequence"));
	result.AssertTrue(flt_seq_distinct.IsInitialised(), ERR("Failed initialisation test for float distinct sequence"));

	return result;
}

TestResult SequenceGenerationTests::InvalidInitialisationTests()
{
	TestResult result = NewResult();

	PrecalculatedRandomSequence<int> int_seq_bounds(20, 10, 10, 10, false);
	result.AssertFalse(int_seq_bounds.IsInitialised(), ERR("Integer sequence with invalid bounds not correctly rejected"));

	PrecalculatedRandomSequence<float> flt_seq_bounds(10.0f, -10.0f, 10, 10, false);
	result.AssertFalse(flt_seq_bounds.IsInitialised(), ERR("Float sequence with invalid bounds not correctly rejected"));

	PrecalculatedRandomSequence<int> int_seq_dist(0, 10, 11, 10, true);
	result.AssertFalse(int_seq_dist.IsInitialised(), ERR("Distinct integer sequence not correctly rejected for inadequate value range"));

	PrecalculatedRandomSequence<float> flt_seq_dist(0.0f, 10.0f, 11, 10, true);
	result.AssertTrue(flt_seq_dist.IsInitialised(), ERR("Distinct float sequence did not correctly regress to non-unique basis and failed initialisation"));

	return result;
}

TestResult SequenceGenerationTests::UniquenessTests()
{
	TestResult result = NewResult();

	int count = 100;
	PrecalculatedRandomSequence<int> seq(0, 10, 10, count, true);
	result.AssertTrue(seq.IsInitialised(), ERR("Source sequence failed initialisation for uniqueness tests"));

	const int expected = (0 + 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9);
	for (int i = 0; i < count; ++i)
	{
		// Rather than check uniqueness of each element against each other, simply test the sum of 
		// all sequence elements against the expected value of SUM(0, 1, ..., N-1)
		int seq_sum = 0;
		const int *ptr = seq.GetSequence(i);
		for (int el = 0; el < 10; ++el)
		{
			seq_sum += ptr[el];
		}

		// Only perform the test assertion once if we fail to avoid noise
		if (seq_sum != expected)
		{
			result.AssertEqual(seq_sum, expected, ERR("Sequence uniqueness test failed; total sequence sum does not match expected value"));
			break;
		}
	}

	return result;
}
