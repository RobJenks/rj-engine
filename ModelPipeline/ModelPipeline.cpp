#include "targetver.h"
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include "PipelineUtil.h"
#include "TransformPipeline.h"
#include "TransformPipelineBuilder.h"
#include "InputTransformerAssimp.h"
#include "BinaryOutputTransform.h"
#include "PipelineStagePassthrough.h"
#include "PipelineStageCentreModel.h"

int main()
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
	pipeline->Transform(fs::path("C:\\Users\\robje\\Downloads\\capsule.obj"), fs::path("C:\\Users\\robje\\Downloads\\capsule.out"));

	// For testing
	// auto m = ModelData::Deserialize(PipelineUtil::ReadBinaryFile(fs::path("C:\\Users\\robje\\Downloads\\capsule.out")));

	// Return success
	std::cout << "\n";
	exit(0);
}

