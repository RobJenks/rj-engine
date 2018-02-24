#pragma once

#include <string>
#include <filesystem>
namespace fs = std::experimental::filesystem;


class ResourceBuilderUtil
{
public:

	static std::string			ReadFileToString(fs::path file);
	static void					WriteDataTofile(fs::path file, const std::string & data);

	static std::string			StringReplace(std::string str, char original, char replacement);
	static std::string			StringReplace(std::string str, const std::string & original, const std::string & replacement);


private:


};

