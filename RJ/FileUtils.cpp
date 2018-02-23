#include <fstream>
#include <ios>
#include "FileUtils.h"




std::string FileUtils::ReadFile(fs::path file)
{
	if (!fs::exists(file)) return "";
	std::ifstream in(fs::absolute(file).string());
	if (in.fail()) return "";

	auto ss = std::ostringstream{};
	ss << in.rdbuf();
	in.close();

	return ss.str();
}

std::string FileUtils::ReadLargeFile(fs::path file)
{
	if (!fs::exists(file)) return "";
	std::ifstream in(fs::absolute(file).string());
	if (in.fail()) return "";

	auto const start_pos = in.tellg();
	in.ignore((std::numeric_limits<std::streamsize>::max)());
	
	auto const char_count = in.gcount();
	in.seekg(start_pos);
	
	auto s = std::string(char_count, char{});
	in.read(&s[0], s.size());
	return s;
}



ByteString FileUtils::ReadBinaryFile(fs::path file)
{
	if (!fs::exists(file)) return ByteString();
	std::ifstream in(fs::absolute(file).string(), std::ios::binary);
	if (in.fail()) return ByteString();

	auto const start_pos = in.tellg();
	in.ignore((std::numeric_limits<std::streamsize>::max)());

	auto const char_count = in.gcount();
	in.seekg(start_pos);

	ByteString b(char_count);
	in.read(&b[0], b.size());
	return b;
}