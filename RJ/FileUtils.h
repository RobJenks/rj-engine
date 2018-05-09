#pragma once

#include <filesystem>
#include "../Definitions/ByteString.h"
namespace fs = std::experimental::filesystem;


class FileUtils
{
public:

	static std::string						ReadFile(fs::path file);

	static std::string						ReadLargeFile(fs::path file);

	static ByteString						ReadBinaryFile(fs::path file);



};
