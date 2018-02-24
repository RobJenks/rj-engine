#include "targetver.h"
#include <vector>
#include <tuple>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <assimp\postprocess.h>
#include <assimp\DefaultLogger.hpp>
#include "PipelineUtil.h"
#include "AssimpLogStream.h"
#include "ModelPipelineConstants.h"
#include "TransformPipeline.h"
#include "TransformPipelineBuilder.h"
#include "InputTransformerAssimp.h"
#include "InputTransformerRjm.h"
#include "BinaryOutputTransform.h"
#include "ObjFormatOutputTransform.h"
#include "PipelineStageOutputModelInfo.h"
#include "PipelineStageCentreModel.h"
#include "PipelineStageAssimpTransform.h"


static const std::vector<std::tuple<std::string, std::string, aiPostProcessSteps>> MESH_OPERATIONS 
{
	{ "gen-uv", "Generate UV coords for the model data", aiPostProcessSteps::aiProcess_GenUVCoords },
	{ "gen-normals", "Generate vertex normals; ignored if normals are already present", aiPostProcessSteps::aiProcess_GenNormals },
	{ "gen-smooth-normals", "Generate smooth vertex normals; incompatible with gen-normals", aiPostProcessSteps::aiProcess_GenSmoothNormals },
	{ "gen-tangents", "Generate tangent space data; requires normal data to be present", aiPostProcessSteps::aiProcess_CalcTangentSpace },
	{ "convert-lh", "Convert to a left-handed coordinate system", aiPostProcessSteps::aiProcess_MakeLeftHanded },
	{ "debone", "Losslessly removes bone data", aiPostProcessSteps::aiProcess_Debone },
	{ "remove-degen", "Remove degenerate points from model data", aiPostProcessSteps::aiProcess_FindDegenerates },
	{ "fix-invalid", "Attempt to fix common exporter errors", aiPostProcessSteps::aiProcess_FindInvalidData },
	{ "fix-inward-norm", "Attempt to fix inward-facing normals", aiPostProcessSteps::aiProcess_FixInfacingNormals },
	{ "flip-uv", "Flips all UVs along the y-axis; incorporated in \"convert-lh\" already", aiPostProcessSteps::aiProcess_FlipUVs },
	{ "cw-winding", "Change winding order from default CCW to CW", aiPostProcessSteps::aiProcess_FlipWindingOrder },
	{ "improve-cache-local", "Attempt to improve vertex cache locality based on 'tipsify' algorithm (O(n))", aiPostProcessSteps::aiProcess_ImproveCacheLocality },
	{ "merge-identical", "Merge identical vertices; only for indexed models", aiPostProcessSteps::aiProcess_JoinIdenticalVertices },
	{ "limit-bone-weights", "Limit number of bones affecting any vertex to 4 (configurable)", aiPostProcessSteps::aiProcess_LimitBoneWeights },
	{ "collapse-scene", "Attempt to simplify the scene graph by collapsing as far as possible", aiPostProcessSteps::aiProcess_OptimizeGraph },
	{ "optimise-mesh", "Attempt to optimise mesh data to reduce the number of draw calls", aiPostProcessSteps::aiProcess_OptimizeMeshes },
	{ "bake-scene-transform", "Remove scene hierarchy and bake transforms into vertex data", aiPostProcessSteps::aiProcess_PreTransformVertices },
	{ "minimise-mats", "Remove any materials not required by the model", aiPostProcessSteps::aiProcess_RemoveRedundantMaterials },
	{ "split-ptype", "Split model data into meshes based on primitive type", aiPostProcessSteps::aiProcess_SortByPType },
	{ "minimise-mesh-bones", "Split meshes if required to minimise the number of bone influences per vertex", aiPostProcessSteps::aiProcess_LimitBoneWeights },
	{ "split-large-meshes", "Split any large meshes into multiple sub-meshes (with configurable thresholds)", aiPostProcessSteps::aiProcess_SplitLargeMeshes },
	{ "bake-uv-transforms", "Bake any UK transformations directly into UV coords", aiPostProcessSteps::aiProcess_TransformUVCoords },
	{ "triangulate", "Triangulate any faces with more than three vertices", aiPostProcessSteps::aiProcess_Triangulate },
	{ "validation", "Perform validation of model data during transformation; enabled by default", aiPostProcessSteps::aiProcess_ValidateDataStructure }
};

void ObjToRjm(const std::string & input, const std::string & target, unsigned int operations)
{
	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerAssimp>(operations)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
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

		fs::path target(fs::absolute(in_path.parent_path()).string() + "/" + in_path.stem().string() + ".rjm");
		ObjToRjm(in, target.string(), operations);
	}
}

void RjmToObj(const std::string & input, const std::string & target, const std::string & generate_material)
{
	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
		.WithOutputTransformer(std::move(std::make_unique<ObjFormatOutputTransform>(generate_material)))
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
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageAssimpTransform>(operations)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
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
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
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
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
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
	std::cout << "   -mat <texture>\tGenerates a material definition for the given texture.  Only if -type=rjm-to-obj\n";
	std::cout << "   -op <operation>\tIncludes the given operation during transformation, where <operation> may be\n";

	for (const auto & op : MESH_OPERATIONS)
	{
		std::cout << "       " << std::get<0>(op) << "\t";
		for (unsigned int i = 0; i < 2U - (unsigned int)((float)std::get<0>(op).size() / 8.0f); ++i) std::cout << "\t";
		std::cout << std::get<1>(op) << "\n";
	}

	std::cout << "   -skip <operation>\tSkip the given operation, where <operation> may be any value available to \"-op\"\n";
	std::cout << "   -args <file>\t\tArguments will be read from the given external file\n";
	std::cout << "\n";
}

unsigned int GetOperation(const std::string & op)
{
	for (const auto & operation : MESH_OPERATIONS)
	{
		if (std::get<0>(operation) == op) return std::get<2>(operation);
	}

	return 0U;
}

void SetLogging(const std::string & mode)
{
	unsigned int severity = 0U;
	if (mode == "verbose")
	{
		ModelPipelineConstants::LogLevel = ModelPipelineConstants::LoggingType::Verbose;
		std::cout << "Enabling verbose logging\n";
		severity = Assimp::Logger::ErrorSeverity::Err | Assimp::Logger::ErrorSeverity::Warn | Assimp::Logger::ErrorSeverity::Info;
	}
	else if (mode == "debug-verbose")
	{
		ModelPipelineConstants::LogLevel = ModelPipelineConstants::LoggingType::DebugVerbose;
		std::cout << "Enabling debug & verbose logging\n";
		severity = Assimp::Logger::ErrorSeverity::Err | Assimp::Logger::ErrorSeverity::Warn | Assimp::Logger::ErrorSeverity::Info | Assimp::Logger::ErrorSeverity::Debugging;
	}
	else
	{
		return;
	}

	Assimp::DefaultLogger::create();
	Assimp::DefaultLogger::get()->attachStream(new AssimpLogStream(), severity);
}

void ReadArgsFromFile(const std::string & file, std::vector<std::string> & argsvector)
{
	fs::path argfile(file);
	if (!fs::exists(argfile))
	{
		std::cerr << "Cannot load args from external file \"" << fs::absolute(argfile) << "\"; file does not exist\n";
		return;
	}

	std::string argstring = PipelineUtil::ReadFileToString(argfile);

	// Split on spaces
	std::vector<std::string> args;
	PipelineUtil::SplitStringQuoted(argstring, args);
	for (auto & arg : args)
	{
		arg = PipelineUtil::TrimString(arg);
	}

	argsvector.insert(argsvector.end(), args.begin(), args.end());
	std::cout << "Loaded " << args.size() << " arguments from file \"" << fs::absolute(argfile) << "\"\n";
}


int main(int argc, const char *argv[])
{
	if (argc < 2 || ((argc - 1) % 2 != 0))
	{
		PrintUsage();
		exit(argc == 1 ? 0 : 1);
	}

	std::vector<std::string> input;
	std::string output;
	std::string type = "process-rjm";
	std::string gen_mat = "";
	bool inplace = false;
	bool inplace_backup = true;
	bool bulk = false;
	unsigned int operations = AssimpIntegration::DefaultOperations();

	// Arguments vector
	std::vector<std::string> args(argv, argv + argc);

	// Process each argument pair in turn
	for (int i = 1; i < args.size(); i += 2)
	{
		std::string key = args[i + 0U];
		std::string val = args[i + 1U];

		if (key == "-i")						input.push_back(val);
		else if (key == "-o")					output = val;
		else if (key == "-type")				type = val;
		else if (key == "-n")					bulk = (val == "bulk");
		else if (key == "-ot")					inplace = (val == "inplace");
		else if (key == "-bk")					inplace_backup = (val == "true");
		else if (key == "-mat")					gen_mat = val;
		else if (key == "-log")					SetLogging(val);
		else if (key == "-args")				ReadArgsFromFile(val, args);
		else if (key == "-op" || key == "-skip")
		{
			auto op = GetOperation(val);
			if (op == 0U)
			{
				std::cerr << "Unknown model pipeline operation \"" << val << "\"; ignoring\n";
			}
			else
			{
				if (key == "-op")
				{
					std::cout << "Adding operation \"" << val << "\" (" << op << ") to pipeline transformation\n";
					operations |= op;
				}
				else
				{
					std::cout << "Excluding operation  \"" << val << "\" (" << op << ") from pipeline transformation\n";
					operations &= (~op);
				}
			}
		}
		else std::cerr << "Unrecognised argument \"" << key << "\"; ignoring\n";
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
	std::cout << "\nModel pipeline operations completed, exiting\n";
	exit(0);
}




/* Test arguments:

const unsigned int COUNT = 11;
argv = new const char*[COUNT] { argv[0],
"-type", "obj-to-rjm",
"-i", "C:/Users/robje/Downloads/sphere.obj",
"-o", "C:/Users/robje/Downloads/sphere.out",
"-op", "gen-normals",
"-op", "gen-tangents"};
argc = COUNT;

const unsigned int COUNT = 15;
if (ix == 1)
{
argv = new const char*[COUNT] { argv[0],
"-type", "obj-to-rjm",
"-i", "C:/Users/robje/Downloads/sphere.obj",
"xxx-o", "C:/Users/robje/Downloads/sphere.out",
"xxx-op", "gen-normals",
"xxx-op", "triangulate",
"-log", loglevel,
"-args", "C:/Users/robje/Downloads/test-args.txt"};
argc = COUNT;
}
else if (ix == 2)
{
argv = new const char*[COUNT] { argv[0],
"-type", "rjm-to-obj",
"-i", "C:/Users/robje/Downloads/sphere.out",
"-o", "C:/Users/robje/Downloads/sphere2.obj",
"-log", loglevel,
"xxx-op", "gen-normals",
"xxx-op", "gen-normals",
"xxx-op", "gen-tangents"};
argc = COUNT;
}
else if (ix == 3)
{
argv = new const char*[COUNT] { argv[0],
"-type", "process-rjm",
"-i", "C:/Users/robje/Downloads/sphere.out",
"-o", "C:/Users/robje/Downloads/sphere2.out",
"-op", "gen-tangents",
"-log", loglevel,
"xxx-op", "gen-normals",
"xxx-op", "gen-tangents"};
argc = COUNT;
}


*/

/* Test methods: 

//ObjTransformTest();
//RjmObjConversionTest();
//BulkRjmObjConversion();

*/

