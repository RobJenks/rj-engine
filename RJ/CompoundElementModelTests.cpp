#include "CompoundElementModelTests.h"
#include "CompoundElementModel.h"
#include "IntVector.h"


TestResult CompoundElementModelTests::LayoutCalculationTests()
{
	TestResult result = NewResult();

	auto size = UINTVECTOR3(4U);
	auto model_ptr = GenerateCompoundElementModelData(size);
	auto * model = model_ptr.get();

	// Debug print out the current model structure
	Game::Log << "\n" << LOG_INFO << "Model components:\n";
	for (UINT i = 0; i < model->GetModelCount(); ++i) Game::Log << LOG_INFO << i << ": " << model->GetModels().at(i).str() << "\n";
	Game::Log << "\n\n" << LOG_INFO << "Layout map (z=0):\n ";
	for (UINT y = 0; y < model->GetSize().y; ++y)
	{
		Game::Log << "\n" << LOG_INFO;
		for (UINT x = 0; x < model->GetSize().x; ++x)
		{
			Game::Log << model->GetModelIndices(x, y, 0).begin << "  ";
		}
	}
	Game::Log << "\n\n";

	// Expected index combinations per query
	std::vector<CompoundElementModel::ModelIndexRange> expected = { 
		{ 0, 0 }, { 0, 0 },	// ix = 0-1
		{ 0, 1 },			// ix = 2
		{ 1, 3 },			// ix = 3
		{ 3, 3 },			// ix = 4
		{ 3, 4 },			// ix = 5
		{ 4, 5 },			// ix = 6
		{ 5, 5 },			// ix = 7
		{ 5, 8 },			// ix = 8
		{ 8, 9 }			// ix = 9
	};

	// Verify all indices we have manually specified
	CompoundElementModel::ModelIndexRange exp;
	auto layout_count = (size.x * size.y * size.z);
	for (UINT i = 0; i < layout_count; ++i)
	{
		// Verify that all indices have the expected value, either explicitly set or the remaining tail of { 9, 9 }
		if (i < 10) exp = expected[i];
		else		exp = CompoundElementModel::ModelIndexRange(9U, 9U);

		auto indices = model->GetModelIndices(DetermineElementLocationFromIndex(i, size));
		result.AssertEqual<CompoundElementModel::CompoundModelIndex>(indices.begin, exp.begin, ERR("Incorrect calculation of index range start"));
		result.AssertEqual<CompoundElementModel::CompoundModelIndex>(indices.end, exp.end, ERR("Incorrect calculation of index range end"));
	}

	// Test final element at end of layout map, in place so we can always use { X, X+1 } for all elements
	result.AssertEqual(
		model->GetModelIndices(size).begin, model->GetModelIndices(size).end,
		ERR("Invalid termination element in model layout map")
	);

	return result;
}



std::unique_ptr<CompoundElementModel> CompoundElementModelTests::GenerateCompoundElementModelData(const UINTVECTOR3 & size) const
{
	auto model_ptr = std::make_unique<CompoundElementModel>(size);

	auto * model = model_ptr.get();
	model->SuspendUpdates();
	{
		model->AddModel(ModelInstance(), CompoundTileModelType::WallStraight, DetermineElementLocationFromIndex(2U, size), Rotation90Degree::Rotate0);
		model->AddModel(ModelInstance(), CompoundTileModelType::WallStraight, DetermineElementLocationFromIndex(3U, size), Rotation90Degree::Rotate0);
		model->AddModel(ModelInstance(), CompoundTileModelType::WallStraight, DetermineElementLocationFromIndex(3U, size), Rotation90Degree::Rotate0);
		model->AddModel(ModelInstance(), CompoundTileModelType::WallStraight, DetermineElementLocationFromIndex(5U, size), Rotation90Degree::Rotate0);
		model->AddModel(ModelInstance(), CompoundTileModelType::WallStraight, DetermineElementLocationFromIndex(6U, size), Rotation90Degree::Rotate0);
		model->AddModel(ModelInstance(), CompoundTileModelType::WallStraight, DetermineElementLocationFromIndex(8U, size), Rotation90Degree::Rotate0);
		model->AddModel(ModelInstance(), CompoundTileModelType::WallStraight, DetermineElementLocationFromIndex(8U, size), Rotation90Degree::Rotate0);
		model->AddModel(ModelInstance(), CompoundTileModelType::WallStraight, DetermineElementLocationFromIndex(8U, size), Rotation90Degree::Rotate0);
		model->AddModel(ModelInstance(), CompoundTileModelType::WallStraight, DetermineElementLocationFromIndex(9U, size), Rotation90Degree::Rotate0);
	}
	model->ResumeUpdates();
	
	return model_ptr;
}

// Primitive method to calculate an element location from an index that is already derived.  This would never be used within
// the main application; only for these tests to make sure there are no manual errors when setting locations in the test
UINTVECTOR3 CompoundElementModelTests::DetermineElementLocationFromIndex(UINT index, const UINTVECTOR3 & size) const
{
	for (UINT x = 0; x < size.x; ++x)
	{
		for (UINT y = 0; y < size.y; ++y)
		{
			for (UINT z = 0; z < size.z; ++z)
			{
				if (ELEMENT_INDEX_EX(x, y, z, size) == index) return UINTVECTOR3(x, y, z);
			}
		}
	}

	assert(false);				// We should never end up here
	return UINTVECTOR3(0U);
}

