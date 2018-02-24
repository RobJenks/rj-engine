#pragma once

#include <string>


class ResourceBuilder
{
public:

	static const std::string DEFAULT_MODEL_PIPELINE_CONFIG;
	static const std::string MODEL_PIPELINE_CONFIG;

	static const std::string ARG_LOG;

	enum BuildStage
	{
		None = 0U,
		ModelPipelineBuild = 1U
	};

	ResourceBuilder(void);
	

	void									Execute(unsigned int build_stages = 0U);

	void									SetLogging(const std::string & log_arg);

	static unsigned int						GetBuildStage(const std::string & stage_name);
	static bool								BuildStageActive(ResourceBuilder::BuildStage stage, unsigned int stage_config);

private:

	void									ExecuteModelPipeline(void);

	void									GenerateDefaultConfiguration(fs::path path);

private:

	std::string								m_logging;


};