#include "targetver.h"
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include "TransformPipeline.h"
#include "TransformPipelineBuilder.h"
#include "InputTransformerAssimp.h"
#include "BinaryOutputTransform.h"
#include "PassthroughPipelineStage.h"

int main()
{

	// Temporary: to test
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerAssimp>()))
		.WithPipelineStage(std::move(std::make_unique<PassthroughPipelineStage>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();
		

	pipeline->Transform(fs::path("C:\\Users\\robje\\Documents\\Other\\Prop.txt"), fs::path("C:\\Users\\robje\\Documents\\Other\\Prop-output.txt"));
	
	std::cin >> std::string();
    return 0;
}

