#include <iostream>
#include <vector>
#include <tuple>
#include <filesystem>
#include <sstream>
#include "ResourceBuilderUtil.h"
#include "ResourceBuilder.h"
namespace fs = std::experimental::filesystem;

// Initialise static variables
const std::string ResourceBuilder::DEFAULT_MODEL_PIPELINE_CONFIG = "./ResourceBuilderDefaultModelPipelineConfig.txt";
const std::string ResourceBuilder::MODEL_PIPELINE_CONFIG = "./ModelBuilderConfig.txt";

const std::string ResourceBuilder::ARG_LOG = "log:";

// Entry point
int main(int argc, const char **argv)
{
	ResourceBuilder resource_builder;
	std::cout << "Starting resource build process\n";

	// Determine config and applicable build stages
	unsigned int build_stages = ResourceBuilder::BuildStage::None;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];

		// Activate build stages if they are specified
		unsigned int build_stage = ResourceBuilder::GetBuildStage(arg);
		if (build_stage != (unsigned int)ResourceBuilder::BuildStage::None)
		{
			std::cout << "Enabling build stage \"" << arg << "\" (" << (unsigned int)build_stage << ")\n";
			build_stages |= (unsigned int)build_stage;
		}
		else
		{
			// Other arguments
			if (arg.substr(0U, ResourceBuilder::ARG_LOG.size()) == ResourceBuilder::ARG_LOG)
			{
				resource_builder.SetLogging(arg);
			}

		}
	}

	// Invoke the resource build with this config
	resource_builder.Execute(build_stages);

	// Exit application
	std::cout << "Resource build process completed\n";
}

ResourceBuilder::ResourceBuilder(void)
	:
	m_logging("normal")
{
}

void ResourceBuilder::Execute(unsigned int build_stages)
{
	// Invoke all stages as required
	if (BuildStageActive(ResourceBuilder::BuildStage::ModelPipelineBuild, build_stages))
	{
		ExecuteModelPipeline();
	}
}

void ResourceBuilder::ExecuteModelPipeline(void)
{
	std::cout << "\n=== Executing model pipeline build process ===\n\n";

	// Default location for model pipeline executables
	fs::path pipeline = fs::absolute("./ModelPipeline.exe");
	if (!fs::exists(pipeline))
	{
		std::cerr << "Cannot locate model pipeline executables (" << pipeline.string() << "); cannot continue\n";
		return;
	}
	else
	{
		std::cout << "Located model pipeline (" << pipeline.string() << ")\n";
	}

	// Check for the presence of the default argument file and generate one if necessary
	fs::path default_config(fs::absolute(fs::path(DEFAULT_MODEL_PIPELINE_CONFIG)));
	if (!fs::exists(default_config))
	{
		GenerateDefaultConfiguration(default_config);
	}

	// We will execute for all OBJ files in the project directory
	fs::path base_dir = fs::absolute("../../RJ/Data"); 
	std::cout << "Project base data directory is \"" << base_dir.string() << "\"\n";

	std::vector<fs::path> files;
	for (auto & entry : fs::recursive_directory_iterator(base_dir, fs::v1::directory_options::follow_directory_symlink))
	{
		fs::path file(entry.path());
		if (fs::is_directory(file)) continue;
		if (file.has_extension() && file.extension().string() == ".obj")
		{
			files.push_back(file);
			std::cout << "Identified input file for model pipeline: \"" << file.string() << "\"\n";
		}
	}

	if (files.empty())
	{
		std::cout << "No applicable files found for model pipeline; terminating\n";
		return;
	}

	std::cout << "Found " << files.size() << " input files for model pipeline\n";

	// Build input configuration for the model pipeline
	std::ostringstream str;
	str << "-args \"" << DEFAULT_MODEL_PIPELINE_CONFIG << "\"";
	if (m_logging != "normal") str << " -log " << m_logging;
	for (auto & file : files)
	{
		str << " " << "-i \"" << fs::absolute(file).string() << "\"";
	}

	// Save configuration to file
	fs::path config_path(fs::absolute(fs::path(MODEL_PIPELINE_CONFIG)));
	ResourceBuilderUtil::WriteDataTofile(config_path, str.str());
	std::cout << "Model pipeline configuration generated (" << config_path.string() << ")\n";

	// Finally, invoke the model pipeline with this generated configuration
	std::cout << "Invoking model pipeline...\n";
	std::string commandline = std::string("\"") +
		std::string("\"") + ResourceBuilderUtil::StringReplace(pipeline.string(), "\\", "\\\\") + std::string("\"") +
		" -args " + std::string("\"") + ResourceBuilderUtil::StringReplace(config_path.string(), "\\", "\\\\") + std::string("\"") +
		std::string("\""); // > \"" + output_file.string() + "\"";
	std::cout << "Cmd: " << commandline << "\n\n";
	system(commandline.c_str());
	
	// Return success
	std::cout << "\nModel pipeline execution completed\n";
	return;
}



void ResourceBuilder::GenerateDefaultConfiguration(fs::path path)
{
	std::cout << "Generating default model pipeline configuration at \"" << path.string() << "\"\n";

	std::ostringstream str;
	str << "-type obj-to-rjm "
		<< "-n bulk "
		<< "-log normal "
		<< "-op triangulate "
		<< "-op gen-normals "
		<< "-op gen-tangents ";

	ResourceBuilderUtil::WriteDataTofile(path, str.str());
}

void ResourceBuilder::SetLogging(const std::string & log_arg)
{
	std::string val = ResourceBuilderUtil::StringReplace(log_arg, ResourceBuilder::ARG_LOG, "");
	if (val == "verbose")
	{
		m_logging = "verbose";
	}
	else if (val == "debug-verbose")
	{
		m_logging = "debug-verbose";
	}
	else
	{
		m_logging = "normal";
	}

	std::cout << "Setting logging level to \"" << m_logging << "\"\n";
}

unsigned int ResourceBuilder::GetBuildStage(const std::string & stage_name)
{
	static const std::vector<std::tuple<std::string, ResourceBuilder::BuildStage>> build_stages =
	{
		{ "modelpipeline", ResourceBuilder::BuildStage::ModelPipelineBuild }
	};

	// Look for a specific build stage
	for (const auto & entry : build_stages)
	{
		if (std::get<0>(entry) == stage_name) return (unsigned int)std::get<1>(entry);
	}

	// The 'all' token should include all build stages from the static collection
	if (stage_name == "all")
	{
		unsigned int all = 0U;
		for (const auto & entry : build_stages) all |= std::get<1>(entry);
		return all;
	}

	return (unsigned int)ResourceBuilder::BuildStage::None;
}

bool ResourceBuilder::BuildStageActive(ResourceBuilder::BuildStage stage, unsigned int stage_config)
{
	return ((stage_config & (unsigned int)stage) == (unsigned int)stage);
}



