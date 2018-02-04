#include "targetver.h"
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include "PipelineUtil.h"
#include "TransformPipeline.h"
#include "TransformPipelineBuilder.h"
#include "InputTransformerAssimp.h"
#include "InputTransformerRjm.h"
#include "BinaryOutputTransform.h"
#include "PipelineStagePassthrough.h"
#include "PipelineStageCentreModel.h"


void ObjTransformTest(void)
{
	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerAssimp>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();

	// Exectute the transformation pipeline
	pipeline->Transform(fs::path("C:\\Users\\robje\\Downloads\\capsule-copy.obj"), fs::path("C:\\Users\\robje\\Downloads\\capsule-copy.out"));

	// For testing
	// auto m = ModelData::Deserialize(PipelineUtil::ReadBinaryFile(fs::path("C:\\Users\\robje\\Downloads\\capsule.out")));
}

void RjmObjConversionTest(void)
{
	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>("testship1_texture.dds", true)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();

	// Exectute the transformation pipeline
	pipeline->Transform(fs::path("C:\\Users\\robje\\Downloads\\testship1.rjm"), fs::path("C:\\Users\\robje\\Downloads\\testship1.out"));

	// For testing
	// auto m = ModelData::Deserialize(PipelineUtil::ReadBinaryFile(fs::path("C:\\Users\\robje\\Downloads\\capsule.out")));
}

void BulkRjmObjConversion(void)
{
	bool overwrite_backups = true;

	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>("", true)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();


	// Set of models to be converted
	std::vector<fs::path> source_files = 
	{
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Models/Misc/unit_cone.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Models/Misc/unit_square.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Models/Misc/unit_line.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Sections/Frigate_Small/frigate_small_midsection1.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Sections/Frigate_Medium/frigate_medium_midsection1.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Tiles/CorridorTiles/corridor_ns.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Tiles/CorridorTiles/corridor_se.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Tiles/CorridorTiles/corridor_nse.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Tiles/CorridorTiles/corridor_nsew.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Tiles/QuartersTiles/quarters_dorm01_interior.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Tiles/QuartersTiles/quarters_dorm01_wall_straight.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Tiles/QuartersTiles/quarters_dorm01_wall_corner.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Tiles/QuartersTiles/quarters_dorm01_connection.rjm"), 
		fs::path("C:/Users/robje/Documents/Visual Studio 2017/Projects/RJ/RJ/Data/Ships/Simple/Testing/testship1/testship1.rjm")
	};

	int index = 0;
	for (auto path : source_files)
	{
		std::cout << "> Processing file " << ++index << " of " << source_files.size() << " (" << path.filename().string() << ")\n";

		// Execute the transformation pipeline for each model in turn
		fs::path target(fs::absolute(path.parent_path()).string() + "/" + path.stem().string() + ".out");
		pipeline->Transform(path, target);

		// If the transformation failed we should skip the model here
		if (!fs::exists(target))
		{
			std::cerr << "Transformation failed, moving to next model\n";
			continue;
		}

		// Rename the original file in order to retain a backup (do not overwrite so that multiple runs will not eliminate backups)
		fs::copy_file(path, fs::path(path.string() + ".backup"), (overwrite_backups ? fs::copy_options::overwrite_existing : fs::copy_options::skip_existing));

		// Rename the target file to the original file to perform an in-place swap
		fs::copy_file(target, path, fs::copy_options::overwrite_existing);

		// Remove the target file now that it has been swapped in for the original file
		fs::remove(target);
	}

}




int main(void)
{
	//ObjTransformTest();
	//RjmObjConversionTest();
	BulkRjmObjConversion();

	// Return success
	std::cout << "\n";
	exit(0);
}

