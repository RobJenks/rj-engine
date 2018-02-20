#include "targetver.h"
#include <vector>
#include <tuple>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <assimp\postprocess.h>
#include "PipelineUtil.h"
#include "TransformPipeline.h"
#include "TransformPipelineBuilder.h"
#include "InputTransformerAssimp.h"
#include "InputTransformerRjm.h"
#include "BinaryOutputTransform.h"
#include "PipelineStagePassthrough.h"
#include "PipelineStageCentreModel.h"


void ObjToRjm(const std::string & input, const std::string & target, unsigned int operations)
{
	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerAssimp>(operations)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();

	// Execute the transformation pipeline
	pipeline->Transform(fs::path(input), fs::path(target));

	// For testing
	// auto m = ModelData::Deserialize(PipelineUtil::ReadBinaryFile(fs::path("C:\\Users\\robje\\Downloads\\capsule.out")));
}

void ObjToRjmBulk(std::vector<std::string> & input, unsigned int operations)
{
	// Exectute the transformation pipeline
	std::cout << "Performing bulk transformation\n";
	int index = 0;
	for (const std::string & in : input)
	{
		fs::path in_path(in);
		std::cout << "Processing file " << ++index << " of " << input.size() << " (" << fs::absolute(in) << ")\n";

		fs::path target(fs::absolute(in_path.parent_path()).string() + "/" + in_path.stem().string() + ".out");
		ObjToRjm(in, target.string(), operations);
	}
}

void RjmToObj(const std::string & input, const std::string & target, const std::string & generate_material)
{
	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>(generate_material, true)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();

	// Execute the transformation pipeline
	pipeline->Transform(fs::path(input), fs::path(target));
}

void RjmToObjBulk(std::vector<std::string> & input, const std::string & generate_material)
{
	// Exectute the transformation pipeline
	std::cout << "Performing bulk transformation\n";
	int index = 0;
	for (const std::string & in : input)
	{
		fs::path in_path(in);
		std::cout << "Processing file " << ++index << " of " << input.size() << " (" << fs::absolute(in) << ")\n";

		fs::path target(fs::absolute(in_path.parent_path()).string() + "/" + in_path.stem().string() + ".out");
		RjmToObj(in, target.string(), generate_material);
	}
}

void ProcessRjm(const std::string & input, const std::string & target, unsigned int operations, bool in_place = false, bool in_place_backup = true)
{
	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>("", false, operations)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStagePassthrough>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();

	// Target depends on whether this is in-place or not
	if (!in_place)
	{
		// Exectute the transformation pipeline
		pipeline->Transform(fs::path(input), fs::path(target));
	}
	else
	{
		// Perform in-place modification of the model data
		fs::path in(input);
		fs::path out(fs::absolute(in.parent_path()).string() + "/" + in.stem().string() + ".out");

		// Transform
		pipeline->Transform(in, out);

		// Check whether the transformation succeeded
		if (!fs::exists(out))
		{
			std::cerr << "RJM transformation failed\n";
		}
		else
		{
			// Create a backup if required
			if (in_place_backup)
			{
				static const bool overwrite_backups = false;
				fs::copy_file(in, fs::path(in.string() + ".backup"), (overwrite_backups ? fs::copy_options::overwrite_existing : fs::copy_options::skip_existing));
			}

			// Rename the target file to the original file to perform an in-place swap
			fs::copy_file(out, in, fs::copy_options::overwrite_existing);

			// Remove the target file now that it has been swapped in for the original file
			fs::remove(out);
		}	
	}
}

void ProcessRjmBulk(std::vector<std::string> & input, unsigned int operations, bool in_place = false, bool in_place_backup = true)
{
	// Exectute the transformation pipeline
	std::cout << "Performing bulk transformation\n";
	int index = 0;
	for (const std::string & in : input)
	{
		fs::path in_path(in);
		std::cout << "Processing file " << ++index << " of " << input.size() << " (" << fs::absolute(in) << ")\n";

		// Target is only relevant if this is not an in-place swap
		std::string target = (in_place ? "" : (fs::absolute(in_path.parent_path()).string() + "/" + in_path.stem().string() + ".out"));
		ProcessRjm(in, target, operations, in_place, in_place_backup);
	}
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



void PrintUsage()
{
	std::cout << "Usage: ModelPipeline key1, value1, [key2, value2], ..., [keyN, valueN]\n";
	std::cout << "\nWhere each tuple may be\n";
	std::cout << "   -i <path>\t\tInput file; only one may be specified unless n=bulk\n";
	std::cout << "   -o <path>\t\tOutput file; will default if not specified.  Ignored if n=bulk or ot=inplace\n";
	std::cout << "   -type <type>\t\tMethod to execute, where <type> may be\n";
	std::cout << "       process-rjm\t\tApply transformations to an RJM model\n";
	std::cout << "       obj-to-rjm\t\tConvert OBJ file to RJM format\n";
	std::cout << "       rjm-to-obj\t\tConvert RJM file to OBJ format\n";
	std::cout << "   -n <type>\t\tNumber of items to be processed; <type> may be \"single\" (default) or \"bulk\"\n";
	std::cout << "   -ot <otype>\t\tOutput type; either \"target\" (default) or \"inplace\"\n";
	std::cout << "   -bk <bool>\t\tDetermines whether backup is created.  Only if ot=inplace.  Default=true\n";
	std::cout << "   -mat <texture>\t\tGenerates a material definition for the given texture.  Only if -type=rjm-to-obj\n";
	std::cout << "   -op <operation>\t\tIncludes the given operation during transformation, where <operation> may be\n";
	std::cout << "       gen-uv\t\tGenerate UV coords for the model data\n";
	std::cout << "       gen-normals\t\tGenerate vertex normals; ignored if normals are already present\n";
	std::cout << "       gen-smooth-normals\t\tGenerate smooth vertex normals; incompatible with gen-normals\n";
	std::cout << "       gen-tangents\t\tGenerate tangent space data; requires normal data to be present\n";
	std::cout << "       convert-lh\t\tConvert to a left-handed coordinate system\n";
	std::cout << "       debone\t\tLosslessly removes bone data\n";
	std::cout << "       remove-degen\t\tRemove degenerate points from model data\n";
	std::cout << "       fix-invalid\t\tAttempt to fix common exporter errors\n";
	std::cout << "       fix-inward-norm\t\tAttempt to fix inward-facing normals\n";
	std::cout << "       flip-uv\t\tFlips all UVs along the y-axis; incorporated in \"convert-lh\" already\n";
	std::cout << "       cw-winding\t\tChange winding order from default CCW to CW\n";
	std::cout << "       improve-cache-local\t\tAttempt to improve vertex cache locality based on 'tipsify' algorithm (O(n))\n";
	std::cout << "       merge-identical\n\nMerge identical vertices; only for indexed models\n";
	std::cout << "       limit-bone-weights\t\tLimit nnumber of bones affecting any vertex to 4 (configurable)\n";
	std::cout << "       collapse-scene\t\tAttempt to simplify the scene graph by collapsing as far as possible\n";
	std::cout << "       optimise-mesh\t\tAttempt to optimise mesh data to reduce the number of draw calls\n";
	std::cout << "       bake-scene-transform\t\tRemove scene hierarchy and bake transforms into vertex data\n";
	std::cout << "       minimise-mats\t\tRemove any materials not required by the model\n";
	std::cout << "       split-ptype\t\tSplit model data into meshes based on primitive type\n";
	std::cout << "       minimise-mesh-bones\t\tSplit meshes if required to minimise the number of bone influences per vertex\n";
	std::cout << "       split-large-meshes\t\tSplt any large meshes into multiple sub-meshes (with configurable thresholds)\n";
	std::cout << "       bake-uv-transforms\t\tBake any UK transformations directly into UV coords\n";
	std::cout << "       triangulate\t\tTriangulate any faces with more than three vertices\n";
	std::cout << "       validation\t\tPerform validation of model data during transformation; enabled by default\n";
	std::cout << "   -skip <operation>\t\tSkip the given operation, where <operation> may be any value available to \"-op\"\n";
	std::cout << "\n";
}

unsigned int GetOperation(const std::string & op)
{
	static const std::vector<std::tuple<std::string, aiPostProcessSteps>> ops 
	{
		{ "gen-uv", aiPostProcessSteps::aiProcess_GenUVCoords }, 
		{ "gen-normals", aiPostProcessSteps::aiProcess_GenNormals }, 
		{ "gen-smooth-normals", aiPostProcessSteps::aiProcess_GenSmoothNormals }, 
		{ "gen-tangents", aiPostProcessSteps::aiProcess_CalcTangentSpace }, 
		{ "convert-lh", aiPostProcessSteps::aiProcess_MakeLeftHanded }, 
		{ "debone", aiPostProcessSteps::aiProcess_Debone }, 
		{ "remove-degen", aiPostProcessSteps::aiProcess_FindDegenerates },
		{ "fix-invalid", aiPostProcessSteps::aiProcess_FindInvalidData }, 
		{ "fix-inward-norm", aiPostProcessSteps::aiProcess_FixInfacingNormals }, 
		{ "flip-uv", aiPostProcessSteps::aiProcess_FlipUVs }, 
		{ "cw-winding", aiPostProcessSteps::aiProcess_FlipWindingOrder }, 
		{ "improve-cache-local", aiPostProcessSteps::aiProcess_ImproveCacheLocality }, 
		{ "merge-identival", aiPostProcessSteps::aiProcess_JoinIdenticalVertices }, 
		{ "limit-bone-weights", aiPostProcessSteps::aiProcess_LimitBoneWeights }, 
		{ "collapse-scene", aiPostProcessSteps::aiProcess_OptimizeGraph }, 
		{ "optimise-mesh", aiPostProcessSteps::aiProcess_OptimizeMeshes }, 
		{ "bake-scene-transform", aiPostProcessSteps::aiProcess_PreTransformVertices }, 
		{ "minimise-mats", aiPostProcessSteps::aiProcess_RemoveRedundantMaterials }, 
		{ "split-large-meshes", aiPostProcessSteps::aiProcess_SplitLargeMeshes }, 
		{ "bake-uv-transforms", aiPostProcessSteps::aiProcess_TransformUVCoords }, 
		{ "triangulate", aiPostProcessSteps::aiProcess_Triangulate }, 
		{ "validation", aiPostProcessSteps::aiProcess_ValidateDataStructure }
	};

	for (auto & operation : ops)
	{
		if (std::get<0>(operation) == op) return std::get<1>(operation);
	}

	return 0U;
}



int main(int argc, const char *argv[])
{
	//ObjTransformTest();
	//RjmObjConversionTest();
	//BulkRjmObjConversion();
	if (argc < 2 || ((argc - 1) % 2 != 0))
	{
		PrintUsage();
		exit(argc == 1 ? 0 : 1);
	}

	std::vector<std::string> input;
	std::string output;
	std::string type = "obj-to-rjm";
	std::string gen_mat = "";
	bool inplace = false;
	bool inplace_backup = true;
	bool bulk = false;
	unsigned int operations = InputTransformerAssimp::DefaultOperations();

	// Process each argument pair in turn
	for (int i = 0; i < (argc - 1) - 1; i += 2)
	{
		std::string key = argv[i + 1];
		std::string val = argv[i + 2];

		if (key == "-i")						input.push_back(val);
		else if (key == "-o")					output = val;
		else if (key == "-type")				type = val;
		else if (key == "-n")					bulk = (val == "bulk");
		else if (key == "-ot")					inplace = (val == "inplace");
		else if (key == "-bk")					inplace_backup = (val == "true");
		else if (key == "-mat")					gen_mat = val;
		else if (key == "-op")
		{
			operations |= GetOperation(val);
		}
		else if (key == "-skip")
		{
			operations &= ~(GetOperation(val));
		}
	}

	// Validate inputs
	if (input.empty()) {								std::cerr << "No input file(s) provided (use -i)\n"; exit(1); }
	else if (output.empty() && !(inplace || bulk)) {	std::cerr << "Ouptut file must be specified unless -ot=inplace or -n=bulk\n"; exit(1); }

	// Perform the requested operation
	if (type == "process-rjm")
	{
		if (bulk)		ProcessRjmBulk(input, operations, inplace, inplace_backup);
		else			ProcessRjm(input.at(0), output, operations, inplace, inplace_backup);
	}
	else if (type == "obj-to-rjm")
	{
		if (bulk)		ObjToRjmBulk(input, operations);
		else			ObjToRjm(input.at(0), output, operations);
	}
	else if (type == "rjm-to-obj")
	{
		if (bulk)		RjmToObjBulk(input, gen_mat);
		else			RjmToObj(input.at(0), output, gen_mat);
	}


	// Return success
	std::cout << "\n";
	exit(0);
}

