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
#include "TransformResult.h"
#include "ModelPipelineConstants.h"
#include "CustomPostProcess.h"
#include "TransformPipeline.h"
#include "TransformPipelineBuilder.h"
#include "InputTransformerAssimp.h"
#include "InputTransformerRjm.h"
#include "BinaryOutputTransform.h"
#include "ObjFormatOutputTransform.h"
#include "PipelineStageOutputModelInfo.h"
#include "PipelineStageUnitScaleModel.h"
#include "PipelineStageCentreModel.h"
#include "PipelineStageAssimpTransform.h"
#include "PipelineStageDirectPostprocess.h"


static const std::vector<std::tuple<std::string, std::string, PostProcess>> MESH_OPERATIONS 
{
	{ "gen-uv", "Generate UV coords for the model data", (PostProcess)aiPostProcessSteps::aiProcess_GenUVCoords },
	{ "gen-normals", "Generate vertex normals; ignored if normals are already present", (PostProcess)aiPostProcessSteps::aiProcess_GenNormals },
	{ "gen-smooth-normals", "Generate smooth vertex normals; incompatible with gen-normals", (PostProcess)aiPostProcessSteps::aiProcess_GenSmoothNormals },
	{ "gen-tangents", "Generate tangent space data; requires normal data to be present", (PostProcess)aiPostProcessSteps::aiProcess_CalcTangentSpace },
	{ "convert-lh", "Convert to a left-handed coordinate system", (PostProcess)aiPostProcessSteps::aiProcess_MakeLeftHanded },
	{ "debone", "Losslessly removes bone data", (PostProcess)aiPostProcessSteps::aiProcess_Debone },
	{ "remove-degen", "Remove degenerate points from model data", (PostProcess)aiPostProcessSteps::aiProcess_FindDegenerates },
	{ "fix-invalid", "Attempt to fix common exporter errors", (PostProcess)aiPostProcessSteps::aiProcess_FindInvalidData },
	{ "fix-inward-norm", "Attempt to fix inward-facing normals", (PostProcess)aiPostProcessSteps::aiProcess_FixInfacingNormals },
	{ "flip-uv", "Flips all UVs along the y-axis; incorporated in \"convert-lh\" already", (PostProcess)aiPostProcessSteps::aiProcess_FlipUVs },
	{ "cw-winding", "Change winding order from default CCW to CW", (PostProcess)aiPostProcessSteps::aiProcess_FlipWindingOrder },
	{ "improve-cache-local", "Attempt to improve vertex cache locality based on 'tipsify' algorithm (O(n))", (PostProcess)aiPostProcessSteps::aiProcess_ImproveCacheLocality },
	{ "merge-identical", "Merge identical vertices; only for indexed models", (PostProcess)aiPostProcessSteps::aiProcess_JoinIdenticalVertices },
	{ "limit-bone-weights", "Limit number of bones affecting any vertex to 4 (configurable)", (PostProcess)aiPostProcessSteps::aiProcess_LimitBoneWeights },
	{ "collapse-scene", "Attempt to simplify the scene graph by collapsing as far as possible", (PostProcess)aiPostProcessSteps::aiProcess_OptimizeGraph },
	{ "optimise-mesh", "Attempt to optimise mesh data to reduce the number of draw calls", (PostProcess)aiPostProcessSteps::aiProcess_OptimizeMeshes },
	{ "bake-scene-transform", "Remove scene hierarchy and bake transforms into vertex data", (PostProcess)aiPostProcessSteps::aiProcess_PreTransformVertices },
	{ "minimise-mats", "Remove any materials not required by the model", (PostProcess)aiPostProcessSteps::aiProcess_RemoveRedundantMaterials },
	{ "split-ptype", "Split model data into meshes based on primitive type", (PostProcess)aiPostProcessSteps::aiProcess_SortByPType },
	{ "minimise-mesh-bones", "Split meshes if required to minimise the number of bone influences per vertex", (PostProcess)aiPostProcessSteps::aiProcess_LimitBoneWeights },
	{ "split-large-meshes", "Split any large meshes into multiple sub-meshes (with configurable thresholds)", (PostProcess)aiPostProcessSteps::aiProcess_SplitLargeMeshes },
	{ "bake-uv-transforms", "Bake any UK transformations directly into UV coords", (PostProcess)aiPostProcessSteps::aiProcess_TransformUVCoords },
	{ "triangulate", "Triangulate any faces with more than three vertices", (PostProcess)aiPostProcessSteps::aiProcess_Triangulate },
	{ "validation", "Perform validation of model data during transformation; enabled by default", (PostProcess)aiPostProcessSteps::aiProcess_ValidateDataStructure }, 
	{ "invert-u", "Invert all U coordinates of the mesh UV mapping (i.e. u' = (1.0f - u)", (PostProcess)CustomPostProcess::InvertU },
	{ "invert-v", "Invert all V coordinates of the mesh UV mapping (i.e. v' = (1.0f - v)", (PostProcess)CustomPostProcess::InvertV }, 
	{ "custom-transform", "Apply a custom transformation given in \"<modelfile>.transform\" to the model geometry", (PostProcess)CustomPostProcess::CustomTransform }
};


PostProcess GetAllOperations(void)
{
	PostProcess all = 0U;
	for (auto & op : MESH_OPERATIONS)
	{
		all |= std::get<2>(op);
	}

	return all;
}

PostProcess GetOperation(const std::string & op)
{
	for (const auto & operation : MESH_OPERATIONS)
	{
		if (std::get<0>(operation) == op) return std::get<2>(operation);
	}

	if (op == "all") return GetAllOperations();

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

void ReadArgsFromFile(const std::string & file, std::vector<std::string> & argsvector, int insert_point = -1)
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

	// Either insert at the specified point or at the end of the arguments vector
	if (insert_point >= 0 && insert_point < argsvector.size())
	{
		argsvector.insert(argsvector.begin() + insert_point, args.begin(), args.end());
	}
	else
	{
		argsvector.insert(argsvector.end(), args.begin(), args.end());
	}

	std::cout << "Loaded " << args.size() << " arguments from file \"" << fs::absolute(argfile) << "\"\n";
}

PostProcess AddModelSpecificOperations(fs::path model_file, PostProcess current_operations)
{
	if (!fs::exists(model_file)) return 0U;

	fs::path config_file = fs::path(fs::absolute(model_file).string() + ".pipeline");
	if (!fs::exists(config_file)) return 0U;
	std::cout << "Model \"" << model_file.filename().string() << "\" has model-specific operation config\n";

	std::vector<std::string> args;
	ReadArgsFromFile(fs::absolute(config_file).string(), args);

	PostProcess operations = current_operations;
	bool add_next = false, skip_next = false;
	for (const auto & arg : args)
	{
		if (arg == "-op")	{ add_next = true; skip_next = false; continue; }
		if (arg == "-skip") { skip_next = true; add_next = false; continue; }

		if (add_next || skip_next)
		{
			PostProcess op = GetOperation(arg);
			if (op != 0U)
			{
				if (add_next)
				{
					std::cout << "Adding model-specific operation for \"" << model_file.filename().string() << "\" of \"" << arg << "\" (" << op << ")\n";
					operations |= op;
				}
				else if (skip_next)
				{
					std::cout << "Excluding model-specific operation for \"" << model_file.filename().string() << "\" of \"" << arg << "\" (" << op << ")\n";
					operations &= ~op;
				}
			}
			add_next = skip_next = false;
		}
	}

	if (operations != current_operations)		std::cout << "Operations set updated from " << current_operations << " to " << operations << " by model-specific config\n";
	else										std::cout << "Operation set remains unchanged at " << operations << " after applying model-specific config\n";

	return operations;
}

TransformResult ObjToRjm(const std::string & input, const std::string & target, PostProcess operations)
{
	// Append any model-specific operations if they exist
	operations = AddModelSpecificOperations(fs::path(input), operations);

	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerAssimp>(operations)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageDirectPostprocess>(operations, input)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageUnitScaleModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();

	// Execute the transformation pipeline
	pipeline->Transform(fs::path(input), fs::path(target));
	
	// For testing
	// auto m = ModelData::Deserialize(PipelineUtil::ReadBinaryFile(fs::path("C:\\Users\\robje\\Downloads\\capsule.out")));

	// Return the overall transform result
	return TransformResult::Single(!pipeline.get()->HasErrors());
}

TransformResult ObjToRjmBulk(std::vector<std::string> & input, PostProcess operations)
{
	TransformResult result;

	// Execute the transformation pipeline
	std::cout << "Performing bulk transformation\n";
	int index = 0;
	for (const std::string & in : input)
	{
		fs::path in_path(in);
		std::cout << "\nProcessing file " << ++index << " of " << input.size() << " (" << fs::absolute(in) << ")\n";

		fs::path target(fs::absolute(in_path.parent_path()).string() + "/" + in_path.stem().string() + ".rjm");
		result += ObjToRjm(in, target.string(), operations);
	}

	return result;
}

TransformResult RjmToObj(const std::string & input, const std::string & target, const std::string & generate_material, PostProcess operations)
{
	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageDirectPostprocess>(operations, input)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageUnitScaleModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
		.WithOutputTransformer(std::move(std::make_unique<ObjFormatOutputTransform>(generate_material)))
		.Build();

	// Execute the transformation pipeline
	pipeline->Transform(fs::path(input), fs::path(target));

	// Return the overall transform result
	return TransformResult::Single(!pipeline.get()->HasErrors());
}

TransformResult RjmToObjBulk(std::vector<std::string> & input, const std::string & generate_material, PostProcess operations)
{
	TransformResult result;

	// Exectute the transformation pipeline
	std::cout << "Performing bulk transformation\n";
	int index = 0;
	for (const std::string & in : input)
	{
		fs::path in_path(in);
		std::cout << "\nProcessing file " << ++index << " of " << input.size() << " (" << fs::absolute(in) << ")\n";

		fs::path target(fs::absolute(in_path.parent_path()).string() + "/" + in_path.stem().string() + ".out");
		result += RjmToObj(in, target.string(), generate_material, operations);
	}

	return result;
}

TransformResult ProcessRjm(const std::string & input, const std::string & target, PostProcess operations, bool in_place = false, bool in_place_backup = true)
{
	TransformResult result;

	// Append any model-specific operations if they exist
	operations = AddModelSpecificOperations(fs::path(input), operations);

	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageDirectPostprocess>(operations, input)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageUnitScaleModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageAssimpTransform>(operations)))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();

	// Target depends on whether this is in-place or not
	if (!in_place)
	{
		// Exectute the transformation pipeline
		pipeline->Transform(fs::path(input), fs::path(target));
		result = TransformResult::Single(!pipeline.get()->HasErrors());
	}
	else
	{
		// Perform in-place modification of the model data
		fs::path in(input);
		fs::path out(fs::absolute(in.parent_path()).string() + "/" + in.stem().string() + ".out");

		// Transform
		pipeline->Transform(in, out);
		result = TransformResult::Single(!pipeline.get()->HasErrors());

		// Check whether the transformation succeeded
		if (!fs::exists(out))
		{
			std::cerr << "RJM transformation failed\n";
			result = TransformResult::SingleFailure();
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

	return result;
}

TransformResult ProcessRjmBulk(std::vector<std::string> & input, PostProcess operations, bool in_place = false, bool in_place_backup = true)
{
	TransformResult result;

	// Exectute the transformation pipeline
	std::cout << "Performing bulk transformation\n";
	int index = 0;
	for (const std::string & in : input)
	{
		fs::path in_path(in);
		std::cout << "\nProcessing file " << ++index << " of " << input.size() << " (" << fs::absolute(in) << ")\n";

		// Target is only relevant if this is not an in-place swap
		std::string target = (in_place ? "" : (fs::absolute(in_path.parent_path()).string() + "/" + in_path.stem().string() + ".out"));
		result += ProcessRjm(in, target, operations, in_place, in_place_backup);
	}

	return result;
}


TransformResult RjmObjConversionTest(void)
{
	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageUnitScaleModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageOutputModelInfo>()))
		.WithOutputTransformer(std::move(std::make_unique<BinaryOutputTransform>()))
		.Build();

	// Exectute the transformation pipeline
	pipeline->Transform(fs::path("C:\\Users\\robje\\Downloads\\testship1.rjm"), fs::path("C:\\Users\\robje\\Downloads\\testship1.out"));
	return TransformResult::Single(!pipeline.get()->HasErrors());

	// For testing
	// auto m = ModelData::Deserialize(PipelineUtil::ReadBinaryFile(fs::path("C:\\Users\\robje\\Downloads\\capsule.out")));
}

TransformResult BulkRjmObjConversion(void)
{
	TransformResult result;
	bool overwrite_backups = true;

	// Basic pipeline configuration
	std::unique_ptr<TransformPipeline> pipeline = TransformPipelineBuilder()
		.WithInputTransformer(std::move(std::make_unique<InputTransformerRjm>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageCentreModel>()))
		.WithPipelineStage(std::move(std::make_unique<PipelineStageUnitScaleModel>()))
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
			result += TransformResult::SingleFailure();
			continue;
		}

		// Rename the original file in order to retain a backup (do not overwrite so that multiple runs will not eliminate backups)
		fs::copy_file(path, fs::path(path.string() + ".backup"), (overwrite_backups ? fs::copy_options::overwrite_existing : fs::copy_options::skip_existing));

		// Rename the target file to the original file to perform an in-place swap
		fs::copy_file(target, path, fs::copy_options::overwrite_existing);

		// Remove the target file now that it has been swapped in for the original file
		fs::remove(target);

		// Record result for this model
		result.Add(!pipeline.get()->HasErrors());
	}

	return result;
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
	PostProcess operations = AssimpIntegration::DefaultOperations();

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
		else if (key == "-args")				ReadArgsFromFile(val, args, i + 2U);
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

	// Output current operation set for info
	std::cout << "Consolidated operation set: " << operations << "\n";

	// Perform the requested operation
	TransformResult result;
	if (type == "process-rjm")
	{
		if (bulk)		result = ProcessRjmBulk(input, operations, inplace, inplace_backup);
		else			result = ProcessRjm(input.at(0), output, operations, inplace, inplace_backup);
	}
	else if (type == "obj-to-rjm")
	{
		if (bulk)		result = ObjToRjmBulk(input, operations);
		else			result = ObjToRjm(input.at(0), output, operations);
	}
	else if (type == "rjm-to-obj")
	{
		if (bulk)		result = RjmToObjBulk(input, gen_mat, operations);
		else			result = RjmToObj(input.at(0), output, gen_mat, operations);
	}

	// Report a status and any errors that occurred
	std::cout << "\nFinal result " << (result.Failure == 0U ? "is successful" : "HAS FAILURES") << ": " <<
		result.Total() << " Total, " << result.Success << " Success, " << result.Failure << " Failed" << 
		(result.Failure != 0U ? "     <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" : "") << "\n";

	// Return success
	std::cout << "Model pipeline operations completed, exiting\n";
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



