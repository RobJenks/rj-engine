#include "ResourceBuilderUtil.h"
#include <sstream>
#include <fstream>

std::string ResourceBuilderUtil::ReadFileToString(fs::path file)
{
	if (!fs::exists(file)) return "";

	std::ifstream input(fs::absolute(file));
	std::stringstream ss;

	while (input >> ss.rdbuf()) { /* Simply stream all data into the string */ }
	input.close();

	return ss.str();
}


void ResourceBuilderUtil::WriteDataTofile(fs::path file, const std::string & data)
{
	std::ofstream out(fs::absolute(file));
	out << data;
	out.close();
}

std::string ResourceBuilderUtil::StringReplace(std::string str, char original, char replacement)
{
	for (auto & ch : str)
	{
		if (ch == original) ch = replacement;
	}

	return str;
}

std::string ResourceBuilderUtil::StringReplace(std::string str, const std::string & original, const std::string & replacement)
{
	size_t index = 0U;
	while (true)
	{
		index = str.find(original, index);
		if (index == std::string::npos) break;

		str.replace(index, original.size(), replacement);
		index += replacement.size();
	}

	return str;
}


