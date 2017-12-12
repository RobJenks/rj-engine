#pragma once

#include <filesystem>
#include "../Definitions/ByteString.h"
namespace fs = std::experimental::filesystem;


class FileUtils
{
public:

	static ByteString						ReadBinaryFile(fs::path file);



};
